#include "ppp.h"
/**
 * @fn функция пинга адреса.
 * 
 * @param void *ch безтиповый указатель на адрес для пинга.
 * @return int при успехе - возвращает 0, иначе код ошибки.
 */
DWORD WINAPI ping_addr( void *ch );

BOOL   b_repeat = 1;
HANDLE h_stdout;
CONSOLE_SCREEN_BUFFER_INFO csbi;
CRITICAL_SECTION cs;

ICMPCreateFile  p_icmp_create_file;
IcmpCloseHandle p_icmp_close_handle;
IcmpSendEcho    p_icmp_send_echo;

char ip_addr[ MAXDNSNAME ][ COLCOUNT ];
/**
 * @fn main точка входа в программу
 */
int main( int argc, char * argv[] )
{
 COORD  mh_coord;
 HANDLE h_thread[ ROWCOUNT ];
 char   pc_head[ COLCOUNT ];
 char   pc_line[ COLCOUNT ];
 unsigned long ul_llen;
 unsigned long ul_result,
               ul_thread_id;
 HINSTANCE h_icmp;
 WSADATA   wsaData;
 int i, i_textcolor;

 h_icmp = LoadLibrary( "icmp.dll" );
  if( !h_icmp )
   printf( "ошибка загрузки icmp.dll\n" );

  if( WSAStartup( MAKEWORD( 1, 0 ), &wsaData ) )
   printf( "ошибка инициализации WSA" );

  p_icmp_create_file  = ( ICMPCreateFile  )GetProcAddress( h_icmp, "IcmpCreateFile"  );
  p_icmp_close_handle = ( IcmpCloseHandle )GetProcAddress( h_icmp, "IcmpCloseHandle" );
  p_icmp_send_echo    = ( IcmpSendEcho    )GetProcAddress( h_icmp, "IcmpSendEcho"    );

  if( ( p_icmp_create_file  == 0 )
   || ( p_icmp_close_handle == 0 )
   || ( p_icmp_send_echo    == 0 ) ) {
   FreeLibrary( h_icmp );
   WSACleanup();
   printf( "ошибка инициализации функций icmp" );
  }

 for( i = 1; i < argc; i++ )
  if( strlen( argv[ i ] ) )
   strcpy( ip_addr[ i - 1 ], argv[ i ] );

 h_stdout = GetStdHandle( STD_OUTPUT_HANDLE );
 GetConsoleScreenBufferInfo( h_stdout, &csbi );

 InitializeCriticalSection( &cs );

 mh_coord.X = 0;
 mh_coord.Y = 0;
 sprintf( pc_head, "%3s %24s %20s %8s %3s %-7s", "#", "ping", "from", "B", "t", "ttl" );

 SetConsoleCursorPosition( h_stdout, mh_coord );
 SetConsoleTextAttribute(  h_stdout, 7 );

 WriteConsole( h_stdout, pc_head, strlen( pc_head ), &ul_result, NULL );

 int l = 0;
 while( l < i - 1 )
  h_thread[ l ] = CreateThread( NULL, 0, ping_addr, ( void * ) l++, 0, &ul_thread_id );

 char c_key;
 while( b_repeat ) {
  c_key = getch();
  switch( c_key ) {
   case 13  : b_repeat = 0; break;
   case 'p' : l = 0;
              while( l <= i ) {
               SuspendThread( h_thread[ l ++ ] );
              }
              mh_coord.X = 32;
              mh_coord.Y = i / 2;
              i_textcolor = 13;
              sprintf( pc_head, ":: paused ::" );
              EnterCriticalSection( &cs );
              SetConsoleCursorPosition( h_stdout, mh_coord );
              SetConsoleTextAttribute(  h_stdout, i_textcolor );
              WriteConsole( h_stdout, pc_head, strlen( pc_head ), &ul_result, NULL );
              LeaveCriticalSection( &cs );
              break;
   case 's' : l = 0;
              while( l <= i ) {
               ResumeThread( h_thread[ l ++ ] );
              }
              break;
   case 80  : l = 0;
              while( l <= i )
               SuspendThread( h_thread[ l ++ ] );
              l = 0;
              mh_coord.X = 0;
              mh_coord.Y = l + 1;
              ReadConsoleOutputCharacter( h_stdout, pc_line, COLCOUNT, mh_coord, &ul_llen );
              i_textcolor = 128;
              SetConsoleCursorPosition( h_stdout, mh_coord );
              SetConsoleTextAttribute(  h_stdout, i_textcolor );
              WriteConsole( h_stdout, pc_line, ul_llen, &ul_result, NULL );
              break;
  } // switch
 } // while

 mh_coord.X = 0;
 mh_coord.Y = i;
 i_textcolor = 15;
 sprintf( pc_head, "killing threads..." );

 EnterCriticalSection( &cs );
 SetConsoleCursorPosition( h_stdout, mh_coord );
 SetConsoleTextAttribute( h_stdout, i_textcolor );
 WriteConsole( h_stdout, pc_head, strlen( pc_head ), &ul_result, NULL );
 LeaveCriticalSection( &cs );

 WaitForMultipleObjects( i, h_thread, 1, INFINITE );

 //_setcursortype( _NORMALCURSOR );
 DeleteCriticalSection( &cs );
 FreeLibrary( h_icmp );
 WSACleanup( );
 return 0;
}
///
DWORD WINAPI ping_addr( void *i )
{
  int j = ( int ) i;
  int i_textcolor = 15;
  int err_count = 0;
  int i_errcode;
  COORD m_coord;
  unsigned long ul_result;
  char *err = new char[ COLCOUNT ];
  char *sfx = new char[ COLCOUNT ];
  char *str = new char[ COLCOUNT ];
  char *hst = new char[ COLCOUNT ];
  HANDLE h_ip;

  m_coord.X = 0;
  m_coord.Y = j + 1;

  EnterCriticalSection( &cs );

  SetConsoleCursorPosition( h_stdout, m_coord );
  SetConsoleTextAttribute( h_stdout, i_textcolor );
  WriteConsole( h_stdout, "   processing...", strlen( "   processing..." ), &ul_result, NULL );
  LeaveCriticalSection( &cs );

  sprintf( err, " OK  " );

   PIP_ECHO_REPLY p_ip_echo = new IP_ECHO_REPLY [ sizeof( IP_ECHO_REPLY ) ];

   struct in_addr p_dest_address;
   struct hostent* p_host_ent;
   p_dest_address.s_addr = inet_addr( ip_addr[ j ] );

   if( p_dest_address.s_addr == INADDR_NONE ) {
    p_host_ent = gethostbyname( ip_addr[ j ] );

    if( p_host_ent == NULL ) {
     i_errcode = WSAGetLastError();
     switch( i_errcode ) {
      case WSANOTINITIALISED : sprintf( err, "WSA not initialised" ); break;
      case WSAENETDOWN       : sprintf( err, "the network subsystem has failed." ); break;
      case WSAHOST_NOT_FOUND : sprintf( err, "host not found." ); break;
      case WSATRY_AGAIN      : sprintf( err, "non-authoritative server failure." ); break;
      case WSANO_RECOVERY    : sprintf( err, "nonrecoverable error occurred." ); break;
      case WSANO_DATA        : sprintf( err, "valid name, no data record of requested type." ); break;
      case WSAEINPROGRESS    : sprintf( err, "a blocking WSA 1.1 call is in progress" ); break;
      case WSAEFAULT         : sprintf( err, "the name argument is not a valid part of the user address space." ); break;
      case WSAEINTR          : sprintf( err, "the call was canceled through WSACancelBlockingCall." ); break;
     }
     sprintf( str, "%3d %32s %16s", j, ip_addr[ j ], err );
     EnterCriticalSection( &cs );

     SetConsoleCursorPosition( h_stdout, m_coord );
     SetConsoleTextAttribute( h_stdout, i_textcolor );
     WriteConsole( h_stdout, str, strlen( str ), &ul_result, NULL );
     LeaveCriticalSection( &cs );
     ExitThread( 0 );
     return 0;
    }
    else
     p_dest_address.s_addr = *( ( unsigned int* )p_host_ent->h_addr_list[ 0 ] );
   }
   p_host_ent = gethostbyaddr((char *) &p_dest_address, 4, AF_INET);
   ////////////////////
       if( p_host_ent == NULL ) {
        i_errcode = WSAGetLastError();
        if( i_errcode != 0 ) {
         if( i_errcode == WSAHOST_NOT_FOUND )
          sprintf( hst, "%32s", "host not found" );
         else
          if( i_errcode == WSANO_DATA )
           sprintf( hst, "%32s", ip_addr[ j ] );
          else
           sprintf( hst, "%32s", "function failed: %ld", i_errcode );
        }
       }
       else
        sprintf( hst, "%32s", p_host_ent->h_name );
   /////////////////////////////

   h_ip = p_icmp_create_file();
   if( h_ip == INVALID_HANDLE_VALUE )
    sprintf( err, "INVALID_HANDLE_VALUE error" );

  if( p_ip_echo == NULL )
   sprintf( err, "ip echo error" );

   char *buf = new char[ 32 ];
   p_ip_echo->Data = ( void * )buf;
   p_ip_echo->DataSize = 32;

  while( b_repeat ) {
   ////
   sprintf( err, " OK  " );

   int i_status = p_icmp_send_echo( h_ip, p_dest_address.s_addr, buf, 32, NULL, p_ip_echo, sizeof( IP_ECHO_REPLY ) + 32, 100 );
   unsigned char* p_ip_ptr = ( unsigned char* ) &p_dest_address.s_addr;

   sprintf( str, "%3d %s ", j, hst );

   if( i_status == 0 ) {
    i_textcolor = ( ( err_count++ ) > 15 )? 8 : 14;
    sprintf( sfx, "  %03i.%03i.%03i.%03i request timeout", *( p_ip_ptr ), *( p_ip_ptr + 1 ), *( p_ip_ptr + 2 ), *( p_ip_ptr + 3 ) );

    strncat( str, sfx, strlen( sfx ) );
    sprintf( err, " BAD" );
   }
   else {
    err_count = 0;
    p_ip_ptr = ( unsigned char* ) &p_ip_echo->Address;
    sprintf( sfx, "  %03i.%03i.%03i.%03i", *( p_ip_ptr ), *( p_ip_ptr + 1 ), *( p_ip_ptr + 2 ), *( p_ip_ptr + 3 ) );
    strncat( str, sfx, strlen( sfx ) );
    i_textcolor = ( j%2 )? 15 : 7;

    switch( p_ip_echo->Status ) {
     case IP_SUCCESS               : sprintf( sfx, "%4i %3i %-6i", p_ip_echo->DataSize,  p_ip_echo->RoundTripTime, p_ip_echo->Options.Ttl );
                                     break;
     case IP_DEST_NET_UNREACHABLE  : sprintf( sfx, " net unreacheble");  i_textcolor = 12;
                                     break;
     case IP_DEST_HOST_UNREACHABLE : sprintf( sfx, " host unreacheble"); i_textcolor = 12;
                                     break;
     case IP_DEST_PORT_UNREACHABLE : sprintf( sfx, " ip unreacheble");   i_textcolor = 12;
                                     break;
     default                       : sprintf( sfx, " request timeout");  i_textcolor = 14;
                                     break;
    }
    strncat( str, sfx, strlen( sfx ) );
   }
   strncat( str, err, strlen( err ) );
   ////
   EnterCriticalSection( &cs );
   SetConsoleCursorPosition( h_stdout, m_coord );
   SetConsoleTextAttribute( h_stdout, i_textcolor );
   WriteConsole( h_stdout, str, strlen( str ), &ul_result, NULL );
   LeaveCriticalSection( &cs );

   Sleep( 1000L );
  }

  p_icmp_close_handle( h_ip );
  delete str, sfx, err;
  ExitThread( 0 );
  return 0;
}
