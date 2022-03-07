
//    common.h

#ifndef __COMMONH
#define __COMMONH

#ifdef SOLARIS
#define _POSIX_C_SOURCE 199506L
#define __EXTENSIONS__    /* seems needed if above present */
#endif


#ifdef WIN32

#ifdef RPC
#define FD_SETSIZE      128    // used by <rpc/rpc.h> - here to be  before  windows.h
#endif

#include <windows.h>
#endif

#include "types.h"
#include "signall.h"

#include <sys/types.h>
#include <string.h>

#ifdef POSIX
#include <sys/time.h>
#endif

#ifndef WIN32
#include <stdlib.h> /* getenv, exit   */
#include <unistd.h>
#include <sched.h>
#else
#endif

#if (defined SOLARIS || defined POSIX)
#include <semaphore.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WIN32

#ifndef ERROR
#define ERROR -1
#endif
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE (1)
#endif

#endif

#define INUSE 1
#define FREE 0

#define OK 0

#define VALID 1


//    default key to use for the Shared Memory Segment - This may be supplied via the config file
//    default key is the same as the default UDP port

#define SHMKEY 10205

//    Shared Memory Key to use

#ifndef WIN32
extern key_t  shmkey;
#else
extern int shmkey;
#endif

#define TS_UDP_PORT 10205
extern int Udp_Port;

//    length of path names used 

#define PATH_LENGTH 100

#define N_DATA_LINKS 4      // typically there will be only 1 data link task

//    The status_table, device_table, driver_table and stream_table are located
//    in the Shared Memory Segment

typedef struct s_status_table {
    short ts_state;
    short ts_ctrl;
    int run_number;
    pid_t master_pid;
    int msg_logging_level;
    int msg_reporting_level;
    int tapeserver_options;
    int tape_block_size;    // bytes
    int disc_file_size;     // Kbytes
} STATUS_TABLE;

extern STATUS_TABLE *status_table;

//    Comms Shared Memory Segment

#define N_REQUEST_LENGTH 1024
#define N_RESPONSE_LENGTH 2048

typedef struct s_comms_table {
    short comms_state;
    short comms_ctrl;
    int comms_action;
    pid_t TS_pid;
    int comms_capM, comms_capL;
    int comms_request_length;
    char comms_request[N_REQUEST_LENGTH];
    int comms_response_length;
    char comms_response[N_RESPONSE_LENGTH];
} COMMS_TABLE;

extern COMMS_TABLE *comms_table;

//    program options - may be read from the configuration file during the server startup
//    default values

#define DATA_BUFFER_SIZE 64           // units 1Kbytes
#define TAPE_BLOCK_SIZE 64            // units 1Kbytes
#define DISC_FILE_SIZE 0x7fffffff     // units 1Kbytes

//    available devices - the names of available devices is read from the configuration file
//    during the server startup

//    format of an available device name - maximum name length is 24 characters (but normally they are about 10)
//    a maximum of 16 devices is possible (typically 1-4)

#define N_DEVICES 16
#define N_DEVICE_NAMELENGTH 24
#define N_DEVICE_PATHLENGTH PATH_LENGTH

typedef struct s_device_table {
    int device_status;
    pid_t driver_pid;
    int device_namelength;
    char device_name[N_DEVICE_NAMELENGTH];
    int generic_device_namelength;
    char generic_device_name[N_DEVICE_NAMELENGTH];
    char driver_task[N_DEVICE_PATHLENGTH];
    int device_info_length;
    char device_info[N_DEVICE_PATHLENGTH];
#ifdef WIN32
    HANDLE driver_event;
#endif
} DEVICE_NAME;

extern DEVICE_NAME *device_names;

//    device states

#define DEVICE_FREE 0
#define DEVICE_CLAIMED 1


typedef struct s_capability {
    int capabilityM, capabilityL;
} CAPABILITY;


#define N_VOLUME_NAMELENGTH 8

typedef struct s_volume_name {
    int namelength;
    char name[N_VOLUME_NAMELENGTH];
} VOLUME_NAME;

#define N_FILE_NAMELENGTH 17

typedef struct s_file_name {
    int namelength;
    char name[N_FILE_NAMELENGTH];
} FILE_NAME;


#define N_DRIVER_INFOLENGTH 100

typedef struct s_driver_info {
    int infolength;
    char info[N_DRIVER_INFOLENGTH];
} DRIVER_INFO;


#define N_DRIVER_DATALENGTH 16384    // in units of words  (ie 4096 => 16 Kbyte)

typedef struct s_driver_data {
    int state;
    int endian;
#if (defined SOLARIS || defined POSIX)
    sem_t sem;    /*  for interlocking access */
#endif
#ifdef GNU
    int sem;
#endif
    int datalength;
    int data[N_DRIVER_DATALENGTH];
} DATA_BUFFER;


//    24 byte data block header

typedef struct s_data_header {
    char header_id[8];
    int header_sequence;
    short  header_stream;
    short  header_tape;
    short  header_MyEndian;
    short  header_DataEndian;
    int header_dataLen;
} DATA_HEADER;


#define REQUEST_SENSE_LEN  29       // length of the returned Req Sense Data
#define INQUIRY_LEN        56       // length of the returned Inquiry Data
#define MODE_SENSE_LEN     12       // length of the returned Mode Sense Data

#define DEVICE_INFO_LENGTH 112

typedef struct s_device_info {
   int requestsenselen;
   char RequestSenseData [REQUEST_SENSE_LEN];
   int modesenselen;
   char ModeSenseData [MODE_SENSE_LEN];
   int inquirylen;
   char InquiryData [INQUIRY_LEN];
} DEVICE_INFO;


//    a maximum of 16 drivers is possible (typically 1-4)

#define N_DRIVERS 16

//    device driver control table

typedef struct s_driver_table {
   int driver_state;
   int driver_pstate;             // previous value of state
   int device_index;
   int clientID;
   int tape_stream;
   int driver_stat;               // result of last procedure call
   int tape_errcode;
   int driver_mode;
   int driver_label;
   int driver_recLen;
   int driver_blkLen;
   int driver_dataLen;
   int driver_density;
   int driver_file_section;
   int driver_blkCnt;
   int driver_bytCnt;
   int driver_KbytCnt;
   int driver_datarate;
   int driver_blockswritten;
   int driver_ioerrors;         //  I/O errors as reported by the OS
   int tape_length;
   int tape_remaining;
   int tape_io_errors;          //  I/O errors as returned by the hardware
   int tape_io_errorrate;
   DRIVER_INFO driver_info;
   VOLUME_NAME driver_volume_name;
   FILE_NAME driver_file_name;
   CAPABILITY user_capability;
   DEVICE_INFO device_info;
   DATA_BUFFER data_buffer;
} DRIVER_TABLE;

extern DRIVER_TABLE *driver_table;


//    a maximum of 4 streams is possible (typically 1-2)

#define N_STREAMS 4

#define N_ASSOCIATION_LIST (N_DRIVERS * 4)   // should be much more than enough

//    data stream control table

typedef struct s_stream_table {
   int  stream_state;      //  = FREE or = INUSE
   int  stream_mode;       //   association mode
   int stream_blkCnt;
   int stream_bytCnt;
   int stream_KbytCnt;
   int stream_datarate;
   int stream_sequence;
   int stream_association_list[N_ASSOCIATION_LIST];
   DATA_BUFFER stream_buffer;
} STREAM_TABLE;

extern STREAM_TABLE *stream_table;


//    device state variables

#define DEV_UNALLOCATED  0
#define DEV_ALLOCATED    1 
#define DEV_MOUNTING     2
#define DEV_MOUNT        3
#define DEV_OPENNING     4
#define DEV_OPEN         5
#define DEV_CLOSING      6
#define DEV_EXECUTING    7
#define DEV_INITIALISING 8
#define DEV_IDENTIFYING  9
#define DEV_PUTTING      10
#define DEV_DEALLOCATING 32
#define DEV_ALLOCATING   33
#define DEV_DISMOUNTING  65
#define DEV_ERRORSTATE   16



//    server state variables

#define TS_HALTED  1
#define TS_GOING   2
#define TS_NOSTORAGE    3

#define TS_PAUSE 8
#define TS_CONTINUE 9

//    server control variables

#define TS_NORMAL 0
#define TS_PAUSED 8

extern int TS_state;      //    current DA state

extern int Run_Number;     //    current Run Number  


//    address of shared data area

extern void * shm_dataarea;

//    address of shared buffer area

extern void * shm_bufferarea;

//    address of shared communications area    -   used for control via Web services

extern void * shm_commsarea;

//    index in driver_table of current driver

extern int currentDriver;

//    index in stream_table of current stream

extern int currentStream;

//    current device

extern DEVICE_NAME device_name;

//    current tape volume

extern VOLUME_NAME volume_name;

//    current file volume

extern FILE_NAME file_name;

//     client ID for  current request

extern int currentID;

//    buffer for information message concerning current operation

#define DRIVER_INFO_LEN 100

extern char Driver_Info [DRIVER_INFO_LEN];
extern int  driver_info_len;


extern pid_t mypid; 
extern pid_t Masterpid;


#ifdef __cplusplus
}
#endif


void report_version();

#ifdef MASTER
void initialise_driver_table(int);
void initialise_stream_table(int);
void ts_action(int);
#else
#ifdef WIN32
void initialise_data_area();
void initialise_driver_table(int);
void initialise_stream_table(int);
#endif
#endif

void claim_data_buffer(int);
void release_data_buffer(int);
void claim_stream_buffer(int);
void release_stream_buffer(int);



#endif
