#include <platform/yl_Sys.h>

#define CMD(x) void x(int argc,char** argv);

CMD(uconTitle)
CMD(uconHelp)
CMD(uconCD)
CMD(uconMD)
CMD(uconPwd)
CMD(uconRD)
CMD(ucondir)
CMD(uconopen)
CMD(ucontype)
CMD(uconheapmem)
CMD(uconfwrite)
CMD(uconfread)
CMD(uconcopy)
CMD(uconren)
CMD(ucondel)
CMD(uconCheck)
CMD(uconcopy)	// flexable file name copy

CMD(uconMemTest)
CMD(uconMemDump)
CMD(uconMemWrite)
CMD(uconXLoad)
CMD(uconXSend)
CMD(uconZLoad)
CMD(uconZSend)
CMD(uconCopyMem)
CMD(uconFillMem)
CMD(uconEnter)
CMD(nfctest)
CMD(uconI2C)
CMD(ucon_sdtest)
//CMD(ucon_movtest)
CMD(uconAudioDec)
//CMD(uconImageResize)
//CMD(uconJPEG)
CMD(uconGDI)
CMD(uconlcd)
//CMD(uconcrd)
CMD(lwip)
CMD(ucon_test_pc)
CMD(ucon_dhrystone)
CMD(uconCPUUsage)

#undef CMD
const UCON_ARG_CMD uconArgCmds[] = {

//.....TEST FUNC.....................................................
	{ 0,				"-----",		"[ HELP FUNCTIONS ]"	},
	{ uconTitle, 		"title",		"Show UCON title."		},
	{ uconHelp,  		"help",			"List commands."		},
//...................................................................

	{ 0,				"-----",		"[ FILE SYSTEM FUNCTIONS ]"	},
	{ uconMD,			"md",			"Make Directory"},
	{ uconCD,			"cd",			"Change Directory"},
	{ uconRD,			"rd",	        "remove Directory"},
	{ uconPwd,			"pwd",			"print current path"},
	{ ucondir,		    "dir",	        "Directory"},
	{ ucontype,			"type",			"display content of file"},
	{ ucontype,			"cat",			"."},
	{ uconfwrite,	    "fwrite",	    "write test"},
	{ uconfread,	    "fread",	    "read test"},
	{ uconren,			"ren",			"rename a file"},
	{ ucondel,			"del",			"delete a file"},
	{ ucondel,			"rm",			"."},
	{ uconcopy,			"copy",			"copy a file"},
	{ uconcopy,			"cp",			"."},
	{ uconCheck,		"check",		"check or repair file system errors"},
//.....END...........................................................

	{ 0,				"-----",		"[ TOOL FUNCTIONS ]"	},
	{ uconheapmem,		"heap",			"display size of free heap memory"},
	{ uconMemTest,		"memtest",		"Memory Test."			},
	{ uconMemDump,		"dump",			"Dumps Memory"			},
	{ uconMemDump,		"d",			"."			},
	{ uconMemDump,		"mem",			"."			},
	{ uconMemWrite,		"wmem",			"Writes a word to memeory"},
	{ uconMemWrite,		"w",			"."},
	{ uconXLoad,		"xload",		"XModem-1K receive"},
	{ uconXSend,		"xsend",		"XModem-1K tranmit"},
    { uconZLoad,		"zload",		"ZModem receive"},
    { uconZSend,		"zsend",		"ZModem tranmit"},
	{ uconCopyMem,		"mcopy",		"COPY memory"},
	{ uconFillMem,		"fill",			"FILL memory"},
	{ uconEnter,		"enter",		"Enter values to memory"},
	{ uconEnter,		"e",			"."},
	{ ucon_sdtest,		"sd",			""},
	{ uconI2C,			"i2c",			""},
//	{ uconImageResize,	"rs",			"image resize test"},
//	{ uconJPEG,			"jp",			"jpeg decoder test"},
	{ uconGDI,			"gdi",			"GDI test"},
	{ uconlcd,			"lcd",			"LCDC VD/HD adjustment"},
//	{ uconcrd,			"crd",			"CRD"},
//	{ ucon_movtest,		"mov",			"quicktime movie funcs test"},
	// ww 2014-0102 commented { uconAudioDec is causing this project unable to link, don't know why yet.
//	{ uconAudioDec,		"adec",			"aac audio file decode"},
	// ww 2014-0102 commented }
    { lwip,				"lwip",			"LWIP"},
	{ ucon_test_pc,		"pc",			"P/C"},
    { ucon_dhrystone,	"dry",			"Dhrystone"},
	{ uconCPUUsage,		"cpu",			"CPU usage"},

//.....END...........................................................

	{ 0, 0, 0 }
};


