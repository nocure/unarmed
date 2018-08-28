#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"

/*--------------------------------------------------------------------------*/
/* Contro Text define                                                       */
/*--------------------------------------------------------------------------*/
#define ZPAD            '*'            /* 052 Pad character begins frames   */
#define ZDLE            030            /* ^X Zmodem escape- `ala BISYNC DLE */
#define ZDLEE           (ZDLE^0100)    /* Escaped ZDLE as transmitted       */
#define ZBIN            'A'            /* Binary frame indicator            */
#define ZHEX            'B'            /* HEX frame indicator               */
#define ZBIN32          'C'            /* Binary frame with 32 bit FCS      */

/*--------------------------------------------------------------------------*/
/* Frame types (see array "frametypes" in zmodem.c)                         */
/*--------------------------------------------------------------------------*/
#define ZRQINIT         0              /* Request receive init              */
#define ZRINIT          1              /* Receive init                      */
#define ZSINIT          2              /* Send init sequence (optional)     */
#define ZACK            3              /* ACK to above                      */
#define ZFILE           4              /* File name from sender             */
#define ZSKIP           5              /* To sender: skip this file         */
#define ZNAK            6              /* Last packet was garbled           */
#define ZABORT          7              /* Abort batch transfers             */
#define ZFIN            8              /* Finish session                    */
#define ZRPOS           9              /* Resume transmit at this position  */
#define ZDATA           10             /* Data packet(s) follow             */
#define ZEOF            11             /* End of file                       */
#define ZFERR           12             /* Fatal Read/Write error Detected   */
#define ZCRC            13             /* Request for file CRC and response */
#define ZCHALLENGE      14             /* Receiver's Challenge              */
#define ZCOMPL          15             /* Request is complete               */
#define ZCAN            16             /* Other end canned with CAN*5       */
#define ZFREECNT        17             /* Request for free bytes on disk    */
#define ZCOMMAND        18             /* Command from sending program      */
#define ZSTDERR         19             /* Send following to stderr          */

/*--------------------------------------------------------------------------*/
/* ZDLE sequences                                                           */
/*--------------------------------------------------------------------------*/
#define ZCRCE           'h'            /* CRC next/frame ends/hdr follows   */
#define ZCRCG           'i'            /* CRC next/frame continues nonstop  */
#define ZCRCQ           'j'            /* CRC next/frame continues/want ZACK*/
#define ZCRCW           'k'            /* CRC next/ZACK expected/end of frame*/
#define ZRUB0           'l'            /* Translate to rubout 0177          */
#define ZRUB1           'm'            /* Translate to rubout 0377          */

/*--------------------------------------------------------------------------*/
/* Z_GetZDL return values (internal)                                        */
/* -1 is general error, -2 is timeout                                       */
/*--------------------------------------------------------------------------*/
#define GOTOR           0400           /* Octal alert! Octal alert!         */
#define GOTCRCE         (ZCRCE|GOTOR)  /* ZDLE-ZCRCE received               */
#define GOTCRCG         (ZCRCG|GOTOR)  /* ZDLE-ZCRCG received               */
#define GOTCRCQ         (ZCRCQ|GOTOR)  /* ZDLE-ZCRCQ received               */
#define GOTCRCW         (ZCRCW|GOTOR)  /* ZDLE-ZCRCW received               */
#define GOTCAN          (GOTOR|030)    /* CAN*5 seen                        */

/*--------------------------------------------------------------------------*/
/* Byte positions within header array                                       */
/*--------------------------------------------------------------------------*/
#define ZF0             3              /* First flags byte                  */
#define ZF1             2
#define ZF2             1
#define ZF3             0
#define ZP0             0              /* Low order 8 bits of position      */
#define ZP1             1
#define ZP2             2
#define ZP3             3              /* High order 8 bits of file pos     */

/*--------------------------------------------------------------------------*/
/* Bit Masks for ZRINIT flags byte ZF0                                      */
/*--------------------------------------------------------------------------*/
#define CANFDX          01             /* Can send and receive true FDX     */
#define CANOVIO         02             /* Can receive data during disk I/O  */
#define CANBRK          04             /* Can send a break signal           */
#define CANCRY          010            /* Can decrypt                       */
#define CANLZW          020            /* Can uncompress                    */
#define CANFC32         040            /* Can use 32 bit Frame Check        */


/*--------------------------------------------------------------------------*/
/* PARAMETERS FOR ZFILE FRAME...                                            */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* Conversion options one of these in ZF0                                   */
/*--------------------------------------------------------------------------*/
#define ZCBIN           1              /* Binary transfer - no conversion   */
#define ZCNL            2              /* Convert NL to local EOLN          */
#define ZCRESUM         3              /* Resume interrupted file transfer  */

/*--------------------------------------------------------------------------*/
/* Management options, one of these in ZF1                                  */
/*--------------------------------------------------------------------------*/
#define ZMNEW           1              /* Transfer iff source newer/longer  */
#define ZMCRC           2              /* Transfer if different CRC/length  */
#define ZMAPND          3              /* Append contents to existing file  */
#define ZMCLOB          4              /* Replace existing file             */
#define ZMSPARS         5              /* Encoding for sparse file          */
#define ZMDIFF          6              /* Transfer if dates/lengths differ  */
#define ZMPROT          7              /* Protect destination file          */

/*--------------------------------------------------------------------------*/
/* Transport options, one of these in ZF2                                   */
/*--------------------------------------------------------------------------*/
#define ZTLZW           1              /* Lempel-Ziv compression            */
#define ZTCRYPT         2              /* Encryption                        */
#define ZTRLE           3              /* Run Length encoding               */

/*--------------------------------------------------------------------------*/
/* Parameters for ZCOMMAND frame ZF0 (otherwise 0)                          */
/*--------------------------------------------------------------------------*/
#define ZCACK1          1              /* Acknowledge, then do command      */

/*--------------------------------------------------------------------------*/
/* Miscellaneous definitions                                                */
/*--------------------------------------------------------------------------*/
#define ZZOK            0
#define ZZERROR         (-1)
#define ZZTIMEOUT       (-2)
#define ZZRCDO          (-3)
#define ZZFUBAR         (-4)

#define XON             ('Q'&037)
#define XOFF            ('S'&037)
#define CPMEOF          ('Z'&037)

#define LZCONV          0              /* Default ZMODEM conversion mode    */
#define LZMANAG         0              /* Default ZMODEM file mode          */
#define LZTRANS         0              /* Default ZMODEM transport mode     */
#define WAZOOMAX        8192           /* Max packet size (WaZOO)           */


/*--------------------------------------------------------------------------*/
/* ASCII MNEMONICS                                                          */
/*--------------------------------------------------------------------------*/
#define NUL 0x00
#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define BEL 0x07
#define BS  0x08
#define HT  0x09
#define LF  0x0a
#define VT  0x0b
#define FF  0x0c
#define CR  0x0d
#define SO  0x0e
#define SI  0x0f
#define DLE 0x10
#define DC1 0x11
#define DC2 0x12
#define DC3 0x13
#define DC4 0x14
#define NAK 0x15
#define SYN 0x16
#define ETB 0x17
#define CAN 0x18
#define EM  0x19
#define SUB 0x1a
#define ESC 0x1b
#define FS  0x1c
#define GS  0x1d
#define RS  0x1e
#define US  0x1f

#define Z_ClearByte()

/*--------------------------------------------------------------------------*/
/* Private data                                                             */
/*--------------------------------------------------------------------------*/

#define ZATTNLEN 32              /* Max length of attention string          */

BYTE  Attn[ZATTNLEN+1]; /* String rx sends to tx on err            */
char  Zconv;            /* ZMODEM file conversion request          */
BYTE  Recbuf[64];       /* Pointer to receive buffer               */
BYTE  *FileBuf;         /* Pointer to received file buffer         */
int   Rxtype;           /* Type of header received                 */
int   Rxframeind;       /* ZBIN ZBIN32,ZHEX type of frame received */
int   Tryzhdrtype;      /* Hdr type to send for Last rx close      */
int   RxCount;          /* Count of data bytes received            */
char  Rxhdr[4];         /* Received header                                  */
char  Txhdr[4];         /* Transmitted header                               */
long  Rxpos;            /* Received file position                           */
DWORD  rxbytes;          /* bytes of data received                           */

int bTimeOut=FALSE;


BYTE *DataPtr;
static BYTE OneSramByte(DWORD pos) {
	return DataPtr[pos];
}


int g_uart_temp_c=-1;
int g_uart_tenths_wait=100;
static int Z_TestOneByte(void) {
	if (g_uart_temp_c>=0) return 1;
	g_uart_temp_c=tq_uartGetCharNB();
	if (g_uart_temp_c>=0) return 1;
	return 0;
}

/*--------------------------------------------------------------------------*/
/* Z GET BYTE                                                               */
/* Get a byte from the modem.                                               */ //从modem中获得一个字节
/* return TIMEOUT if no read within timeout tenths,                         */
/*--------------------------------------------------------------------------*/
int Z_GetByte(int tenths)
{
/*
  bTimeOut = FALSE;
  tenths = tenths / 10;
  SetSysTimer(tenths);
  while (GetSysTimer()) { if (Z_TestOneByte())  return Z_GetOneByte(); }
  bTimeOut = TRUE;
  return '\xff';
*/
/*
int c;
  bTimeOut = FALSE;
  while (!bTimeOut) {
	  c=yl_uartGetChar();
	  if (c>=0) {
		  return c;
	  }
	  tenths-=2000;
	  if (tenths<0) bTimeOut = TRUE;
  }
  return '\xff';
*/
int c;

	bTimeOut = FALSE;
	if (g_uart_temp_c>=0) {
		g_uart_temp_c=-1;
		return g_uart_temp_c;
	}

	c=tq_uartGetCharT(tenths);
	if (c>=0) {
		return c;
	}
	bTimeOut = TRUE;
	return -1;
}

/*--------------------------------------------------------------------------*/
/* Z TIMED READ                                                             */
/* Read a character from the modem line with timeout.                       */  //从modem中读取一个字符with timeout
/*  Eat parity, XON and XOFF characters.                                    */
/*--------------------------------------------------------------------------*/
BYTE _Z_TimedRead()
{
  for (;;) {
    BYTE c = Z_GetByte(30000); if (bTimeOut) return c;
    if ((c & 0x7f) == XON) ;
    else if ((c & 0x7f) == XOFF) ;
    else return c;
    }
}

/*--------------------------------------------------------------------------*/
/* Z UPDATE CRC                                                             */
/* update CRC                                                               */   //更新CRC
/*--------------------------------------------------------------------------*/
#ifdef CRCTABLE
/* crctab calculated by Mark G. Mendel, Network Systems Corporation */
static WORD crctab[256] = {                                                      //CRC表
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

#define Z_UpdateCRC(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)

#else

WORD Z_UpdateCRC(WORD c, WORD crc)                           //更新CRC（Z_UpdateCRC）
{ int count;

  for (count=8; --count>=0; ) {
    if (crc & 0x8000) {
      crc <<= 1;
      crc += (((c<<=1) & 0400)  !=  0);
      crc ^= 0x1021;
      }
    else {
      crc <<= 1;
      crc += (((c<<=1) & 0400)  !=  0);
      }
    }
  return crc;
}
#endif

//#define SENDBYTE(c) SendChar(1,c)
#define SENDBYTE(c) putch(c)

/* Send a byte as two hex digits */
static BYTE hex[] = "0123456789abcdef";
#define Z_PUTHEX(i,c) {i=(c);SENDBYTE(hex[((i)&0xF0)>>4]);SENDBYTE(hex[(i)&0xF]);}

/*--------------------------------------------------------------------------*/
/* Z SEND HEX HEADER                                                        */
/* Send ZMODEM HEX header hdr of type type                                  */
/*--------------------------------------------------------------------------*/   //发送zmodem……
void Z_SendHexHeader(BYTE type,char *hdr)                                        //Z_SendHexHeader
{ int n; BYTE i; WORD crc;

  SENDBYTE(ZPAD);
  SENDBYTE(ZPAD);
  SENDBYTE(ZDLE);
  SENDBYTE(ZHEX);
  Z_PUTHEX(i,type);

  crc = Z_UpdateCRC(type, 0);
  for (n=4; --n >= 0;) {
    Z_PUTHEX(i,(*hdr));
    crc = Z_UpdateCRC(((unsigned short)(*hdr++)), crc);
    }
  crc = Z_UpdateCRC(0,crc);
  crc = Z_UpdateCRC(0,crc);
  Z_PUTHEX(i,(crc>>8));
  Z_PUTHEX(i,crc);

  /* Make it printable on remote machine */                                //使得在远程的机器上可打印
  SENDBYTE('\x0d'); SENDBYTE('\x0a');

  /* Uncork the remote in case a fake XOFF has stopped data flow */
  if (type != ZFIN) SENDBYTE(021);
}

/*--------------------------------------------------------------------------*/
/* Z GET ZDL                                                                */
/* Read a byte, checking for ZMODEM escape encoding                         */
/* including CAN*5 which represents a quick abort                           */
/* 0x10,0x11,0x13,0x90,0x91,0x93 & 0x18 is special treatment                */
/*--------------------------------------------------------------------------*/
int Z_GetZDL(void)                                                      //Z_GetZDL……
{ int c;

  c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
  if (c != ZDLE) return c;

  c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
  switch (c) {
    case CAN:
      c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
      if (c != CAN) return c;
      c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
      if (c != CAN) return c;
      c = Z_GetByte(1000); if (bTimeOut) return ZZTIMEOUT;
      if (c != CAN) return c;
      return (GOTCAN);
    case ZCRCE:
    case ZCRCG:
    case ZCRCQ:
    case ZCRCW: return (c | GOTOR);
    case ZRUB0: return 0x7F;
    case ZRUB1: return 0xFF;
    default:
      if ((c & 0x60) == 0x40) return c ^ 0x40;
      else return ZZERROR;
    }
}

/*--------------------------------------------------------------------------*/
/* Z GET BINARY HEADER                                                      */
/* Receive a binary style header (type and position)                        */   //接受一个二进制的style header（类型和位置）
/*--------------------------------------------------------------------------*/
int _Z_GetBinaryHeader(char *hdr)                         //_Z_GetBinaryHeader“？”
{ int c,n; WORD crc;

  if ((c   = Z_GetZDL()) & ~0xFF)   return c;
  Rxtype   = c;
  crc      = Z_UpdateCRC(c, 0);

  for (n=4; --n >= 0;) {
    if ((c = Z_GetZDL()) & ~0xFF) return c;
    crc = Z_UpdateCRC(c, crc);
    *hdr++ = c;
    }
  if ((c   = Z_GetZDL()) & ~0xFF) return c;
  crc      = Z_UpdateCRC(c, crc);
  if ((c   = Z_GetZDL()) & ~0xFF) return c;

  crc = Z_UpdateCRC(c, crc);
  if (crc & 0xFFFF) return ZZERROR;     //CRC error

  return Rxtype;
}

/*--------------------------------------------------------------------------*/
/* Z GET HEX                                                                */
/* Decode two lower case hex digits into an 8 bit byte value                */
/*--------------------------------------------------------------------------*/
int _Z_GetHex(void)                                                 //_Z_GetHex“？”
{ int ch,cl;

  ch = _Z_TimedRead(); if (bTimeOut) return ZZTIMEOUT;
  ch -= '0';
  if (ch > 9) ch -= ('a' - ':');
  if (ch & 0xf0) return ZZERROR;

  cl = _Z_TimedRead(); if (bTimeOut) return ZZTIMEOUT;
  cl -= '0';
  if (cl > 9) cl -= ('a' - ':');
  if (cl & 0xf0) return ZZERROR;

  return ((ch << 4) | cl);
}

/*--------------------------------------------------------------------------*/
/* Z GET HEX HEADER                                                         */
/* Receive a hex style header (type and position)                           */
/*--------------------------------------------------------------------------*/
int _Z_GetHexHeader(char *hdr)                                     //_Z_GetHexHeader“？”
{ int c,n; WORD crc;

  if ((c   = _Z_GetHex()) < 0) return c;
  Rxtype   = c;
  crc      = Z_UpdateCRC(c, 0);

  for (n=4; --n >= 0;) {
    if ((c = _Z_GetHex()) < 0) return c;
    crc      = Z_UpdateCRC(c, crc);
    *hdr++   = c;
    }
  if ((c = _Z_GetHex()) < 0) return c;
  crc = Z_UpdateCRC(c, crc);
  if ((c = _Z_GetHex()) < 0) return c;
  crc = Z_UpdateCRC(c, crc);
  if (crc & 0xFFFF) return ZZERROR;     //CRC error         //CRC错误
  c = Z_GetByte(1000);
  if (!bTimeOut) { if (c == '\x0d') Z_GetByte(1000); } // Throw away possible cr/lf     //丢弃可能的cr/lf

  return Rxtype;
}

/*--------------------------------------------------------------------------*/
/* Z PULL LONG FROM HEADER                                                  */
/* Recover a long integer from a header                                     */          //接收一个长整型数from a header
/*--------------------------------------------------------------------------*/
DWORD _Z_PullLongFromHeader(char *hdr)                                                  //_Z_PullLongFromHeader“？”
{ DWORD l;

  l = hdr[ZP3];
  l = (l << 8) | hdr[ZP2];
  l = (l << 8) | hdr[ZP1];
  l = (l << 8) | hdr[ZP0];
  return l;
}

/*--------------------------------------------------------------------------*/
/* Z LONG TO HEADER                                                         */
/* Store long integer pos in Txhdr                                          */
/*--------------------------------------------------------------------------*/
void Z_PutLongIntoHeader(DWORD pos)                                                     //Z_PutLongIntoHeader“？”
{
  Txhdr[ZP0] = pos;
  Txhdr[ZP1] = pos>>8;
  Txhdr[ZP2] = pos>>16;
  Txhdr[ZP3] = pos>>24;
}

/*--------------------------------------------------------------------------*/
/* Z GET HEADER                                                             */
/* Read a ZMODEM header to hdr, either binary or hex.                       */
/*   On success, set Zmodem to 1 and return type of header.                 */         //将zodem设为1，然后返回header的类型
/*   Otherwise return negative on error                                     */         //否则返回negative on error
/*--------------------------------------------------------------------------*/
int Z_GetHeader(char *hdr)                                                             //Z_GetHeader“？”
{ int c,n,cancount;

  n = 10;   /* Max characters before start of frame */                                 //……
  cancount = 5;

Again:
  Rxframeind = Rxtype = 0;

  c = _Z_TimedRead(); if (bTimeOut) c = ZZTIMEOUT;
  switch (c) {
    case ZPAD: break; // This is what we want.                                        //这是我们要求的
    case ZZRCDO:
    case ZZTIMEOUT: goto Done; //Timeout                                              //时间用完
    case CAN: if (--cancount <= 0) { c = ZCAN; goto Done; } //Cancel no break         //取消，而不是间断
    default:
Agn2: if (--n <= 0) return ZCAN;
      if (c != CAN) cancount = 5;
      goto Again;
    }

  cancount = 5;
Splat:
  c = _Z_TimedRead(); if (bTimeOut) c = ZZTIMEOUT;
  switch (c) {
    case ZDLE: break; // This is what we want.                                        //这是我们要求的
    case ZPAD: goto Splat;
    case ZZRCDO:
    case ZZTIMEOUT: goto Done; //Timeout                                              //时间用完
    default: goto Agn2;
    }

  c = _Z_TimedRead(); if (bTimeOut) c = ZZTIMEOUT;
  switch (c) {
    case ZBIN: //BIN head
      Rxframeind = ZBIN;
      c =  _Z_GetBinaryHeader(hdr);
      break;
    case ZHEX: //HEX head
      Rxframeind = ZHEX;
      c =  _Z_GetHexHeader(hdr);
      break;
    case CAN: //Cancel
      if (--cancount <= 0) { c = ZCAN; goto Done; }
      goto Agn2;
    case ZZRCDO:
    case ZZTIMEOUT: goto Done; //Timeout
    default: goto Agn2;
    }

  Rxpos = _Z_PullLongFromHeader(hdr);
Done:
  return c;
}

/*--------------------------------------------------------------------------*/
/* RZ RECEIVE DATA                                                          */  //RZ接收数据
/* Receive array buf of max length with ending ZDLE sequence                */  //用zdle和CRC接收阵列缓存，返回结束字符或错误
/* and CRC.  Returns the ending character or error code.                    */
/*--------------------------------------------------------------------------*/
int RZ_ReceiveData(BYTE *buf,int length)
{ int c,d; WORD crc;

  crc   = RxCount   = 0;
  for (;;) {
    if ((c = Z_GetZDL()) & ~0xff) {
CRCfoo:
      switch (c) {
        case GOTCRCE:
        case GOTCRCG:
        case GOTCRCQ:
        case GOTCRCW: // C R C s
          crc = Z_UpdateCRC(((d=c)&0xff), crc);
          if ((c=Z_GetZDL()) & ~0xff) goto CRCfoo;

          crc = Z_UpdateCRC(c, crc);
          if ((c=Z_GetZDL()) & ~0xff) goto CRCfoo;

          crc = Z_UpdateCRC(c, crc);
          if (crc & 0xffff) return ZZERROR;     //CRC error             //CRC错误
          return d;
        case GOTCAN:    return ZCAN;    //Cancel                        //取消
        case ZZTIMEOUT: return c;       //Timeout                       //时间结束
        case ZZRCDO:    return c;       //No Carrier                    //没有载体
        default:        return c;       //Something bizarre             //有异常
        }
      }
    if (--length < 0) return ZZERROR;   //Long pkt                      //……
    ++RxCount;
    *buf++ = c;
    crc = Z_UpdateCRC(c, crc);
    continue;
    }
}

/*--------------------------------------------------------------------------*/
/* RZ ACK BIBI                                                              */  //……
/* Ack a ZFIN packet, let byegones be byegones                              */  //……
/*--------------------------------------------------------------------------*/
void RZ_AckBibi(void)
{ int n;

  Z_PutLongIntoHeader(0L);
  for (n=4; --n;) { BYTE c;
    Z_SendHexHeader(ZFIN, Txhdr);
    c = Z_GetByte(1000); if (bTimeOut) return;
    if (c == 'O') Z_GetByte(1000);      // Discard 2nd 'O'
    }
}

/*--------------------------------------------------------------------------*/
/* RZ INIT RECEIVER                                                         */
/* Initialize for Zmodem receive attempt, try to activate Zmodem sender     */  //初始化zmodem结束，试图去接受zmodem发送端
/* Handles ZSINIT, ZFREECNT, and ZCOMMAND frames                            */  //……
/*                                                                          */
/* Return codes:                                                            */
/*    ZFILE .... Zmodem filename received                                   */  //zmodem文件名接收
/*    ZCOMPL ... transaction finished                                       */  //处理完成
/*    ERROR .... any other condition                                        */  //其他情况
/*--------------------------------------------------------------------------*/
int RZ_InitReceiver(void)                                                       //RZ_InitReceiver，初始化接收器
{ int n,errors = 0;

  for (n=3; --n>=0; ) {
    Z_PutLongIntoHeader(0L);
    Txhdr[ZF0] = CANFDX;        // | CANOVIO;
    Z_SendHexHeader(Tryzhdrtype, Txhdr);

AGAIN:
    switch (Z_GetHeader(Rxhdr)) {
      case ZRQINIT:     //USED
        Z_PutLongIntoHeader(0L);
        Txhdr[ZF0] = CANFDX;    // | CANOVIO;
        Z_SendHexHeader(ZRINIT, Txhdr);
        goto AGAIN;
      case ZFILE:       //USED
        Zconv = Rxhdr[ZF0];
        Tryzhdrtype = ZRINIT;
        if (RZ_ReceiveData(Recbuf,WAZOOMAX) == GOTCRCW) return ZFILE;
        Z_SendHexHeader(ZNAK, Txhdr);
        goto AGAIN;
      case ZSINIT:      //not used
        if (RZ_ReceiveData(Attn, ZATTNLEN) == GOTCRCW) Z_SendHexHeader(ZACK,Txhdr);
        else Z_SendHexHeader(ZNAK,Txhdr);
        goto AGAIN;
      case ZFREECNT:    //not used
        Z_PutLongIntoHeader(WAZOOMAX);
        Z_SendHexHeader(ZACK, Txhdr);
        goto AGAIN;
      case ZCOMMAND:    //not used
        /*-----------------------------------------*/
        /* Paranoia is good for you...             */
        /* Ignore command from remote, but lie and */
        /* say we did the command ok.              */
        /*-----------------------------------------*/
        if (RZ_ReceiveData(Recbuf,WAZOOMAX) == GOTCRCW) {
          Z_PutLongIntoHeader(0L);
          do { Z_SendHexHeader(ZCOMPL, Txhdr); }
             while (++errors<10 && Z_GetHeader(Rxhdr) != ZFIN);
          RZ_AckBibi();
          return ZCOMPL;
          }
        else Z_SendHexHeader(ZNAK, Txhdr);
        goto AGAIN;
      case ZCOMPL:      //not used
        goto AGAIN;
      case ZFIN:        //USED
        RZ_AckBibi(); return ZCOMPL;
      case ZCAN:        //USED
      case ZZRCDO:
        return ZZERROR;
      case ZZTIMEOUT:   //USED
        return ZZERROR;
      }
    }
    return ZZERROR;
}

/*--------------------------------------------------------------------------*/
/* RZ GET HEADER                                                            */
/* Process incoming file information header                                 */  //输入文件信息header
/*--------------------------------------------------------------------------*/
DWORD filesize; BYTE filename[32];
int RZ_GetHeader(void)                                                          //RZ_GetHeader……
{ BYTE *p; BYTE i=0;

  filesize = 0L;
  p = Recbuf; while (*p) filename[i++] = *p++; filename[i] = '\0'; p++;
  while (*p) { filesize = filesize * 10 + (*p - '0'); p++; }

  return ZZOK;
}

/*--------------------------------------------------------------------------*/
/* RZ SAVE TO DISK                                                          */
/* Writes the received file data to the output file.                        */
/* If in ASCII mode, stops writing at first ^Z, and converts all            */
/*   solo CR's or LF's to CR/LF pairs.                                      */
/*--------------------------------------------------------------------------*/
//Program Flash  address=base+ *prxbytes; cnt=RxCount;
int RZ_SaveToDisk(DWORD *prxbytes)
{
  *prxbytes += RxCount;
  return ZZOK;
}

/*--------------------------------------------------------------------------*/
/* RZ RECEIVE FILE                                                          */
/* Receive one file; assumes file name frame is preloaded in Recbuf         */
/*--------------------------------------------------------------------------*/
int RZ_ReceiveFile(BYTE *p)
{ int c,n;

  n        = 10;        //retry times;
  rxbytes  = 0L;

  for (;;) {
    Z_PutLongIntoHeader(rxbytes);
    Z_SendHexHeader(ZRPOS, Txhdr);
NxtHdr:
    switch (c = Z_GetHeader(Rxhdr)) {
    case ZDATA: // Data Packet
          if (Rxpos != rxbytes) {
          if ( --n < 0) return ZZERROR;
          continue;
          }
MoreData:
        switch (c = RZ_ReceiveData(FileBuf+rxbytes,WAZOOMAX)) {
          case ZCAN:
          case ZZRCDO:  return ZZERROR; //CAN or CARRIER
          case ZZERROR: // CRC error
            if (--n<0) return ZZERROR;
            continue;
          case ZZTIMEOUT: //Timeout
            if (--n<0) return ZZERROR;
            continue;
          case GOTCRCW: // End of frame
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            Z_PutLongIntoHeader(rxbytes);
            Z_SendHexHeader(ZACK, Txhdr);
            goto NxtHdr;
          case GOTCRCQ: // Zack expected
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            Z_PutLongIntoHeader(rxbytes);
            Z_SendHexHeader(ZACK, Txhdr);
            goto MoreData;
          case GOTCRCG: // Non-stop
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            goto MoreData;
          case GOTCRCE: // Header to follow
            n = 10;
            if (RZ_SaveToDisk(&rxbytes) == ZZERROR) return ZZERROR;
            goto NxtHdr;
          }
      case ZNAK:
      case ZZTIMEOUT: // Packed was probably garbled
        if ( --n < 0) return ZZERROR;
        continue;
      case ZFILE: // Sender didn't see our ZRPOS yet
        RZ_ReceiveData(Recbuf, WAZOOMAX);
        continue;
      case ZEOF: // End of the file
        if (Rxpos != rxbytes) continue;
        return c;
      case ZZERROR: // Too much garbage in header search error
        if ( --n < 0) return ZZERROR;
        continue;

      default: return ZZERROR;
      }
    }
}

/*--------------------------------------------------------------------------*/
/* RZFILES                                                                  */
/* Receive a batch of files using ZMODEM protocol                           */
/*--------------------------------------------------------------------------*/
int RZ_ReceiveBatch(BYTE *p)
{ int c;

  for (;;) {
    switch (c = RZ_ReceiveFile(p)) {
      case ZEOF:
      case ZSKIP:
        switch (RZ_InitReceiver()) {
          case ZCOMPL:   return ZZOK;
          case ZFILE:    break;
          default:       return ZZERROR;
          }
        break;
      default: return c;
      }
    }
}

/*--------------------------------------------------------------------------*/
/* GET ZMODEM                                                               */
/* Receive a batch of files.                                                */
/* returns TRUE (1) for good xfer, FALSE (0) for bad                        */
/* can be called from f_upload or to get mail from a WaZOO Opus             */
/*--------------------------------------------------------------------------*/

DWORD get_Zmodem(BYTE *p)
{ int i;

  Tryzhdrtype = ZRINIT;

  Z_ClearByte();

  FileBuf = (BYTE *)p;
  i = RZ_InitReceiver();
  if ((i == ZCOMPL) || ((i == ZFILE) && ((RZ_ReceiveBatch(p)) == ZZOK))) {
    printf("%d bytes received!!!\n",rxbytes);
    Z_ClearByte();
    return rxbytes;
    }
  else {
    printf("Download error!!!\n");
    Z_ClearByte();
    return 0L;
    }
}


/*--------------------------------------------------------------------------*/
/* Private data                                                             */
/*--------------------------------------------------------------------------*/

DWORD  Txpos;              /* Transmitted file position               */
int Rxbuflen = 0;          /* Receiver's max buffer length            */


/*--------------------------------------------------------------------------*/
/* ZS SEND BYTE                                                             */
/* Send character c with ZMODEM escape sequence encoding.                   */
/* Escape XON, XOFF. Escape CR following @ (Telenet net escape)             */
/*--------------------------------------------------------------------------*/
void ZS_SendByte(BYTE c)
{
    static BYTE lastsent;

    switch (c)
    {
        case 015:
        case 0215:  /*--------------------------------------------------*/
                    /*                                                  */
                    /*--------------------------------------------------*/
            if ((lastsent & 0x7F) != '@') goto SendIt;
        case 020:
        case 021:
        case 023:
        case 0220:
        case 0221:
        case 0223:
        case ZDLE:  /*--------------------------------------------------*/
                    /* Quoted characters                                */
                    /*--------------------------------------------------*/
            SENDBYTE(ZDLE);
            c ^= 0x40;

        default:    /*--------------------------------------------------*/
                    /* Normal character output                          */
SendIt:             /*--------------------------------------------------*/
            SENDBYTE(lastsent = c);
    }
}

/*--------------------------------------------------------------------------*/
/* ZS SEND BINARY HEADER                                                    */
/* Send ZMODEM binary header hdr of type type                               */
/*--------------------------------------------------------------------------*/
void ZS_SendBinaryHeader(int type, char *hdr)
{
    char *hptr;
    WORD crc;
    int  n;

    SENDBYTE(ZPAD);
    SENDBYTE(ZDLE);
    SENDBYTE(ZBIN);
    ZS_SendByte(type);

    crc = Z_UpdateCRC(type, 0);

    hptr  = hdr;
    for (n=4; --n >= 0;)
    {
        ZS_SendByte(*hptr);
        crc = Z_UpdateCRC(((unsigned short)(*hptr++)), crc);
    }

    crc = Z_UpdateCRC(0,crc);
    crc = Z_UpdateCRC(0,crc);
    ZS_SendByte(crc>>8);
    ZS_SendByte(crc);
}

/*--------------------------------------------------------------------------*/
/* ZS GET RECEIVER INFO                                                     */
/* Get the receiver's init parameters                                       */
/*--------------------------------------------------------------------------*/
int ZS_GetReceiverInfo()
{
    int   n;

    for (n=10; --n>=0; )
    {
        switch ( Z_GetHeader(Rxhdr) )
        {
            case ZCHALLENGE:    /*--------------------------------------*/
                                /* Echo receiver's challenge number     */
                                /*--------------------------------------*/
                Z_PutLongIntoHeader(Rxpos);
                Z_SendHexHeader(ZACK, Txhdr);
                continue;
            case ZCOMMAND:      /*--------------------------------------*/
                                /* They didn't see our ZRQINIT          */
                                /*--------------------------------------*/
                Z_PutLongIntoHeader(0L);
                Z_SendHexHeader(ZRQINIT, Txhdr);
                continue;
            case ZRINIT:        /*--------------------------------------*/
                                /*                                      */
                                /*--------------------------------------*/
                Rxbuflen = ((WORD)Rxhdr[ZP1]<<8)|Rxhdr[ZP0];
                return ZZOK;
            case ZCAN:
            case ZZRCDO:
            case ZZTIMEOUT:     /*--------------------------------------*/
                                /*                                      */
                                /*--------------------------------------*/
                return ZZERROR;
            case ZRQINIT:       /*--------------------------------------*/
                                /*                                      */
                                /*--------------------------------------*/
                if (Rxhdr[ZF0] == ZCOMMAND) continue;
            default:            /*--------------------------------------*/
                                /*                                      */
                                /*--------------------------------------*/
                Z_SendHexHeader(ZNAK, Txhdr);
                continue;
        }
    }
    return ZZERROR;
}

/*--------------------------------------------------------------------------*/
/* ZS SEND DATA                                                             */
/* Send binary array buf with ending ZDLE sequence frameend                 */
/*--------------------------------------------------------------------------*/
void ZS_SendData(DWORD pos, DWORD end, unsigned short frameend)
{
    WORD crc;

    crc = 0;
    while (pos < end)
    {
        BYTE b = OneSramByte(pos++);
        ZS_SendByte(b);
        crc = Z_UpdateCRC(((unsigned short)b), crc);
    }
    SENDBYTE(ZDLE);
    SENDBYTE(frameend);
    crc = Z_UpdateCRC(frameend, crc);

    crc = Z_UpdateCRC(0,crc);
    crc = Z_UpdateCRC(0,crc);
    ZS_SendByte(crc>>8);
    ZS_SendByte(crc);
}

int modify = 40;
//void ZS_SendFileName(BYTE *name, int numK)
void ZS_SendFileName(BYTE *name, int lenB)
{
    WORD crc = 0;
    BYTE str[] = "00000000 000 0\0", *p;
//    DWORD lenB = numK * 1024;

    p = str;
    while (lenB >= 100000) { lenB -= 100000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
    while (lenB >=  10000) { lenB -=  10000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
    while (lenB >=   1000) { lenB -=   1000; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
    while (lenB >=    100) { lenB -=    100; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
    while (lenB >=     10) { lenB -=     10; (*p)++; } if (p != str) p++; else if (*p != '0') p++;
    while (lenB >=      1) { lenB -=      1; (*p)++; } p++; *p++ = ' ';

    modify++;
    while (modify >= 100) { modify -= 100; (*p)++; }  p++;
    while (modify >=  10) { modify -=  10; (*p)++; }  p++;
    while (modify >=   1) { modify -=   1; (*p)++; }  p++;
    *p++ = ' '; *p++ = '0'; *p = '\0';

    for (;;)
    {
        ZS_SendByte(*name);
        crc = Z_UpdateCRC(((unsigned short)*name), crc);
        if (*name) ; else break;
        name++;
    }
    p = str;
    for (;;)
    {
        ZS_SendByte(*p);
        crc = Z_UpdateCRC(((unsigned short)*p), crc);
        if (*p) ; else break;
        p++;
    }
    SENDBYTE(ZDLE);
    SENDBYTE(ZCRCW);
    crc = Z_UpdateCRC(ZCRCW, crc);

    crc = Z_UpdateCRC(0, crc);
    crc = Z_UpdateCRC(0, crc);
    ZS_SendByte(crc >> 8);
    ZS_SendByte(crc);

//  SENDBYTE(XON);
}

/*--------------------------------------------------------------------------*/
/* ZS SYNC WITH RECEIVER                                                    */
/* Respond to receiver's complaint, get back in sync with receiver          */
/*--------------------------------------------------------------------------*/
int ZS_SyncWithReceiver()
{
    int c;
    int num_errs = 7;

    while(1)
    {
        c = Z_GetHeader(Rxhdr);

        switch (c)
        {
            case ZZTIMEOUT:  if ((num_errs--) >= 0) break;
            case ZCAN:
            case ZABORT:
            case ZFIN:
            case ZZRCDO:
                        /*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
                return ZZERROR;
            case ZRPOS: /*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
                Txpos = Rxpos;
                return c;
            case ZSKIP: /*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
            case ZRINIT:/*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
                return c;
            case ZACK:  /*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
                return c;
            default:    /*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
                ZS_SendBinaryHeader(ZNAK, Txhdr);
                continue;
        }
    }
}

/*--------------------------------------------------------------------------*/
/* ZS SEND FILE DATA                                                        */
/* Send the data in the file                                                */
/*--------------------------------------------------------------------------*/
int ZS_SendFileData(DWORD end, int blk)
{
    int c, e;
//  int newcnt,
    int blklen, maxblklen;

    if (Rxbuflen && (blk > Rxbuflen)) maxblklen = Rxbuflen;
    else maxblklen = blk;
    blklen = maxblklen;

SomeMore:
    if (Z_TestOneByte())
    {
WaitAck:
        switch (c = ZS_SyncWithReceiver())
        {
            default:    /*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
                return ZZERROR;
            case ZSKIP: /*-----------------------------------------*/
                        /* Skip this file                          */
                        /*-----------------------------------------*/
                return c;
            case ZACK:  /*-----------------------------------------*/
                        /*                                         */
                        /*-----------------------------------------*/
                break;
            case ZRPOS: /*-----------------------------------------*/
                        /* Resume at this position                 */
                        /*-----------------------------------------*/
                break;
            case ZRINIT:/*-----------------------------------------*/
                        /* Receive init                            */
                        /*-----------------------------------------*/
                return ZZOK;
        }
    }

    while (Z_TestOneByte())
    {
    	c=(int)Z_GetByte(1000);
    	if (c<0) break;
        switch (c)
        {
            case CAN:
            case ZZRCDO:
            case ZPAD:  goto WaitAck;
        }
    }

    Z_PutLongIntoHeader(Txpos);
    ZS_SendBinaryHeader(ZDATA, Txhdr);

    do
    {
        c = blklen;
        if ((Txpos + c) > end) c = end - Txpos;
        if (c < blklen) e = ZCRCE;
//      else if (Rxbuflen && ((newcnt -= c) <= 0)) e = ZCRCW;
        else e = ZCRCG;

        ZS_SendData(Txpos, Txpos + c, e);
        Txpos += c;

        if (e == ZCRCW) goto WaitAck;

        while (Z_TestOneByte())
        {
            switch (Z_GetByte(1000))
            {
                case CAN:
                case ZZRCDO:
                case ZPAD:  /*--------------------------------------*/
                            /* Interruption detected;               */
                            /* stop sending and process complaint   */
                            /*--------------------------------------*/
                    ZS_SendData(Txpos, Txpos, ZCRCE);
                    goto WaitAck;
            }
        }
    } while (e == ZCRCG);

    while(1)
    {
        Z_PutLongIntoHeader(Txpos);
        ZS_SendBinaryHeader(ZEOF, Txhdr);

        switch (ZS_SyncWithReceiver())
        {
            case ZACK:      /*-----------------------------------------*/
                            /*                                         */
                            /*-----------------------------------------*/
                continue;
            case ZRPOS:     /*-----------------------------------------*/
                            /* Resume at this position...              */
                            /*-----------------------------------------*/
                goto SomeMore;
            case ZRINIT:    /*-----------------------------------------*/
                            /* Receive init                            */
                            /*-----------------------------------------*/
                return ZZOK;
            case ZSKIP:     /*-----------------------------------------*/
                            /* Request to skip the current file        */
                            /*-----------------------------------------*/
                return c;
            default:        /*-----------------------------------------*/
                            /*                                         */
                            /*-----------------------------------------*/
                return ZZERROR;
        }
    }
}

/*--------------------------------------------------------------------------*/
/* ZS SEND FILE                                                             */
/* Send ZFILE frame and begin sending ZDATA frame                           */
/*--------------------------------------------------------------------------*/
//int ZS_SendFile(BYTE *name, int numK)
int ZS_SendFile(BYTE *name, int len)
{   int c;

    for (;;)
    {
        Txhdr[ZF0] = LZCONV;    /* Default file conversion mode */
        Txhdr[ZF1] = LZMANAG;   /* Default file management mode */
        Txhdr[ZF2] = LZTRANS;   /* Default file transport mode */
        Txhdr[ZF3] = 0;
        ZS_SendBinaryHeader(ZFILE, Txhdr);
        ZS_SendFileName(name, len);

Again:
        switch (c = Z_GetHeader(Rxhdr))
        {
            case ZRINIT:    /*-----------------------------------------*/
                            /*                                         */
                            /*-----------------------------------------*/
                goto Again;

            case ZCAN:
            case ZCRC:
            case ZZRCDO:
            case ZZTIMEOUT:
            case ZFIN:
            case ZABORT:
                            /*-----------------------------------------*/
                            /*                                         */
                            /*-----------------------------------------*/
                return ZZERROR;

            case ZSKIP:     /*-----------------------------------------*/
                            /* Other system wants to skip this file    */
                            /*-----------------------------------------*/
                 return c;

            case ZRPOS:     /*-----------------------------------------*/
                            /* Resend from this position...            */
                            /*-----------------------------------------*/
                Txpos = Rxpos;
//                return ZS_SendFileData(numK * 1024L, 1024);
                return ZS_SendFileData(len,1024);
        }
    }
}

/*--------------------------------------------------------------------------*/
/* ZS END SEND                                                              */
/* Say BIBI to the receiver, try to do it cleanly                           */
/*--------------------------------------------------------------------------*/
void ZS_EndSend()
{
    while(1)
    {
        Z_PutLongIntoHeader(0L);
        ZS_SendBinaryHeader(ZFIN, Txhdr);

        switch (Z_GetHeader(Rxhdr))
        {
            case ZFIN:      /*-----------------------------------------*/
                            /*                                         */
                            /*-----------------------------------------*/
                SENDBYTE('O');
                SENDBYTE('O');
            case ZCAN:
            case ZZRCDO:
            case ZZTIMEOUT: /*-----------------------------------------*/
                            /*                                         */
                            /*-----------------------------------------*/
                 return;
        }
    }
}

/*--------------------------------------------------------------------------*/
/* SEND ZMODEM (send a file)                                                */
/*   returns TRUE (1) for good xfer, FALSE (0) for bad                      */
/*   sends one file per call; 'fsent' flags start and end of batch          */
/*--------------------------------------------------------------------------*/
//DWORD send_Zmodem(BYTE *name, int numK)
DWORD send_Zmodem(BYTE *name, BYTE* addr, int len)
{
	DataPtr=addr;
    Z_ClearByte();

    Z_PutLongIntoHeader(0L);
    Z_SendHexHeader(ZRQINIT, Txhdr);

    if (ZS_GetReceiverInfo() == ZZERROR) return 0;

    /*--------------------------------------------------------------------*/
    /* Check the results                                                  */
    /*--------------------------------------------------------------------*/
//    if (ZS_SendFile(name, numK) == ZZERROR) { ZS_EndSend(); return 0; }
//    else                                    { ZS_EndSend(); return numK; }

    if (ZS_SendFile(name, len) == ZZERROR) { ZS_EndSend(); return 0; }
    else                                    { ZS_EndSend(); return len; }

}


