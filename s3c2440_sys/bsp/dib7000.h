#ifndef __DIB7000_H_
#define __DIB7000_H_

extern BYTE DIB7KDefaultAddress[2];
extern BYTE DIB7KAddress[2];

extern float RF;
extern float _freq[10];
extern int   _bw[10];
extern int   _IQ[10];
extern int   freq_all;

#define INIT_DVB_CHANNEL(ch) do {    \
	    (ch)->RfKHz            = 0;  \
	    (ch)->Bw               = 0;  \
	    (ch)->nfft             = -1; \
	    (ch)->guard            = -1; \
	    (ch)->invspec          = -1; \
	    (ch)->nqam             = -1; \
	    (ch)->intlv_native     = -1; \
	    (ch)->vit_hrch         = -1; \
	    (ch)->vit_select_hp    = -1; \
	    (ch)->vit_alpha        = -1; \
	    (ch)->vit_code_rate_hp = -1; \
	    (ch)->vit_code_rate_lp = -1; \
} while (0)

#define TunerAddr             0xC0

int TDA6651_ChangeFreq(BYTE wDevice,DWORD wRf,BYTE wBw);

int DIB7KEnumeration(BYTE *DefaultAddress,BYTE NumOfDemod);
int DIB7KDemodReset(BYTE Device);
int DIB7KInit(BYTE Device);
int DIB7KSetBandWidth(BYTE Device, BYTE BW_Idx);
int DIB7KDoAutoSearch(BYTE *Device, BYTE NumOfDemod, struct DIBDVBChannel *CD);
int DIB7KTune(BYTE Device, struct DIBDVBChannel *CD);
int DIB7KSetDivIn(BYTE Device, BYTE OnOff);
int DIB7KDemodSetOutputMode(BYTE Device, BYTE OutputMode);
int DIB7KTuneDiversity(BYTE NumOfDemod, struct DIBDVBChannel *CD);
int DIB7KConnect(BYTE Device, BYTE TunerAddress);
int DIB7KDisConnect(BYTE Device, BYTE TunerAddress);
int DIB7KTunerInit(BYTE Device);
int DIB7KSetTuner(BYTE Device, struct DIBDVBChannel *CD);

int DIB7KTuneSerial(BYTE NumOfDemod, struct DIBDVBChannel *CD);
#endif
