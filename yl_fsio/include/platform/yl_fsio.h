/******************************************************************************
 * YL_FSIO.h -- YL File system I/O routines
 * ============================================================================
 *
 *
 * 2008-xxxx Chris
 *
 *
 *(\~~/)(\~~/)
 *('.' )(^.^ )
 * (_(__ (_(__)~*
 *****************************************************************************/
#ifndef YL_FSIO_H_
#define YL_FSIO_H_

#define MAX_PATH_LENGTH 256		//FX_MAX_LONG_NAME_LEN

#ifndef BUFSIZ
# define BUFSIZ 4096
#endif

typedef void* FILE;
#define __FILE_defined // ww 2018-0604 added, to avoid conflicting types with new gcc/dev

#include <platform/YL_Type.h>

#define	O_RDONLY	0		/* +1 == FREAD */
#define	O_WRONLY	1		/* +1 == FWRITE */
#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	O_APPEND	0x0008
#define	O_CREAT		0x0200
#define	O_TRUNC		0x0400

struct	stat {
  mode_t	st_mode;
  off_t		st_size;
  struct tm	st_atime;
  struct tm	st_mtime;
  struct tm	st_ctime;
};

/* Directory part */
struct dirent {
	unsigned int	d_off;
	unsigned short	d_reclen;
	unsigned char	d_type;
	char			d_name[256];
};

typedef void* DIR;

/* File types to use for d_type */
#define DT_UNKNOWN	 0
#define DT_DIR		 4
#define DT_REG		 8
#define DT_RDONLY	0x01		//FX_READ_ONLY
#define DT_HIDDEN	0x02		//FX_HIDDEN
#define DT_ARCHV	0x20		//FX_ARCHIVE
//#define DT_VOL		0x08		//FX_VOLUME
//#define DT_SYS		0x04		//FX_SYSTEM
//#define DT_DIR		0x10		//FX_DIRECTORY


// make stat -> st_mode equal to attribute in FILEX
#define S_ISDIR(m)	(((m)&DT_DIR) == DT_DIR)
#define S_ISREG(m)	(((m)&DT_REG) == DT_REG)
#define S_ISHID(m)	(((m)&DT_HIDDEN) == DT_HIDDEN)
#define S_ISARC(m)	(((m)&DT_ARCHV) == DT_ARCHV)

#define SEEK_SET	0		//FX_SEEK_BEGIN
#define SEEK_END	1		//FX_SEEK_END
#define SEEK_CUR 	2		//FX_SEEK_FORWARD

extern FILE *_fopen(const char *, const char *);
extern int _fclose(FILE * __f);
extern int _fseek(FILE * __f, off_t __o, int __w);
extern off_t _ftell(FILE * __f);
extern int _stat(const char *, struct stat *);
extern int _truncate(const char *, off_t);
extern int _fputs(const char *, FILE *);
extern int _feof(FILE *fp);
extern int _fputc(int, FILE *);
extern int _fgetc(FILE *);
extern char *_fgets(char *, int, FILE *);
extern size_t _fread(void *, size_t, FILE *);
extern size_t _fwrite(const void *, size_t, FILE *);
extern int _rename(const char *, const char *);
extern int _unlink(const char *);
extern int _fflush(void);
extern FILE* _tmpfile(void);
int _ferror(FILE* stream);

extern DIR *_opendir(const char *);
extern struct dirent *_readdir(DIR *);
extern void _rewinddir(DIR *);
extern off_t _telldir(DIR *);
extern int _chdir(const char *);
extern int _mkdir(const char *);		/* mode permission no longer needed */
extern int _rmdir(const char *);
extern char *_getcwd(char *, unsigned long);
extern int _closedir(DIR *);
/* chris: the following is for unicode pathname manipulation */
extern int chdirW(const ucs2_t *path_name);
extern int mkdirW(const ucs2_t *path_name);
extern int rmdirW(const ucs2_t *path_name);

extern int wildcmp(const char *wild, const char *string);

/* chris: the following is for unicode pathname manipulation */
extern FILE *fopenW(const ucs2_t *file, const char *mode);
extern int unlinkW(const ucs2_t * file_name);
extern int statW(const ucs2_t *path, struct stat *buf);
ucs2_t* getcwdW(ucs2_t* absPath,unsigned int size);


// ==== makepath.c ======================================
void  yl_splitpath(const char* inpath, char *out);
void yl_makepath(char * path, const char * drive, const char *dir, const char * filename, const char * ext);
char* yl_fullpath(char* ppart);
void specialcombine(char* pathbuf);
void beautifypath(char* start);
void pathcat(char* s1, char* s2);

void  yl_splitpathW(const ucs2_t* inpath, ucs2_t *out);
void yl_makepathW(ucs2_t *path, const ucs2_t *drive, const ucs2_t *dir,const ucs2_t *filename, const ucs2_t *ext);
ucs2_t* yl_fullpathW(ucs2_t* ppart);
void specialcombineW(ucs2_t* pathbuf);
void beautifypathW(ucs2_t* start);
void pathcatW(ucs2_t* s1, ucs2_t* s2);


// ==== filefiles.c ==============================================================
#define	FNM_NOMATCH	1	/* Match failed. */
#define	FNM_MATCH	0	/* Match ok. */

#define SP_DRV(b)		(((char**)b)[0])
#define SP_DIR(b)		(((char**)b)[1])
#define SP_NAME(b)		(((char**)b)[2])
#define SP_EXT(b)		(((char**)b)[3])
#define SP_EXTRA_SPACE	0x20
#define SP_MAX_PATH		(MAX_PATH_LENGTH+SP_EXTRA_SPACE)

#define WSP_DRV(b)		(((ucs2_t**)b)[0])
#define WSP_DIR(b)		(((ucs2_t**)b)[1])
#define WSP_NAME(b)		(((ucs2_t**)b)[2])
#define WSP_EXT(b)		(((ucs2_t**)b)[3])
#define WSP_MAX_PATH	((MAX_PATH_LENGTH+SP_EXTRA_SPACE)*2)

#define FF_TIME_AFTER		0x04000000
#define FF_TIME_BEFORE		0x08000000
#define FF_SIZE_BIGGER		0x10000000
#define FF_SIZE_SMALLER		0x20000000
#define FF_ATTR_AND			0x40000000
#define FF_ATTR_OR			0x80000000

#define FA_RDONLY			0x01		//FX_READ_ONLY
#define FA_HIDDEN			0x02		//FX_HIDDEN
#define FA_SYSTEM			0x04		//FX_SYSTEM
#define FA_LABEL			0x08		//FX_VOLUME
#define FA_DIR				0x10		//FX_DIRECTORY
#define FA_ARCH 			0x20		//FX_ARCHIVE

typedef struct _ffblk {
	DWORD	size;				/* if size > 0, compare size */
	struct tm	time;				/* convet to time_t before use it */
	DWORD	attr;
	char	*search_path;
	char	*rule_set;

	void *pfx_path;	// caller must prepare the sturcture
	void *pfx_media;		// media pointer will be valid after calling find_first
	DWORD	entry_offset;
	DWORD	cwd_num_entries;

	DWORD magic;

	int		ff_hasuname;			// the file has unicode name
	ucs2_t* ff_longname;
	char* 	ff_name;
	DWORD	ff_attr;
	DWORD	ff_size;
	struct tm	ff_time;

} ffblk;

ffblk* find_init(void);
int	find_first(ffblk *ff);
int	find_next(ffblk *ff);	/* return NULL if no more */

// ==== charconv.c ==============================================================
int fx_big5_name_get(void* media_ptr, char *src_short_name, BYTE *dest_big5_name, int dest_size);

// ==============================================================================


#define fopen 	_fopen
#define fclose	_fclose
#define fseek	_fseek
#define ftell	_ftell
#define	stat(p,b)	_stat((p),(b))
#define truncate	_truncate
#define fputs	_fputs
#define fputc	_fputc
#define putc(c,f)  _fputc((c),(f))
#define fgetc	_fgetc
#define fgets	_fgets
#define getc(f) _fgetc(f)
#define feof 	_feof
#define	fread	_fread
#define fwrite	_fwrite
#define rename	_rename
#define unlink	_unlink
#define fflush	_fflush
#define tmpfile _tmpfile
#define ferror _ferror

#define opendir	_opendir
#define readdir	_readdir
#define rewinddir 	_rewinddir
#define telldir	_telldir
#define chdir	_chdir
#define mkdir	_mkdir
#define rmdir	_rmdir
#define getcwd	_getcwd
#define closedir	_closedir

#define splitpath			yl_splitpath
#define makepath			yl_makepath
#define fullpath			yl_fullpath

#define splitpathW			yl_splitpathW
#define makepathW			yl_makepathW
#define fullpathW			yl_fullpathW



/**********************************************************************************************//**
 * @defgroup GROUP_SYS_DRVMAN Drive Manager
 * @{
 *************************************************************************************************/

// ==== Drive Manager APIs =========================
/** @brief 磁碟組態, Disk Geometry
 *
 * @sa IOCTRL_DRVIVE_GET_GEO
 */
typedef struct {
	int cylinders;	///< number of tracks (磁柱面)
	int heads;		///< heads (磁頭)
	int sectors;	///< Sectors per track (磁區), 通常為512 bytes
	int sec_size;	///< Sector size in bytes, 磁區大小
	int cluster;	///< sectors per cluster (磁簇; 磁區叢集), 磁碟機之最小的存取單位, 由多個sectors組成
} DRIVE_GEO_T;


/**@brief 裝置驅動程式的副程式宣告
 *
 * @param media_ptr 媒體控制指標
 *
 * @note 此函式型態取決於所使用之 File system。
 *
 * @sa REGISTER_DRIVE_T, yl_dmRegister
 */
typedef void (*DRIVERFUNC)(void *media_ptr);


/**@brief 裝置控制命令處理程式的副程式宣告
 *
 * @param arg0		通常為裝置驅動程式所需之 extra infomation
 * @param arg1		通常為 IO 控制碼
 * @param arg2		通常為控制碼參數
 *
 * @return 該控制碼之傳回值
 *
 * @note 此函式功能取決於裝置驅動程式之內部設計
 *
 * @sa REGISTER_DRIVE_T, yl_dmRegister, yl_dmIOCtrl
 */
typedef int (*IOCTL)(DWORD arg0, DWORD arg1, DWORD arg2);


/**@brief 註冊裝置參數
 *
 * @sa yl_dmRegister
 */
typedef struct {
	char* name;			///< 裝置名稱，可為 null
	DRIVERFUNC driver;	///< 裝置驅動程式
	void* ext;			///< 裝置的額外資訊, 將會呈現在  FX_MEDIA_PTR.fx_media_driver_info, @note 內容取決於所使用之 File system。
	char* addr;			///< 裝置Buffer的起始記憶體位置。 若位置值為0, 則另外使用配置的記憶體。
	int bufsize;		///< 工作緩衝區大小, 以儲存媒體最小儲存單位為主; ex:FAT使用的最小單位為512 bytes
	IOCTL ioctl;		///< 裝置的控制命令處理程式
} REGISTER_DRIVE_T;


/**@brief 格式化參數
 *
 * @sa yl_dmFormat
 */
typedef struct FORMAT_PARAM_S {
	char* vol;			///< 檔案系統標籤(volume label)
	int nFATs;			///< FAT的數量
	int nRootDirEnt;	///< 根目錄之目錄數量
	int nHiddenSec;		///< 隱藏磁區之數量
} FORMAT_PARAM_T;

/**********************************************************************************************//**
 * @defgroup GROUP_SYS_CORRECT_OPT Check & Correcting options
 * @{
 * @sa yl_dmMediaCheckRepair
 *************************************************************************************************/
#define	FAT_CHAIN_ERROR		0x01		//FX_FAT_CHAIN_ERROR		///< FAT 表錯誤
#define DIRECTORY_ERROR		0x02		//FX_DIRECTORY_ERROR		///< 目錄結構損毀
#define LOST_CLUSTER_ERROR	0x04		//FX_LOST_CLUSTER_ERROR	///< 佔用不連續的磁簇遺失

/**<
 * 在磁碟機使用一段時間之後, 可能因為檔案的異動次數增加, 造成了可用磁簇(free cluster)並不會連續;
 * 此時若一個檔案存放在磁碟機上, 可能無法佔用連續的磁簇, 又作業系統在刪除或修改檔案時, 應用程式可能因為
 * 某種緣故或斷電而造成當機, 或是不正常地結束程式. 此時檔案的存取動作若未完成, 可能會造成某些磁簇雖然已經
 * 不使用了, 但是檔案配置表仍標示它為使用狀態, 造成佔用的磁簇遺失. 此種狀況就稱為lost cluster.
 */

#define	FILE_SIZE_ERROR		0x08		//FX_FILE_SIZE_ERROR		///< 檔案大小錯誤
/**<
 * 與所註冊的檔案大小與檔案鏈的大小不同，僅為偵測選項，非修復選項之一。
 */
/**@} GROUP_SYS_CORRECT_OPT *********************************************************************/



/**@brief 向系統註冊儲存裝置
 *
 * @param drvnum 欲註冊之磁碟槽編號(0~3;A~D)
 * @param rdrv 註冊之資訊
 *
 * @return
 * 0: OK
 * - @ref YLERR_ARG
 * - @ref YLERR_DRIVE, 磁碟槽編號不合法
 *
 * @note 將裝置註冊後在系統上還未能使用, 須經過掛載後才能使用裝置
 *
 * @sa yl_dmMount
 */
int yl_dmRegister(int drvnum, REGISTER_DRIVE_T *rdrv);


/**@brief 解安裝(註冊)
 *
 * @param drvnum 欲解除之磁碟槽編號(0~3;A~D)
 *
 * @return no return
 */
int yl_dmUnregister(int drvnum);


/**@brief 得知磁碟槽是否掛載
 *
 * @param drvnum 欲得知是否掛載之磁碟槽編號(0~3;A~D)
 *
 * @return
 * - 0: 無掛載
 * - 1: 有掛載
 */
int yl_dmIsMounted(int drvnum);


/**@brief 將磁碟槽掛載到系統上
 *
 * @param drvnum 欲掛載之磁碟槽編號(0~3;A~D)
 *
 * @return
 * - @ref YLERR_OK
 * - @ref YLERR_FAILURE, 不明錯誤
 * - @ref YLERR_ARG
 * - @ref YLERR_FS
 * - @ref YLERR_FAT
 * - @ref YLERR_DRIVERIO
 * - @ref YLERR_DRIVE ,磁碟槽編號不合法
 */
int yl_dmMount(int drvnum);


/**@brief 將磁碟槽從系統上卸載下來
 *
 * @param drvnum 欲卸載之磁碟槽編號(0~3;A~D)
 *
 * @return
 * - @ref YLERR_OK
 * - @ref YLERR_FAILURE, 不明錯誤
 * - @ref YLERR_DRIVERIO
 * - @ref YLERR_DRIVE ,磁碟槽編號不合法
 */
int yl_dmUnmount(int drvnum);


/**@brief 將磁碟槽格式化
 *
 * @param drvnum 欲格式化之磁碟槽編號(0~3;A~D)
 * @param param 格式化參數, 給0則自動使用預設參數
 * 預設格式化之參數為:
 * - vol : NULL
 * - nFATs : 2
 * - nRootDirEnt : 224
 * - nHiddenSec : 0
 *
 * @return
 * - @ref YLERR_OK
 * - @ref YLERR_FAILURE, 不明錯誤
 * - @ref YLERR_DRIVERIO
 * - @ref YLERR_DRIVE ,磁碟槽編號不合法
 * - @ref YLERR_NOTSUPP, 此磁碟槽不支援格式化
 */
int yl_dmFormat(int drvnum, FORMAT_PARAM_T* param);


/**@brief 對磁碟槽下控制命令及參數
 *
 * @param drvnum 欲控制之磁碟槽編號(0~3;A~D)
 * @param arg1 控制命令
 * @param arg2 控制參數; 呼叫者依照控制命令提供所需要的參數
 *
 * @return
 * - @ref YLERR_DRIVE ,磁碟槽編號不合法
 * - @ref YLERR_NOTSUPP
 * - 其他: 參照各控制命令之傳回值
 *
 * @note arg2為一指標的指標, 呼叫者要提供; 它會回傳另一指標位址到所提供的指標中
 *
 * @sa GROUP_SYS_DRVMAN_IOCTRL
 */
int yl_dmIOCtrl(int drvnum, DWORD arg1, DWORD arg2);


/**@brief 路徑解析
 *
 * 解析路徑，將相對或絕對路徑，與 Task 當前工作目錄合併，傳回對應之檔案系統控制指標以及不含磁碟槽之絕對路徑。
 * 當前工作目錄，若未更換過預設為 "A:/"
 *
 * @param path 欲解析之路徑
 * @param out 不含磁碟槽之路徑
 * @param media 該磁碟槽的控制區塊指標
 *
 * @return
 * - !=0: 磁碟槽編號
 * - @ref YLERR_NOMEM
 * - @ref YLERR_INVAL
 */
int yl_dmParsePath(const char* path, char **out, void **media);


/**@brief Unicode的路徑解析
 *
 * 解析路徑，將相對或絕對路徑，與 Task 當前工作目錄合併，傳回對應之檔案系統控制指標以及不含磁碟槽之絕對路徑。
 * 當前工作目錄，若未更換過預設為 "A:/"
 *
 * @param path 欲解析之路徑(Unicode)
 * @param out_short_path 不含磁碟槽之短檔名路徑
 * @param which_media 該磁碟槽的控制區塊指標
 *
 * @return
 * - !=0: 磁碟槽編號
 * - @ref YLERR_NOMEM
 * - @ref YLERR_INVAL
 * - @ref YLERR_INVLPATH
 */
int yl_dmParsePathW(const ucs2_t *path, char **out_short_path, void **which_media);


/**@brief 取得磁碟槽之控制區塊指標
 *
 * @param drvnum 欲取得控制區塊指標之磁碟槽編號(0~3;A~D)
 *
 * @return
 * - !=0 : 該磁碟槽之控制指標
 * - 0 : 此磁碟槽編號不存在
 */
void* yl_dmGetFSHandle(int drvnum);


/**@brief 取得該控制區塊指標之磁碟槽編號
 *
 * @param mp 欲得知磁碟槽編號之控制指標
 *
 * @return
 * - @ref YLERR_FAILURE, 不明錯誤
 * - @ref YLERR_DRIVE, 輸入之控制區塊指標為 0 時發生
 */
int yl_dmGetDrvIndex(void *mp);


/**@brief 媒體檢查/修復
 *
 * @param mp 欲檢查/修復之磁碟槽編號的控制區塊指標
 * @param correct_option 欲修復之項目; 0為不修復, 修復項目為:@ref GROUP_SYS_CORRECT_OPT
 * @param error_detected 回傳發現之錯誤項目
 *
 * @return
 * - @ref YLERR_OK
 * - @ref YLERR_FAILURE, 不明錯誤
 * - @ref YLERR_FAT
 * - @ref YLERR_DRIVERIO
 * - @ref YLERR_DRIVE
 * - @ref YLERR_NOMEM
 */
int yl_dmMediaCheckRepair(void *mp, DWORD correct_option, DWORD *error_detected);


/**@brief 取得磁碟槽之可使用空間(byte)
 *
 * @param mp 欲得知之磁碟槽的控制區塊指標
 * @param freespace 可使用之空間
 *
 * @return
 * - @ref YLERR_OK
 * - @ref YLERR_DRIVE
 */
int yl_dmGetMediaFreeSpace(void *mp, DWORD *freespace);


/** @brief 取得磁碟槽之已使用空間(byte)
 *
 * @param mp 欲得知已使用的磁碟槽編號之控制區塊指標
 * @param usage 已使用之空間
 *
 * @return
 * - @ref YLERR_OK
 * - @ref YLERR_DRIVE
 */
int yl_dmGetMediaUsage(void *mp, DWORD *usage);


/** @brief 取得磁碟槽之總容量(byte)
 *
 * @param mp 欲得知磁碟槽編號之控制區塊指標
 * @param total 傳回之磁碟槽總容量
 *
 * @return
 * - @ref YLERR_OK
 * - @ref YLERR_DRIVE
 * - @ref YLERR_NOTSUPP
 */
int yl_dmGetMediaTotalSpace(void *mp, DWORD *total);

/**********************************************************************************************//**
 * @defgroup GROUP_SYS_DRVMAN_IOCTRL IO Control Codes
 * @{
 * @sa yl_dmIOCtrl
 *************************************************************************************************/
/** @brief 取得磁碟槽資訊
 *
 * @note 呼叫者須提供一@ref DRIVE_GEO_T型態之變數, 給@ref yl_dmIOCtrl的arg2\n;
 * 呼叫結束之後, 該變數就填滿DRIVE_GEO_T之相關資訊
 *
 * @sa yl_dmIOCtrl, DRIVE_GEO_T
 */
#define IOCTRL_DRVIVE_GET_GEO				0x0001

/** @brief 全新格式化
 *
 * @sa yl_dmIOCtrl
 *
 * @note 在yl_dmIOCtrl毋須給arg2參數
 */
#define IOCTRL_NFTL_RAW_FORMAT				0x0002

/** @brief 低階格式化
 *
 * @sa yl_dmIOCtrl
 *
 * @note 在yl_dmIOCtrl毋須給arg2參數
 */
#define IOCTRL_NFTL_LOW_LEVEL_FORMAT		0x0003

/** @brief 初始化NFTL
 *
 * @note NFTL(NAND Flash Translation Layer), 主要功能是將Logical Block Address對應到\n
 * 實體的flash memory address, 藉此來達到能讓flash使用FAT等用在block device這類的file system
 *
 * @note 在yl_dmIOCtrl毋須給arg2參數
 *
 * @sa yl_dmIOCtrl
 */
#define IOCTRL_NFTL_INITIALIZE				0x0004

/** @brief 取得邏輯/實體轉換表
 *
 * @note 須提供16位元(WORD**)存放向量表之陣列指標, 會回傳向量表的指標
 *
 * @sa yl_dmIOCtrl
 */
#define IOCTRL_NFTL_LP_TABLE				0x0005

/** @brief 取得區塊鏈結表
 *
 * @note 須提供16位元(WORD**)存放向量表之陣列指標;\n
 * 宣告args[2], args[0]為向量表的指標; args[1]為向量表的開頭位址(整數)
 *
 * @sa yl_dmIOCtrl
 */
#define IOCTRL_NFTL_VEC_TABLE				0x0006

/** @brief 取得區塊年齡表
 *
 * @note 每清除一次, 年齡就加1; 須提供16位元(WORD**)存放向量表之陣列指標, 會回傳向量表的指標
 *
 * @sa yl_dmIOCtrl
 */
#define IOCTRL_NFTL_AGE_TABLE				0x0007

/** @brief 錯誤檢查之狀態
 *
 * @note @ref yl_dmIOCtrl之回傳值為所修復之次數; arg2要提供一指標, 會傳回不可修復之次數
 *
 * @sa yl_dmIOCtrl
 */
#define IOCTRL_NFTL_ECC_STATUS				0x0008

/** @brief 顯示NFTL資訊
 *
 * @sa yl_dmIOCtrl
 *
 * @note NFTL(NAND Flash Translation Layer), 主要功能是將Logical Block Address對應到\n
 * 實體的flash memory address, 藉此來達到能讓flash使用FAT等用在block device這類的file system
 */
#define IOCTRL_NFTL_DISPINFO				0x0009

/** @brief 重置RAM DISK的空間, 將之清為0
 *
 * @note 在yl_dmIOCtrl毋須給arg2參數
 *
 * @sa yl_dmIOCtrl
 */
#define IOCTRL_RAMDRV_RESET_MEMORY			0x0010
/**@} GROUP_SYS_DRVMAN_IOCTRL *********************************************************************/

/**@} GROUP_SYS_DRVMAN *********************************************************************/


#endif /*YL_FSIO_H_*/
