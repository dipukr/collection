/*
*
* rtl_file.cpp
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
#include "c-smile.h"
#include "vm.h"
#include "rtl.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <unistd.h>
#define _S_IFDIR	S_IFDIR
#endif

namespace c_smile
{
  static bool
    get_stat ( const char *path, struct stat &sstat )
  {
    return ::stat ( path, &sstat ) == 0;
  }

  // NODE CLASS
  CLASS *NODE::INSTANCE::klass = 0;

  VALUE
    NODE::ctor ( int argc, VALUE *argv )
  {
    INSTANCE *me = new INSTANCE ();
    if ( argc == 1 && argv [ 0 ].is_string () )
    {
      me->path = string ( argv [ 0 ] );
    }
    return VALUE ( me );
  }

  void
    NODE::error_ne ( NODE::INSTANCE *me )
  {
    VM::error ( "'%s' does not exist", (const char *) me->path );
  }

  void
    NODE::error_nf ( NODE::INSTANCE *me )
  {
    VM::error ( "'%s' is not a file", (const char *) me->path );
  }

  void
    NODE::error_nd ( NODE::INSTANCE *me )
  {
    VM::error ( "'%s' is not a folder", (const char *) me->path );
  }

  void
    NODE::error_access ( NODE::INSTANCE *me )
  {
    VM::error ( "'%s' access denied", (const char *) me->path );
  }

  VALUE
    NODE::size ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    struct stat st;

    if ( !get_stat ( me->path, st ) )
      error_ne ( me );

    if ( argc == 1 ) // set
    {
      if ( st.st_mode & _S_IFDIR == 0 )
        error_nf ( me );
      chktype ( 0, DT_INTEGER );
      int sz = int ( argv [ 0 ] );
      if ( sz < 0 )
        sz = 0;

      int fh;
      // open a file
#ifdef _WIN32
      if ( (fh = _open ( me->path, _O_RDWR, _S_IREAD | _S_IWRITE ) )  != -1  )
      {
        _chsize ( fh, sz );
        int r = _filelength ( fh );
        _close ( fh );
        return r;
      }
#else
      if ( (fh = open ( me->path, O_RDWR ) )  != -1  )
      {
        ftruncate ( fh, sz );
        int r = lseek ( fh, 0, SEEK_END );
        close ( fh );
        return r;
      }
#endif
      else
        error_access ( me );
    }
    else
    {
      return VALUE ( int ( st.st_size ) );
    }

    return VM::undefined;
  }

  VALUE
    NODE::exist ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;
    bool exist = get_stat ( me->path, st );
    return VALUE ( exist );

  }

#if !defined _WIN32
  void
    splitpath ( string& path, string& dir, string& fname, string& ext )
  {
    int	start_pos, end_pos;

    start_pos = path.last_index_of ( '/' );
    end_pos   = path.last_index_of ( '.' );

    if ( start_pos == -1 && ( end_pos == -1 || end_pos == 0 ) )
    {
      fname = path;
    }
    else if ( start_pos == -1 && end_pos > 0  )
    {
      fname = path.substr ( 0, end_pos );
      ext   = path.substr ( end_pos + 1 );
    }
    else if (  start_pos > 0 &&
             ( end_pos == -1 ||
               end_pos == path.length () - 1
             )
            )
    {
      dir   = path.substr ( 0, start_pos );
      fname = path.substr ( start_pos + 1 );
    }
    else
    {
      dir   = path.substr ( 0, start_pos );
      fname = path.substr ( start_pos + 1, end_pos - start_pos - 1 );
      ext   = path.substr ( end_pos + 1 );
    }
  }

  void
    makepath ( string& path, string& dir, string& fname, string& ext )
  {
    string newpath;

    if ( !dir.is_empty () )
    {
      newpath = dir;
      newpath += '/';
    }

    if ( !fname.is_empty () )
    {
      newpath += fname;
    }

    if ( !ext.is_empty () )
    {
      newpath += '.';
      newpath += ext;
    }

    path = newpath;
  }
#endif

  VALUE
    NODE::name ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

#ifdef _WIN32
    char drive [ _MAX_DRIVE ];
    char dir   [ _MAX_DIR   ];
    char fname [ _MAX_FNAME ];
    char ext   [ _MAX_EXT   ];
    _splitpath ( me->path, drive, dir, fname, ext );

    if ( argc == 1 ) // set
    {
      char path_buffer [ _MAX_PATH ];

      if ( !argv [ 0 ].is_string () )
        error_parameters ();

      _makepath ( path_buffer, drive, dir, string ( argv [ 0 ] ), ext );
      me->path = path_buffer;
    }

    return VALUE ( (const char *) fname );

#else

    string dir;
    string fname;
    string ext;

    splitpath ( me->path, dir, fname, ext );

    if ( argc == 1 )
    {
      if ( !argv [ 0 ].is_string () )
        error_parameters ();

      fname = string ( argv [ 0 ] );
      makepath ( me->path, dir, fname, ext );
    }

    return VALUE ( new STRING ( fname ) );
#endif
  }

  VALUE
    NODE::extension ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

#ifdef _WIN32
    char drive [ _MAX_DRIVE ];
    char dir   [ _MAX_DIR   ];
    char fname [ _MAX_FNAME ];
    char ext   [ _MAX_EXT   ];

    _splitpath ( me->path, drive, dir, fname, ext );

    if ( argc == 0 ) // get
      return VALUE ( (const char *) &ext [ 1 ] );
    else // set
    {
      chktype ( 0, DT_STRING );
      me->path.printf ( "%s%s%s.%s", drive, dir, fname,
                        (const char *) CSTR ( argv [ 0 ].v.v_string ) );
    }

#else

    string dir;
    string fname;
    string ext;

    splitpath ( me->path, dir, fname, ext );

    if ( argc == 0 ) // get
    {
      return VALUE ( new STRING ( ext ) );
    }
    else
    {
      chktype ( 0, DT_STRING );
      ext = CSTR ( argv [ 0 ].v.v_string );
      makepath ( me->path, dir, fname, ext );
    }

#endif

    return argv [ 0 ];
  }

  VALUE
    NODE::name_extension ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

#ifdef _WIN32
    char drive [ _MAX_DRIVE ];
    char dir   [ _MAX_DIR   ];
    char fname [ _MAX_FNAME ];
    char ext   [ _MAX_EXT   ];

    _splitpath ( me->path, drive, dir, fname, ext );

    string ne ( fname );
    ne += ext;
#else
    string dir;
    string fname;
    string ext;
    string ne;

    splitpath ( me->path, dir, fname, ext );

    dir = "";
    makepath ( ne, dir, fname, ext );
#endif

    return VALUE ( new STRING ( ne ) );
  }

  VALUE
    NODE::dir ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

#ifdef _WIN32
    char drive [ _MAX_DRIVE ];
    char dir   [ _MAX_DIR   ];
    char fname [ _MAX_FNAME ];
    char ext   [ _MAX_EXT   ];

    _splitpath ( me->path, drive, dir, fname, ext );

    if ( argc == 1 ) // set
    {
      char path_buffer [ _MAX_PATH ];

      if ( !argv [ 0 ].is_string () )
        error_parameters ();

      _makepath ( path_buffer, drive, string ( argv [ 0 ] ), fname, ext );
      me->path = path_buffer;
    }

    return VALUE ( (const char *) dir );
#else
    string dir;
    string fname;
    string ext;

    splitpath ( me->path, dir, fname, ext );

    if ( argc == 1 )
    {
      if ( !argv [ 0 ].is_string () )
        error_parameters ();

      dir = string ( argv [ 0 ] );
      makepath ( me->path, dir, fname, ext );
    }

    return VALUE ( new STRING ( dir ) );
#endif
  }

  VALUE
    NODE::drive ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

#ifdef _WIN32
    char drive [ _MAX_DRIVE ];
    char dir   [ _MAX_DIR   ];
    char fname [ _MAX_FNAME ];
    char ext   [ _MAX_EXT   ];

    _splitpath ( me->path, drive, dir, fname, ext );

    if ( argc == 1 ) // set
    {
      char path_buffer [ _MAX_PATH ];

      if ( !argv [ 0 ].is_string () )
        error_parameters ();

      _makepath ( path_buffer, string ( argv [ 0 ] ), dir, fname, ext );
      me->path = path_buffer;
    }

    return VALUE ( (const char *) drive );
#else
    return VALUE ( (const char *) "" );
#endif
  }

  VALUE
    NODE::atime ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;

    if ( !get_stat ( me->path, st ) )
      error_ne ( me );

    date_time dt ( st.st_atime );
    return VALUE ( double ( dt.absolute_millis () ) );
  }

  VALUE
    NODE::ctime ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;

    if ( !get_stat ( me->path, st ) )
      error_ne ( me );

    date_time dt ( st.st_ctime );
    return VALUE ( double ( dt.absolute_millis () ) );
  }

  VALUE
    NODE::mtime ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;

    if ( !get_stat ( me->path, st ) )
      error_ne ( me );

    date_time dt ( st.st_mtime );
    return VALUE ( double ( dt.absolute_millis () ) );
  }

  VALUE
    NODE::path ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 )
    { // set
      chktype ( 0, DT_STRING );
      me->path = CSTR ( argv [ 0 ].v.v_string );
    }
    return VALUE ( me->path );
  }

  VALUE
    NODE::is_folder ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;

    if ( !get_stat ( me->path, st ) )
      return VALUE ( false );

    return VALUE ( ( st.st_mode & _S_IFDIR ) != 0 );
  }

  VALUE
    NODE::is_file ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;

    if ( !get_stat ( me->path, st ) )
      return VALUE ( false );

    return VALUE ( ( st.st_mode & _S_IFDIR ) == 0 );
  }

  VALUE
    NODE::nodes ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    struct stat st;

    if ( !get_stat ( me->path, st ) )
      error_ne ( me );
    if ( st.st_mode & _S_IFDIR == 0 )
      error_nd ( me );

    ARRAY *arr = new ARRAY ( 0 );

#ifdef _WIN32
    string wildcard;
    int attributes_include = _A_ARCH | _A_NORMAL | _A_RDONLY | _A_SUBDIR;
    int attributes_exclude = _A_HIDDEN | _A_SYSTEM;

    if ( argc >= 1 )
    {
      chktype ( 0, DT_STRING );
      wildcard = CSTR ( argv [ 0 ].v.v_string );
    }
    else
      wildcard = "*.*";

    if ( argc >= 2 )
    {
      chktype ( 1, DT_INTEGER );
      attributes_include = argv [ 0 ].v.v_integer;
    }

    if ( argc == 3 )
    {
      chktype ( 2, DT_INTEGER );
      attributes_exclude = argv [ 0 ].v.v_integer;
    }

    _finddata_t fdta;
    long hfile;
    string pathname_ = me->path + "\\";
    string pathname = pathname_ + wildcard;
    for ( bool b = ( hfile = _findfirst ( pathname, &fdta ) ) != -1L;
          b;
          b = ( _findnext ( hfile, &fdta ) == 0 ) )
    {
      if (
          ( fdta.attrib & attributes_include ) != 0 &&
          ( fdta.attrib & attributes_exclude ) == 0 &&
            strcmp ( fdta.name, "." ) != 0          &&
            strcmp ( fdta.name, ".." ) != 0
         )
      {
        VALUE v ( new NODE::INSTANCE ( pathname_ + fdta.name ) );
        arr->push ( v );
      }
    }
    _findclose ( hfile );
#endif
    return VALUE ( arr );
  }

  VALUE
    NODE::remove ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    struct stat st;

    if ( !get_stat ( me->path, st ) )
      error_ne ( me );

    string fp = me->path;
#ifdef _WIN32
    fp.replace ( "/", "\\" );
#endif
    if ( ( st.st_mode & _S_IFDIR ) == 0 )
      return ( unlink ( me->path ) == 0 );
    else
      return ( rmdir ( me->path ) == 0 );
  }

  VALUE
    NODE::make_folder ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

#ifdef _WIN32
    int r = mkdir ( me->path );
#else
    int r = mkdir ( me->path, 0644 );
#endif
    if ( r == 0 )
      return 0;

    if ( errno == EEXIST )
      VM::error ( "'%s' already exitst", (const char *) me->path );
    else if ( errno == ENOENT )
      VM::error ( "'%s' path not found", (const char *) me->path );
    else
      VM::error ( "'%s' cannot be created" );

    return VALUE ( false );
  }

  VALUE
    NODE::rename ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    struct stat st;

    if ( get_stat ( me->path, st ) )
      VM::error ( "'%s' does not exitst", (const char *) me->path );

    chktype ( 0, DT_STRING );
    string npath = CSTR ( argv [ 0 ].v.v_string );

    int r = ::rename ( me->path, npath );

    if ( r == 0 )
    {
      me->path = npath;
      return VALUE ( true );
    }
    if ( errno == EEXIST )
      VM::error ( "'%s' already exitst", (const char *) me->path );
    else if ( errno == ENOENT )
      VM::error ( "'%s' path not found", (const char *) me->path );
    else
      VM::error ( "'%s' cannot be created" );

    return VALUE ( false );
  }

  VALUE
    NODE::tostring ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    return me->path;
  }

  VALUE
    NODE::stream ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    struct stat st;
    int flags = sal::file::fo_shared;
    int mode = 0;

    if ( get_stat ( me->path, st ) )
    {
      if ( (st.st_mode & _S_IFDIR ) != 0 )
        error_nf ( me );
    }

    if ( argc != 1 || !argv->is_string () )
      error_parameters ();

    file_stream *fs = new file_stream ();
    fs->name ( me->path );
    if ( !fs->open ( CSTR ( argv [ 0 ].v.v_string ) ) )
    {
      VM::error ( "'%s' not found", (const char *) fs->name () );
    }
    return VALUE ( new STREAM::INSTANCE ( fs, me ) );
  }

  VALUE
    NODE::do_mode_prop ( int argc, VALUE *argv, int mask )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;

    if ( !get_stat ( me->path, st ) )
      error_ne ( me );

    return VALUE ( (st.st_mode & mask ) != 0 );
  }

  VALUE
    NODE::can_read ( int argc, VALUE *argv )
  {
    return do_mode_prop ( argc, argv, S_IREAD );
  }

  VALUE
    NODE::can_write ( int argc, VALUE *argv )
  {
    return do_mode_prop ( argc, argv, S_IWRITE );
  }

  VALUE
    NODE::can_exec ( int argc, VALUE *argv )
  {
    return do_mode_prop ( argc, argv, S_IEXEC );
  }

  VALUE
    NODE::is_pipe ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

    struct stat st;
    if ( !get_stat ( me->path, st ) )
      error_ne ( me );

    return VALUE ( false );
  }

  VALUE
    NODE::is_link ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 ) // set
      error_read_only ();

#ifdef _WIN32
    char drive [ _MAX_DRIVE ];
    char dir   [ _MAX_DIR   ];
    char fname [ _MAX_FNAME ];
    char ext   [ _MAX_EXT   ];

    _splitpath ( me->path, drive, dir, fname, ext );

    return ( strcmp ( ext, ".lnk" ) == 0 );
#else
    struct stat st;

    if ( get_stat ( me->path, st ) )
    {
      return ( ( st.st_mode & S_IFLNK ) == S_IFLNK );
    }
#endif
  }

};
