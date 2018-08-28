#ifndef SCT_H_
#define SCT_H_

#include <platform/YL_OSWrap.h>

#ifdef _INSIDE_SCT_C_
#define OSOBJDEF(id) void *id;
#else
#define OSOBJDEF(id) extern void *id;
#endif

#define DEFAULT_STACK_SIZE		2048

// === TASK IDs =====================================================
OSOBJDEF(TSKID_INIT)
OSOBJDEF(TSKID_UCON)
OSOBJDEF(TSKID_WAKEUP)
OSOBJDEF(TSKID_DM9K_DRV)
OSOBJDEF(TSKID_HMI)
OSOBJDEF(TSKID_GUI)
OSOBJDEF(TSKID_MOTOR)

// ===============================
OSOBJDEF(FLGID_SDI_INT)
OSOBJDEF(FLGID_DM9K_INT)

// ===============================
OSOBJDEF(SEMID_UARTPRINTF)
OSOBJDEF(SEMID_PRIVMEM)
OSOBJDEF(SEMID_MTX_TIMER)

// ===============================
OSOBJDEF(MBXID_UART_RECV)
OSOBJDEF(MBXID_WAKEUP_RECV)
OSOBJDEF(MBXID_GUI)
OSOBJDEF(MBXID_HMI)
OSOBJDEF(MBXID_MOTOR_DRV)

// ===============================
OSOBJDEF(CYCID_SYSTEMTICK)
OSOBJDEF(CYCID_PILI_LED)
OSOBJDEF(CYCID_EXTIO_POLL)
OSOBJDEF(CYCID_SCR_CLOCK)
OSOBJDEF(CYCID_AD_POLL)


// ====================================================================================
// ====================================================================================
/**
 * \brief TSK_DEF_T
 */
typedef struct {
	void** id;			///< Designated task id
    FUNCENTRY task;			// Task start address (entry point).
    int stksz;				// Define the task stack size.
    unsigned int atr_pri;	// attributes & priority
} TSK_DEF_T;

/**
 * \brief FLG_DEF_T
 */
typedef struct {
	void** id;			///< Designated Event flag id
    DWORD iflgptn; 		// Initial Event Flag Pattern.
} FLG_DEF_T;

/**
 * \brief SEM_DEF_T
 */
typedef struct {
	void** id;		///< Designated Semaphore id
    int isemcnt;	// Initial semaphore count.
    int maxsem;		// Maximum semaphore count.
} SEM_DEF_T;

/**
 * \brief MBX_DEF_T
 */
typedef struct {
	void** id;		///< Designated Mail Box id
	int bufcnt;		// Buffer message count.
} MBX_DEF_T;


/**
 * \brief MTX_DEF_T
 */
typedef struct {
	void** id;		///< Designated Mutex id
} MTX_DEF_T;


/**
 * \brief ALM_DEF_T
 */
typedef struct {
	void** id;			///< Designated Alarm id
	FUNCENTRY almhdr;	// Alarm Handler Address.
	DWORD almtim;		// Alarm time.
	DWORD almatr;		// Alarm Handler Attributes.
} ALM_DEF_T;


/**
 * \brief CYC_DEF_T
 */
typedef struct {
	void** id;			///< Designated Cyclic id
	FUNCENTRY cychdr;	// Cyclic Handler Address.
	DWORD cyctim;		// Cycle time.
	DWORD cycact;		// Cyclic Handler Activation.
} CYC_DEF_T;


#endif /*SCT_H_*/
