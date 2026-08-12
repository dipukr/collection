/*
*
* streams.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include "c-smile.h"
#include "streams.h"
#include "osfile.h"
#include "rtl.h"
#include <stdlib.h>

#ifdef _WIN32
#include <share.h>
#include <io.h>
#endif

namespace c_smile
{
  using namespace sal;

  const char eol [] = "\n";
  const size_t eol_length = sizeof eol;

  bool
    io_stream::check_read ()
  {
    if ( can_read () )
      return true;

    err = read_only;
    err_message = "stream is read-only";
    return false;
  }

  bool
    io_stream::check_write ()
  {
    if ( can_write () )
      return true;

    err = write_only;
    err_message = "stream is write-only";
    return false;
  }

  bool
    io_stream::read ( int &d )
  {
    //#ifdef __I386__
    return read ( (char * ) &d, 4 );
    //#endif
  }

  bool
    io_stream::write ( int d )
  {
    //#ifdef __I386__
    return write ( (char * ) &d, 4 );
    //#endif
  }

  bool
    io_stream::read ( double &d )
  {
    //#ifdef __I386__
    return read ( (char *) &d, 8 );
    //#endif
  }

  bool
    io_stream::write ( double d )
  {
    //#ifdef __I386__
    return write ( (char * ) &d, 8 );
    //#endif
  }

  bool
    io_stream::read ( string &s  )
  {
    char b [ 1024 ];
    int l = 0;
    read ( l );
    assert ( l < 1024 );
    bool r = read ( b, l );
    b [ l ] = 0;
    s = b;
    return r;
  }

  bool
    io_stream::write ( const char *s )
  {
    int l = strlen ( s );
    write ( l );
    return write ( s, l );
  }

  bool
    io_stream::read ( bool &d  )
  {
    char c = 0;
    bool r = read ( &c, 1 );
    d = ( c != 0 );
    return r;
  }

  bool
    io_stream::write ( bool d  )
  {
    char c = d ? 0xFF : 0x00;
    return write ( &c, 1 );
  }

  console_stream::console_stream ( type t ) :_stream ( 0 )
  {
    switch ( _type = t  )
    {
    case STDIN:
      _stream = stdin; break;
    case STDOUT:
      _stream = stdout; break;
    case STDERR:
      _stream = stderr; break;
    }
  }

  const char *
    console_stream::name ()
  {
    switch ( _type  )
    {
    case STDIN: return "stdin";
    case STDOUT: return "stdout";
    case STDERR: return "stderr";
    }
    return "";
  }

  bool
    console_stream::get ( char& c  )
  {
    if ( !_stream ) return false;
    if ( feof ( _stream ) ) return false;
    c = fgetc ( _stream );
    return true;
  }

  bool
    console_stream::get ( string& s  )
  {
    if ( !_stream )
      return false;
    char buf [ 2048 ];
    if ( fgets ( buf, sizeof buf, _stream ) )
    {
      s = buf;
      return true;
    }
    return false;
  }

  bool
    console_stream::put ( char c  )
  {
    return fputc ( c, _stream ) != EOF;
  }

  bool
    console_stream::put ( char const *d  )
  {
    return fputs ( d, _stream ) != EOF;
  }

  file_stream::file_stream ()
  {
    _file = 0;
  }

  file_stream::~file_stream ()
  {
    close ();
  }

  bool
    file_stream::close ()
  {
    if ( !_file )
      return false;
    if ( can_write () )
      _file->flush ();
    err = _file->close ();
    return io_result ();
  }

  bool
    file_stream::io_result  ()
  {
    if ( err == file::ok )
      return true;
    char buf [ 256 ];
    _file->get_error_text ( file::iop_status ( err ), buf, 256 );
    err_message = buf;
    return false;
  }

  bool
    file_stream::get ( char& c  )
  {
    if ( !check_read () ) return false;
    err = _file->read ( &c, 1 );
    return io_result ();

  }

  bool
    file_stream::put ( char c  )
  {
    if ( !check_write () ) return false;
    file::iop_status r = _file->write ( &c, 1 );
    return io_result ();
  }

  bool
    file_stream::put ( char const *d  )
  {
    if ( !check_write () ) return false;
    size_t sz = 0;
    err = _file->write ( d, strlen ( d ) );
    return io_result ();

  }

  //int shflags [] = { SH_DENYNO, SH_DENYRD, SH_DENYWR, SH_DENYRW };

  //"r", // Opens a text file for reading.
  //"w", // Creates a new text file for writing, or opens and truncates a file to 0 length.
  //"a", // Appends ( opens a file for writing at the end of the file, or creates a file for writing ) .
  //"R", exclusive   // Opens a text file for reading.
  //"W", exclusive   // Creates a new text file for writing, or opens and truncates a file to 0 length.
  //"A", exclusive   // Appends ( opens a file for writing at the end of the file, or creates a file for writing ) .


  bool
    file_stream::open ( const char *mode  )
  {
    close ();
    int   modes = file::fa_read;
    int   flags = file::fo_sync;
    bool  go_end = false;
    switch ( tolower ( mode [ 0 ] ) )
    {
    default:
    case 'r':
      modes = ( mode [ 1 ] == '+' ) ? file::fa_readwrite: file::fa_read;
      break;
    case 'w':
      modes = ( mode [ 1 ] == '+' ) ? file::fa_readwrite: file::fa_write;
      flags |= ( file::fo_truncate | file::fo_create );
      break;
    case 'a':
      modes = ( mode [ 1 ] == '+' ) ? file::fa_readwrite: file::fa_write;
      flags |= file::fo_create;
      go_end = true;
      break;
    }

    flags |= ( tolower ( mode [ 0 ] ) == ( int ) mode [ 0 ] ) ?
    file::fo_shared : file::fo_exclusive;

    if ( _file == 0 ) _file = new os_file ( _name );
    err = _file->open ( (file::access_mode ) modes, flags );

    if ( err == file::ok && go_end  )
      return position ( 0, at_end );

    return io_result ();
  }

  bool
    file_stream::position ( int pos, anchor_t anchor )
  {
    if ( !_file ) return false;

    err = _file->set_position ( fposi_t ( pos ), sal::file::whence_mode ( anchor ) );
    return io_result ();

  }

  size_t
    file_stream::position ()
  {
    if ( !_file ) return 0;
    fposi_t pos;
    err = _file->get_position ( pos );
    return ( size_t ) pos;

  }

  size_t
    file_stream::size ()
  {
    if ( !_file ) return 0;
    fsize_t sz;
    err = _file->get_size ( sz );
    if ( io_result ()  )
      return ( size_t ) sz;
    else
      return 0;
  }

  bool
    file_stream::size ( size_t i  )
  {
    if ( !_file )
      return false;
    fsize_t sz = i;
    err = _file->set_size ( sz );
    return io_result ();

  }

  bool
    file_stream::read ( char *d, int sz, int* size_read  )
  {
    if ( !_file )
      return 0;
    err = _file->read ( d, size_t ( sz ) );
    if ( size_read ) *size_read = _file->num_bytes ();
    return io_result ();

  }

  bool
    file_stream::write ( const char *d, int size, int* size_written  )
  {
    err = _file->write ( d, size_t ( size ) );
    if ( size_written ) *size_written = _file->num_bytes ();
    return io_result ();
  }

  in_memory_stream::in_memory_stream () : _pos ( 0 )
  {
  }

  in_memory_stream::~in_memory_stream ()
  {
    close ();
  }

  bool
    in_memory_stream::close ()
  {
    _buffer.clear ();
    _pos = 0;
    return true;
  }

  bool
    in_memory_stream::open ( const char * )
  {
    close ();
    return true;
  }

  bool
    in_memory_stream::position ( int pos, anchor_t anchor )
  {
    switch ( anchor  )
    {
    case at_absolute:
      break;
    case at_current:
      pos = ( int ) _pos + pos;
      if ( pos < 0 ) pos = 0;
      break;
    case at_end:
      pos = ( int ) size ()  - 1 + pos;
      break;
    }
    if ( pos < 0  )
    {
      err_message = "negative stream position";
      return false;
    }
    if ( pos >= ( int ) size () ) size ( pos+1 );

    _pos = ( size_t ) pos;

    return true;
  }

  size_t
    in_memory_stream::position  ()
  {
    return _pos;
  }

  bool
    in_memory_stream::read ( char *d, int sz, int* sz_read )
  {
    int toread = size ()  - _pos;
    if ( toread > 0  )
    {
      if ( toread > sz ) toread = sz;
      memcpy ( d, &_buffer [ _pos ], toread );
      _pos += toread;
    }
    else toread = 0;
    if ( sz_read ) *sz_read = toread;

    return ( toread == sz );
  }

  bool
    in_memory_stream::write ( const char *d, int sz, int *sz_written )
  {
    if ( _pos + sz > size () )
      size ( _pos + sz );
    memcpy ( &_buffer [ _pos ], d, sz );
    _pos += sz;
    if ( sz_written ) *sz_written = sz;
    return true;
  }

  const char *
    in_memory_stream::name ()
  {
    return "in_memory";
  }

  void
    in_memory_stream::name ( const char *name )
  {
  }

  socket_stream::socket_stream ( socket_t *socket, bool binary )
        :_pos ( size_t ( -1 ) ), _socket ( socket ), _is_binary ( binary )
  {
  }

  socket_stream::~socket_stream ()
  {
    close ();
  }

  bool
    socket_stream::close ()
  {
    bool r = false;
    if ( _socket  )
    {
      r = _socket->close ();
      _socket = 0;
    }
    return r;
  }

  bool
    socket_stream::open ( const char * )
  {
    return true;
  }

  bool
    socket_stream::get ( char &c )
  {
    return read ( &c, 1 );
  }

  bool
    socket_stream::put ( char c )
  {
    if ( check_write () ) return _socket->write ( &c, 1 );
    return false;
  }

  bool
    socket_stream::put ( char const *d )
  {
    if ( check_write () ) return _socket->write ( d, strlen ( d ) );
    return false;
  }

  bool
    socket_stream::read ( char *d, int size, int* size_read )
  {
    size_t sz_read = 0;
    bool r = false;
    if ( check_read () )
      r = _socket->read ( d, size, &sz_read );
    if ( size_read ) *size_read = sz_read;
    return r;
  }

  bool
    socket_stream::write ( char const *d, int size, int* size_written )
  {
    bool r = false;
    if ( check_write () ) r = _socket->write ( d, size );
    if ( size_written ) *size_written = r ? size : 0;
    return r;
  }

  bool
    socket_stream::check_read ()
  {
    if ( _socket == 0  )
    {
      err = not_opened;
      err_message = "stream is closed";
      return false;
    }
    return io_stream::check_read ();
  }

  bool
    socket_stream::check_write ()
  {
    if ( _socket == 0  )
    {
      err = not_opened;
      err_message = "stream is closed";
      return false;
    }
    return io_stream::check_write ();
  }

  const char *
    socket_stream::name ()
  {
    if ( _socket ) return _socket->addr ();
    return "";
  }

  void
    socket_stream::name ( const char *name  )
  {
  }

};
