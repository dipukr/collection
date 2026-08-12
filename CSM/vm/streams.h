/*
*
* streams.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_streams_h
#define __cs_streams_h

#include "c-smile.h"
#include "file.h"
#include "sockio.h"

namespace c_smile
{
  const size_t eof = ( size_t ) -1;

  // i/o stream dispatch structure
  class io_stream
  {
  protected:
    string    _name;
  public:

    enum io_status
    {
      read_only   = 2,  // file is read-only
      write_only  = 1,  // file is write-only
      ok          = 0,
      not_opened  = -1, // file was not previously opened
      end_of_file = -2, // read beyond end of file or no space to extend file
      lock_error  = -3, // file is used by another program
      other_error = -4  // other errors
    };

    enum anchor_t
    {
      at_absolute = 0, at_current = 1, at_end = 2
    };

    int             err; // io_status
    string          err_message;

    io_stream  ()
    {
    };

    virtual
      ~io_stream ()
    {
    };

    virtual bool
      at_eof ()
    {
      return err != 0;
    }

    virtual const char *
      name ()
    {
      return _name;
    }

    virtual void
      name ( const char *new_name )
    {
      close ();  _name = new_name;
#ifdef _WIN32
      _name.replace ( "/", "\\" );
#endif
    }

    virtual bool
      open ( const char *mode = 0 )
    {
      return false;
    }

    virtual bool
      is_random  ()
    {
      return false;
    }

    virtual bool
      is_binary ()
    {
      return false;
    }

    virtual bool
      can_read ()
    {
      return false;
    }

    virtual bool
      can_write ()
    {
      return false;
    }

    virtual bool check_read ();
    virtual bool check_write ();

    virtual bool
      close ()
    {
      return false;
    }

    virtual bool
      remove ()
    {
      return false;
    }

    virtual int
      length ()
    {
      return 0;
    }

    virtual void
      length ( int sz )
    {
    }

    virtual bool
      get ( char &c )
    {
      return false;
    }

    virtual bool
      get ( string& s )
    {
      return false;
    }

    virtual bool
      put ( char c )
    {
      return false;
    }

    virtual bool
      put ( char const *d )
    {
      return false;
    }

    virtual bool
      position ( int pos, anchor_t anchor = at_absolute )
    {
      return false;
    }

    virtual size_t
      position ()
    {
      return eof;
    }

    virtual size_t
      size ()
    {
      return 0;
    }

    virtual bool
      size ( size_t i )
    {
      return false;
    }

    virtual bool
      read ( char *d, int size, int* size_read = 0 )
    {
      return false;
    }

    virtual bool
      write ( char const *d, int size, int* size_written = 0 )
    {
      return false;
    }

    bool
      read ( char &c )
    {
      return get ( c );
    }

    bool
      write ( char c )
    {
      return put ( c );
    }

    bool read  ( int &d );
    bool write ( int d );

    bool read  ( double &d );
    bool write ( double d );

    bool read  ( string &d );
    bool write ( const char * );

    bool read  ( bool &d );
    bool write ( bool d );

  };

  class console_stream: public io_stream
  {
  public:
    enum type
    {
      STDIN = 0,
      STDOUT = 1,
      STDERR = 2
    };
    console_stream ( type t );

    virtual
      ~console_stream  ()
    {
      close  ();
    }

    const char * name ();

    virtual bool
      is_random ()
    {
      return false;
    }

    virtual bool
      is_binary ()
    {
      return false;
    }

    virtual bool
      can_read ()
    {
      return _type == STDIN;
    }

    virtual bool
      can_write ()
    {
      return _type != STDIN;
    }

    virtual bool get ( char& c );
    virtual bool get ( string& s );
    virtual bool put ( char c );
    virtual bool put ( char const *d );
  protected:
    FILE *_stream;
    type  _type;
    console_stream  () : _stream ( 0   )
    {
    }

  };

  class file_stream: public io_stream
  {

  public:
    file_stream ();

    virtual ~file_stream ();

    virtual bool
      is_random  ()
    {
      return true;
    }

    virtual bool
      is_binary  ()
    {
      return true;
    }

    virtual bool
      can_read  ()
    {
      return true;
    }

    virtual bool
      can_write  ()
    {
      return true;
    }

    virtual bool close ();
    virtual bool get ( char& c );
    virtual bool put ( char  c );
    virtual bool put ( char const *d );

    virtual bool    position ( int pos, anchor_t anchor = at_absolute );
    virtual size_t  position ();

    virtual size_t  size ();
    virtual bool    size ( size_t sz );

    virtual bool read ( char *d, int size, int* size_read = 0 );
    virtual bool write ( char const *d, int size, int* size_written = 0 );

    virtual bool open ( const char *mode = 0 );

  protected:
    bool io_result ();

    sal::file *_file;
  };

  class in_memory_stream: public io_stream
  {
#define BLOCK_SIZE 0x0FFF

  public:
    in_memory_stream ();

    virtual ~in_memory_stream ();

    virtual bool
      is_random  ()
    {
      return true;
    }

    virtual bool
      is_binary  ()
    {
      return true;
    }

    virtual bool
      can_read  ()
    {
      return true;
    }

    virtual bool
      can_write  ()
    {
      return true;
    }

    virtual bool close ();
    virtual bool open ( const char *mode = 0 );

    virtual bool    position ( int pos, anchor_t anchor = at_absolute );
    virtual size_t  position ();

    virtual size_t
      size  ()
    {
      return _buffer.size  ();
    }

    virtual bool read ( char *d, int size, int* size_read = 0 );
    virtual bool write ( char const *d, int size, int* size_written = 0 );

    virtual const char *  name ();
    virtual void          name ( const char *name );

  protected:
    bool
      size ( size_t size )
    {
      _buffer.size ( size );
      return true;
    }

    size_t                  _pos;
    array<unsigned char>    _buffer;

  };


  class socket_stream: public io_stream
  {

  public:
    socket_stream ( sal::socket_t *socket, bool binary = true );
    virtual ~socket_stream ();

    virtual bool
      is_random  ()
    {
      return false;
    }

    virtual bool
      is_binary  ()
    {
      return _is_binary;
    }

    virtual bool
      can_read  ()
    {
      return true;
    }

    virtual bool
      can_write  ()
    {
      return true;
    }

    virtual bool close ();
    virtual bool open ( const char *mode = 0 );

    virtual bool
      position ( int pos, anchor_t anchor = at_absolute   )
    {
      return false;
    }

    virtual size_t
      position  ()
    {
      return _pos;
    }

    virtual size_t
      size ()
    {
      return ( size_t ) -1;
    }

    virtual bool get ( char &c );
    virtual bool put ( char c );
    virtual bool put ( char const *d );

    virtual bool    read ( char *d, int size, int* size_read = 0 );
    virtual bool    write ( char const *d, int size, int* size_written = 0 );

    virtual const char *  name ();
    virtual void          name ( const char *name );

    virtual bool check_read ();
    virtual bool check_write ();

  protected:

    size_t                    _pos;
    sal::socket_t *           _socket;
    bool                      _is_binary;

  };


  class archive
  {
    io_stream *             _body;
    array<symbol_t>         _symbols;

  public:

    archive () : _body ( 0 )
    {
    }

    ~archive  ()
    {
    }

    void read ( char *buf, size_t size );
    void write ( const char *buf, size_t size );

    symbol_t  map ( symbol_t extern_sym );

    void    write_class_ref ( const CLASS *cls );
    void    read_class_ref ( CLASS * &cls );

    void    read_class ( CLASS *cls );
    void    write_class ( const CLASS *cls );

    void    write_literal ( const VALUE &val );
    void    read_literal ( VALUE &val );

    void    read ( bool& v );
    void    write ( bool v );

    void    read ( int& v );
    void    write ( int v );

    void    read ( symbol_t& v );
    void    write ( symbol_t v );

    void    read ( double & v );
    void    write ( double v );

    void    read ( string & v );
    void    write ( const char *v );

    void    read ( VALUE& val );
    void    write ( const VALUE& val );

    void    read ( DICTIONARY* &dict );
    void    write ( const DICTIONARY * dict );

    void    read ( CLASS* &cls );
    void    write ( const CLASS * cls );

    void    read ( CODE* &c );
    void    write ( const CODE* c );

    void    read_code_ref ( CODE* &c );
    void    write_code_ref ( const CODE* c );

    void    read ( OBJECT* &obj );
    void    write ( const OBJECT * obj );

    void    read ( STRING* &str );
    void    write ( const STRING * str );

    void    read ( ARRAY* &vec );
    void    write ( const ARRAY * obj );

    void    read ( ENTRY& vec );
    void    write ( const ENTRY& vec );

    void    read ( PACKAGE* &pkg );
    void    write ( const PACKAGE * pkg );

    void  save ( io_stream* ios, VALUE& val );    // save val to the ios;
    void  save ( io_stream* ios, PACKAGE *pkg );  // save package to the ios;

    void  save ( io_stream* ios, symbol_t mainpackage );  // all classes in current VM::session

    VALUE load ( io_stream* ios );               // load val from ios;


  };

};

#endif
