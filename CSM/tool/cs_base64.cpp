/*
*
* cs_base64.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include "cs_basic.h"
#include "cs_array.h"
#include "cs_string.h"

namespace tool
{
  //
  // code characters for values 0..63
  //
  static char alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

  //
  // lookup table for converting base64 characters to value in range 0..63
  //
  static signed char codes [ 256 ];
  static bool codes_empty = true;

  /**
  * returns an array of base64-encoded characters to represent the
  * passed data array.
  *
  * @param data the array of bytes to encode
  * @return base64-coded character string.
  */
  string
    base64_encode ( const byte* data, int data_length )
  {
    int     out_length = ( ( data_length + 2 ) / 3 ) * 4;
    string  out ( '\0', out_length );
    //
    // 3 bytes encode to 4 chars.  Output is always an even
    // multiple of 4 characters.
    //
    for ( int i = 0, index = 0; i < data_length; i += 3, index += 4 )
    {
      bool quad = false;
      bool trip = false;

      int val = ( 0xFF & (int) data [ i ] );
      val <<= 8;
      if ( ( i + 1 ) < data_length )
      {
        val |= ( 0xFF & (int) data [ i + 1 ] );
        trip = true;
      }
      val <<= 8;
      if ( ( i + 2 ) < data_length )
      {
        val |= ( 0xFF & (int) data [ i + 2 ] );
        quad = true;
      }
      out [ index + 3 ] = alphabet [ ( quad ? ( val & 0x3F ) : 64 ) ];
      val >>= 6;
      out [ index + 2 ] = alphabet [ ( trip ? ( val & 0x3F ) : 64 ) ];
      val >>= 6;
      out [ index + 1 ] = alphabet [ val & 0x3F ];
      val >>= 6;
      out [ index + 0 ] = alphabet [ val & 0x3F ];
    }
    return out;
  }

  /**
  * Decodes a BASE-64 encoded stream to recover the original
  * data. White space before and after will be trimmed away,
  * but no other manipulation of the input will be performed.
  *
  * As of version 1.2 this method will properly handle input
  * containing junk characters (newlines and the like) rather
  * than throwing an error. It does this by pre-parsing the
  * input and generating from that a count of VALID input
  * characters.
  **/
  bool
    base64_decode ( const char *data, int data_length, array<byte>& out )
  {
    int i;
    if ( codes_empty )
    {
      for ( i = 0; i < 256; i++ )
        codes [ i ] = -1;
      for ( i = 'A'; i <= 'Z'; i++ )
        codes [ i ] = (byte) (      i - 'A' );
      for ( i = 'a'; i <= 'z'; i++ )
        codes [ i ] = (byte) ( 26 + i - 'a' );
      for ( i = '0'; i <= '9'; i++ )
        codes [ i ] = (byte) ( 52 + i - '0' );
      codes [ '+' ] = 62;
      codes [ '/' ] = 63;
      codes_empty = false;
    }

    // as our input could contain non-BASE64 data (newlines,
    // whitespace of any sort, whatever) we must first adjust
    // our count of USABLE data so that...
    // (a) we don't misallocate the output array, and
    // (b) think that we miscalculated our data length
    //     just because of extraneous throw-away junk

    int tempLen = data_length;
    int ix;
    for( ix = 0; ix < data_length; ix++ )
    {
      if ( codes [ data [ ix ] ] < 0 )  //(data[ix] > 255) ||
        --tempLen;    // ignore non-valid chars and padding
    }
    // calculate required length:
    //  -- 3 bytes for every 4 valid base64 chars
    //  -- plus 2 bytes if there are 3 extra base64 chars,
    //     or plus 1 byte if there are 2 extra.

    int len = ( tempLen / 4 ) * 3;
    if ( ( tempLen % 4 ) == 3 )
      len += 2;
    if ( ( tempLen % 4 ) == 2 )
      len += 1;

    //byte *out = new byte [ len ];
    //memset ( out, 0, len );
    byte zero = 0;
    out.size ( len );

    int shift = 0;   // # of excess bits stored in accum
    int accum = 0;   // excess bits
    int index = 0;

    // we now go through the entire array (NOT using the 'tempLen' value)
    for ( ix = 0; ix < data_length; ix++ )
    {
      int value = ( data [ ix ] > 255 ) ? -1 : codes [ data [ ix ] ];

      if ( value >= 0 )           // skip over non-code
      {
        accum <<= 6;            // bits shift up by 6 each time thru
        shift += 6;             // loop, with new bits being put in
        accum |= value;         // at the bottom.
        if ( shift >= 8 )       // whenever there are 8 or more shifted in,
        {
          shift -= 8;         // write them out (from the top, leaving any
          out [ index++ ] =      // excess at the bottom for next iteration.
            (byte) ( ( accum >> shift ) & 0xff );
        }
      }
      // we will also have skipped processing a padding null byte ('=') here;
      // these are used ONLY for padding to an even length and do not legally
      // occur as encoded data. for this reason we can ignore the fact that
      // no index++ operation occurs in that special case: the out[] array is
      // initialized to all-zero bytes to start with and that works to our
      // advantage in this combination.
    }

    // if there is STILL something wrong we just have to return false
    return ( index == out.size () );
  }

};
