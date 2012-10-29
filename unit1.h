#ifndef icmp_class_implH
#define icmp_class_implH

#include < winsock.h >
#include < windows.h >
#include < wincon.h >
#include < stddef.h >
#include < stdlib.h >
#include < stdio.h >
#include < conio.h >


// Reply statuses:
#define IP_STATUS_BASE 11000
#define IP_SUCCESS 0
#define IP_BUF_TOO_SMALL 11001
#define IP_DEST_NET_UNREACHABLE 11002
#define IP_DEST_HOST_UNREACHABLE 11003
#define IP_DEST_PROT_UNREACHABLE 11004
#define IP_DEST_PORT_UNREACHABLE 11005
#define IP_NO_RESOURCES 11006
#define IP_BAD_OPTION 11007
#define IP_HW_ERROR 11008
#define IP_PACKET_TOO_BIG 11009
#define IP_REQ_TIMED_OUT 11010
#define IP_BAD_REQ 11011
#define IP_BAD_ROUTE 11012
#define IP_TTL_EXPIRED_TRANSIT 11013
#define IP_TTL_EXPIRED_REASSEM 11014
#define IP_PARAM_PROBLEM 11015
#define IP_SOURCE_QUENCH 11016
#define IP_OPTION_TOO_BIG 11017
#define IP_BAD_DESTINATION 11018
#define IP_ADDR_DELETED 11019
#define IP_SPEC_MTU_CHANGE 11020
#define IP_MTU_CHANGE 11021
#define IP_UNLOAD 11022
#define IP_GENERAL_FAILURE 11050
#define MAX_IP_STATUS 11050
#define IP_PENDING 11255

#define MAXDNSNAME 64
#define ROWCOUNT   20
#define COLCOUNT   80
//---------------------------------------------------------------------------

typedef struct {
   unsigned char Ttl;                                 // Time To Live
   unsigned char Tos;                                 // Type Of Service
   unsigned char Flags;                               // IP header flags
   unsigned char OptionsSize;                         // Size in bytes of options data
   unsigned char *OptionsData;                        // Pointer to options data
} IP_OPTION_INFORMATION, * PIP_OPTION_INFORMATION;

//---------------------------------------------------------------------------

typedef struct {
   unsigned int Address;                    // Replying address
   unsigned long  Status;                   // Reply status
   unsigned long  RoundTripTime;            // RTT in milliseconds
   unsigned short DataSize;                 // Echo data size
   unsigned short Reserved;                 // Reserved for system use
   void *Data;                              // Pointer to the echo data
   IP_OPTION_INFORMATION Options;           // Reply options
} IP_ECHO_REPLY, * PIP_ECHO_REPLY;

typedef HANDLE ( WINAPI* ICMPCreateFile  )( VOID );
typedef BOOL   ( WINAPI* IcmpCloseHandle )( HANDLE );
typedef DWORD  ( WINAPI* IcmpSendEcho    )( HANDLE,
                                            DWORD,
                                            LPVOID,
                                            WORD,
                                            PIP_OPTION_INFORMATION,
                                            LPVOID,
                                            DWORD,
                                            DWORD );
#endif