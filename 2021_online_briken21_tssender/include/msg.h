/*******************************************************************************
* msg.h			definitions to be used in conjunction with the
*				eurogam register server c library
*/
#ifndef INCmsgh
#define INCmsgh

#include <sys/types.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#endif

#if defined (__cplusplus) || defined (c_plusplus)
extern "C" {
#endif

#define MSG_VERS 0

#define MESSAGE_SERVER "egmsg"	   /* host name of central message server */

enum messageSeverityLevel
{
	MSG_S_MINIMUM     = 0,
	MSG_S_ZERO        = 0,
	MSG_S_TEST        = 1,
	MSG_S_DEBUG       = 2,
	MSG_S_INFORMATION = 3,
	MSG_S_WARNING     = 4,
	MSG_S_ERROR       = 5,
	MSG_S_ALARM       = 6,
	MSG_S_FAULT       = 7,
	MSG_S_FAILURE     = 8,
	MSG_S_CATASTROPHE = 9,
	MSG_S_ACTION      = 9,         /* Action if with class 9           */
	MSG_S_MAXIMUM     = 9
};

enum messageClass
{
	MSG_C_TEST        = 0,         /* for testing                       */
	MSG_C_MISC        = 1,         /* no class for this message         */
	MSG_C_AUTOFILL    = 2,         /* autofill system                   */
	MSG_C_HV          = 3,         /* high voltage system               */
	MSG_C_SPECTRUM    = 4,         /* spectrum server                   */
	MSG_C_SAS	  = MSG_C_SPECTRUM, /* spectrum server              */
	MSG_C_FRONTEND    = 5,         /* frontend VXI cards                */
	MSG_C_HISTOGRAM   = 6,         /* histogrammer                      */
	MSG_C_RM          = 7,   /* VXI resource manager auto configuration */
	MSG_C_ERS         = 8,         /* eurogam register server           */
	MSG_C_EB          = 9,         /* Event Builder                     */
	MSG_C_ACTION      = 9,         /* Action if with severity 9         */
	MSG_C_CAMAC       = 10,        /* VME CAMAC i/f                     */
	MSG_C_EGLK        = 11,        /* eurogam link (now unused)         */
	MSG_C_EGTS        = 12,        /* tape server                       */
	MSG_C_ROCO        = 13,        /* readout controler interupts       */
	MSG_C_RACK        = 14,        /* Rack monitoring system            */
	MSG_C_ST          = 15,        /* VXI self test functions           */
	MSG_C_WATCHDOG    = 16,        /* System watchdog                   */
	MSG_C_FDDI        = 17,        /* Event builder FDDI link           */
	MSG_C_HAWK        = 18,        /* Event builder Hawk Ethernet link  */
	MSG_C_VME         = 19,        /* VME register server functions     */
	MSG_C_PCI         = 20,        /* PCI register server functions     */
	MSG_C_NWALL       = 21,        /* EB Neutron Wall DA system         */
	MSG_C_GS          = MSG_C_NWALL, /* GREAT/SAGE Server DA system     */
	MSG_C_SHOBJMAN    = 22,        /* Win32 Shared Object Manager       */

	MSG_C_MEIS        = 30,        /* MEIS - miscellaneous message      */
	MSG_C_STEPPER     = 31,        /* MEIS - stepper motors             */

	MSG_C_ACS         = 40,        /* Exogam Acquistion Command Serveur */
	MSG_C_SURV        = 41,        /* Exogam Surveyor Serveur           */
	MSG_C_EXOGAMD2VB  = 42,        /* Exogam D2VB (input) Serveur       */
	MSG_C_OUTPUT      = 43,        /* Exogam Output Serveur             */

	MSG_C_MAXIMUM     = 99
};



#ifndef OK /* define possible function returns */
#define OK 0
#endif

#ifndef ERROR
#define ERROR (-1)
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

/*
#ifndef u_int
#define u_int unsigned int
#endif
*/

#ifdef VXWORKS
#define u_int unsigned int
#endif

typedef struct msgList {
	u_int iclass;
	u_int lowLevel;
	u_int highLevel;
	u_int count;
	struct msgList *next;
}MSG_LIST;

/* extern int errno; */
#ifdef WIN32
#define DllImport	__declspec( dllimport )
#define DllExport	__declspec( dllexport )
#else
#define DllImport
#define DllExport 
#endif

#ifdef MSGLIB_EXPORTS
#define MSGLIB_API DllExport
#else
#define MSGLIB_API DllImport
#endif

#if defined (__STDC__) || defined (__cplusplus) || defined (c_plusplus)

MSGLIB_API int	msgInitialise (char *host);
MSGLIB_API char *	msgPerror (char *string);
MSGLIB_API char *	msgVersion (void);
MSGLIB_API char *	msgSeverity (u_int level);

MSGLIB_API int	msgNullProc (void);
MSGLIB_API int	msgDefineMessage (u_int id, u_int xclass, u_int level, char *source, char *body);

#else

MSGLIB_API int	msgInitialise ();
MSGLIB_API char *	msgPerror ();
MSGLIB_API char *	msgVersion ();
MSGLIB_API char *	msgSeverity ();

MSGLIB_API int	msgNullProc ();
MSGLIB_API int	msgDefineMessage ();

#endif	/* __STDC__ */

#if defined (__cplusplus) || defined (c_plusplus)
}
#endif

#endif /* INCmsgh */
