/*
** $Id: systext.c 8013 2007-10-30 05:11:41Z xwyan $
**
** systext.c: GetSysText function.
**
** Copyright (C) 2003 ~ 2007 Feynman Software.
** Copyright (C) 1999 ~ 2002 Wei Yongming.
**
** All rights reserved by Feynman Software.
**
**
** Current maintainer: Wei Yongming.
**
** Create date: 2000/12/31
*/

#include <string.h>
#include <platform/yl_sys.h>

#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

/*
 * This function translates system strings.
 * You can use gettext to return the text.
 *
 * System text as follows:
 *
 const char* SysText [] =
 {
    "Windows",                  // 0
    "Start",                    // 1
    "Refresh Background",       // 2
    "Close All Windows",        // 3
    "End Session",              // 4
    "Operations",               // 5
    "Minimize",                 // 6
    "Maximize",                 // 7
    "Restore",                  // 8
    "Close",                    // 9
    "OK",                       // 10
    "Next",                     // 11
    "Cancel",                   // 12
    "Previous",                 // 13
    "Yes",                      // 14
    "No",                       // 15
    "Abort",                    // 16
    "Retry",                    // 17
    "Ignore",                   // 18
    "About MiniGUI...",         // 19
    "Open File",                // 20
    "Save File",                // 21
    "Color Selection",          // 22
    "Switch Layer",             // 23
    "Delete Layer",             // 24
    "Error",                     // 25
    "LOGO",                      // 26
    "Current Path",              // 27
    "File",                      // 28
#if (!defined (__NOUNIX__) || defined (WIN32)) && defined (_EXT_CTRL_LISTVIEW)
    "Location",                 //0 + 29
    "Up",                       //1 + 29
    "Name",                     //2 + 29
    "Size",                     //3 + 29
    "Access Mode",              //4 + 29
    "Last Modify Time",         //5 + 29
    "Open",                     //6 + 29
    "File Name",                //7 + 29
    "File Type",                //8 + 29
    "Show Hide File",           //9 + 29
    "Sorry! not find %s ", //10 + 29
    "Can't Read %s !",        //11 + 29
    "Can't Write  %s !",      //12 + 29
    "Information",              //13 + 29
    "R",                        //14 + 29
    "W",                        //15 + 29
    "WR",                       //16 + 29
    "Save",                     //17 + 29
    "File %s exists, Replace or not?", //18 + 29
#endif
    NULL
};
*/

const char** local_SysText;

const char* SysText [] =
{
    "Windows",                  // 0
    "Start",                    // 1
    "Refresh Background",       // 2
    "Close All Windows",        // 3
    "End Session",              // 4
    "Operations",               // 5
    "Minimize",                 // 6
    "Maximize",                 // 7
    "Restore",                  // 8
    "Close",                    // 9
    "OK",                       // 10
    "Next",                     // 11
    "Cancel",                   // 12
    "Previous",                 // 13
    "Yes",                      // 14
    "No",                       // 15
    "Abort",                    // 16
    "Retry",                    // 17
    "Ignore",                   // 18
    "About MiniGUI...",         // 19
    "Open File",                // 20
    "Save File",                // 21
    "Color Selection",          // 22
    "Switch Layer",             // 23
    "Delete Layer",             // 24
    "Error",                     // 25
    "LOGO",                      // 26
    "Current Path",              // 27
    "File",                      // 28
#if (!defined (__NOUNIX__) || defined (WIN32)) && defined (_EXT_CTRL_LISTVIEW)
    "Location",                 //0 + 29
    "Up",                       //1 + 29
    "Name",                     //2 + 29
    "Size",                     //3 + 29
    "Access Mode",              //4 + 29
    "Last Modify Time",         //5 + 29
    "Open",                     //6 + 29
    "File Name",                //7 + 29
    "File Type",                //8 + 29
    "Show Hide File",           //9 + 29
    "Sorry! not find %s ", //10 + 29
    "Can't Read %s !",        //11 + 29
    "Can't Write  %s !",      //12 + 29
    "Information",              //13 + 29
    "R",                        //14 + 29
    "W",                        //15 + 29
    "WR",                       //16 + 29
    "Save",                     //17 + 29
    "File %s exists, Replace or not?", //18 + 29
#endif
    NULL
};

#if defined(_GB_SUPPORT) | defined (_GBK_SUPPORT) | defined (_GB18030_SUPPORT)
static const char* SysText_GB [] =
{
    "敦諳",              // 0
    "羲宎",              // 1
    "芃陔掖劓",             // 2
    "壽敕垀衄敦諳",         // 3
    "賦旰頗趕",             // 4
    "敦諳紱釬",             // 5
    "郔苤趙",               // 6
    "郔湮趙",               // 7
    "閥葩",                 // 8
    "壽敕",                 // 9
    "隅",                 // 10
    "狟珨祭",               // 11
    "秏",                 // 12
    "奻珨祭",               // 13
    "岆(Y)",                // 14
    "瘁(N)",                // 15
    "笝砦(A)",              // 16
    "笭彸(R)",              // 17
    "綺謹(I)",              // 18
    "壽衾 MiniGUI...",      // 19
    "湖羲恅璃",             // 20
    "悵湔恅璃",             // 21
    "晇伎恁寁",             // 22
    "遙脯",               // 23
    "刉壺脯",               // 24
    "渣昫",                 // 25
    "芞梓",                 // 26
    "絞繚噤",             // 27
    "恅璃",                 // 28
#if (!defined (__NOUNIX__) || defined (WIN32)) && defined (_EXT_CTRL_LISTVIEW)
    "脤梑弇衾",                                 //0 + 29
    "奻珨撰",                                   //1 + 29
    "靡備",                                     //2 + 29
    "湮苤",                                     //3 + 29
    "溼恀癹",                                 //4 + 29
    "奻棒党蜊奀潔",                             //5 + 29
    "湖羲",                                     //6 + 29
    "恅 璃 靡",                                 //7 + 29
    "恅璃濬倰",                                 //8 + 29
    "珆尨笐紲恅璃",                             //9 + 29
    "勤祥ㄛ帤梑善硌隅腔醴翹ㄩ %s ",     //10 + 29
    "祥夔黍 %s !",                          //11 + 29
    "勤 %s 羶衄迡癹!",                      //12 + 29
    "枑尨陓洘",                                 //13 + 29
    "黍",                                       //14 + 29
    "迡",                                       //15 + 29
    "黍迡",                                     //16 + 29
    "悵湔",                                     //17 + 29
    "恅璃 %s 湔婓ㄛ岆瘁杸遙?"                   //18 + 29
#endif
};
#endif

#ifdef _BIG5_SUPPORT
static const char* SysText_BIG5 [] =
{
    "窗口",
    "幵始",
    "刷新背景",
    "關閉所有窗口",
    "結束會話",
    "窗口操作",
    "最小化",
    "最大化",
    "恢复",
    "關閉",
    "确定",
    "下一步",
    "取消",
    "上一步",
    "是(Y)",
    "否(N)",
    "終止(A)",
    "重試(R)",
    "忽略(I)",
    "關于 MiniGUI...",
    "打開文件",
    "保存文件",
	"顏色選擇",
    "切換層",
    "刪除層",
    "錯誤",                                 // 25
    "圖標",                                 // 26
    "當前路徑",                             // 27
    "文件",                                 // 28
#if (!defined (__NOUNIX__) || defined (WIN32)) && defined (_EXT_CTRL_LISTVIEW)
    "查找位于",                             //0 + 29
    "上一級",                               //1 + 29
    "名稱",                                 //2 + 29
    "大小",                                 //3 + 29
    "訪問權限",                             //4 + 29
    "上次修改時間",                         //5 + 29
    "打開",                                 //6 + 29
    "文 件 名",                             //7 + 29
    "文件類型",                             //8 + 29
    "顯示隱藏文件",                         //9 + 29
    "對不起，未找到指定的目錄： %s ", //10 + 29
    "不能讀取 %s !",                      //11 + 29
    "對 %s 沒有寫權限",                   //23 + 29
    "提示信息",                             //13 + 29
    "讀",                                   //14 + 29
    "寫",                                   //15 + 29
    "讀寫",                                 //16 + 29
    "保存",                                 //17 + 29
    "文件 %s 存在，是否替換?",              //18 + 29
#endif
    NULL
};
#endif

void __mg_init_local_sys_text (void)
{
    const char* charset = GetSysCharset (TRUE);

    local_SysText = SysText;

    if (charset == NULL)
        charset = GetSysCharset (FALSE);

#ifdef _GB_SUPPORT
    if (strcmp (charset, FONT_CHARSET_GB2312_0) == 0) {
        local_SysText = SysText_GB;
	}
#endif

#ifdef _GBK_SUPPORT
    if (strcmp (charset, FONT_CHARSET_GBK) == 0) {
        local_SysText = SysText_GB;
	}
#endif

#ifdef _GB18030_SUPPORT
    if (strcmp (charset, FONT_CHARSET_GB18030_0) == 0) {
        local_SysText = SysText_GB;
	}
#endif

#ifdef _BIG5_SUPPORT
    if (strcmp (charset, FONT_CHARSET_BIG5) == 0) {
        local_SysText = SysText_BIG5;
	}
#endif

}

const char* GUIAPI GetSysText (unsigned int id)
{
    if (id > IDS_MGST_MAXNUM)
        return NULL;

    return local_SysText [id];
}

#ifdef _UNICODE_SUPPORT
static const char* SysText_GB_UTF8 [] =
{
    "蝒",              // 0
    "撘��",              // 1
    "�瑟�",             // 2
    "�喲���蝒",         // 3
    "蝏�隡�",             // 4
    "蝒��",             // 5
    "�����",
    "��之��",
    "�Ｗ�",                 // 8
    "�喲",                 // 9
    "蝖桀�",                 // 10
    "銝�甇�",
    "��",                 // 12
    "銝�甇�",
    "��Y)",                // 14
    "��N)",                // 15
    "蝏迫(A)",              // 16
    "��(R)",              // 17
    "敹賜(I)",              // 18
    "�喃� MiniGUI...",      // 19
    "���辣",             // 20
    "靽��辣",             // 21
    "憸�",             // 22
    "�撅�",
    "�撅�",
    "�秤",                 // 25
    "�暹�",                 // 26
    "敶�頝臬�",             // 27
    "�辣",                 // 28
#if (!defined (__NOUNIX__) || defined (WIN32)) && defined (_EXT_CTRL_LISTVIEW)
    "�交雿�",                                 //0 + 29
    "銝�蝥�",
    "�妍",                                     //2 + 29
    "憭批�",                                     //3 + 29
    "霈輸��",                                 //4 + 29
    "銝活靽格�園",                             //5 + 29
    "��",                                     //6 + 29
    "��隞���",                                 //7 + 29
    "�辣蝐餃�",                                 //8 + 29
    "�曄內���辣",                             //9 + 29
    "撖嫣�韏瘀��芣�唳�摰��桀�嚗� %s ",
    "銝霂餃� %s !",                          //11 + 29
    "撖�%s 瘝⊥�����",                      //12 + 29
    "�內靽⊥",                                 //13 + 29
    "霂�",
    "��",
    "霂餃�",                                     //16 + 29
    "靽�",                                     //17 + 29
    "�辣 %s 摮嚗�行��"                   //18 + 29
#endif
};

static const char* SysText_BIG5_UTF8 [] =
{
	"蝒",              // 0
    "��",              // 1
    "�瑟�",             // 2
    "�����蝒",         // 3
    "蝯��店",             // 4
    "蝒��",             // 5
    "�����",               // 6
    "��之��",               // 7
    "�Ｗ儔",                 // 8
    "��",                 // 9
    "蝣箏�",                 // 10
    "銝�甇�",               // 11
    "��",                 // 12
    "銝�甇�",               // 13
    "��Y)",                // 14
    "��N)",                // 15
    "蝯迫(A)",              // 16
    "�岫(R)",              // 17
    "敹賜(I)",              // 18
    "�� MiniGUI...",      // 19
    "���辣",             // 20
    "靽��辣",             // 21
    "憿�豢�",             // 22
    "��撅�",               // 23
    "�芷撅�",               // 24
    "�航炊",                 // 25
    "��",                 // 26
    "�嗅�頝臬�",             // 27
    "�辣",                 // 28
#if (!defined (__NOUNIX__) || defined (WIN32)) && defined (_EXT_CTRL_LISTVIEW)
    "�交雿�",                                 //0 + 29
    "銝�蝝�",                                   //1 + 29
    "�迂",                                     //2 + 29
    "憭批�",                                     //3 + 29
    "閮芸�甈�",                                 //4 + 29
    "銝活靽格��",                             //5 + 29
    "��",                                     //6 + 29
    "��隞���",                                 //7 + 29
    "�辣憿�",                                 //8 + 29
    "憿舐內�梯��辣",                             //9 + 29
    "撠�韏瘀��芣�唳�摰��桅�嚗�s ",     //10 + 29
    "銝霈�� %s !",                          //11 + 29
    "撠�%s 瘝�撖急���",                      //12 + 29
    "�內靽⊥",                                 //13 + 29
    "霈�",                                       //14 + 29
    "撖�",                                       //15 + 29
    "霈�神",                                     //16 + 29
    "靽�",                                     //17 + 29
    "�辣 %s 摮嚗�行��",                  //18 + 29
#endif
    NULL
};

const char** GUIAPI GetSysTextInUTF8 (const char* language)
{
    if (strncasecmp (language, "zh_CN", 5) == 0) {
        return SysText_GB_UTF8;
	}
    else if (strncasecmp (language, "zh_TW", 5) == 0) {
        return SysText_BIG5_UTF8;
	}

    return SysText;
}
#endif /* _UNICODE_SUPPORT */

