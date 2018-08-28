/******************************************************************************
 * ucon.c -- Uart Console
 * ============================================================================
 *
 * 2008-0811 ww		created
 * 2008-0814 ww		added ANSI key parsing (up down left right)
 * 2008-0818 ww		added Command Line editing. supports cursor left/right, insert/back-delete keys
 * 					added 5 Command histories by pressing up key.
 * 2008-1002 ww		modified command history mechanism to cyclic buffer that uses pointer index.
 * 					added handling change drive letter.(a: b:...)
 * 2008-1008 ww		modified argument parser. changed parameter delimeter from '/' to '-' to fit path.
 * 2009-0422 ww		* a whole new command history implementation that uses linked list.
 * 					It now checks duplicated for string and newly used string is raised to the top of list.
 * 					* Increased console version to v1.1
 *
 *****************************************************************************/
#include "platform/YL_SYS.h"
#include "platform/TQ_BSP.h"
#include <platform/yl_fsio.h>
#include <string.h>

#define UCON_MAX_ARGS			10
#define UCON_INPUT_BUFFER		64
#define UCON_MAX_INPUT_CHAR		(UCON_INPUT_BUFFER-1)
#define UCON_NUM_CMD_HISTORY	5

#define ANSI_CHAR_RETURN	0x0d
#define ANSI_CHAR_NEWLINE	0x0a
#define ANSI_CHAR_BACK		0x08
#define ANSI_CHAR_BACK1		0x7f
#define ANSI_CHAR_END		0x00

extern const UCON_ARG_CMD uconArgCmds[];

static char *uconArgv[UCON_MAX_ARGS];

static char g_CmdBuf[UCON_INPUT_BUFFER];
static char g_CmdBuf_Args[UCON_INPUT_BUFFER];

typedef struct _CMDHIST_LIST_T{
	char string[UCON_INPUT_BUFFER];
	struct _CMDHIST_LIST_T *next;
} CMDHIST_LIST_T;

CMDHIST_LIST_T *g_CmdHistHead;

static int g_CurPos,g_CurLen,g_CurHist,g_uconEntered;

static int g_ansiptr;
static char g_ansibuf[16];


// -----------------------------------------------
// Show title.
void uconTitle( int argc, char **argv ) {

	printf( "\n\n\n\n\n" );

	printf( "     /-------------------------\\\n" );
	printf( "     |      I am unARMed       |\n" );
	printf( "     |    UART CONSOLE V1.1    |\n" );
	printf( "     \\-------------------------/\n" );
	printf("Built on %s %s\n",__DATE__,__TIME__);
	printf( "\n\n" );

	printf("FIN : %-3dMHz  ",FIN/MEGA);
	printf("CPU : %-3dMHz  ",CPU_FREQ/MEGA);
	printf("FCLK: %-3dMHz\n",FCLK/MEGA);
	printf("HCLK: %-3dMHz  ",HCLK/MEGA);
	printf("PCLK: %-3dMHz  ",PCLK/MEGA);
	printf("UCLK: %-3dMHz\n",UCLK/MEGA);

	printf("\nMALLOC memroy: %d bytes Free.\n\n",memGetStatus(0));

}


// List available commands.
void uconHelp( int argc, char **argv ) {
int i;
int iLines;

	printf( "\nAvailable commands:\n" );

	//.....List commands in arg cmd table.............
	iLines = 0;
	for( i = 0 ; uconArgCmds[ i ].cmd != NULL ; i++ ) {
		//.....Separate groups.............
		if( uconArgCmds[ i ].func == NULL ) {
			iLines++;
			printf( "\n" );
		}

		//.....Show command string....................
		printf( "  %10s", uconArgCmds[ i ].cmd );

		/*.....Show help........................*/
		if( uconArgCmds[ i ].desc != NULL ) {
			printf( " -----> " );
			printf( uconArgCmds[ i ].desc );
		}
		printf( "\n" );
/*
		if( ++iLines > 15 ) {
			printf( "Press any key to continue....\r" );
			iLines=uartGetChar( UART0 );
			if (iLines==27) break; //ESC
			else if (iLines==3) break; //CTRL-C
			iLines = 0;
		}
*/
	}

	printf( "\n" );
}


// -----------------------------------------------

static void uconPushHist(void) {
CMDHIST_LIST_T *pch,*p1,*p2;
	p2=0;
	p1=0;
	pch=g_CmdHistHead;
	while (pch) {
		p2=p1;
		p1=pch;
		pch=pch->next;
	}
	p2->next=0;
	p1->next=g_CmdHistHead;
	g_CmdHistHead=p1;
//	if (p1->string) free(p1->string);
	strcpy(p1->string,g_CmdBuf);
}


static char* uconGetHist(int idx) {
CMDHIST_LIST_T *pch;
	pch=g_CmdHistHead;
	while (pch&&idx) {
		pch=pch->next;
		idx--;
	}
	return pch->string;
}


void uconRaiseHist(void) {
CMDHIST_LIST_T *pch,*p1;
int idx=g_CurHist;
	if (idx<=0) return;
	p1=0;
	pch=g_CmdHistHead;
	while (pch&&idx) {
		p1=pch;
		pch=pch->next;
		idx--;
	}
	if (!pch) return;
	if (p1) {
		p1->next=pch->next;
	}
	else return ;
	pch->next=g_CmdHistHead;
	g_CmdHistHead=pch;

	g_CurHist=0;

	return;
}

int uconIsCmdInHist(void) {
CMDHIST_LIST_T *pch;
int idx;

	pch=g_CmdHistHead;
	idx=0;
	while (pch) {
		if (!stricmp (pch->string,g_CmdBuf)) return idx;
		pch=pch->next;
		idx++;
	}
	return -1;
}


static int uconArgParse( void )
/*-----NOTE---------------------------------------------------------------
 * Parse command line to arg line.
 * If command line is
 *
 *		^^^cmd-name^p1^^-p2^^^/p3^^^^@
 *
 * command line will be modified to
 *
 *		^^^cmd-name@p1@^-p2@^^/p3@^^^@
 *         |        |   |     |
 * offset 03       12  16    22
 * argv    0        1   2     3
 *
 *		@ mean NULL char, ^ mean space char.
 *
 * And argv[ 0 ] = &g_CmdBuf[  3 ],
 *	   argv[ 1 ] = &g_CmdBuf[ 12 ],
 *	   argv[ 2 ] = &g_CmdBuf[ 16 ],
 *	   argv[ 3 ] = &g_CmdBuf[ 22 ].
 *
 * And argc = 4.
 *-----START--------------------------------------------------------------*/
{
int argc;
char* pArgStart;
char* pCmdLine;

	strcpy(g_CmdBuf_Args,g_CmdBuf);
	pCmdLine=g_CmdBuf_Args;

	//.....Clear uconArgv................
	for (argc=0;argc<UCON_MAX_ARGS;argc++) {
		uconArgv[ argc ] = NULL;
	}

	pArgStart=NULL;
	for(argc=0 ; *pCmdLine && argc<UCON_MAX_ARGS ; pCmdLine++ ) {
		if (!isspace(*pCmdLine)) {
			if (!pArgStart)  pArgStart=pCmdLine;
		}
		else if (pArgStart) {
			*pCmdLine=0;
			uconArgv[argc++]=pArgStart;
			pArgStart=NULL;
		}

	}
	if (pArgStart) uconArgv[argc++]=pArgStart;

	if (*pCmdLine) printf("*** Got more then 9 arguments, exceeding arguments are lost !!!\n");

	return argc;		// Return count
}


static void uconCleanBuf() {
	// clear up current line
	while (g_CurLen) {
		putch(ANSI_CHAR_BACK);
		putch(' ');
		putch(ANSI_CHAR_BACK);
		g_CurLen--;
	}
	g_CurPos=0;
}


static int uconInsertChar(int c) {
int t1,t2,cp;

	if (g_CurLen>=UCON_MAX_INPUT_CHAR) return 0;

	g_CurLen++;
	t1=g_CmdBuf[g_CurPos];
	g_CmdBuf[g_CurPos++]=c;
	putch(c);
	for (cp=g_CurPos;cp<g_CurLen;cp++) {
		t2=g_CmdBuf[cp];
		g_CmdBuf[cp]=t1;
		putch(t1);
		t1=t2;
	}
	for (cp=g_CurLen;cp>g_CurPos;cp--) {
		putch(ANSI_CHAR_BACK);
	}
	return 1;
}


static void uconReplaceCurrentLineWithHist(void) {
int c;
char* hist;
	uconCleanBuf();
	hist=uconGetHist(g_CurHist);
	while ((c=hist[g_CurLen])) {
		g_CmdBuf[g_CurLen]=c;
		putch(c);
		g_CurLen++;
	}
	g_CurPos=g_CurLen;
}


static int uconBackDeleteChar(void) {
int cp;
int c;

	if (g_CurPos<=0) return 0;

	g_CurLen--;
	g_CurPos--;
	putch(ANSI_CHAR_BACK);
	for (cp=g_CurPos;cp<g_CurLen;cp++) {
		c=g_CmdBuf[cp+1];
		putch(c);
		g_CmdBuf[cp]=c;
	}
	putch(' ');
	putch(ANSI_CHAR_BACK);
	for (cp=g_CurLen;cp>g_CurPos;cp--) {
		putch(ANSI_CHAR_BACK);
	}
	return 1;
}


static void uconUpKey(void) {
char* hist;
	// send back last cmd
	for (;g_CurPos<g_CurLen;g_CurPos++) { // to line end
		printf("\x1B[C"); // go right 1 char
	}
	g_CurHist++;
	if (g_CurHist<UCON_NUM_CMD_HISTORY) {
		hist=uconGetHist(g_CurHist);
		if ((hist[0])) {
			uconReplaceCurrentLineWithHist();
		}
		else { // history is empty
			g_CurHist--;	// get back to where it was
		}
	}
	else {
		g_CurHist--;	// get back to where it was
	}
}


static void uconDownKey(void) {
	for (;g_CurPos<g_CurLen;g_CurPos++) { // go to line end
		printf("\x1B[C"); // go right 1 char
	}
	if (g_CurHist>0) {
		g_CurHist--;
		uconReplaceCurrentLineWithHist();
	}
	else if (g_CurHist==0) {
		g_CurHist--;
		uconCleanBuf();
	}
}


static void uconRightKey(void) {
	if (g_CurPos<g_CurLen) {
		g_CurPos++;
		printf("\x1B[C"); // go right 1 char
	}
}


static void uconLeftKey(void) {
	if (g_CurPos>0) {
		g_CurPos--;
		printf("\x1B[D"); // go left 1 char
	}
}


static void uconHomeKey(void) {
	for (;g_CurPos>0;g_CurPos--) {
		printf("\x1B[D"); // go left 1 char
	}

}


static void uconEndKey(void) {
	for (;g_CurPos<g_CurLen;g_CurPos++) {
		printf("\x1B[C"); // go right 1 char
	}
}


static void uconDeleteKey(void) {
	if (g_CurPos<g_CurLen) {
		g_CurPos++;
		printf("\x1B[C"); // go right 1 char
		uconBackDeleteChar();
	}
}


static void uconANSIParse(void) {
char c;
int len;
char* str=g_ansibuf;

	len=strlen(str);
	c=str[len-1];
	switch(c) {
		case 'A':	// up key
			uconUpKey();
		break;

		case 'B':	// down key
			uconDownKey();
		break;

		case 'C':	// right key
			uconRightKey();
		break;

		case 'D':	// left
			uconLeftKey();
		break;

		case 'H':	// home
			uconHomeKey();
		break;

		case 'K':	// end
			uconEndKey();
		break;

		case 'G':	// center (5)
		break;

		case '~':	// telnet, when using skyeye simulator with UART on TELNET.
			str[len-1]=0; // make it null terminated
			c=atoi(&str[2]);
			str[len-1]='~';	// put the '~' back;
			switch(c) {
				case 1:	// home
					uconHomeKey();
					break;

				case 4:	// end
					uconEndKey();
					break;

				case 3:	// delete
					uconDeleteKey();
					break;

				case 2:		// insert
				case 5:		// PgUp
				case 6: 	// PgDn
				case 11:	// F1
				case 12:	// F2
				case 13:	// F3
				case 14:	// F4
				case 15:	// F5
				case 17:	// F6
				case 18:	// F7
				case 19:	// F8
				case 20:	// F9
				case 21:	// F10
				case 23:	// F11
				case 24:	// F12
					break;

			}
		break;

	}
	// if not processed, just ignore.
}


void uconInit(void) {
int i;

	g_uconEntered=1;	// force reset command line at the beginning

#ifdef OLD_CMD_HIST
	for (i=0;i<UCON_NUM_CMD_HISTORY;i++) {
		g_CmdHist[i][0]=0;
	}
	g_CmdHistHead=0;
#else
CMDHIST_LIST_T *pch;
	for (i=0;i<UCON_NUM_CMD_HISTORY;i++) {
		pch=(CMDHIST_LIST_T*)malloc(sizeof(CMDHIST_LIST_T));
		*pch->string=0;
		pch->next=g_CmdHistHead;
		g_CmdHistHead=pch;
	}

#endif

}


void uconLineReset(void) {
	g_ansiptr=0;
	g_CurPos=0;
	g_CurLen=0;
	g_CmdBuf[0]=0;
	g_CurHist=-1;
}


int uconExec(void) {
int argc,i;

#if 0
	// handles drive letters ...   and preceding spaces
	i=0;
	if (g_CurLen>=2) {
		while (g_CmdBuf[i]&&isspace(g_CmdBuf[i])) i++; // eat up spaces
		if (g_CmdBuf[i+1]==':') {
			if (isalpha(g_CmdBuf[i])) {
				i=chdir(&g_CmdBuf[i]);
				if (i<0) printf("Invalid path %s\n",g_CmdBuf);
				return 0;
			}
		}
	}
#endif

	argc=uconArgParse();
	if( argc == 0 ) return 0;

	//.....Find out command from uconArgCmds..............
	for( i = 0 ; uconArgCmds[ i ].cmd != NULL ; i++ ) {
		if( uconArgCmds[ i ].func == NULL ) {
			continue;
		}

		if( stricmp( uconArgv[ 0 ], uconArgCmds[ i ].cmd ) == 0 ) {
			//.....Call to handle.....................
			(*uconArgCmds[ i ].func)( argc, uconArgv );
			printf( "\n" );
			return 1;
		}
	}

	if( uconArgCmds[ i ].cmd == NULL )
	{
		//.....NOT FOUND...............
		printf( "BAD COMMAND!\n" );
	}
	return 0;
}


int uconInputCheck(void) {
int c;

	c=tq_uartGetChar();
	if (c<=0) {
		if (c==UART_ERROR_RECV) {
			printf("\n!! UART ERROR !!\n");
		}
		return 0;
	}

	if (g_ansiptr==0) {
		if (c==27) {	// ESC
			g_ansibuf[g_ansiptr++]=c;
		}
		else {
			if (g_ansiptr==0) {
				switch (c) {
					case ANSI_CHAR_RETURN: // return key
						putch(c);		// send back
						putch('\n');	// send a new-line
						g_CmdBuf[g_CurLen] = 0;

						return 1;
					break;

					case 24:	// CTRL-X Cancel
						printf(" \\\n");
						g_CurPos=0;
						g_CurLen=0;
					break;

					case ANSI_CHAR_BACK: // back key
					case ANSI_CHAR_BACK1: // back key
						uconBackDeleteChar();
					break;

					case 4:	// CTRL-D Delete
						uconDeleteKey();
					break;

					case 9:		// CTRL-I up
						uconUpKey();
					break;

					case 11:	// CTRL-K down
						uconDownKey();
					break;

					case 12:	// CTRL-L right key
						uconRightKey();
					break;

					case 10:	// CTRL-J left
						uconLeftKey();
					break;

					case 21:	// CTRL-U home
						uconHomeKey();
					break;

					case 15:	// CTRL-O end
						uconEndKey();
					break;

					default:
						if (c>=32) {
							uconInsertChar(c);
						}
				}
			}
		}
	}
	else if (g_ansiptr==1) {
		if (c=='[') {	// '['
			g_ansibuf[g_ansiptr++]=c;
		}
		else if (c!=27) g_ansiptr=0;	// abort sequence
	}
	else if (g_ansiptr>=2) {
		if ( ((c>='0')&&(c<='9')) || (c==';') ) {	// parameters
			g_ansibuf[g_ansiptr++]=c;
		}
		else {	// end of sequence
			g_ansibuf[g_ansiptr++]=c;
			g_ansibuf[g_ansiptr]=0;
			uconANSIParse();
			g_ansiptr=0;// end of ansi sequence
		}
	}
	return 0;
}


char* uconGetCmdBuf(void) {
	return g_CmdBuf;
}


char* uconAsk(void) {

	uconLineReset();
	while(!uconInputCheck());

	return g_CmdBuf;
}


void ucon_tsk(unsigned long id) {
int ret;

	uconInit();
	uconTitle(0,NULL);

	while (1) {
		if (g_uconEntered) {
#if 0
			getcwd(g_CmdBuf,UCON_MAX_INPUT_CHAR);
			printf(g_CmdBuf);
#endif
			printf(">");
			uconLineReset();
		}
		g_uconEntered=uconInputCheck();
		if (g_uconEntered) {
			ret=uconExec();
			// push keyed-in string into history if ...
			// (1) typed-in string length is > 2  and-->
			// (2) the string is newly typed-in   or...
			// (3) the string is retrived from history but modified.
			// (4) and most importantly.... successfully execuated !!
			if (ret&&g_CurLen>2) {
				g_CurHist=uconIsCmdInHist();
				if (g_CurHist==-1) uconPushHist();
				else uconRaiseHist();
			}
		}
	}
}

