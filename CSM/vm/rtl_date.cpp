/*
*
* rtl_date.cpp
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
#include "arithmetic.h"
#include "rtl.h"
#include <time.h>
#include "cs_datetime.h"

namespace c_smile
{
  CLASS *DATE::INSTANCE::klass = 0;

  VALUE
    DATE::ctor ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = new INSTANCE ();
    if ( argc == 0 )
    {
      time_t t;
      ::time ( &t );
      me->_dt = *localtime ( &t );
      return VALUE ( me );
    }

    int year = 1;
    int month = 1;
    int day = 1;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int millis = 0;

    chktype ( 0, DT_INTEGER );
    year = int ( argv [ 0 ] );
    if ( argc > 1 )
    {
      chktype ( 1, DT_INTEGER );
      month = int ( argv [ 1 ] );
    }
    if ( argc > 2 )
    {
      chktype ( 2, DT_INTEGER );
      day = int ( argv [ 2 ] );
    }
    if ( argc > 3 )
    {
      chktype ( 3, DT_INTEGER );
      hour = int ( argv [ 3 ] );
    }
    if ( argc > 4 )
    {
      chktype ( 4, DT_INTEGER );
      minute = int ( argv [ 4 ] );
    }
    if ( argc > 5 )
    {
      chktype ( 5, DT_INTEGER );
      second = int ( argv [ 5 ] );
    }
    if ( argc > 6 )
    {
      chktype ( 6, DT_INTEGER );
      millis = int ( argv [ 6 ] );
    }

    if ( !me->_dt.set ( year, month, day, hour, minute, second, millis ) )
      VM::error ( "invalid date" );
    return VALUE ( me );
  }

  VALUE
    DATE::tostring ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = ( DATE::INSTANCE * ) _this_.v.v_thing;
    return VALUE ( me->_dt.operator string () );
  }

  VALUE
    DATE::day ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = ( DATE::INSTANCE * ) _this_.v.v_thing;

    if ( argc )
    {
      if ( !argv [ 0 ].is_int () )
        error_parameters ();
      me->_dt.day ( int ( argv [ 0 ] ) );
    }
    return me->_dt.day ();
  }

  VALUE
    DATE::month ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;

    if ( argc )
    {
      if ( !argv [ 0 ].is_int () )
        error_parameters ();
      me->_dt.month ( int ( argv [ 0 ] ) );
    }
    return me->_dt.month ();
  }

  VALUE
    DATE::year ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;

    if ( argc )
    {
      if ( !argv [ 0 ].is_int () )
        error_parameters ();
      me->_dt.year ( int ( argv [ 0 ] ) );
    }
    return me->_dt.year ();
  }

  VALUE
    DATE::day_of_week ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;
    if ( argc )
      error_read_only ();
    return me->_dt.day_of_week ();
  }

  VALUE
    DATE::day_of_year ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = ( DATE::INSTANCE * ) _this_.v.v_thing;
    if ( argc )
      error_read_only ();
    return me->_dt.day_of_year ();
  }

  VALUE
    DATE::hours ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;

    if ( argc )
    {
      if ( !argv [ 0 ].is_int () )
        error_parameters ();
      me->_dt.hours ( int ( argv [ 0 ] ) );
    }
    return me->_dt.hours ();
  }

  VALUE
    DATE::minutes ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;

    if ( argc )
    {
      if ( !argv [ 0 ].is_int () )
        error_parameters ();
      me->_dt.minutes ( int ( argv [ 0 ] ) );
    }
    return me->_dt.minutes ();
  }

  VALUE
    DATE::seconds ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;

    if ( argc )
    {
      if ( !argv [ 0 ].is_int () )
        error_parameters ();
      me->_dt.seconds ( int ( argv [ 0 ] ) );
    }

    return me->_dt.seconds ();
  }

  VALUE
    DATE::abstime ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = ( DATE::INSTANCE * ) _this_.v.v_thing;

    if ( argc )
    {
      if ( !argv [ 0 ].is_number () )
        error_parameters ();
      me->_dt.absolute_millis ( datetime_t ( double ( argv [ 0 ] ) ));
    }

    return VALUE ( (double) me->_dt.absolute_millis () );
  }

  VALUE
    DATE::cast ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = new DATE::INSTANCE ();
    if ( argc == 0 || argc > 1 )
      return ctor ( argc, argv );
    if ( argv [ 0 ].is_int () ) // number of days
      me->_dt.absolute_millis ( (datetime_t ) ( int ( argv [ 0 ] ) * 1000 * 60 * 24 ) );
    else if ( argv [ 0 ].is_float () )
      me->_dt.absolute_millis ( (datetime_t ) double ( argv [ 0 ] ) );
    else error_parameters ();
    return VALUE ( me );
  }

  // ( a ) If the number of the year is divisible by 4 then the year is a Leap Year, except
  // ( b ) If the number of the year ends in 00 then the year is not a leap year; however
  // ( c ) If the number of the year ends in 00 and the year is divisible by 400 the year is a Leap Year.

  VALUE
    DATE::is_leap ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;
    int year = me->_dt.year ();
    bool is_leap_year = ( ( year & 3 ) == 0 ) &&
                        ( ( year % 100 ) != 0 ||
                        ( year % 400 ) == 0 );
    return is_leap_year;
  }


  VALUE
    DATE::format ( int argc, VALUE *argv )
  {
    DATE::INSTANCE *me = (DATE::INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 && argv [ 0 ].is_string () )
    {
      struct tm syst;
      me->_dt.systemtime ( syst );
      char buffer [ 1024 ];

      buffer [
        strftime ( buffer, 1024, CSTR ( argv [ 0 ].v.v_string ), &syst )
             ] = 0;

      return VALUE ( new STRING ( buffer ) );
    }
    else
      error_parameters ();

    return VM::undefined;
  }

};
