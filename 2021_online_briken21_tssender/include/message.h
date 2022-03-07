//    message.h

#ifndef __MESSAGEH
#define __MESSAGEH


#define MSG_TEST         1
#define MSG_DEBUG        2
#define MSG_INFORMATION  3
#define MSG_WARNING      4
#define MSG_ERROR        5
#define MSG_ALARM        6
#define MSG_FAULT        7
#define MSG_FAILURE      8
#define MSG_ACTION       9

#define MSG_TRACE        0

#define MSG_INQUIRY     17
#define MSG_MODESENSE   18
#define MSG_READ        19
#define MSG_WRITE       20
#define MSG_REQUESTSENSE 21
#define MSG_SCSICOMMAND 22
#define MSG_LOGGING     23
#define MSG_LINKSPINNER 24


#define MSG_C_TS        12        /* tape server */


//    procedure prototypes

void report_message(int);
void message(int, const char *);
void trace(const char *);
void msg_init();

void LogMessage(int);
void InitLogging();

void ReportError();
void ReportErrorMsg(char *);

#ifndef NOLOGGING

#ifdef __cplusplus
extern "C" {
#endif

extern int msgInitialise (char *);
extern int msgDefineMessage (int , int , int , char *, char *);

#ifdef __cplusplus
}
#endif

#endif


//    console message buffer

#define MESSAGE_BUFFER_LENGTH 160
extern char message_buffer[MESSAGE_BUFFER_LENGTH];

//    reporting levels  (defaults)

#define LOGGING_LEVEL     0x000001f0L     //  to message logger
#define REPORTING_LEVEL   0x0080fff8L     //  to stdout stream

extern int msg_logging_level;
extern int msg_reporting_level;


#endif
