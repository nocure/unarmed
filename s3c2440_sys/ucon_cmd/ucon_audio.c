/*
 * ucon_audio.c
 *
 *  Created on: 2009/8/14
 *      Author: Chris Chiu
 */

#include "platform/YL_SYS.h"
#include <platform/yl_fsio.h>
#include <platform/yl_errno.h>
#include "platform/TQ_BSP.h"		// ccchiu: test I2S
#include <string.h>

#include <neaacdec.h>
#include <mp4ff.h>

#include "../audio/audio.h"


#define CLOCKS_PER_SEC	1000	/* use OSTICK, 1 clock = 1 ms */
#define MAX_CHANNELS 6 /* make this higher to support files with
                          more channels */


static int quiet = 0;

/* FAAD file buffering routines */
typedef struct {
    long bytes_into_buffer;
    long bytes_consumed;
    long file_offset;
    unsigned char *buffer;
    int at_eof;
    FILE *infile;
} aac_buffer;


static int fill_buffer(aac_buffer *b)
{
    int bread;

    if (b->bytes_consumed > 0)
    {
        if (b->bytes_into_buffer)
        {
            memmove((void*)b->buffer, (void*)(b->buffer + b->bytes_consumed),
                b->bytes_into_buffer*sizeof(unsigned char));
        }

        if (!b->at_eof)
        {
            bread = fread((void*)(b->buffer + b->bytes_into_buffer),
                b->bytes_consumed, b->infile);

            if (bread != b->bytes_consumed)
                b->at_eof = 1;

            b->bytes_into_buffer += bread;
        }

        b->bytes_consumed = 0;

        if (b->bytes_into_buffer > 3)
        {
            if (memcmp(b->buffer, "TAG", 3) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 11)
        {
            if (memcmp(b->buffer, "LYRICSBEGIN", 11) == 0)
                b->bytes_into_buffer = 0;
        }
        if (b->bytes_into_buffer > 8)
        {
            if (memcmp(b->buffer, "APETAGEX", 8) == 0)
                b->bytes_into_buffer = 0;
        }
    }

    return 1;
}

static void advance_buffer(aac_buffer *b, int bytes)
{
    b->file_offset += bytes;
    b->bytes_consumed = bytes;
    b->bytes_into_buffer -= bytes;
	if (b->bytes_into_buffer < 0)
		b->bytes_into_buffer = 0;
}

static int adts_sample_rates[] = {96000,88200,64000,48000,44100,32000,24000,22050,16000,12000,11025,8000,7350,0,0,0};

static int adts_parse(aac_buffer *b, int *bitrate, float *length)
{
    int frames, frame_length;
    int t_framelength = 0;
    int samplerate=0;
    float frames_per_sec, bytes_per_frame;

    /* Read all frames to ensure correct time and bitrate */
    for (frames = 0; /* */; frames++)
    {
        fill_buffer(b);

        if (b->bytes_into_buffer > 7)
        {
            /* check syncword */
            if (!((b->buffer[0] == 0xFF)&&((b->buffer[1] & 0xF6) == 0xF0)))
                break;

            if (frames == 0)
                samplerate = adts_sample_rates[(b->buffer[2]&0x3c)>>2];

            frame_length = ((((unsigned int)b->buffer[3] & 0x3)) << 11)
                | (((unsigned int)b->buffer[4]) << 3) | (b->buffer[5] >> 5);

            t_framelength += frame_length;

            if (frame_length > b->bytes_into_buffer)
                break;

            advance_buffer(b, frame_length);
        } else {
            break;
        }
    }

    frames_per_sec = (float)samplerate/1024.0f;
    if (frames != 0)
        bytes_per_frame = (float)t_framelength/(float)(frames*1000);
    else
        bytes_per_frame = 0;
    *bitrate = (int)(8. * bytes_per_frame * frames_per_sec + 0.5);
    if (frames_per_sec != 0)
        *length = (float)frames/frames_per_sec;
    else
        *length = 1;

    return 1;
}



uint32_t read_callback(void *user_data, void *buffer, uint32_t length)
{
    return fread(buffer, length, (FILE*)user_data);
}

uint32_t seek_callback(void *user_data, uint64_t position)
{
    return fseek((FILE*)user_data, position, SEEK_SET);
}

/* MicroSoft channel definitions */
#define SPEAKER_FRONT_LEFT             0x1
#define SPEAKER_FRONT_RIGHT            0x2
#define SPEAKER_FRONT_CENTER           0x4
#define SPEAKER_LOW_FREQUENCY          0x8
#define SPEAKER_BACK_LEFT              0x10
#define SPEAKER_BACK_RIGHT             0x20
#define SPEAKER_FRONT_LEFT_OF_CENTER   0x40
#define SPEAKER_FRONT_RIGHT_OF_CENTER  0x80
#define SPEAKER_BACK_CENTER            0x100
#define SPEAKER_SIDE_LEFT              0x200
#define SPEAKER_SIDE_RIGHT             0x400
#define SPEAKER_TOP_CENTER             0x800
#define SPEAKER_TOP_FRONT_LEFT         0x1000
#define SPEAKER_TOP_FRONT_CENTER       0x2000
#define SPEAKER_TOP_FRONT_RIGHT        0x4000
#define SPEAKER_TOP_BACK_LEFT          0x8000
#define SPEAKER_TOP_BACK_CENTER        0x10000
#define SPEAKER_TOP_BACK_RIGHT         0x20000
#define SPEAKER_RESERVED               0x80000000

static long aacChannelConfig2wavexChannelMask(NeAACDecFrameInfo *hInfo)
{
    if (hInfo->channels == 6 && hInfo->num_lfe_channels)
    {
        return SPEAKER_FRONT_LEFT + SPEAKER_FRONT_RIGHT +
            SPEAKER_FRONT_CENTER + SPEAKER_LOW_FREQUENCY +
            SPEAKER_BACK_LEFT + SPEAKER_BACK_RIGHT;
    } else {
        return 0;
    }
}

static char *position2string(int position)
{
    switch (position)
    {
    case FRONT_CHANNEL_CENTER: return "Center front";
    case FRONT_CHANNEL_LEFT:   return "Left front";
    case FRONT_CHANNEL_RIGHT:  return "Right front";
    case SIDE_CHANNEL_LEFT:    return "Left side";
    case SIDE_CHANNEL_RIGHT:   return "Right side";
    case BACK_CHANNEL_LEFT:    return "Left back";
    case BACK_CHANNEL_RIGHT:   return "Right back";
    case BACK_CHANNEL_CENTER:  return "Center back";
    case LFE_CHANNEL:          return "LFE";
    case UNKNOWN_CHANNEL:      return "Unknown";
    default: return "";
    }

    return "";
}

static void print_channel_info(NeAACDecFrameInfo *frameInfo)
{
    /* print some channel info */
    int i;
    long channelMask = aacChannelConfig2wavexChannelMask(frameInfo);

    printf("  ---------------------\n");
    if (frameInfo->num_lfe_channels > 0)
    {
        printf(" | Config: %2d.%d Ch     |", frameInfo->channels-frameInfo->num_lfe_channels, frameInfo->num_lfe_channels);
    } else {
        printf(" | Config: %2d Ch       |", frameInfo->channels);
    }
    if (channelMask)
        printf(" WARNING: channels are reordered according to\n");
    else
        printf("\n");
    printf("  ---------------------");
    if (channelMask)
        printf("  MS defaults defined in WAVE_FORMAT_EXTENSIBLE\n");
    else
        printf("\n");
    printf(" | Ch |    Position    |\n");
    printf("  ---------------------\n");
    for (i = 0; i < frameInfo->channels; i++)
    {
        printf(" | %.2d | %-14s |\n", i, position2string((int)frameInfo->channel_position[i]));
    }
    printf("  ---------------------\n");
    printf("\n");
}

static int FindAdtsSRIndex(int sr)
{
    int i;

    for (i = 0; i < 16; i++)
    {
        if (sr == adts_sample_rates[i])
            return i;
    }
    return 16 - 1;
}

static unsigned char *MakeAdtsHeader(int *dataSize, NeAACDecFrameInfo *hInfo, int old_format)
{
    unsigned char *data;
    int profile = (hInfo->object_type - 1) & 0x3;
    int sr_index = ((hInfo->sbr == SBR_UPSAMPLED) || (hInfo->sbr == NO_SBR_UPSAMPLED)) ?
        FindAdtsSRIndex(hInfo->samplerate / 2) : FindAdtsSRIndex(hInfo->samplerate);
    int skip = (old_format) ? 8 : 7;
    int framesize = skip + hInfo->bytesconsumed;

    if (hInfo->header_type == ADTS)
        framesize -= skip;

    *dataSize = 7;

    data = malloc(*dataSize * sizeof(unsigned char));
    memset(data, 0, *dataSize * sizeof(unsigned char));

    data[0] += 0xFF; /* 8b: syncword */

    data[1] += 0xF0; /* 4b: syncword */
    /* 1b: mpeg id = 0 */
    /* 2b: layer = 0 */
    data[1] += 1; /* 1b: protection absent */

    data[2] += ((profile << 6) & 0xC0); /* 2b: profile */
    data[2] += ((sr_index << 2) & 0x3C); /* 4b: sampling_frequency_index */
    /* 1b: private = 0 */
    data[2] += ((hInfo->channels >> 2) & 0x1); /* 1b: channel_configuration */

    data[3] += ((hInfo->channels << 6) & 0xC0); /* 2b: channel_configuration */
    /* 1b: original */
    /* 1b: home */
    /* 1b: copyright_id */
    /* 1b: copyright_id_start */
    data[3] += ((framesize >> 11) & 0x3); /* 2b: aac_frame_length */

    data[4] += ((framesize >> 3) & 0xFF); /* 8b: aac_frame_length */

    data[5] += ((framesize << 5) & 0xE0); /* 3b: aac_frame_length */
    data[5] += ((0x7FF >> 6) & 0x1F); /* 5b: adts_buffer_fullness */

    data[6] += ((0x7FF << 2) & 0x3F); /* 6b: adts_buffer_fullness */
    /* 2b: num_raw_data_blocks */

    return data;
}

/* globals */
char *progName;

static const char *file_ext[] =
{
    NULL,
    ".wav",
    ".aif",
    ".au",
    ".au",
    ".pcm",
    NULL
};

static void usage(void)
{
    printf("\nUsage:\n");
    printf("%s [options] infile.aac\n", progName);
    printf("Options:\n");
    printf(" -h    Shows this help screen.\n");
    printf(" -i    Shows info about the input file.\n");
    printf(" -a X  Write MPEG-4 AAC ADTS output file.\n");
    printf(" -t    Assume old ADTS format.\n");
    printf(" -o X  Set output filename.\n");
    printf(" -f X  Set output format. Valid values for X are:\n");
    printf("        1:  Microsoft WAV format (default).\n");
    printf("        2:  RAW PCM data.\n");
    printf(" -b X  Set output sample format. Valid values for X are:\n");
    printf("        1:  16 bit PCM data (default).\n");
    printf("        2:  24 bit PCM data.\n");
    printf("        3:  32 bit PCM data.\n");
    printf("        4:  32 bit floating point data.\n");
    printf("        5:  64 bit floating point data.\n");
    printf(" -s X  Force the samplerate to X (for RAW files).\n");
    printf(" -l X  Set object type. Supported object types:\n");
    printf("        1:  Main object type.\n");
    printf("        2:  LC (Low Complexity) object type.\n");
    printf("        4:  LTP (Long Term Prediction) object type.\n");
    printf("        23: LD (Low Delay) object type.\n");
    printf(" -d    Down matrix 5.1 to 2 channels\n");
    printf(" -w    Write output to stdio instead of a file.\n");
    printf(" -g    Disable gapless decoding.\n");
    printf(" -q    Quiet - suppresses status messages.\n");
    printf("Example:\n");
    printf("       %s infile.aac\n", progName);
    printf("       %s infile.mp4\n", progName);
    printf("       %s -o outfile.wav infile.aac\n", progName);
    printf("       %s -w infile.aac > outfile.wav\n", progName);
    printf("       %s -a outfile.aac infile.aac\n", progName);
    return;
}

static int decodeAACfile(char *aacfile, char *sndfile, char *adts_fn,
                  int def_srate, int object_type, int outputFormat, int fileType,
                  int downMatrix, int infoOnly, int adts_out, int old_format,
                  float *song_length)
{
    int tagsize;
    unsigned long samplerate;
    unsigned char channels;
    void *sample_buffer;

    audio_file *aufile=0;

    FILE *adtsFile=0;
    unsigned char *adtsData;
    int adtsDataSize;

    NeAACDecHandle hDecoder;
    NeAACDecFrameInfo frameInfo;
    NeAACDecConfigurationPtr config;

    char percents[200];
    int percent, old_percent = -1;
    int bread, fileread;
    int header_type = 0;
    int bitrate = 0;
    float length = 0;
    unsigned long	acum = 0;		/*ccchiu add for dec time measurement */

    int first_time = 1;

    aac_buffer b;

    memset(&b, 0, sizeof(aac_buffer));

    if (adts_out)
    {
        adtsFile = fopen(adts_fn, "wb");
        if (adtsFile == NULL)
        {
            printf("Error opening file: %s\n", adts_fn);
            return 1;
        }
    }

    b.infile = fopen(aacfile, "rb");
    if (b.infile == NULL)
    {
        /* unable to open file */
        printf("Error opening file: %s\n", aacfile);
        return 1;
    }

    fseek(b.infile, 0, SEEK_END);
    fileread = ftell(b.infile);
    fseek(b.infile, 0, SEEK_SET);

    if (!(b.buffer = (unsigned char*)malloc(FAAD_MIN_STREAMSIZE*MAX_CHANNELS)))
    {
        printf("Memory allocation error\n");
        return 0;
    }
    memset(b.buffer, 0, FAAD_MIN_STREAMSIZE*MAX_CHANNELS);

    bread = fread(b.buffer, FAAD_MIN_STREAMSIZE*MAX_CHANNELS, b.infile);
    b.bytes_into_buffer = bread;
    b.bytes_consumed = 0;
    b.file_offset = 0;

    if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
        b.at_eof = 1;

    tagsize = 0;
    if (!memcmp(b.buffer, "ID3", 3))
    {
        /* high bit is not used */
        tagsize = (b.buffer[6] << 21) | (b.buffer[7] << 14) |
            (b.buffer[8] <<  7) | (b.buffer[9] <<  0);

        tagsize += 10;
        advance_buffer(&b, tagsize);
        fill_buffer(&b);
    }

    hDecoder = NeAACDecOpen();

    /* Set the default object type and samplerate */
    /* This is useful for RAW AAC files */
    config = NeAACDecGetCurrentConfiguration(hDecoder);
    if (def_srate)
        config->defSampleRate = def_srate;
    config->defObjectType = object_type;
    config->outputFormat = outputFormat;
    if (downMatrix) {
    	config->options|=DOWN_MATRIX;
    }
    if (old_format) {
    	config->options|=OLD_ADTS_FORMAT;
    }
//    config->downMatrix = downMatrix;
//    config->useOldADTSFormat = old_format;

    //config->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(hDecoder, config);

    /* get AAC infos for printing */
    header_type = 0;
    if ((b.buffer[0] == 0xFF) && ((b.buffer[1] & 0xF6) == 0xF0))
    {
        adts_parse(&b, &bitrate, &length);
        fseek(b.infile, tagsize, SEEK_SET);

        bread = fread(b.buffer, FAAD_MIN_STREAMSIZE*MAX_CHANNELS, b.infile);
        if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
            b.at_eof = 1;
        else
            b.at_eof = 0;
        b.bytes_into_buffer = bread;
        b.bytes_consumed = 0;
        b.file_offset = tagsize;

        header_type = 1;
    } else if (memcmp(b.buffer, "ADIF", 4) == 0) {
        int skip_size = (b.buffer[4] & 0x80) ? 9 : 0;
        bitrate = ((unsigned int)(b.buffer[4 + skip_size] & 0x0F)<<19) |
            ((unsigned int)b.buffer[5 + skip_size]<<11) |
            ((unsigned int)b.buffer[6 + skip_size]<<3) |
            ((unsigned int)b.buffer[7 + skip_size] & 0xE0);

        length = (float)fileread;
        if (length != 0)
        {
            length = ((float)length*8.f)/((float)bitrate) + 0.5f;
        }

        bitrate = (int)((float)bitrate/1000.0f + 0.5f);

        header_type = 2;
    }

    *song_length = length;

    fill_buffer(&b);
    if ((bread = NeAACDecInit(hDecoder, b.buffer,
        b.bytes_into_buffer, &samplerate, &channels)) < 0)
    {
        /* If some error initializing occured, skip the file */
        printf("Error initializing decoder library.\n");
        if (b.buffer)
            free(b.buffer);
        NeAACDecClose(hDecoder);
        fclose(b.infile);
        return 1;
    }
    advance_buffer(&b, bread);
    fill_buffer(&b);

    /* print AAC file info */
    printf("%s file info:\n", aacfile);
    switch (header_type)
    {
    case 0:
        printf("RAW\n\n");
        break;
    case 1:
        printf("ADTS, %.3f sec, %d kbps, %d Hz\n\n",
            length, bitrate, samplerate);
        break;
    case 2:
        printf("ADIF, %.3f sec, %d kbps, %d Hz\n\n",
            length, bitrate, samplerate);
        break;
    }

#if 0
    tq_uda134x_init();
    tq_uda134x_startup(PCM_STREAM_PLAYBACK);
    printf("init I2S pcm format PCM_FORMAT_S16LE, AAC outputFormat: %d\n", outputFormat);
    tq_uda134x_hw_params(samplerate, PCM_FORMAT_S16LE);
#endif

    if (infoOnly)
    {
        NeAACDecClose(hDecoder);
        fclose(b.infile);
        if (b.buffer)
            free(b.buffer);
        return 0;
    }

    do
    {
    	unsigned long otick = OSTICK;	// added by ccchiu
        sample_buffer = NeAACDecDecode(hDecoder, &frameInfo,
            b.buffer, b.bytes_into_buffer);
        acum += (OSTICK -otick);		// added by ccchiu
        //printf("accumulated processing time: %d msec\n", acum);

        if (adts_out == 1)
        {
            int skip = (old_format) ? 8 : 7;
            adtsData = MakeAdtsHeader(&adtsDataSize, &frameInfo, old_format);

            /* write the adts header */
            fwrite(adtsData, adtsDataSize, adtsFile);

            /* write the frame data */
            if (frameInfo.header_type == ADTS)
                fwrite(b.buffer + skip, frameInfo.bytesconsumed - skip, adtsFile);
            else
                fwrite(b.buffer, frameInfo.bytesconsumed, adtsFile);
        }

        /* update buffer indices */
        advance_buffer(&b, frameInfo.bytesconsumed);

        if (frameInfo.error > 0)
        {
            printf("Error: %s\n",
                NeAACDecGetErrorMessage(frameInfo.error));
        }

        /* open the sound file now that the number of channels are known */
        if (first_time && !frameInfo.error)
        {
            /* print some channel info */
            print_channel_info(&frameInfo);

            if (!adts_out)
            {
                /* open output file */
            	aufile = open_audio_file(sndfile, frameInfo.samplerate, frameInfo.channels,
                        outputFormat, fileType, aacChannelConfig2wavexChannelMask(&frameInfo));
                if (aufile == NULL)
                {
                    if (b.buffer)
                        free(b.buffer);
                    NeAACDecClose(hDecoder);
                    fclose(b.infile);
                    return 0;
                }
            } else {
                printf("Writing output MPEG-4 AAC ADTS file.\n\n");
            }
            first_time = 0;
        }

        percent = min((int)(b.file_offset*100)/fileread, 100);
        if (percent > old_percent)
        {
            old_percent = percent;
            sprintf(percents, "%d%% decoding %s.", percent, aacfile);
            printf("%s\r", percents);
#ifdef _WIN32
            SetConsoleTitle(percents);
#endif
        }

        if ((frameInfo.error == 0) && (frameInfo.samples > 0) && (!adts_out))
        {
#if 0
        	static unsigned int cnt = 0;
        	printf("bits per sample: %d, samples: %d, #sample: %d\n", aufile->bits_per_sample, frameInfo.samples, cnt++);
        	memcpy(audioBuffer, sample_buffer, frameInfo.samples * aufile->bits_per_sample/8);
        	tq_uda134x_trigger(PCM_TRIGGER_START, PCM_STREAM_PLAYBACK);
#endif	/* ccchiu test */
            if (write_audio_file(aufile, sample_buffer, frameInfo.samples, 0) == 0)
                break;
            fflush();		// added by ccchiu
		}

        /* fill buffer */
        fill_buffer(&b);

        if (b.bytes_into_buffer == 0)
            sample_buffer = NULL; /* to make sure it stops now */

    } while (sample_buffer != NULL);

    NeAACDecClose(hDecoder);

    if (adts_out == 1)
    {
        fclose(adtsFile);
    }

    printf("total dec time: %d msecs\n", acum);
    fclose(b.infile);

    if (!first_time && !adts_out)
        close_audio_file(aufile);

    fflush();			// added by ccchiu

    if (b.buffer)
        free(b.buffer);

    return frameInfo.error;
}

static int GetAACTrack(mp4ff_t *infile)
{
    /* find AAC track */
    int i, rc;
    int numTracks = mp4ff_total_tracks(infile);

    for (i = 0; i < numTracks; i++)
    {
        unsigned char *buff = NULL;
        unsigned int buff_size = 0;
        mp4AudioSpecificConfig mp4ASC;

        mp4ff_get_decoder_config(infile, i, &buff, &buff_size);

        if (buff)
        {
            rc = NeAACDecAudioSpecificConfig(buff, buff_size, &mp4ASC);
            free(buff);

            if (rc < 0)
                continue;
            return i;
        }
    }

    /* can't decode this */
    return -1;
}

#if 0
static const unsigned long srates[] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000,
    12000, 11025, 8000
};
#endif

static int decodeMP4file(char *mp4file, char *sndfile, char *adts_fn,
                  int outputFormat, int fileType, int downMatrix, int noGapless,
                  int infoOnly, int adts_out, float *song_length)
{
    int track;
    unsigned long samplerate;
    unsigned char channels;
    void *sample_buffer;

    mp4ff_t *infile;
    long sampleId, numSamples;

    audio_file *aufile=0;

    FILE *mp4File;
    FILE *adtsFile=0;
    unsigned char *adtsData;
    int adtsDataSize;

    NeAACDecHandle hDecoder;
    NeAACDecConfigurationPtr config;
    NeAACDecFrameInfo frameInfo;
    mp4AudioSpecificConfig mp4ASC;

    unsigned char *buffer;
    unsigned int buffer_size;

    char percents[200];
    int percent, old_percent = -1;

    int first_time = 1;

    /* for gapless decoding */
    unsigned int useAacLength = 1;
    unsigned int initial = 1;
    unsigned int framesize;
    unsigned long timescale;


    /* initialise the callback structure */
    mp4ff_callback_t *mp4cb = malloc(sizeof(mp4ff_callback_t));

    mp4File = fopen(mp4file, "rb");
    mp4cb->read = read_callback;
    mp4cb->seek = seek_callback;
    mp4cb->user_data = mp4File;


    hDecoder = NeAACDecOpen();

    /* Set configuration */
    config = NeAACDecGetCurrentConfiguration(hDecoder);
    config->outputFormat = outputFormat;
    if (downMatrix) {
    	config->options|=DOWN_MATRIX;
    }
//    config->downMatrix = downMatrix;
    //config->dontUpSampleImplicitSBR = 1;
    NeAACDecSetConfiguration(hDecoder, config);

    if (adts_out)
    {
        adtsFile = fopen(adts_fn, "wb");
        if (adtsFile == NULL)
        {
            printf("Error opening file: %s\n", adts_fn);
            return 1;
        }
    }

    infile = mp4ff_open_read(mp4cb);
    if (!infile)
    {
        /* unable to open file */
        printf("Error opening file: %s\n", mp4file);
        return 1;
    }

    if ((track = GetAACTrack(infile)) < 0)
    {
        printf("Unable to find correct AAC sound track in the MP4 file.\n");
        NeAACDecClose(hDecoder);
        mp4ff_close(infile);
        free(mp4cb);
        fclose(mp4File);
        return 1;
    }

    buffer = NULL;
    buffer_size = 0;
    mp4ff_get_decoder_config(infile, track, &buffer, &buffer_size);

    if(NeAACDecInit2(hDecoder, buffer, buffer_size,
                    &samplerate, &channels) < 0)
    {
        /* If some error initializing occured, skip the file */
        printf("Error initializing decoder library.\n");
        NeAACDecClose(hDecoder);
        mp4ff_close(infile);
        free(mp4cb);
        fclose(mp4File);
        return 1;
    }

    timescale = mp4ff_time_scale(infile, track);
    framesize = 1024;
    useAacLength = 0;

    if (buffer)
    {
        if (NeAACDecAudioSpecificConfig(buffer, buffer_size, &mp4ASC) >= 0)
        {
            if (mp4ASC.frameLengthFlag == 1) framesize = 960;
            if (mp4ASC.sbr_present_flag == 1) framesize *= 2;
        }
        free(buffer);
    }

    /* print some mp4 file info */
    printf("%s file info:\n\n", mp4file);
    {
        char *tag = NULL, *item = NULL;
        int k, j;
        char *ot[6] = { "NULL", "MAIN AAC", "LC AAC", "SSR AAC", "LTP AAC", "HE AAC" };
        long samples = mp4ff_num_samples(infile, track);
        float f = 1024.0;
        float seconds;
        if (mp4ASC.sbr_present_flag == 1)
        {
            f = f * 2.0;
        }
        seconds = (float)samples*(float)(f-1.0)/(float)mp4ASC.samplingFrequency;

        *song_length = seconds;

        printf("%s\t%.3f secs, %d ch, %d Hz\n\n", ot[(mp4ASC.objectTypeIndex > 5)?0:mp4ASC.objectTypeIndex],
            seconds, mp4ASC.channelsConfiguration, mp4ASC.samplingFrequency);

#define PRINT_MP4_METADATA
#ifdef PRINT_MP4_METADATA
        j = mp4ff_meta_get_num_items(infile);
        for (k = 0; k < j; k++)
        {
            if (mp4ff_meta_get_by_index(infile, k, &item, &tag))
            {
                if (item != NULL && tag != NULL)
                {
                    printf("%s: %s\n", item, tag);
                    free(item); item = NULL;
                    free(tag); tag = NULL;
                }
            }
        }
        if (j > 0) printf("\n");
#endif
    }

    if (infoOnly)
    {
        NeAACDecClose(hDecoder);
        mp4ff_close(infile);
        free(mp4cb);
        fclose(mp4File);
        return 0;
    }

    numSamples = mp4ff_num_samples(infile, track);

    for (sampleId = 0; sampleId < numSamples; sampleId++)
    {
        int rc;
        long dur;
        unsigned int sample_count;
        unsigned int delay = 0;

        /* get acces unit from MP4 file */
        buffer = NULL;
        buffer_size = 0;

        dur = mp4ff_get_sample_duration(infile, track, sampleId);
        rc = mp4ff_read_sample(infile, track, sampleId, &buffer,  &buffer_size);
        if (rc == 0)
        {
            printf("Reading from MP4 file failed.\n");
            NeAACDecClose(hDecoder);
            mp4ff_close(infile);
            free(mp4cb);
            fclose(mp4File);
            return 1;
        }

        sample_buffer = NeAACDecDecode(hDecoder, &frameInfo, buffer, buffer_size);

        if (adts_out == 1)
        {
            adtsData = MakeAdtsHeader(&adtsDataSize, &frameInfo, 0);

            /* write the adts header */
            fwrite(adtsData, adtsDataSize, adtsFile);

            fwrite(buffer, frameInfo.bytesconsumed, adtsFile);
        }

        if (buffer) free(buffer);

        if (!noGapless)
        {
            if (sampleId == 0) dur = 0;

            if (useAacLength || (timescale != samplerate)) {
                sample_count = frameInfo.samples;
            } else {
                sample_count = (unsigned int)(dur * frameInfo.channels);
                if (sample_count > frameInfo.samples)
                    sample_count = frameInfo.samples;

                if (!useAacLength && !initial && (sampleId < numSamples/2) && (sample_count != frameInfo.samples))
                {
                    printf("MP4 seems to have incorrect frame duration, using values from AAC data.\n");
                    useAacLength = 1;
                    sample_count = frameInfo.samples;
                }
            }

            if (initial && (sample_count < framesize*frameInfo.channels) && (frameInfo.samples > sample_count))
                delay = frameInfo.samples - sample_count;
        } else {
            sample_count = frameInfo.samples;
        }

        /* open the sound file now that the number of channels are known */
        if (first_time && !frameInfo.error)
        {
            /* print some channel info */
            print_channel_info(&frameInfo);

            if (!adts_out)
            {
                /* open output file */
            	aufile = open_audio_file(sndfile, frameInfo.samplerate, frameInfo.channels,
                        outputFormat, fileType, aacChannelConfig2wavexChannelMask(&frameInfo));
                if (aufile == NULL)
                {
                    NeAACDecClose(hDecoder);
                    mp4ff_close(infile);
                    free(mp4cb);
                    fclose(mp4File);
                    return 0;
                }
            }
            first_time = 0;
        }

        if (sample_count > 0) initial = 0;

        percent = min((int)((sampleId*100)/numSamples), 100);
        if (percent > old_percent)
        {
            old_percent = percent;
            sprintf(percents, "%d%% decoding %s.", percent, mp4file);
            printf("%s\r", percents);
#ifdef _WIN32
            SetConsoleTitle(percents);
#endif
        }

        if ((frameInfo.error == 0) && (sample_count > 0) && (!adts_out))
        {
            if (write_audio_file(aufile, sample_buffer, sample_count, delay) == 0)
                break;
        }

        if (frameInfo.error > 0)
        {
            printf("Warning: %s\n",
                NeAACDecGetErrorMessage(frameInfo.error));
        }
    }

    NeAACDecClose(hDecoder);

    if (adts_out == 1)
    {
        fclose(adtsFile);
    }

    mp4ff_close(infile);

    if (!first_time && !adts_out)
        close_audio_file(aufile);

    free(mp4cb);
    fclose(mp4File);

    return frameInfo.error;
}

void uconAudioDec(int argc, char *argv[])
{
    int result;
    int infoOnly = 0;
    int object_type = LC;
    int def_srate = 0;
    int downMatrix = 0;
    int format = 1;
    int outputFormat = FAAD_FMT_16BIT;
    int outfile_set = 0;
    int adts_out = 0;
    int old_format = 0;
    int showHelp = 0;
    int mp4file = 0;
    int noGapless = 0;
    char *fnp;
    char aacFileName[255];
    char audioFileName[255];
    char adtsFileName[255];
    unsigned char header[8];
    float length = 0;
    FILE *hMP4File;

/* System dependant types */
    //clock_t begin;
    unsigned long begin;

    unsigned long cap = NeAACDecGetCapabilities();


    /* begin process command line */
    progName = argv[0];
    getopt(0,0,0); // reset getopt state

    int c = -1;
    while ((c=getopt(argc,argv,"o:a:s:f:b:l:gdhitq")) != EOF) {
/*
        static struct option long_options[] = {
            { "quiet",      0, 0, 'q' },
            { "outfile",    0, 0, 'o' },
            { "adtsout",    0, 0, 'a' },
            { "oldformat",  0, 0, 't' },
            { "format",     0, 0, 'f' },
            { "bits",       0, 0, 'b' },
            { "samplerate", 0, 0, 's' },
            { "objecttype", 0, 0, 'l' },
            { "downmix",    0, 0, 'd' },
            { "info",       0, 0, 'i' },
            { "stdio",      0, 0, 'w' },
            { "stdio",      0, 0, 'g' },
            { "help",       0, 0, 'h' },
            { 0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "o:a:s:f:b:l:wgdhitq",
            long_options, &option_index);

       	if (c == -1)
            break;

*/

        switch (c) {
        case 'o':
            if (optarg)
            {
                outfile_set = 1;
                strcpy(audioFileName, optarg);
            }
            break;
        case 'a':
            if (optarg)
            {
                adts_out = 1;
                strcpy(adtsFileName, optarg);
            }
            break;
        case 's':
            if (optarg)
            	yl_strtol(optarg, &def_srate);
            else
            	def_srate = 0;
            break;
        case 'f':
            if (optarg) {
            	yl_strtol(optarg, &format);
            	if (format > 2)
            		showHelp = 1;
            }
            else
            	format = 1;
            break;
        case 'b':
            if (optarg) {
            	yl_strtol(optarg, &outputFormat);
            	if (outputFormat > 5)
            		showHelp = 1;
            }
            else
            	outputFormat = FAAD_FMT_16BIT; /* just use default */
            break;
        case 'l':
            if (optarg)
            {
            	yl_strtol(optarg, &object_type);
                if ((object_type != LC) &&
                    (object_type != MAIN) &&
                    (object_type != LTP) &&
                    (object_type != LD))
                {
                	showHelp = 1;
                }
            }
            else
                object_type = LC; /* default */
            break;
        case 't':
            old_format = 1;
            break;
        case 'd':
            downMatrix = 1;
            break;
        case 'g':
            noGapless = 1;
            break;
        case 'i':
            infoOnly = 1;
            break;
        case 'h':
            showHelp = 1;
            break;
        case 'q':
            quiet = 1;
            break;
        default:
            break;
        }
    }


    printf(" *********** Ahead Software MPEG-4 AAC Decoder V%s ******************\n\n", FAAD2_VERSION);
    printf(" Build: %s\n", __DATE__);
    printf(" Copyright 2002-2004: Ahead Software AG\n");
    printf(" http://www.audiocoding.com\n");
    if (cap & FIXED_POINT_CAP)
        printf(" Fixed point version\n");
    else
        printf(" Floating point version\n");
    printf("\n");
    printf(" This program is free software; you can redistribute it and/or modify\n");
    printf(" it under the terms of the GNU General Public License.\n");
    printf("\n");
    printf(" **************************************************************************\n\n");


    /* check that we have at least two non-option arguments */
    /* Print help if requested */
    if (((argc - optind) < 1) || showHelp)
    {
        usage();
        return;
    }

    /* point to the specified file name */
    strcpy(aacFileName, argv[optind]);

#ifdef _WIN32
    begin = GetTickCount();
#else
    begin = OSTICK;
#endif

    /* Only calculate the path and open the file for writing if
       we are not writing to stdout.
     */
    if(!outfile_set)
    {
        strcpy(audioFileName, aacFileName);

        fnp = (char *)strrchr(audioFileName,'.');

        if (fnp)
            fnp[0] = '\0';

        strcat(audioFileName, file_ext[format]);
    }

    /* check for mp4 file */
    mp4file = 0;
    hMP4File = fopen(aacFileName, "rb");
    if (!hMP4File)
    {
        printf("Error opening file: %s\n", aacFileName);
        return;
    }
    fread(header, 8, hMP4File);
    fclose(hMP4File);
    if (header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p')
        mp4file = 1;

    if (mp4file)
    {
        result = decodeMP4file(aacFileName, audioFileName, adtsFileName,
            outputFormat, format, downMatrix, noGapless, infoOnly, adts_out, &length);
    } else {
        result = decodeAACfile(aacFileName, audioFileName, adtsFileName,
            def_srate, object_type, outputFormat, format, downMatrix, infoOnly, adts_out,
            old_format, &length);
    }

    if (!result && !infoOnly)
    {
#ifdef _WIN32
        float dec_length = (float)(GetTickCount()-begin)/1000.0;
        SetConsoleTitle("FAAD");
#else
        /* clock() grabs time since the start of the app but when we decode
           multiple files, each file has its own starttime (begin).
         */
        float dec_length = (float)(OSTICK - begin)/(float)CLOCKS_PER_SEC;
#endif
        printf("Decoding %s took: %5.2f sec. %5.2fx real-time.\n", aacFileName,
            dec_length, length/dec_length);
    }

    return;
}
