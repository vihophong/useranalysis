
#ifndef TYPESH
#define TYPESH

#ifdef WIN32

typedef unsigned char    u_char;
typedef unsigned short    u_short;
typedef unsigned int    u_int;
typedef unsigned long    u_long;
typedef unsigned char    uchar_t;
typedef unsigned char    uint8_t;
typedef  char    int8_t;
typedef unsigned short    ushort_t;
typedef unsigned short    uint16_t;
typedef  short    int16_t;
typedef unsigned int    uint_t;
typedef unsigned int    uint32_t;
typedef  int    int32_t;
typedef unsigned long    ulong_t;

typedef long        t_scalar_t;    /* historical versions */
typedef unsigned long    t_uscalar_t;

typedef unsigned long    upad64_t;
typedef unsigned int    u_offset_t;
typedef  int     intptr_t;

#endif



#ifdef WIN32

typedef unsigned long p_pid;
typedef unsigned long pid_t;

typedef long key_t;

#endif


#endif
