/*
*
* rtl_socket.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "c-smile.h"
#include "vm.h"
#include "rtl.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif


namespace c_smile
{
  // SOCKET CLASS

  using namespace sal;

  VALUE
    SOCKET::close ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = (SOCKET::INSTANCE *) _this_.v.v_thing;
    if ( me->socket )
      return VALUE ( (bool) me->socket->close () );
    return VALUE ( false );
  }

  VALUE
    SOCKET::accept ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = ( SOCKET::INSTANCE * ) _this_.v.v_thing;
    if ( !me->server )
      VM::error ( "%s::accept is not server socket" );

    sal::socket_t* conn_socket = me->socket->accept ();

    if ( conn_socket == 0 )
    {
      char buffer [ 1024 ];
      me->socket->get_error_text ( buffer, 1024 );
      if ( buffer [ 0 ] )
        VM::error ( buffer );
      return VM::undefined;
    }

    SOCKET::INSTANCE * socki = new SOCKET::INSTANCE ();
    socki->socket = conn_socket;
    socki->server = false;
    socki->domain = me->domain;

    return VALUE ( socki );
  }

  VALUE
    SOCKET::connect ( int argc, VALUE *argv )
  {
    string        address;
    sal::socket_t::socket_domain
                  domain = sal::socket_t::sock_global_domain;
    int           max_attempts = DEFAULT_CONNECT_MAX_ATTEMPTS;
    time_t        tout = DEFAULT_RECONNECT_TIMEOUT;

    if ( argc >= 1 )
    {
      chktype ( 0, DT_STRING );
      address = CSTR ( argv [ 0 ].v.v_string );
    }
    else
      error_parameters ();

    if ( argc >= 2 )
    {
      chktype ( 1, DT_INTEGER );
      domain = (socket_t::socket_domain) argv [ 1 ].v.v_integer;
    }

    if ( argc >= 3 )
    {
      chktype ( 2, DT_INTEGER );
      max_attempts = argv [ 2 ].v.v_integer;
    }

    if ( argc >= 4 )
    {
      chktype ( 3, DT_INTEGER );
      tout = argv [ 3 ].v.v_integer;
    }

    if ( argc >= 5 )
      error_parameters ();

    socket_t* conn_socket = socket_t::connect ( address, domain, max_attempts, tout );

    if ( conn_socket == 0 )
      return VM::undefined;
    //TODO: raise error here

    SOCKET::INSTANCE * socki = new SOCKET::INSTANCE ();
    socki->socket = conn_socket;
    socki->server = false;
    socki->domain = domain;

    return VALUE ( socki );
  }

  VALUE
    SOCKET::socket ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = new SOCKET::INSTANCE ();
    socket_t::socket_domain domain = socket_t::sock_global_domain;
    string address;
    int listen_queue_size = DEFAULT_LISTEN_QUEUE_SIZE;

    if ( argc == 1 )
    {
      chktype ( 0, DT_STRING );
      address = CSTR ( argv [ 0 ].v.v_string );
    }
    else if ( argc == 2 )
    {
      chktype ( 0, DT_STRING );
      address = CSTR ( argv [ 0 ].v.v_string );
      chktype ( 1, DT_INTEGER );
      domain = (socket_t::socket_domain) argv [ 1 ].v.v_integer;
    }
    else if ( argc == 3 )
    {
      chktype ( 0, DT_STRING );
      address = CSTR ( argv [ 0 ].v.v_string );
      chktype ( 1, DT_INTEGER );
      domain = (socket_t::socket_domain) argv [ 1 ].v.v_integer;
      chktype ( 2, DT_INTEGER );
      listen_queue_size = argv [ 2 ].v.v_integer;
    }
    else
      error_parameters ();

    me->server = true;
    if ( domain == socket_t::sock_local_domain )
      me->socket = socket_t::create_local ( address, listen_queue_size );
    else
      me->socket = socket_t::create_global ( address, listen_queue_size );

    return VALUE ( me );
  }

  VALUE
    SOCKET::stream ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = ( SOCKET::INSTANCE * ) _this_.v.v_thing;
    if ( me->socket )
    {
      socket_stream *ss = new socket_stream ( me->socket );
      STREAM::INSTANCE *si = new STREAM::INSTANCE ( ss, me );
      return VALUE ( si );
    }
    return VM::null;
  }

  VALUE
    SOCKET::remote_addr ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = (SOCKET::INSTANCE *) _this_.v.v_thing;

    if ( argc )
      error_read_only ();

    if ( me->socket )
    {
      return VALUE ( new STRING ( me->socket->remote_addr () ));
    }

    return VM::null;
  }

  VALUE
    SOCKET::remote_port ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = (SOCKET::INSTANCE *) _this_.v.v_thing;

    if ( argc )
      error_read_only ();

    if ( me->socket )
    {
      return me->socket->remote_port ();
    }

    return VM::null;
  }

  VALUE
    SOCKET::name_by_addr ( int argc, VALUE *argv )
  {
    if ( argc != 1 ) error_parameters ();
    chktype ( 0, DT_STRING );
    unsigned long hostaddr = inet_addr ( CSTR ( argv [ 0 ].v.v_string ) );
    if ( hostaddr != INADDR_NONE )
    {
      struct hostent* hp =
#ifdef _WIN32
      gethostbyaddr ( (char * ) &hostaddr, sizeof hostaddr, AF_INET );
#else
      gethostbyaddr ( (const char * ) &hostaddr, sizeof hostaddr, AF_INET );
#endif
      if ( hp && hp->h_name && hp->h_name [ 0 ] && hp->h_addr_list [ 0 ] )
        return VALUE ( new STRING ( hp->h_name ) );
    }
    return VM::null;
  }

  VALUE
    SOCKET::addr_by_name ( int argc, VALUE *argv )
  {
    if ( argc != 1 )
      error_parameters ();

    chktype ( 0, DT_STRING );

    struct in_addr insock;    // inet socket address
    struct hostent*    hp;    // entry in hosts table
    unsigned long hostaddr = inet_addr ( CSTR ( argv [ 0 ].v.v_string ) );

    if ( hostaddr != INADDR_NONE )
    {
      if ( ( hp = gethostbyaddr ( (char *) &hostaddr,
                                  sizeof hostaddr,
                                  AF_INET ) ) == NULL )
        return VM::null;
    }
    else if ( ( hp = gethostbyname ( CSTR ( argv [ 0 ].v.v_string ) ) ) == NULL
             || hp->h_addrtype != AF_INET )
      return VM::null;

    memcpy ( &insock, hp->h_addr, hp->h_length );

    return VALUE ( new STRING ( (const char * ) inet_ntoa ( insock ) ) );
  }

  VALUE
    SOCKET::addr ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = ( SOCKET::INSTANCE * ) _this_.v.v_thing;

    if ( argc )
      error_read_only ();

    if ( me->socket )
    {
      return me->socket->addr ();
    }

    return VM::null;
  }

  VALUE
    SOCKET::port ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = (SOCKET::INSTANCE *) _this_.v.v_thing;
    if ( argc )
      error_read_only ();

    if ( me->socket )
    {
      return me->socket->port ();
    }

    return VM::null;
  }

  VALUE
    SOCKET::hostname ( int argc, VALUE *argv )
  {
    if ( argc ) error_parameters ();

    char localhost [ 256 ];
    gethostname ( localhost, sizeof localhost );

    return VALUE ( new STRING ( localhost ) );
  }

  VALUE
    SOCKET::shutdown ( int argc, VALUE *argv )
  {
    SOCKET::INSTANCE *me = (SOCKET::INSTANCE *) _this_.v.v_thing;

    if ( me->server )
    {
      me->socket->cancel_accept ();
    }
    else
      me->socket->shutdown ();

    return VM::undefined;
  }

  CLASS * SOCKET::INSTANCE::klass = 0;

  SOCKET::SOCKET ( PACKAGE* package ) : CLASS ( "socket", 0, package )
  {
    add_function ( "socket", socket );
    add_function ( "close",  close  );
    add_function ( "accept", accept );
    add_static_function ( "connect", connect );
    add_function ( "stream", stream );

    add_function ( "shutdown", shutdown );

    //enum access_mode
    add_static_data ( "any_domain",    VALUE ( int ( socket_t::sock_any_domain ) ) );
    add_static_data ( "local_domain",  VALUE ( int ( socket_t::sock_local_domain ) ) );
    add_static_data ( "global_domain", VALUE ( int ( socket_t::sock_global_domain ) ) );


    add_property ( "remote_addr", remote_addr );
    add_property ( "remote_port", remote_port );

    add_property ( "addr", addr );
    add_property ( "port", port );

    add_static_function ( "name_by_addr", name_by_addr );
    add_static_function ( "addr_by_name", addr_by_name );

    add_static_function ( "hostname", hostname );

    SOCKET::INSTANCE::klass = this;

  }

};
