/*
*
* cs_basic.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_BASIC_H
#define __cs_BASIC_H

#if defined(__GNUC__) && !defined (_GNU_SOURCE )
#define _GNU_SOURCE
#endif

#include <string.h>
#include <iostream.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>

/****************************************************************************/

namespace tool
{

  typedef unsigned char       byte;

#ifdef __GNUC__
  typedef unsigned short    	word;
  typedef unsigned long     	dword;
  typedef unsigned long long  qword;
#else
  typedef unsigned __int16    word;
  typedef unsigned __int32    dword;
  typedef unsigned __int64    qword;
#endif

  typedef wchar_t uchar;

  typedef const uchar * uchar_cptr;

#ifndef NULL
#define NULL 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif


#ifndef min         // Hopefully this isn't already defined as a macro.

  template <class T>
  inline T
    min ( T arg1, T arg2 )
  {
    return ( arg1 < arg2 ) ? arg1 : arg2;
  }

  inline char *
    min ( char *arg1, char *arg2 )
  {
    return ( strcmp ( arg1, arg2 ) < 0 ) ? arg1 : arg2;
  }

#endif

  /****************************************************************************/

#ifndef max         // Hopefully this isn't already defined as a macro.

  template <class T>
  inline T
    max ( T arg1, T arg2 )
  {
    return ( arg2 < arg1 ) ? arg1 : arg2;
  }

  inline char *
    max ( char *arg1, char *arg2 )
  {
    return ( strcmp ( arg2, arg1 ) < 0 ) ? arg1 : arg2;
  }

#endif

  /****************************************************************************/

  template <class T>
  inline void
    swap ( T &arg1, T &arg2 )
  {
    T tmp = arg1;
    arg1 = arg2;
    arg2 = tmp;
  }

  /****************************************************************************/

  inline void
    check_mem ( void *ptr )
  {
    // Declare string as static so that it isn't defined per instantiation
    // in case the compiler actually does decide to inline this code.
    static const char *const mem_err = "Error allocating memory.\n";
    if ( ptr == NULL )
    {
      cerr << mem_err;
      assert ( 0 );
    }
  }

  /****************************************************************************/

#define _TODO_STR( x ) #x
#define TODO_STR( x ) _TODO_STR( x )
#define TODO( desc ) message( __FILE__ "(" TODO_STR( __LINE__ ) ") : warning TODO: " desc )

  class resource
  {
  public:
    resource ()
    {
      _ref_cntr = 0;
    }
    virtual ~resource ()
    {
      assert ( _ref_cntr == 0 );
    }
    //	private:
    unsigned _ref_cntr;
  };

  template <class T>
  class handle
  {
  public:
    handle ()
    {
      _ptr = NULL;
    }

    handle ( T* p )
    {
      _ptr = NULL;
      _set ( p );
    }

    handle ( const handle<T>& p )
    {
      _ptr = NULL;
      _set ( p._ptr );
    }

    ~handle ()
    {
      _set ( NULL );
    }

    handle<T>&
      operator= ( T *p )
    {
      _set ( p );
      return *this;
    }

    handle<T>&
      operator= ( const handle<T>& p )
    {
      _set ( p._ptr );
      return *this;
    }

    T*
      operator-> () const
    {
      return _ptr;
    }

    operator T* () const
    {
      return _ptr;
    }

    bool
      is_null () const
    {
      return _ptr == NULL;
    }

    //	private:
    T* _ptr;
    void
      _set ( T *p )
    {
      if ( _ptr == p )
        return;
      if ( _ptr )
      {
        _ptr->_ref_cntr--;
        if ( _ptr->_ref_cntr == 0 )
          delete _ptr;
      }
      _ptr = p;
      if ( _ptr )
        _ptr->_ref_cntr++;
    }
  };

  template <class c_key>
  unsigned int
    hash ( const c_key &the_key ); 

};

#endif
