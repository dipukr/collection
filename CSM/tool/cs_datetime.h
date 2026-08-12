/*
*
* cs_datetime.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_datetime_h
#define __cs_datetime_h

#include <time.h>
#include "cs_string.h"

namespace tool
{
#ifdef _WIN32
  typedef __int64             datetime_t;
#define ZEROTIME  0
#else
#if defined(__osf__ )
  typedef signed   long       datetime_t;
#define ZEROTIME  0L
#else
#if defined(__GNUC__)
  typedef signed   long long  datetime_t;
#define ZEROTIME  0LL
#else
#error "integer 8 byte type is not defined"
#endif
#endif
#endif

  class date_time
  {
  public:
    static const date_time now ();
  public:

    date_time ();
    date_time ( const date_time &dt );
    date_time ( const datetime_t dt );

    date_time ( const time_t time );
    date_time (int year, int month, int day,
    int hours, int minutes, int seconds,
    int milli = 0, int micro = 0, int nano = 0 );

    // operators
  public:
    const date_time& operator= ( const date_time& dt );
    const date_time& operator= ( const datetime_t dts );
    const date_time& operator= ( const struct tm& syst );

    bool operator== ( const date_time& date ) const;
    bool operator!= ( const date_time& date ) const;
    bool operator< ( const date_time& date ) const;
    bool operator> ( const date_time& date ) const;
    bool operator<= ( const date_time& date ) const;
    bool operator>= ( const date_time& date ) const;

    operator datetime_t () const
    {
      return _time;
    }
    
    operator string () const;

    // Operations
  public:
    bool set ( int year, int month, int day,
    int hours, int minutes, int seconds,
    int milli = 0, int micro = 0, int nano = 0 );
    bool set_date ( int year, int month, int day );
    bool set_time ( int hours, int minutes, int seconds );
    bool set_frac_time ( int millis, int micros, int nanos );

    bool systemtime ( struct tm& syst );

    // getters
    int year    () const;
    int month   () const;     // month of year (1 = Jan)
    int day     () const;     // day of month (0-31)
    int hours   () const;     // hour in day (0-23)
    int minutes () const;     // minute in hour (0-59)
    int seconds () const;     // second in minute (0-59)
    int millis  () const;     // millisecond in minute (0-999)
    int micros  () const;     // microsecond in minute (0-999)
    int nanos   () const;     // nanosecond in minute (0-999), step of 100ns
    int day_of_week () const; // 1=Sun, 2=Mon, ..., 7=Sat
    int day_of_year () const; // days since start of year, Jan 1 = 1

    // setters
    void nanos   ( int nv );
    void micros  ( int nv );
    void millis  ( int nv );
    void seconds ( int nv );
    void minutes ( int nv );
    void hours   ( int nv );
    void day     ( int nv );
    void month   ( int nv );
    void year    ( int nv );

    datetime_t  absolute_millis ();
    void        absolute_millis ( datetime_t t );

    // formatting
    // ....

  public:
    datetime_t         _time;

  private:
    struct datetime_s
    {
      int  year;
      unsigned int month;
      unsigned int day;
      unsigned int hour;
      unsigned int minute;
      unsigned int second;
      unsigned int millis;
      unsigned int micros;
      unsigned int nanos;
      unsigned int day_of_year;
      unsigned int day_of_week;
    };
    static int month_day_in_year [ 13 ];

    static bool cvt ( datetime_t& dst, const datetime_s& src );
    static bool cvt ( datetime_s& dst, const datetime_t& src );
    static bool cvt (  struct tm& dst, const datetime_s& src );
    static bool cvt ( datetime_s& dst, const  struct tm& src );

    //friend class date_time_span;
  };


  inline
    date_time::date_time () : _time ( ZEROTIME )
  {
    ;
  }  // 1 Jan-1601, 00:00, 100ns clicks

  inline
    date_time::date_time ( const date_time &src )
  {
    _time = src._time;
  }

  inline
    date_time::date_time ( datetime_t t )
  {
    _time = t;
  }

  inline
    date_time::date_time ( const time_t timeSrc )
  {
    struct tm _tm = *( localtime ( &timeSrc ) );
    *this = _tm;
  }

  inline const
    date_time& date_time::operator= ( const date_time& src )
  {
    _time = src._time;
    return *this;
  }

  inline const
    date_time& date_time::operator= ( const datetime_t src )
  {
    _time = src;
    return *this;
  }

  inline bool
    date_time::set_date ( int year, int month, int day )
  {
    return set ( year, month, day, 0, 0, 0, 0, 0, 0 );
  }

  inline bool
    date_time::set_time ( int hour, int min, int sec )
  {
    return set ( 1601, 1, 1, hour, min, sec, 0, 0, 0 );
  }

  inline bool
    date_time::set_frac_time ( int millis, int micros, int nanos )
  {
    return set ( 1601, 1, 1, 0, 0, 0, millis, micros, nanos );
  }

  inline bool
    date_time::operator== ( const date_time& date ) const
  {
    return ( _time == date._time );
  }

  inline bool
    date_time::operator!= ( const date_time& date ) const
  {
    return ( _time != date._time );
  }

  inline bool
    date_time::operator< ( const date_time& date ) const
  {
    return ( _time < date._time );
  }

  inline bool
    date_time::operator> ( const date_time& date ) const
  {
    return ( _time > date._time );
  }

  inline bool
    date_time::operator<= ( const date_time& date ) const
  {
    return ( _time <= date._time );
  }

  inline bool
    date_time::operator>= ( const date_time& date ) const
  {
    return ( _time >= date._time );
  }

};

#endif
