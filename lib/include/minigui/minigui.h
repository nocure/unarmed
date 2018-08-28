/**
 * \file minigui.h
 * \author Wei Yongming <ymwei@minigui.org>
 * \date 2002/01/06
 *
 * This file includes global and miscellaneous interfaces of MiniGUI.
 *
 \verbatim

    Copyright (C) 2002-2007 Feynman Software.
    Copyright (C) 1998-2002 Wei Yongming.

    All rights reserved by Feynman Software.

    This file is part of MiniGUI, a compact cross-platform Graphics
    User Interface (GUI) support system for real-time embedded systems.

 \endverbatim
 */

/*
 * $Id: minigui.h 7820 2007-10-11 07:29:00Z xwyan $
 *
 *             MiniGUI for Linux/uClinux, eCos, uC/OS-II, VxWorks,
 *                     pSOS, ThreadX, NuCleus, OSE, and Win32.
 *
 *             Copyright (C) 2002-2007 Feynman Software.
 *             Copyright (C) 1998-2002 Wei Yongming.
 */

#ifndef _MGUI_MINIGUI_H
  #define _MGUI_MINIGUI_H

/* #include <stdio.h> */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

    /**
     * \addtogroup global_vars Global variables
     * @{
     */

    /**
     * \defgroup rect_vars Global Rectangles
     * @{
     */

/**
 * \var RECT g_rcScr
 * \brief Contains the rectangle of the whole screen.
 */
extern MG_EXPORT RECT g_rcScr;

/**
 * \def g_rcDesktop
 * \brief Contains the rectangle of desktop of the application.
 *
 * \a g_rcDesktop is defined as an alias (macro) of \a g_rcScr.
 *
 * \sa g_rcScr
 */
#define g_rcDesktop     g_rcScr

    /** @} end of rect_vars */

    /**
     * \defgroup lite_vars MiniGUI-Processes specific variables
     * @{
     */

/**
* \fn int GUIAPI InitGUI (int, const char **)
* \brief Initialize MiniGUI.
*
* The meaning of two parameters is same with parameters of main function.
*
**/
MG_EXPORT int GUIAPI InitGUI (int, const char **);

/**
* \fn void GUIAPI TerminateGUI (int rcByGUI)
* \brief Terminate MiniGUI.
*
* \param rcByGUI The flags whether close desktop thread or not.
*
**/
MG_EXPORT void GUIAPI TerminateGUI (int rcByGUI);

/**
* \fn void GUIAPI MiniGUIPanic (int exitcode)
* \brief The panic of MiniGUI application.
*
* The function forces to close GAL and IAL engine.
*
* \param exitcode The value of exitcode, now it can be any values.
*
**/
MG_EXPORT void GUIAPI MiniGUIPanic (int exitcode);

    /**
     * \addtogroup fns Functions
     * @{
     */

    /**
     * \addtogroup global_fns Global/general functions
     * @{
     */

    /**
     * \defgroup init_fns Initialization and termination functions
     *
     * Normally, the only entry of any MiniGUI application is \a MiniGUIMain.
     * The application will terminate when you call \a exit(3) or just
     * return from \a MiniGUIMain.
     *
     * Example 1:
     *
     * \include miniguimain.c
     *
     * Example 2:
     *
     * \include hello_world.c
     *
     * @{
     */

/**
 * \fn BOOL GUIAPI ReinitDesktopEx (BOOL init_sys_text)
 * \brief Re-initializes the desktop.
 *
 * When you changed the charset or the background picture of the desktop,
 * you should call this function to re-initialize the local system text
 * (when \a init_sys_text is TRUE), the background picture, and the desktop
 * menu.
 *
 * \param init_sys_text Indicates whether to initialize the local system text.
 *
 * \return TRUE on success, otherwise FALSE.
 *
 * \sa ReinitDesktop
 */
MG_EXPORT BOOL GUIAPI ReinitDesktopEx (BOOL init_sys_text);

/**
 * \def ReinitDesktop()
 * \brief Re-initializes the desktop including the local system text.
 *
 * \return TRUE on success, otherwise FALSE.
 *
 * \note This function defined as a macro calling \a ReinitDesktopEx with
 * \a init_sys_text set to TRUE.
 *
 * \sa ReinitDesktopEx
 */
#define ReinitDesktop()    ReinitDesktopEx (TRUE)

/**
 * \fn void GUIAPI ExitGUISafely (int exitcode)
 * \brief Exits your MiniGUI application safely.
 *
 * Calling this function will terminate your MiniGUI application. This
 * function will restore console attributes and call \a exit() function and
 * pass \a exitcode to it.
 *
 * \param exitcode The exit status will be passed to exit(3) function.
 *
 * \return This function will not return.
 *
 * \sa exit(3)
 */
MG_EXPORT void GUIAPI ExitGUISafely (int exitcode);

#ifdef _USE_MINIGUIENTRY
  #define main_entry minigui_entry
  int minigui_entry (int args, const char* arg[]);
#else
  #define main_entry main
#endif

/**
 * \def MiniGUIMain
 * \brief The main entry of a MiniGUI application.
 *
 * This function should be defined by your application. Before Version 1.6.1,
 * MiniGUI defines \a main() function in libminigui library for your
 * application, and call \a MiniGUIMain() in this \a main() function.
 * The \a main() defined by MiniGUI is responsible of initializing and
 * terminating MiniGUI.
 *
 * After version 1.6.1, MiniGUI defines MiniGUIMain as a macro.
 *
 * \param args The number of arguments passed to \a main() by operating system.
 * \param argv The arguments passed to \a main() by operating system.
 *
 * \return The exit status will be retured to the parent process.
 */
#define MiniGUIMain \
MiniGUIAppMain (int args, const char* argv[]); \
int main_entry (int args, const char* argv[]) \
{ \
    int iRet = 0; \
    if (InitGUI (args, argv) != 0) { \
        return 1; \
    } \
    iRet = MiniGUIAppMain (args, argv); \
    TerminateGUI (iRet); \
    return iRet; \
} \
int MiniGUIAppMain

/**
 * \def IDM_DTI_FIRST
 * \brief The minimum interger value of command ID when user customize
 * desktop menu.
**/

#define IDM_DTI_FIRST   (300)

/**
* \fn typedef void GUIAPI (*CustomizeDesktopMenuFunc) (HMENU hDesktopMenu, \
* int iPos)
* \brief The function pointer of customizing desktop menu function.
*
* When the user clicks right mouse button on desktop,
* MiniGUI will display a menu for user. You can use this
* function to customize the desktop menu. e.g. add a new
* menu item.
* Please use an integer larger than IDM_DTI_FIRST as the
* command ID.
*
* \param hDesktopMenu The default menu handle.
* \param iPos The insertion position.
**/
MG_EXPORT typedef void GUIAPI (*CustomizeDesktopMenuFunc) (HMENU hDesktopMenu,
                int iPos);

/**
* \fn typedef int GUIAPI (*CustomDesktopCommandFunc) (int id)
* \brief The function pointer of customizing desktop command function.
*
* When user choose a custom menu item on desktop menu,
* MiniGUI will call this function, and pass the command ID
* of selected menu item.
*
* \param id The command ID of menu item.
*
* \return Default zero value.
**/
MG_EXPORT typedef int GUIAPI (*CustomDesktopCommandFunc) (int id);

/**
* \fn CustomizeDesktopMenuFunc CustomizeDesktopMenu
* \brief MiniGUI default desktop menu function.
* \sa CustomizeDesktopMenuFunc
**/
extern MG_EXPORT CustomizeDesktopMenuFunc CustomizeDesktopMenu;

/**
* \fn CustomizeDesktopCommandFunc CustomDesktopCommand
* \brief MiniGUI default desktop command function.
* \sa CustomizeDesktopCommandFunc
**/
extern MG_EXPORT CustomDesktopCommandFunc CustomDesktopCommand;

    /** @} end of init_fns */

    /**
     * \defgroup mouse_calibrate Mouse calibration.
     * @{
     */

#ifdef _MISC_MOUSECALIBRATE

/**
 * \fn BOOL SetMouseCalibrationParameters (const POINT* src_pts,\
                const POINT* dst_pts)
 * \brief Sets the parameters for doing mouse calibration.
 *
 * This function set the parameters for doing mouse calibration.
 * You should pass five source points and five destination points.
 *
 * Normally, the points should be the upper-left, upper-right, lower-right,
 * lower-left, and center points on the touch panel. The source point is
 * the coordinates before calibrating, and the destination point is the
 * desired coordinates after calibrating.
 *
 * This function will try to evaluate a matrix to calibrate. If the points
 * are okay, MiniGUI will do the calibration after getting a point from the
 * underlay IAL engine.
 *
 * \param src_pts The pointer to an array of five source points.
 * \param dst_pts The pointer to an array of five destination points.
 *
 * \return TRUE for success, FALSE for bad arguments.
 *
 * \note This function is available when _MISC_MOUSECALIBRATE
 *       (option: --enable-mousecalibrate) defined.
 */
MG_EXPORT BOOL GUIAPI SetMouseCalibrationParameters (const POINT* src_pts,
                const POINT* dst_pts);

/**
 * \fn void GUIAPI GetOriginalMousePosition (int* x, int* y)
 * \brief Gets the original mouse position.
 *
 * \param x The pointer used to return the x coordinate of original mouse position.
 * \param y The pointer used to return the y coordinate of original mouse position.
 *
 * \return none.
 *
 * \note This function is available when _MISC_MOUSECALIBRATE
 *       (option: --enable-mousecalibrate) defined.
 */
MG_EXPORT void GUIAPI GetOriginalMousePosition (int* x, int* y);

#endif /* _MISC_MOUSECALIBRATE */

    /** @} end of mouse_calibrate */

    /**
     * \defgroup about_dlg About MiniGUI dialog
     * @{
     */
#ifdef _MISC_ABOUTDLG
MG_EXPORT  void GUIAPI OpenAboutDialog (void);
#endif /* _MISC_ABOUTDLG */

    /** @} end of about_dlg */
/**
 * \defgroup etc_fns Configuration file operations
 *
 * The configuration file used by MiniGUI have a similiar format as
 * M$ Windows INI file, i.e. the file consists of sections, and
 * the section consists of key-value pairs, like this:
 *
 * \code
 * [system]
 * # GAL engine
 * gal_engine=fbcon
 *
 * # IAL engine
 * ial_engine=console
 *
 * mdev=/dev/mouse
 * mtype=PS2
 *
 * [fbcon]
 * defaultmode=1024x768-16bpp
 *
 * [qvfb]
 * defaultmode=640x480-16bpp
 * display=0
 * \endcode
 *
 * Assume that the configuration file named \a my.cfg, if you want get
 * the value of \a mdev in \a system section, you can call
 * \a GetValueFromEtcFile in the following way:
 *
 * \code
 * char buffer [51];
 *
 * GetValueFromEtcFile ("my.cfg", "system", "mdev", buffer, 51);
 * \endcode
 *
 * Example:
 *
 * \include cfgfile.c
 *
 * @{
 */

/**
* \def ETC_MAXLINE
* \brief The max line number of etc file.
**/
#define ETC_MAXLINE             1024

/**
* \def ETC_FILENOTFOUND
* \brief No found etc file.
**/
#define ETC_FILENOTFOUND        -1
/**
* \def ETC_SECTIONNOTFOUND
* \brief No found section in etc file.
**/
#define ETC_SECTIONNOTFOUND     -2
/**
* \def ETC_KEYNOTFOUND
* \brief No found key in etc file.
**/
#define ETC_KEYNOTFOUND         -3
/**
* \def ETC_TMPFILEFAILED
* \brief Create tmpfile failed.
**/
#define ETC_TMPFILEFAILED       -4
/**
* \def ETC_FILEIOFAILED
* \brief IO operation failed to etc file.
**/
#define ETC_FILEIOFAILED        -5
/**
* \def ETC_INTCONV
* \brief  Convert the value string to an integer failed.
**/
#define ETC_INTCONV             -6
/**
* \def ETC_INVALIDOBJ
* \brief Invalid object to etc file.
**/
#define ETC_INVALIDOBJ          -7
/**
* \def ETC_READONLYOBJ
* \brief Read only to etc file.
**/
#define ETC_READONLYOBJ         -8
/**
* \def ETC_OK
* \brief Operate success to etc file.
**/
#define ETC_OK                  0

/** Etc The current config section information */
typedef struct _ETCSECTION
{
/** Allocated number of keys */
int key_nr_alloc;
/** Key number in the section */
int key_nr;
/** Name of the section */
char *name;
/** Array of keys */
char** keys;
/** Array of values */
char** values;
} ETCSECTION;
/** Data type of pointer to a ETCSECTION */
typedef ETCSECTION* PETCSECTION;

/** ETC_S The current config file information*/
typedef struct _ETC_S
{
/** Allocated number of sections */
int sect_nr_alloc;
/** Number of sections */
int section_nr;
/** Pointer to section arrays */
PETCSECTION sections;
} ETC_S;

/**
 * \fn GUIAPI GetValueFromEtc (GHANDLE hEtc, const char* pSection,\
                const char* pKey, char* pValue, int iLen)
 * \brief Gets value from a configuration etc object.
 *
 * This function gets value from an etc object, similar to GetValueFromEtcFile.
 * This function gets the value of the key \a pKey in the section \a pSection
 * of the etc object \a hEtc, and saves the value to the buffer pointed to
 * by \a pValue.
 *
 * \param hEtc Handle to the etc object.
 * \param pSection The section name in which the value located.
 * \param pKey The key name of the value.
 * \param pValue The value will be saved in this buffer.
 * \param iLen The length in bytes of the buffer. This function will set value
 *        if the iLen is less than 1.
 *
 * \return ETC_OK on success, < 0 on error.
 *
 * \retval ETC_OK               Gets value successfullly.
 * \retval ETC_INVALIDOBJ       Invalid etc object.
 * \retval ETC_SECTIONNOTFOUND  Can not find the specified section in the
 *                              configuration file.
 * \retval ETC_KEYNOTFOUND      Can not find the specified key in the section.
 * \retval ETC_READONLYOBJ      The etc object is read-only.
 *
 * \sa GetValueFromEtcFile, LoadEtcFile, UnloadEtcFile
 */
MG_EXPORT int GUIAPI GetValueFromEtc (GHANDLE hEtc, const char* pSection,
                const char* pKey, char* pValue, int iLen);

/**
 * \fn int GUIAPI GetIntValueFromEtc (GHANDLE hEtc, const char* pSection,\
                const char* pKey, int* pValue)
 * \brief Gets the integer value from a configuration etc object.
 *
 * \sa GetValueFromEtc, GetIntValueFromEtcFile
 */
MG_EXPORT int GUIAPI GetIntValueFromEtc (GHANDLE hEtc, const char* pSection,
                const char* pKey, int* pValue);

/**
 * \def SetValueToEtc(GHANDLE hEtc, const char* pSection,\
                const char* pKey, char* pValue)
 * \brief Sets the value in the etc object.
 *
 * This fuctions sets the value in the etc object, somewhat similiar
 * to \sa SetValueToEtcFile.
 *
 * \sa SetValueToEtcFile, GetValueFromEtc
 */
#define SetValueToEtc(hEtc, pSection, pKey, pValue) \
        GetValueFromEtc(hEtc, pSection, pKey, pValue, -1)

/**
 * \fn GHANDLE GUIAPI FindSectionInEtc (GHANDLE hEtc,\
                const char* pSection, BOOL bCreateNew)
 * \brief Finds/Creates a section from an etc object.
 *
 * This function look for a section named \a pSection from the etc object
 * \a hEtc. If there is no such section in the etc object and \a bCreateNew
 * is TRUE, the function will create an empty section.
 *
 * \param hEtc Handle to the etc object.
 * \param pSection The name of the section.
 * \param bCreateNew Indicate whether to create a new section.
 *
 * \return The handle to the section, 0 if not found or creatation failed.
 *
 * \sa GetValueFromEtcSec, GetIntValueFromEtcSec, SetValueInEtcSec
 */
MG_EXPORT GHANDLE GUIAPI FindSectionInEtc (GHANDLE hEtc,
                const char* pSection, BOOL bCreateNew);

/**
 * \fn int GUIAPI GetValueFromEtcSec (GHANDLE hSect,\
                const char* pKey, char* pValue, int iLen)
 * \brief Gets value from an etc section object.
 *
 * This function gets value from an etc section object, similar
 * to GetValueFromEtc. It gets the value of the key \a pKey in the
 * section \a hSect, and saves the value to the buffer pointed to
 * by \a pValue.
 *
 * \param hSect The handle to the section.
 * \param pKey The key name of the value.
 * \param pValue The value will be saved in this buffer.
 * \param iLen The length in bytes of the buffer. This function will set value
 *        if the iLen is less than 1.
 *
 * \return ETC_OK on success, < 0 on error.
 *
 * \retval ETC_OK               Gets value successfullly.
 * \retval ETC_INVALIDOBJ       Invalid etc object.
 * \retval ETC_KEYNOTFOUND      Can not find the specified key in the section.
 * \retval ETC_READONLYOBJ      The section object is read-only.
 *
 * \sa GetValueFromEtcFile, GetValueFromEtc, FindSectionInEtc
 */
MG_EXPORT int GUIAPI GetValueFromEtcSec (GHANDLE hSect,
                const char* pKey, char* pValue, int iLen);

/**
 * \fn int GUIAPI GetIntValueFromEtcSec (GHANDLE hSect,\
                const char* pKey, int* pValue)
 * \brief Gets an integer value from an etc section object.
 *
 * This function gets an integer value from an etc section object,
 * similar to GetIntValueFromEtc. It gets the value of the key \a pKey
 * in the section \a hSect, and saves the value to the buffer pointed to
 * by \a pValue.
 *
 * \param hSect The handle to the section.
 * \param pKey The key name of the value.
 * \param pValue The value will be saved in this buffer.
 *
 * \return ETC_OK on success, < 0 on error.
 *
 * \retval ETC_OK               Gets value successfullly.
 * \retval ETC_INVALIDOBJ       Invalid etc object.
 * \retval ETC_KEYNOTFOUND      Can not find the specified key in the section.
 * \retval ETC_INTCONV          Can not convert the value string to an integer.
 *
 * \sa GetValueFromEtcFile, GetValueFromEtc, FindSectionInEtc
 */
MG_EXPORT int GUIAPI GetIntValueFromEtcSec (GHANDLE hSect,
                const char* pKey, int* pValue);

/**
 * \fn int GUIAPI SetValueToEtcSec (GHANDLE hSect,\
                const char* pKey, char* pValue)
 * \brief Sets the value in the etc section object.
 *
 * This fuctions sets the value in the etc section object \a hSect,
 * somewhat similiar to SetValueToEtc \sa SetValueToEtc.
 *
 * \sa GetValueFromEtc, FindSectionInEtc
 */
MG_EXPORT int GUIAPI SetValueToEtcSec (GHANDLE hSect,
                const char* pKey, char* pValue);

/**
 * \fn int GUIAPI RemoveSectionInEtc (GHANDLE hEtc, const char* pSection)
 * \brief Removes a section in etc object.
 *
 * This function removes a section named \a pSection from the etc object
 * \a hEtc.
 *
 * \param hEtc The handle to the etc object.
 * \param pSection The name of the pSection;
 *
 * \return ETC_OK on success, < 0 on error.
 *
 * \retval ETC_OK               Gets value successfullly.
 * \retval ETC_INVALIDOBJ       Invalid etc object.
 * \retval ETC_READONLYOBJ      The etc object is read-only.
 * \retval ETC_SECTIONNOTFOUND  Can not find the specified section in the
 *                              etc object.
 *
 * \sa RemoveSectionInEtcFile
 */
MG_EXPORT int GUIAPI RemoveSectionInEtc (GHANDLE hEtc, const char* pSection);

/* global MiniGUI etc file object */
extern MG_EXPORT GHANDLE hMgEtc;

/**
 *  \fn static inline int GetMgEtcValue(const char* pSection,\
                        const char *pKey, char *pValue, int iLen)
 * \brief Gets value from MiniGUI configuration etc object
 *
 * This fuctions gets the value from MiniGUi configuration etc object,
 * somewhat similiar to GetValueFromEtcFile and GetValueFromEtc
 * \sa GetValueFromEtcFile \sa GetValueFromEtc.
 */
static inline int GetMgEtcValue(const char* pSection,
                const char *pKey, char *pValue, int iLen)
{

    return GetValueFromEtc (hMgEtc, pSection, pKey, pValue, iLen);
}

/**
 * \fn static inline int GetMgEtcIntValue (const char *pSection,\
                       const char* pKey, int *value)
 * \brief Gets integer value from MiniGUI configuration etc object
 *
 * This fuctions get integer value from MiniGUI configuration etc object
 * some what similiar to GetIntValueFromEtcFile and GetIntValueFromEtc
 * \sa GetIntValueFromEtcFile \sa GetIntValueFromEtc
 */
static inline int GetMgEtcIntValue (const char *pSection,
                const char* pKey, int *value)
{

    return GetIntValueFromEtc (hMgEtc, pSection, pKey, value);
}

    /** @} end of etc_fns */


#ifdef _CLIPBOARD_SUPPORT

    /**
     * \addtogroup clipboard_fns ClipBoard Operations
     * @{
     */

/**
* \def LEN_CLIPBOARD_NAME
* \brief The maximum length of the name of clipboard.
**/
#define LEN_CLIPBOARD_NAME      15
/**
* \def NR_CLIPBOARDS
* \brief The maximum number of clipboard.
**/
#define NR_CLIPBOARDS           4

/**
* \def CBNAME_TEXT
* \brief The default clipboard name of text control.
**/
#define CBNAME_TEXT             ("text")

/**
* \def CBERR_OK
* \brief Operate clipboard success.
* \sa CreateClipBoard
**/
#define CBERR_OK        0
/**
* \def CBERR_BADNAME
* \brief Bad name to clipboard.
* \sa CreateClipBoard
**/
#define CBERR_BADNAME   1
/**
* \def CBERR_NOMEM
* \brief No enough memory to clipboard.
* \sa CreateClipBoard
**/
#define CBERR_NOMEM     2

/**
* \def CBOP_NORMAL
* \brief Overwrite operation to clipboard.
**/
#define CBOP_NORMAL     0
/**
* \def CBOP_APPEND
* \brief Append the new data to clipboarda after the old data.
**/
#define CBOP_APPEND     1

/**
 * \fn int GUIAPI CreateClipBoard (const char* cb_name, size_t size)
 * \brief Creates a new clipboard.
 *
 * This function creates a new clipboard with the name \a cb_name.
 * MiniGUI itself creates a clipboard for text copying/pasting
 * called CBNAME_TEXT.
 *
 * \param cb_name The name of the new clipboard.
 * \param size The size of the clipboard.
 *
 * \retval CBERR_OK         The clipboard created.
 * \retval CBERR_BADNAME    Duplicated clipboard name.
 * \retval CBERR_NOMEM      No enough memory.
 *
 * \sa DestroyClipBoard
 */
MG_EXPORT int GUIAPI CreateClipBoard (const char* cb_name, size_t size);

/**
 * \fn int GUIAPI DestroyClipBoard (const char* cb_name)
 * \brief Destroys a new clipboard.
 *
 * This function destroys a clipboard with the name \a cb_name.
 *
 * \param cb_name The name of the clipboard.
 *
 * \retval CBERR_OK         The clipboard has been destroyed.
 * \retval CBERR_BADNAME    Can not find the clipboard with the name.
 *
 * \sa CreateClipBoard
 */
MG_EXPORT int GUIAPI DestroyClipBoard (const char* cb_name);

/**
 * \fn int GUIAPI SetClipBoardData (const char* cb_name,\
                void* data, size_t n, int cbop)
 * \brief Sets the data of a clipboard.
 *
 * This function sets the data into the clipboard named \a cb_name.
 *
 * \param cb_name The name of the clipboard.
 * \param data The pointer to the data.
 * \param n The length of the data.
 * \param cbop Type of clipboard operations, can be one of
 *        the following values:
 *          - CBOP_NORMAL\n
 *            Overwrite the old data.
 *          - CBOP_APPEND\n
 *            Append the new data after the old data.
 *
 * \retval CBERR_OK         Success.
 * \retval CBERR_BADNAME    Bad clipboard name.
 * \retval CBERR_NOMEM      No enough memory.
 *
 * \sa GetClipBoardDataLen, GetClipBoardData
 */
MG_EXPORT int GUIAPI SetClipBoardData (const char* cb_name,
                void* data, size_t n, int cbop);

/**
 * \fn size_t GUIAPI GetClipBoardDataLen (const char* cb_name)
 * \brief Gets the length of the data of a clipboard.
 *
 * This function gets the data length of the clipboard named \a cb_name.
 *
 * \param cb_name The name of the clipboard.
 *
 * \return The size of the data if success, otherwise zero.
 *
 * \sa GetClipBoardData
 */
MG_EXPORT size_t GUIAPI GetClipBoardDataLen (const char* cb_name);

/**
 * \fn size_t GUIAPI GetClipBoardData (const char* cb_name,\
                void* data, size_t n)
 * \brief Gets the data of a clipboard.
 *
 * This function gets the all data from the clipboard named \a cb_name.
 *
 * \param cb_name The name of the clipboard.
 * \param data The pointer to a buffer will save the data.
 * \param n The length of the buffer.
 *
 * \return The size of the data got if success, otherwise zero.
 *
 * \sa GetClipBoardByte
 */
MG_EXPORT size_t GUIAPI GetClipBoardData (const char* cb_name,
                void* data, size_t n);

/**
 * \fn int GUIAPI GetClipBoardByte (const char* cb_name,\
                int index, unsigned char* byte);
 * \brief Gets a byte from a clipboard.
 *
 * This function gets a byte from the clipboard named \a cb_name.
 *
 * \param cb_name The name of the clipboard.
 * \param index The index of the byte.
 * \param byte The buffer saving the returned byte.
 *
 * \retval CBERR_OK         Success.
 * \retval CBERR_BADNAME    Bad clipboard name.
 * \retval CBERR_NOMEM      The index is beyond the data in the clipboard.
 *
 * \sa GetClipBoardData
 */
MG_EXPORT int GUIAPI GetClipBoardByte (const char* cb_name,
                int index, unsigned char* byte);

    /** @} end of clipboard_fns */

#endif /* _CLIPBOARD_SUPPORT */

    /**
     * \addtogroup misc_fns Miscellaneous functions
     * @{
     */

/**
 * \fn void GUIAPI Ping (void)
 * \brief Makes a beep sound.
 * \sa Beep
 */
MG_EXPORT void GUIAPI Ping (void);

/**
 * \def Beep
 * \brief Alias of Ping.
 * \sa Ping
 */
#define Beep Ping

/**
 * \fn void GUIAPI Tone (int frequency_hz, int duration_ms)
 * \brief Makes a tone.
 *
 * This function will return after the tone. Thus, your program
 * will be blocked when the tone is being played.
 *
 * \param frequency_hz The frequency of the tone in hertz.
 * \param duration_ms The duration of the tone in millisecond.
 *
 * \bug When MiniGUI runs on X Window, the tone can not be played correctly.
 *
 * \sa Ping
 */
MG_EXPORT void GUIAPI Tone (int frequency_hz, int duration_ms);

/**
 * \fn void* GUIAPI GetOriginalTermIO (void)
 * \brief Gets \a termios structure of the original terminal before
 *        initializing MiniGUI.
 *
 * \return The pointer to the original \a termios structure.
 */
MG_EXPORT void* GUIAPI GetOriginalTermIO (void);

    /** @} end of misc_fns */

    /**
     * \defgroup fixed_str Length-Fixed string operations
     *
     * MiniGUI maintains a private heap for length-fixed strings, and allocates
     * length-fixed strings from this heap for window caption, menu item text,
     * and so on. You can also use this private heap to allocate length-fixed
     * strings.
     *
     * \include fixstr.c
     *
     * @{
     */

/**
 * \fn char* GUIAPI FixStrAlloc (int len)
 * \brief Allocates a buffer for a length-fixed string.
 *
 * This function allocates a buffer from the length-fixed string heap
 * for a string which is \a len bytes long (does not include the null
 * character of the string).
 *
 * \note You can change the content of the string, but do not change the
 *       length of this string (shorter is valid) via \a strcat function or
 *       other equivalent functions or operations.
 *
 * \param len The length of the string.
 *
 * \return The pointer to the buffer on success, otherwise NULL.
 *
 * \sa FreeFixStr
 */
MG_EXPORT char* GUIAPI FixStrAlloc (int len);

/**
 * \fn char* GUIAPI FixStrDup (const char* str)
 * \brief Duplicates a length-fixed string.
 *
 * This function allocates a buffer from the length-fixed string heap
 * and copy the string \a str to the buffer.
 *
 * \note You can change the content of the string, but do not change the
 *       length of this string (shorter is valid) via \a strcat function or
 *       other equivalent functions or operations.
 *
 * \param str The pointer to the string will be duplicated.
 *
 * \return The pointer to the buffer on success, otherwise NULL.
 *
 * \sa FreeFixStr
 */
MG_EXPORT char* GUIAPI FixStrDup (const char* str);

/**
 * \fn void GUIAPI FreeFixStr (char* str)
 * \brief Frees a length-fixed string.
 *
 * This function frees the buffer used by the length-fixed string \a str.
 *
 * \param str The length-fixed string.
 *
 * \note Do not use \a free to free the length-fixed string.
 *
 * \sa FixStrAlloc
 */
MG_EXPORT void GUIAPI FreeFixStr (char* str);

    /** @} end of fixed_str */

    /**
     * \defgroup cursor_fns Cursor operations
     * @{
     */

#ifndef _CURSOR_SUPPORT
static inline void do_nothing (void) { return; }
#endif

#ifdef _CURSOR_SUPPORT

/**
 * \fn HCURSOR GUIAPI LoadCursorFromFile (const char* filename)
 * \brief Loads a cursor from a M$ Windows cursor file.
 *
 * This function loads a cursor from M$ Windows *.cur file
 * named \a filename and returns the handle to loaded cursor.
 * The returned handle can be used by \a SetCursor to set new mouse cursor.
 *
 * \param filename The path name of the cursor file.
 *
 * \return Handle to the cursor, zero on error.
 *
 * \note MiniGUI does not support 256-color or animation cursor.
 *
 * \sa SetCursor
 */
  HCURSOR GUIAPI LoadCursorFromFile (const char* filename);

/**
 * \fn HCURSOR GUIAPI LoadCursorFromMem (const void* area)
 * \brief Loads a cursor from a memory area.
 *
 * This function loads a cursor from a memory area pointed to by \a area.
 * The memory has the same layout as a M$ Windows CURSOR file.
 * The returned handle can be used by \a SetCursor to set new mouse cursor.
 *
 * \param area The pointer to the cursor data.
 *
 * \return Handle to the cursor, zero on error.
 *
 * \note MiniGUI does not support 256-color or animation cursor.
 *
 * \sa SetCursor
 */
  HCURSOR GUIAPI LoadCursorFromMem (const void* area);

/**
 * \fn HCURSOR GUIAPI CreateCursor (int xhotspot, int yhotspot, int w, int h,\
                const BYTE* pANDBits, const BYTE* pXORBits, int colornum)
 * \brief Creates a cursor from memory data.
 *
 * This function creates a cursor from memory data rather than cursor file.
 * \a xhotspot and \a yhotspot specify the hotpot of the cursor, \a w
 * and \a h are the width and the height of the cursor respectively.
 * \a pANDBits and \a pXORBits are AND bitmask and XOR bitmask of the cursor.
 * MiniGUI currently support mono-color cursor and 16-color cursor, \a colornum
 * specifies the cursor's color depth. For mono-color, it should be 1, and for
 * 16-color cursor, it should be 4.
 *
 * \param xhotspot The x-coordinate of the hotspot.
 * \param yhotspot The y-coordinate of the hotspot.
 * \param w The width of the cursor.
 * \param h The height of the cursor.
 * \param pANDBits The pointer to AND bits of the cursor.
 * \param pXORBits The pointer to XOR bits of the cursor.
 * \param colornum The bit-per-pixel of XOR bits.
 *
 * \return Handle to the cursor, zero on error.
 *
 * \note MiniGUI only support 2-color or 16-color cursor.
 */
  HCURSOR GUIAPI CreateCursor (int xhotspot, int yhotspot, int w, int h,
               const BYTE* pANDBits, const BYTE* pXORBits, int colornum);

/**
 * \fn BOOL GUIAPI DestroyCursor (HCURSOR hcsr)
 * \brief Destroys a cursor object.
 *
 * This function destroys a cursor object specified by \a hcsr.
 *
 * \param hcsr Handle to the cursor.
 *
 * \return TRUE on success, otherwise FALSE.
 */
  BOOL GUIAPI DestroyCursor (HCURSOR hcsr);

/**
 * \fn HCURSOR GUIAPI GetSystemCursor (int csrid)
 * \brief Gets the handle to a system cursor by its identifier.
 *
 * MiniGUI creates (MAX_SYSCURSORINDEX + 1) system cursors for application.
 * You can use \a GetSystemCursor to get the handle to these system cursors.
 * The identifier can be one of the following:
 *
 *  - IDC_ARROW\n
 *    Normal arrow cursor.
 *  - IDC_IBEAM\n
 *    'I' shaped beam cursor, indicate an input field.
 *  - IDC_PENCIL\n
 *    Pencil-shape cursor.
 *  - IDC_CROSS\n
 *    Cross cursor.
 *  - IDC_MOVE\n
 *    Moving cursor.
 *  - IDC_SIZENWSE\n
 *    Sizing cursor, along north-west and south-east.
 *  - IDC_SIZENESW\n
 *    Sizing cursor, along north-east and south-west.
 *  - IDC_SIZEWE\n
 *    Sizing cursor, along west and east.
 *  - IDC_SIZENS\n
 *    Sizing cursor, along north and south.
 *  - IDC_UPARROW\n
 *    Up arrow cursor.
 *  - IDC_NONE\n
 *    None cursor.
 *  - IDC_HELP\n
 *    Arrow with question.
 *  - IDC_BUSY\n
 *    Busy cursor.
 *  - IDC_WAIT\n
 *    Wait cursor.
 *  - IDC_RARROW\n
 *    Right arrow cursor.
 *  - IDC_COLOMN\n
 *    Cursor indicates column.
 *  - IDC_ROW\n
 *    Cursor indicates row.
 *  - IDC_DRAG\n
 *    Draging cursor.
 *  - IDC_NODROP\n
 *    No droping cursor.
 *  - IDC_HAND_POINT\n
 *    Hand point cursor.
 *  - IDC_HAND_SELECT\n
 *    Hand selection cursor.
 *  - IDC_SPLIT_HORZ\n
 *    Horizontal splitting cursor.
 *  - IDC_SPLIT_VERT\n
 *    Vertical splitting cursor.
 *
 * \param csrid The identifier of the system cursor.
 * \return Handle to the system cursor, otherwise zero.
 */
MG_EXPORT HCURSOR GUIAPI GetSystemCursor (int csrid);

/**
 * \fn HCURSOR GUIAPI GetCurrentCursor (void)
 * \brief Gets the handle to the current cursor.
 *
 * This function retrives the current cursor and returns its handle.
 *
 * \return Handle to the current system cursor, zero means no current cursor.
 */
MG_EXPORT HCURSOR GUIAPI GetCurrentCursor (void);
#else
//  #define LoadCursorFromFile(filename)    (do_nothing(), 0)
//  #define CreateCursor(x, y, w, h, ANDbs, XORbs, cr) (do_nothing(), 0)
//  #define DestroyCursor(hcsr)             (do_nothing(), 0)
//  #define GetSystemCursor(csrid)          (do_nothing(), 0)
//  #define GetCurrentCursor()              (do_nothing(), 0)
#endif /* _CURSOR_SUPPORT */

#define MAX_SYSCURSORINDEX    22

/* System cursor index. */
/** System arrow cursor index. */
#define IDC_ARROW       0
/** System beam cursor index. */
#define IDC_IBEAM       1
/** System pencil cursor index. */
#define IDC_PENCIL      2
/** System cross cursor index. */
#define IDC_CROSS       3
/** System move cursor index. */
#define IDC_MOVE        4
/** System  size northwest to southeast cursor index. */
#define IDC_SIZENWSE    5
/** System size northeast to southwest cursor index. */
#define IDC_SIZENESW    6
/** System west to east cursor index. */
#define IDC_SIZEWE      7
/** System north to south cursor index. */
#define IDC_SIZENS      8
/** System up arrow cursor index. */
#define IDC_UPARROW     9
/** System none cursor index. */
#define IDC_NONE        10
/** System help cursor index. */
#define IDC_HELP        11
/** System busy cursor index. */
#define IDC_BUSY        12
/** System wait cursor index. */
#define IDC_WAIT        13
/** System right arrow cursor index. */
#define IDC_RARROW      14
/** System colomn cursor index. */
#define IDC_COLOMN      15
/** System row cursor index. */
#define IDC_ROW         16
/** System drag cursor index. */
#define IDC_DRAG        17
/** System nodrop cursor index. */
#define IDC_NODROP      18
/** System hand point cursor index. */
#define IDC_HAND_POINT  19
/** System hand select cursor index. */
#define IDC_HAND_SELECT 20
/** System horizontal split cursor index. */
#define IDC_SPLIT_HORZ  21
/** System vertical cursor index. */
#define IDC_SPLIT_VERT  22

/**
 * \fn void GUIAPI ClipCursor (const RECT* prc)
 * \brief Clips the cursor range.
 *
 * This function sets cursor's clipping rectangle. \a prc
 * is the new clipping rectangle in screen coordinates. If \a prc is NULL,
 * \a ClipCursor will disable cursor clipping.
 *
 * \param prc The clipping rectangle.
 * \return None.
 */
MG_EXPORT void GUIAPI ClipCursor (const RECT* prc);

/**
 * \fn void GUIAPI GetClipCursor (RECT* prc)
 * \brief Gets the current cursor clipping rectangle.
 *
 * This function copies the current clipping rectangle to
 * a RECT pointed to by \a prc.
 *
 * \param prc The clipping rectangle will be saved to this rectangle.
 * \return None.
 */
MG_EXPORT void GUIAPI GetClipCursor (RECT* prc);

/**
 * \fn void GUIAPI GetCursorPos (POINT* ppt)
 * \brief Gets position of the current cursor.
 *
 * This function copies the current mouse cursor position to
 * a POINT structure pointed to by \a ppt.
 *
 * \param ppt The position will be saved in this buffer.
 * \return None.
 *
 * \sa SetCursorPos, POINT
 */
MG_EXPORT void GUIAPI GetCursorPos (POINT* ppt);

/**
 * \fn void GUIAPI SetCursorPos (int x, int y)
 * \brief Sets position of the current cursor.
 *
 * This function sets mouse cursor position with the given
 * arguments: \a (\a x,\a y).
 *
 * \param x The x-corrdinate of the expected poistion.
 * \param y The y-corrdinate of the expected poistion.
 * \return None.
 *
 * \sa GetCursorPos
 */
MG_EXPORT void GUIAPI SetCursorPos (int x, int y);

#ifdef _CURSOR_SUPPORT
/**
 * \fn HCURSOR GUIAPI SetCursorEx (HCURSOR hcsr, BOOL set_def)
 * \brief Changes the current cursor.
 *
 * This function changes the current cursor to be \a hcsr,
 * and/or sets it to be the default cursor.
 *
 * If you pass \a set_def as TRUE, the expected cursor will be the default
 * cursor. The default cursor will be used when you move the cursor to
 * the desktop.
 *
 * \param hcsr The expected cursor handle.
 * \param set_def Indicates whether setting the cursor as the default cursor.
 *
 * \return The old cursor handle.
 *
 * \sa SetCursor, SetDefaultCursor, GetDefaultCursor
 */
 MG_EXPORT HCURSOR GUIAPI SetCursorEx (HCURSOR hcsr, BOOL set_def);

/**
 * \def SetCursor(hcsr)
 * \brief Changes the current cursor.
 *
 * This function changes the current cursor to be \a hcsr.
 *
 * \param hcsr The expected cursor handle.
 * \return The old cursor handle.
 *
 * \note This function defined as a macro calling \a SetCursorEx with
 *       passing \a set_def as FALSE.
 *
 * \sa SetCursorEx, SetDefaultCursor
 */
  #define SetCursor(hcsr) SetCursorEx (hcsr, FALSE)

/**
 * \def SetDefaultCursor(hcsr)
 * \brief Changes the current cursor, and set it as the default cursor.
 *
 * This function changes the current cursor to be \a hcsr, and set it as
 * the default cursor.
 *
 * \param hcsr The expected cursor handle.
 * \return The old cursor handle.
 *
 * \note This function defined as a macro calling \a SetCursorEx with
 * passing \a set_def as TRUE.
 *
 * \sa SetCursorEx, SetCursor
 */
  #define SetDefaultCursor(hcsr) SetCursorEx (hcsr, TRUE)

/**
 * \fn HCURSOR GUIAPI GetDefaultCursor (void)
 * \brief Gets the default cursor.
 *
 * This function gets the current default cursor.
 *
 * \return The current default cursor handle.
 *
 * \sa SetCursorEx, SetDefaultCursor
 */
MG_EXPORT  HCURSOR GUIAPI GetDefaultCursor (void);

#else
  #define SetCursorEx(hcsr, set_def)    (do_nothing(), 0)
  #define SetCursor(hcsr)               (do_nothing(), 0)
  #define SetDefaultCursor(hcsr)        (do_nothing(), 0)
  #define GetDefaultCursor()            (do_nothing(), 0)
#endif /* _CURSOR_SUPPORT */

#ifdef _CURSOR_SUPPORT

/**
 * \fn int GUIAPI ShowCursor (BOOL fShow)
 * \brief Shows or hides cursor.
 *
 * This function shows or hides cursor according to the argument \a fShow.
 * Show cursor when \a fShow is TRUE, and hide cursor when \a fShow is FALSE.
 * MiniGUI maintains a showing count value. Calling \a ShowCursor once, the
 * count will increase when \a fShow is TRUE, or decrease one when FALSE.
 * When the count is less than 0, the cursor will disapear actually.
 *
 * \param fShow Indicates show or hide the cursor.
 *
 * \return Cursor showing count value.
 */
MG_EXPORT int GUIAPI ShowCursor (BOOL fShow);
#else
  #define ShowCursor(fShow)             (do_nothing(), 0)
#endif /* _CURSOR_SUPPORT */

    /** @} end of cursor_fns */

    /**
     * \defgroup key_status Asynchronous key status functions
     * @{
     */

/**
 * \fn BOOL GUIAPI GetKeyStatus (UINT uKey)
 * \brief Gets a key or a mouse button status.
 *
 * This function gets a key or a mouse button status, returns TRUE
 * when pressed, or FALSE when released. \a uKey indicates
 * the key or mouse button. For keys on keyboard, \a uKey should be
 * the scancode of the key, for mouse button, \a uKey should be one
 * value of the following:
 *
 *  - SCANCODE_LEFTBUTTON\n
 *    Left mouse button.
 *  - SCANCODE_MIDDLBUTTON\n
 *    Middle mouse button.
 *  - SCANCODE_RIGHTBUTTON\n
 *    Right mouse button.
 *
 * These constants and the scancodes of keys are defined in <minigui/common.h>.
 *
 * \param uKey Indicates the key or mouse button.
 *
 * \return Returns TRUE when pressed, or FALSE when released.
 *
 * \sa GetShiftKeyStatus
 */
MG_EXPORT BOOL GUIAPI GetKeyStatus (UINT uKey);

/**
 * \fn DWORD GUIAPI GetShiftKeyStatus (void)
 * \brief Gets status of the shift keys.
 *
 * This function gets ths status of the shift keys, the returned value
 * indicates the status of shift keys -- CapsLock, ScrollLock, NumLock,
 * Left Shift, Right Shift, Left Ctrl, Right Ctrl, Left Alt, and Right Alt.
 * You can use KS_* ORed with the status value to determine one shift key's
 * status:
 *
 *  - KS_CAPSLOCK\n
 *    Indicates that CapsLock is locked.
 *  - KS_NUMLOCK\n
 *    Indicates that NumLock is locked.
 *  - KS_SCROLLLOCK\n
 *    Indicates that ScrollLock is locked.
 *  - KS_LEFTCTRL\n
 *    Indicates that left Ctrl key is pressed.
 *  - KS_RIGHTCTRL\n
 *    Indicates that right Ctrl key is pressed.
 *  - KS_CTRL\n
 *    Indicates that either left or right Ctrl key is pressed.
 *  - KS_LEFTALT\n
 *    Indicates that left Alt key is pressed.
 *  - KS_RIGHTALT\n
 *    Indicates that right Alt key is pressed.
 *  - KS_ALT\n
 *    Indicates that either left or right Alt key is pressed.
 *  - KS_LEFTSHIFT\n
 *    Indicates that left Shift key is pressed.
 *  - KS_RIGHTSHIFT\n
 *    Indicates that right Shift key is pressed.
 *  - KS_SHIFT\n
 *    Indicates that either left or right Shift key is pressed.
 *
 * These constants are defined in <minigui/common.h>.
 *
 * \return The status of the shift keys.
 * \sa key_defs
 */
MG_EXPORT DWORD GUIAPI GetShiftKeyStatus (void);

    /** @} end of key_status */

    /**
     * \defgroup sys_text Internationlization of system text
     * @{
     */

/**
 * \def IDS_MGST_WINDOW
 * \brief The identifier of the window system text.
 */
#define IDS_MGST_WINDOW          0

/**
 * \def IDS_MGST_START
 * \brief The identifier of the start system text.
 */
#define IDS_MGST_START           1

/**
 * \def IDS_MGST_REFRESH
 * \brief The identifier of the refresh background system text.
 */
#define IDS_MGST_REFRESH         2

/**
 * \def IDS_MGST_CLOSEALLWIN
 * \brief The identifier of the close all windows system text.
 */
#define IDS_MGST_CLOSEALLWIN     3

/**
 * \def IDS_MGST_ENDSESSION
 * \brief The identifier of the end session system text.
 */
#define IDS_MGST_ENDSESSION      4

/**
 * \def IDS_MGST_OPERATIONS
 * \brief The identifier of the operations system text.
 */
#define IDS_MGST_OPERATIONS      5

/**
 * \def IDS_MGST_MINIMIZE
 * \brief The identifier of the minimize system text.
 */
#define IDS_MGST_MINIMIZE        6

/**
 * \def IDS_MGST_MAXIMIZE
 * \brief The identifier of the maximize system text.
 */
#define IDS_MGST_MAXIMIZE        7

/**
 * \def IDS_MGST_RESTORE
 * \brief The identifier of the restore system text.
 */
#define IDS_MGST_RESTORE         8

/**
 * \def IDS_MGST_CLOSE
 * \brief The identifier of the close system text.
 */
#define IDS_MGST_CLOSE           9

/**
 * \def IDS_MGST_OK
 * \brief The identifier of the ok system text.
 */
#define IDS_MGST_OK              10

/**
 * \def IDS_MGST_NEXT
 * \brief The identifier of the next system text.
 */
#define IDS_MGST_NEXT            11

/**
 * \def IDS_MGST_CANCEL
 * \brief The identifier of the cancel system text.
 */
#define IDS_MGST_CANCEL          12

/**
 * \def IDS_MGST_PREV
 * \brief The identifier of the previous system text.
 */
#define IDS_MGST_PREV            13

/**
 * \def IDS_MGST_YES
 * \brief The identifier of the yes system text.
 */
#define IDS_MGST_YES             14

/**
 * \def IDS_MGST_NO
 * \brief The identifier of the no system text.
 */
#define IDS_MGST_NO              15

/**
 * \def IDS_MGST_ABORT
 * \brief The identifier of the abort system text.
 */
#define IDS_MGST_ABORT           16

/**
 * \def IDS_MGST_RETRY
 * \brief The identifier of the retry system text.
 */
#define IDS_MGST_RETRY           17

/**
 * \def IDS_MGST_IGNORE
 * \brief The identifier of the ignore system text.
 */
#define IDS_MGST_IGNORE          18

/**
 * \def IDS_MGST_ABOUTMG
 * \brief The identifier of the about minigui system text.
 */
#define IDS_MGST_ABOUTMG         19

/**
 * \def IDS_MGST_OPENFILE
 * \brief The identifier of the open file system text.
 */
#define IDS_MGST_OPENFILE        20

/**
 * \def IDS_MGST_SAVEFILE
 * \brief The identifier of the save file system text.
 */
#define IDS_MGST_SAVEFILE        21

/**
 * \def IDS_MGST_COLORSEL
 * \brief The identifier of the color selection system text.
 */
#define IDS_MGST_COLORSEL        22

/**
 * \def IDS_MGST_SWITCHLAYER
 * \brief The identifier of the switch layer system text.
 */
#define IDS_MGST_SWITCHLAYER     23

/**
 * \def IDS_MGST_DELLAYER
 * \brief The identifier of the delete layer system text.
 */
#define IDS_MGST_DELLAYER        24

/**
 * \def IDS_MGST_ERROR
 * \brief The identifier of the error system text.
 */
#define IDS_MGST_ERROR           25

/**
 * \def IDS_MGST_LOGO
 * \brief The identifier of the logo system text.
 */
#define IDS_MGST_LOGO            26

/**
 * \def IDS_MGST_CURRPATH
 * \brief The identifier of the current path system text.
 */
#define IDS_MGST_CURRPATH        27

/**
 * \def IDS_MGST_FILE
 * \brief The identifier of the file system text.
 */
#define IDS_MGST_FILE            28

/**
 * \def IDS_MGST_LOCATION
 * \brief The identifier of the location system text.
 */
#define IDS_MGST_LOCATION        29

/**
 * \def IDS_MGST_UP
 * \brief The identifier of the up system text.
 */
#define IDS_MGST_UP              30

/**
 * \def IDS_MGST_NAME
 * \brief The identifier of the name system text.
 */
#define IDS_MGST_NAME            31

/**
 * \def IDS_MGST_SIZE
 * \brief The identifier of the size system text.
 */
#define IDS_MGST_SIZE            32

/**
 * \def IDS_MGST_ACCESSMODE
 * \brief The identifier of the access mode system text.
 */
#define IDS_MGST_ACCESSMODE      33

/**
 * \def IDS_MGST_LASTMODTIME
 * \brief The identifier of the last modify time system text.
 */
#define IDS_MGST_LASTMODTIME     34

/**
 * \def IDS_MGST_OPEN
 * \brief The identifier of the open system text.
 */
#define IDS_MGST_OPEN            35

/**
 * \def IDS_MGST_FILENAME
 * \brief The identifier of the file name system text.
 */
#define IDS_MGST_FILENAME        36

/**
 * \def IDS_MGST_FILETYPE
 * \brief The identifier of the file type system text.
 */
#define IDS_MGST_FILETYPE        37

/**
 * \def IDS_MGST_SHOWHIDEFILE
 * \brief The identifier of the show hide file system text.
 */
#define IDS_MGST_SHOWHIDEFILE    38

/**
 * \def IDS_MGST_NOTFOUND
 * \brief The identifier of the not found file system text.
 */
#define IDS_MGST_NOTFOUND        39

/**
 * \def IDS_MGST_NR
 * \brief The identifier of the can't read system text.
 */
#define IDS_MGST_NR              40

/**
 * \def IDS_MGST_NW
 * \brief The identifier of the can't write system text.
 */
#define IDS_MGST_NW              41

/**
 * \def IDS_MGST_INFO
 * \brief The identifier of the information system text.
 */
#define IDS_MGST_INFO            42

/**
 * \def IDS_MGST_R
 * \brief The identifier of the read system text.
 */
#define IDS_MGST_R               43

/**
 * \def IDS_MGST_W
 * \brief The identifier of the write system text.
 */
#define IDS_MGST_W               44

/**
 * \def IDS_MGST_WR
 * \brief The identifier of the read and write system text.
 */
#define IDS_MGST_WR              45

/**
 * \def IDS_MGST_SAVE
 * \brief The identifier of the save system text.
 */
#define IDS_MGST_SAVE            46

/**
 * \def IDS_MGST_FILEEXIST
 * \brief The identifier of the file exist system text.
 */
#define IDS_MGST_FILEEXIST       47

#define IDS_MGST_MAXNUM          47

/**
 * \fn const char* GetSysText (unsigned int id);
 * \brief Translates system text to localized text.
 *
 * When MiniGUI display some system messages, it will call \a GetSysText
 * function to translate system text from English to other language.
 * Global variable \a SysText contains all text used by MiniGUI in English.
 *
 * \a GetSysText function returns localized text from \a local_SysText.
 * MiniGUI have already defined localized sytem text for en_US, zh_CN.GB2312
 * and zh_TW.Big5 locales. MiniGUI initializes \a local_SysText to
 * point one of above localized system text when startup.  You can also
 * let \a local_SysText point to your customized string array.
 *
 * \param id The system text id.
 * \return The localized text.
 *
 * \sa SysText, local_SysText
 */
MG_EXPORT const char* GetSysText (unsigned int id);

/**
 * \var const char* SysText []
 * \brief Contains all text used by MiniGUI in English.
 *
 * System text defined as follows in MiniGUI:
 *
 * \code
 * const char* SysText [] =
 * {
 *    "Windows...",
 *    "Start...",
 *    "Refresh Background",
 *    "Close All Windows",
 *    "End Session",
 *    "Operations...",
 *    "Minimize",
 *    "Maximize",
 *    "Restore",
 *    "Close",
 *    "OK",
 *    "Next",
 *    "Cancel",
 *    "Previous",
 *    "Yes",
 *    "No",
 *    "Abort",
 *    "Retry",
 *    "Ignore",
 *    "About MiniGUI...",
 *    "Open File",
 *    "Save File",
 *    "Color Selection",
 *    "Switch Layer",
 *    "Delete Layer",
 *    "Error",
 *    "LOGO",
 *    "Current Path",
 *    "File",
 *  #if (!defined (__NOUNIX__) || defined (WIN32)) && defined (_EXT_CTRL_LISTVIEW)

 *    "Location",
 *    "Up",
 *    "Name",
 *    "Size",
 *    "Access Mode",
 *    "Last Modify Time",
 *    "Open",
 *    "File Name",
 *    "File Type",
 *    "Show Hide File",
 *    "Sorry! not find %s ",
 *    "Can't Read %s !",
 *    "Can't Write  %s !",
 *    "Information",
 *    "R",
 *    "W",
 *    "WR",
 *    "Save",
 *    "File %s exists, Replace or not?",
 *  #endif
 *    NULL
 * };
 * \endcode
 *
 * \sa GetSysText, local_SysText
 */
extern MG_EXPORT const char* SysText [];

/**
 * \var const char** local_SysText
 * \brief The pointer to the current localized system text array.
 *
 * Changing \a local_SysText will lead to \a GetSysText returns a different
 * localized system text. Please set it after calling \a SetDesktopRect,
 * and send desktop a MSG_REINITSESSION message (call \a ReinitDesktop
 * function) after assigned a different value to this variable.
 *
 * \sa GetSysText, SysText, ReinitDesktopEx
 */
extern MG_EXPORT const char** local_SysText;

#ifdef _UNICODE_SUPPORT

/**
 * \fn const char** GUIAPI GetSysTextInUTF8 (const char* language)
 * \brief Gets the localized system text array in UTF-8 for
 *        a specified language.
 *
 * This function returns the localized system text array in UTF-8 encode
 * for the specified language. You can use the returned localized system
 * text array to set \a local_SysText if your system logical fonts are using
 * UNICODE/UTF-8 charset.
 *
 * \param language The language name. So far, you can specify the language
 *        as 'zh_CN' (for simlified Chinese)
 *        or 'zh_TW' (for tranditional Chinese).
 *
 * \return The pointer to the localized system text array.
 *
 * \sa SysText, local_SysText
 */
extern MG_EXPORT const char** GUIAPI GetSysTextInUTF8 (const char* language);

#endif /* _UNICODE_SUPPORT */

    /** @} end of sys_text */

    /**
     * \defgroup str_helpers String operation helpers
     * @{
     */

/**
 * \fn char* strnchr (const char* s, size_t n, int c);
 * \brief Locates character in the first \a n characters of string \a s.
 *
 * \param s The pointer to the string.
 * \param n The number of first characters will be searched.
 * \param c The expected character.
 *
 * \return Returns a pointer to the first occurrence of the character \a c
 *         in the string \a s.
 *
 * \sa strchr(3)
 */
MG_EXPORT char* strnchr (const char* s, size_t n, int c);

/**
 * \fn int substrlen (const char* text, int len, int delimiter,\
                int* nr_delim)
 * \brief Locates a substring delimited by one or more delimiters in the
 * first \a len characters of string \a text.
 *
 * \param text The pointer to the string.
 * \param len The number of first characters will be searched.
 * \param delimiter The delimiter which delimites the substring from other.
 * \param nr_delim  The number of continuous delimiters will be returned
 *        through this pointer.
 *
 * \return The length of the substring.
 *
 * \sa strstr(3)
 */
MG_EXPORT int substrlen (const char* text, int len, int delimiter,
                int* nr_delim);

/**
 * \fn char* strtrimall (char* src);
 * \brief Deletes all space characters.
 *
 * This function deletes the blank space, form-feed('\\f'), newline('\\n'),
 * carriage return('\\r'), horizontal tab('\\t'),and vertical tab('\\v')
 * in the head and the tail of the string.
 *
 * \param src The pointer to the string.
 *
 * \return Returns a pointer to the string.
 */
MG_EXPORT char * strtrimall (char* src);

    /** @} end of str_helpers */

    /** @} end of global_fns */

    /** @} end of fns */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _MGUI_MINIGUI_H */

