/*
 * Run length encoded pattern
*/
#include <platform/yl_sys.h>
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include "gal.h"
#include "cliprect.h"
#include "internals.h"
#include "ctrlclass.h"
#include "dc.h"
#include "pixel_ops.h"
typedef struct sRLE_FORMAT
{
 	Uint16	reclen; 	/* Pattern Record Length:The total bytes of the pattern, low byte first */
 	Uint8	rleformat; 	/*RLE format, 1: 2 colors, 2: 4 colors, 3: 8 colors, 4: 16 colors */
	Uint16	width; 		/* Pattern width */
 	Uint16	height; 	/* Pattern height */
 	Uint8	colortno; 	/* Pattern total color number 2||4||8||16 */
 	Uint8	coloridx; 	/* decoder color index of fill to memory buffer */
 	Uint8	colorlen; 	/* decoder color len fill to memory buffer */
 	Uint8	patcoloridx[16];	/* max colors is 16 */
} RLE_FORMAT, *pRLE_FORMAT;

#define RLE_TRANSPARENT_INDEX 245
//#define RLE_TRANSPARENT_COLOR 0xaabbccdd

static const Uint32 PaletteTable1[]={
	//  0xAARRGGBB
		0x00000000,
		0x00800000,
		0x00008000,
		0x00808000,
		0x00000080,
		0x00800080,
		0x00008080,
		0x00c0c0c0,
		0x00c0dcc0,
		0x00a6caf0,
		0x00000000,
		0x00000033,
		0x00000066,
		0x00000099,
		0x000000cc,
		0x000000ff,
		0x00003300,
		0x00003333,
		0x00003366,
		0x00003399,
		0x000033cc,
		0x000033ff,
		0x00006600,
		0x00006633,
		0x00006666,
		0x00006699,
		0x000066cc,
		0x000066ff,
		0x00009900,
		0x00009933,
		0x00009966,
		0x00009999,
		0x000099cc,
		0x000099ff,
		0x0000cc00,
		0x0000cc33,
		0x0000cc66,
		0x0000cc99,
		0x0000cccc,
		0x0000ccff,
		0x0000ff00,
		0x0000ff33,
		0x0000ff66,
		0x0000ff99,
		0x0000ffcc,
		0x0000ffff,
		0x00330000,
		0x00330033,
		0x00330066,
		0x00330099,
		0x003300cc,
		0x003300ff,
		0x00333300,
		0x00333333,
		0x00333366,
		0x00333399,
		0x003333cc,
		0x003333ff,
		0x00336600,
		0x00336633,
		0x00336666,
		0x00336699,
		0x003366cc,
		0x003366ff,
		0x00339900,
		0x00339933,
		0x00339966,
		0x00339999,
		0x003399cc,
		0x003399ff,
		0x0033cc00,
		0x0033cc33,
		0x0033cc66,
		0x0033cc99,
		0x0033cccc,
		0x0033ccff,
		0x0033ff00,
		0x0033ff33,
		0x0033ff66,
		0x0033ff99,
		0x0033ffcc,
		0x0033ffff,
		0x00660000,
		0x00660033,
		0x00660066,
		0x00660099,
		0x006600cc,
		0x006600ff,
		0x00663300,
		0x00663333,
		0x00663366,
		0x00663399,
		0x006633cc,
		0x006633ff,
		0x00666600,
		0x00666633,
		0x00666666,
		0x00666699,
		0x006666cc,
		0x006666ff,
		0x00669900,
		0x00669933,
		0x00669966,
		0x00669999,
		0x006699cc,
		0x006699ff,
		0x0066cc00,
		0x0066cc33,
		0x0066cc66,
		0x0066cc99,
		0x0066cccc,
		0x0066ccff,
		0x0066ff00,
		0x0066ff33,
		0x0066ff66,
		0x0066ff99,
		0x0066ffcc,
		0x0066ffff,
		0x00990000,
		0x00990033,
		0x00990066,
		0x00990099,
		0x009900cc,
		0x009900ff,
		0x00993300,
		0x00993333,
		0x00993366,
		0x00993399,
		0x009933cc,
		0x009933ff,
		0x00996600,
		0x00996633,
		0x00996666,
		0x00996699,
		0x009966cc,
		0x009966ff,
		0x00999900,
		0x00999933,
		0x00999966,
		0x00999999,
		0x009999cc,
		0x009999ff,
		0x0099cc00,
		0x0099cc33,
		0x0099cc66,
		0x0099cc99,
		0x0099cccc,
		0x0099ccff,
		0x0099ff00,
		0x0099ff33,
		0x0099ff66,
		0x0099ff99,
		0x0099ffcc,
		0x0099ffff,
		0x00cc0000,
		0x00cc0033,
		0x00cc0066,
		0x00cc0099,
		0x00cc00cc,
		0x00cc00ff,
		0x00cc3300,
		0x00cc3333,
		0x00cc3366,
		0x00cc3399,
		0x00cc33cc,
		0x00cc33ff,
		0x00cc6600,
		0x00cc6633,
		0x00cc6666,
		0x00cc6699,
		0x00cc66cc,
		0x00cc66ff,
		0x00cc9900,
		0x00cc9933,
		0x00cc9966,
		0x00cc9999,
		0x00cc99cc,
		0x00cc99ff,
		0x00cccc00,
		0x00cccc33,
		0x00cccc66,
		0x00cccc99,
		0x00cccccc,
		0x00ccccff,
		0x00ccff00,
		0x00ccff33,
		0x00ccff66,
		0x00ccff99,
		0x00ccffcc,
		0x00ccffff,
		0x00ff0000,
		0x00ff0033,
		0x00ff0066,
		0x00ff0099,
		0x00ff00cc,
		0x00ff00ff,
		0x00ff3300,
		0x00ff3333,
		0x00ff3366,
		0x00ff3399,
		0x00ff33cc,
		0x00ff33ff,
		0x00ff6600,
		0x00ff6633,
		0x00ff6666,
		0x00ff6699,
		0x00ff66cc,
		0x00ff66ff,
		0x00ff9900,
		0x00ff9933,
		0x00ff9966,
		0x00ff9999,
		0x00ff99cc,
		0x00ff99ff,
		0x00ffcc00,
		0x00ffcc33,
		0x00ffcc66,
		0x00ffcc99,
		0x00ffcccc,
		0x00ffccff,
		0x00ffff00,
		0x00ffff33,
		0x00ffff66,
		0x00ffff99,
		0x00ffffcc,
		0x00ffffff,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0xff010101,	// transparent color
		0x00fffbf0,
		0x00a0a0a4,
		0x00808080,
		0x00ff0000,
		0x0000ff00,
		0x00ffff00,
		0x000000ff,
		0x00ff00ff,
		0x0000ffff,
		0x00ffffff,
};

#define  PaletteLength (sizeof(PaletteTable1)/sizeof(Uint32))

static Uint32 GetRLEColor(int index){
	if((index<0)||(index>PaletteLength)) return 0;
	return *(PaletteTable1+index);
}


int CreateBitmapFromRLP(PBITMAP pBmp, BYTE* pattern) {
RLE_FORMAT	rle;

	if(!pattern) return -1;
	if(!pBmp) return -1;

	rle.reclen	  = (pattern[1]<<8 | pattern[0]) - 1;
	rle.rleformat =  pattern[2];	/* pPattern[2] offset 2 */
	rle.width	  =  pattern[3];
	rle.height	  =  pattern[4];
	rle.colortno = 1 << rle.rleformat;

	pBmp->bmType = 0;
	pBmp->bmBitsPerPixel = 32;
	pBmp->bmBytesPerPixel = 4;
	pBmp->bmColorKey = 0;
	pBmp->bmAlpha = 0;
	pBmp->bmAlphaPixelFormat = NULL;
	pBmp->bmWidth = rle.width;
	pBmp->bmHeight = rle.height;
	pBmp->bmPitch = rle.width*pBmp->bmBytesPerPixel;

	pBmp->bmBits = malloc(pBmp->bmWidth*pBmp->bmHeight*pBmp->bmBytesPerPixel);
	if (!pBmp->bmBits) {
		return -1;
	}

	Uint32 dataoffset = 5;// + rle.colortno;
	Uint8 iCnt=0;
	for ( iCnt = 0;iCnt < rle.colortno;iCnt++ ) {
		rle.patcoloridx[iCnt] = pattern[dataoffset++];	/* beginner offset 7 ~ 14 (ex: 8 colors) */
		if(rle.patcoloridx[iCnt]==RLE_TRANSPARENT_INDEX) {
			pBmp->bmColorKey = GetRLEColor(rle.patcoloridx[iCnt]);
			pBmp->bmType|= BMP_TYPE_COLORKEY;
		}
	}
	Uint32 bufferoffset = 0;
	Uint32 fillcolor_char = 0;
	Uint16 final_pixel = pBmp->bmHeight*pBmp->bmWidth;
	Uint32* buffer = (Uint32*)pBmp->bmBits;
	while( dataoffset < rle.reclen )
 	{
 		/* count color index of pattern */
		rle.coloridx = *(pattern + dataoffset) & ( rle.colortno-1);
		/* count color repeat times of pattern index_color */
 		rle.colorlen =  (*(pattern + dataoffset) >> rle.rleformat) +1 ;

 		//fillcolor_char = PaletteTable[rle.coloridx];
 		fillcolor_char = GetRLEColor(rle.patcoloridx[rle.coloridx]);

		dataoffset++;	/* Adjust offset to next pPattern */
		/* Decoder RLE PATTERN ]. */
		/* feed Pattern data to OSD memory buffer */
		while ( rle.colorlen-- ){
			if(bufferoffset < final_pixel){
				*buffer = fillcolor_char;
				buffer++;
			}
			bufferoffset++;
		}
	} /* while */

	return 0;
}


void PutRLP(HDC hdc, int x, int y, BYTE* pattern) {
PDC pdc;
register Uint32 yline;
register Uint32 xstart;
register Uint32 xend;
Uint32 PAL_transp_idx=-1;		// index number of transparent in RLP palette
Uint32 idxmask;
Uint32 rleformat;
Uint32 width;
Uint32 coloridx,colorlen;
Uint32 patcoloridx[16];
register int thisrun;
register int breakline;
BYTE* pat_end;

	if(!pattern) return;
    if (!(pdc = __mg_check_ecrgn (hdc))) return;

	pdc->cur_ban = NULL;
	pdc->step = 1;

	thisrun	  = (pattern[1]<<8 | pattern[0]);
	pat_end=pattern+thisrun;
	rleformat =  pattern[2];	/* pPattern[2] offset 2 */
	if (rleformat&0x80) {	// big RLP, width or height is bigger then 255
		width = (pattern[4]<<16)|pattern[3];
		thisrun= (pattern[6]<<16)|pattern[5];;
		pattern+=7;
	}
	else {	// small RLP
		width = pattern[3];
		thisrun= pattern[4];
		pattern+=5;
	}

    /* set rc_output to the image rect rect. */
    pdc->rc_output.left=x;
    pdc->rc_output.top=y;
    pdc->rc_output.right=x+width;
    pdc->rc_output.bottom=y+thisrun;


	colorlen = 1 << rleformat&0x7f;
	for ( thisrun = 0;thisrun < colorlen;thisrun++ ){
		/* beginner offset 7 ~ 14 (ex: 8 colors) */
		if( (patcoloridx[thisrun] = *pattern++) == RLE_TRANSPARENT_INDEX ) {
			PAL_transp_idx=thisrun;
		}
	}

    idxmask=colorlen-1;
    yline=y;
    xstart=0;
    xend=x-1;

    ENTER_DRAWING (pdc);

	while( pattern<pat_end ) {
		thisrun = *pattern++;

		colorlen=(thisrun>>rleformat)+1;
		coloridx=thisrun&idxmask;

		pdc->cur_pixel = GetRLEColor(patcoloridx[coloridx]);

		while (colorlen) {

			thisrun=width-xstart;
			breakline=(int)colorlen-thisrun;
			if (breakline<0) {  // ends within the line
				thisrun=colorlen;
			}

			if((pdc->bkmode!=BM_TRANSPARENT)||(coloridx!=PAL_transp_idx)) {
				if (thisrun>1) _dc_draw_hline_clip(pdc, x+xstart, xend+thisrun, yline);
				else _dc_set_pixel_clip(pdc, x+xstart,yline);
			}

			if (breakline<0) { // ends within the line
				xstart+=thisrun;
				xend+=thisrun;
				break;
			}
			else {	// needs to break to next line
				xstart=0;
				xend=x-1;
				yline++;
				if (!breakline) break;	// just at end of line
				colorlen-=thisrun;
			}
		}
	}

    LEAVE_DRAWING (pdc);
    UNLOCK_GCRINFO (pdc);

}


#if 0
int PutRLP1(HDC hdc, int x, int y, char* pattern) {
	PDC pdc;
	int rtn = FALSE;
	if(!pattern) return rtn; /* Null Point */
    if (!(pdc = __mg_check_ecrgn (hdc))) return rtn;

    /* set rc_output to the image rect rect. */

	pdc->cur_ban = NULL;
	pdc->step = 1;
	RLE_FORMAT	rle;
	rle.reclen	  = (pattern[1]<<8 | pattern[0]) - 1;
	rle.rleformat =  pattern[2];	/* pPattern[2] offset 2 */
	rle.width	  =  pattern[3];
	rle.height	  =  pattern[4];
	rle.colortno = 1 << rle.rleformat;

    pdc->rc_output.left=x;
    pdc->rc_output.top=y;
    pdc->rc_output.right=x+rle.width;
    pdc->rc_output.bottom=y+rle.height;

	Uint32 dataoffset = 5;// + rle.colortno;
	Uint8 iCnt=0;
	for ( iCnt = 0;iCnt < rle.colortno;iCnt++ ){
		rle.patcoloridx[iCnt] = pattern[dataoffset++];	/* beginner offset 7 ~ 14 (ex: 8 colors) */
	}
	Uint32 bufferoffset = 0;
	Uint32 y_index = y;
	Uint32 x_index = 0;
	Uint32 x_loop = 0;
	Uint32 x_end = 0;
	Uint32 RLP_transparent = GetRLEColor(RLE_TRANSPARENT_INDEX);

    ENTER_DRAWING (pdc);
	while( dataoffset < rle.reclen )
	{
		/* count color index of pattern */
		rle.coloridx = *(pattern + dataoffset) & ( rle.colortno-1);
		/* count color repeat times of pattern index_color */
		rle.colorlen =	(*(pattern + dataoffset) >> rle.rleformat) +1 ;
		pdc->cur_pixel = GetRLEColor(rle.patcoloridx[rle.coloridx]);
		dataoffset++;	/* Adjust offset to next pPattern */
		/* Decoder RLE PATTERN ]. */
		/* feed Pattern data to OSD memory buffer */

		x_loop = (x_index+rle.colorlen)/rle.width;
		x_end = (x_index+rle.colorlen)%rle.width;
		//y_index = bufferoffset%rle.width;

		if(x_loop==0){
			if((pdc->cur_pixel!=RLP_transparent)||(pdc->bkmode!=BM_TRANSPARENT))
				_dc_draw_hline_clip(pdc, x+x_index, x+x_end, y_index);
		}
		else{
			if((pdc->cur_pixel!=RLP_transparent)||(pdc->bkmode!=BM_TRANSPARENT)){
				_dc_draw_hline_clip(pdc, x+x_index, x+rle.width, y_index);
				while(--x_loop){
					y_index++;
					_dc_draw_hline_clip(pdc, x, x+rle.width, y_index);
				}
				y_index++;
				if(x_end!=0){
					_dc_draw_hline_clip(pdc, x, x+x_end, y_index);
				}
			}
			else{
				y_index +=x_loop;
			}
		}

		x_index = x_end;
		bufferoffset+=rle.colorlen;
	} /* while */
    LEAVE_DRAWING (pdc);
    UNLOCK_GCRINFO (pdc);
	rtn = TRUE;
	return rtn;
}
#endif


