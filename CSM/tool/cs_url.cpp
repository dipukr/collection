/*
*
* cs_url.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include "cs_url.h"
#include <ctype.h>

#if !defined(_WIN32)
#define strnicmp	strncasecmp
#endif


namespace tool
{
  struct protoport
  {
    const char *proto;
    int         port;
  };

  static protoport protoports[] =
  {
    { "ftp",    21   },
    { "gopher", 70   },
    { "http",   80   },
    { "https",  443  },
    { "socks",  1080 } 
  };


  /*
  * ParseURL
  *
  * Turns a URL into a URLParts structure
  *
  * The good stuff was written by Rob May <robert.may@rd.eng.bbc.co.uk>
  * and heavily mangled/modified by john to suit his own weird style.
  * Made somewhat smarter (err, completely re-written) by GN 1997May02
  */
  bool
    url::parse ( const char * src )
  {
    const char *s, *t;
    char *fragmark;             /* '#' fragment marker if any */
    /* NB Fragments  (which the chimera source calls 'anchors' are part
    * of HTML href's but _not_ properly speaking of URLs;  they are handled
    * entirely at the client end and not by the server.
    * Nevertheless we look for them  (this routine should really be called
    * ParseHREF)  and store a fragment identifier separately if we find one.
    * --GN
    */

    /* RFC1738 says spaces in URLs are to be ignored -- GN 1997May02 */
    array<char> buffer;

    //t = start = buffer;
    for ( s = src; *s; s++ )
      if ( !isspace ( *s ) )
        buffer.push ( *s );
    buffer.push ( '\0' );

    char *start = &buffer [ 0 ];

    /* Lousy hack for URNs */
    if ( strnicmp ( start, "urn:", 4 ) == 0 )
    {
      protocol = "urn";
      filename = &buffer [ 4 ];
      return true;
    }
    /* Less lousy hack for URLs which say so */
    if (strnicmp(start, "url:", 4) == 0)
      s = start + 4;
    else
      s = start;

    /*
    * Check to see if there is a protocol (scheme) name.
    * Matches /^[A-Za-z0-9\+\-\.]+:/ in PERLese.
    */
    for ( t = s; *t; t++ )
    {
      if ( !isalnum ( *t ) && *t != '-' && *t != '+' && *t != '.' )
        break;
    }
    if ( *t == ':' )
    {
      protocol = string ( s, t - s );
      s = ++t;
    }
    /*
    * Check whether this is an 'Internet' URL i.e. the next bit begins
    * with "//".  In this case, what follows up to the next slash ought
    * to parse as "//user:passwd@host.dom.ain:port/" with almost every
    * component optional, and we'll continue later with s pointing at the
    * trailing slash.  If there is no further slash, we'll add one and
    * return.-- None of the fields are supposed to contain any visible
    * (unencoded)  colons, slashes or atsigns.
    */
    if ( s [ 0 ] == '/'  &&  s [ 1 ] == '/' )  /* looking at "//" */
    {
      char *atsign;             /* if present, user:passwd precedes it */
      char *colon;              /* colon separators after user or host */
      char *tslash;             /* trailing slash */

      s += 2;
      tslash = strchr ( s, '/' );
      if ( tslash != NULL )
        *tslash = '\0';         /* split the string, we'll undo this later */

      atsign = strchr ( s, '@' );

      if ( atsign != NULL )     /* a username is present, possibly empty */
      {
        *atsign = '\0';         /* split the string again */
        colon = strchr ( s, ':' );

        if ( colon != NULL )      /* a passwd is also present */
        {
          *colon = '\0';
          password = atsign + 1;
        }
        username = s;
        s = atsign + 1;
      }

      colon = strchr ( s, ':' );

      if ( colon != NULL )        /* a port is specified */
      {
        *colon = '\0';
        port = atoi ( colon + 1 );
      }

      hostname = s;

      if ( tslash == NULL )       /* nothing further */
      {
        filename = "/";
        goto fillport;
      }
      *tslash = '/';	/* restore the slash */
      s = tslash;			/* and stay there, don't step beyond */
    }

    /*
    * End of special treatment of Internet URLs.  Now s points at what
    * chimera calls the filename part  (if any).
    */
    fragmark = strchr ( s, '#' );

    if ( fragmark != NULL )
    {
      *fragmark = '\0';
      anchor = fragmark + 1;
    }

    filename = s;  /* everything else goes here */

  fillport:
    if ( port == 0 )
    {

      for ( int i = 0; i < sizeof ( protoports ) / sizeof ( protoport ); i++ )
      if ( stricmp ( protoports [ i ].proto, protocol ) == 0 )
      {
        port = protoports [ i ].port;
        break;
      }
    }
    return true;
  }


  /*
  * escape URL
  *
  * Puts escape codes in URLs.  (More complete than it used to be;
  * GN Jan 1997.  We escape all that isn't alphanumeric, "safe" or "extra"
  * as spec'd in RFCs 1738, 1808 and 2068.)
  */
  bool
    is_url_char ( unsigned char c )
  {
    if ( c > 128 )
      return false;
    if ( isalnum ( c ) )
      return true;
    if ( strchr ( "$-_.!*'(),", c ) )
      return true;
    return false;
  }


  string
    url::escape ( const char *src, bool space_to_plus )
  {
    const char *cp;
    static char *hex = "0123456789ABCDEF";

    array<char> buffer;

    for ( cp = src; *cp; cp++ )
    {
      if ( *cp == ' ' && space_to_plus )
      {
        buffer.push ( '+' );
      }
      else if ( is_url_char ( (unsigned char) *cp ) || ( *cp == '+' && !space_to_plus ) )
      {
        buffer.push ( *cp );
      }
      else
      {
        buffer.push ( '%' );
        buffer.push ( hex [ (unsigned char) *cp / 16 ] );
        buffer.push ( hex [ (unsigned char) *cp % 16 ] );
      }
    }

    buffer.push ( '\0' );
    return string ( &buffer [ 0 ] );
  }

  /*
  * UnescapeURL
  *
  * Converts the escape codes (%xx) into actual characters.  NOT complete.
  * Could do everthing in place I guess.
  */
  string url::unescape ( const char *src )
  {
    const char *cp;
    char hex [ 3 ];

    array<char> buffer;

    for  ( cp = src; *cp; cp++ )
    {
      if ( *cp == '%' )
      {
        cp++;
        if ( *cp == '%' )
          buffer.push ( *cp );
        else
        {
          hex [ 0 ] = *cp;
          cp++;
          hex [ 1 ] = *cp;
          hex [ 2 ] = '\0';
          buffer.push ( (char) strtol ( hex, NULL, 16 ) );
        }
      }
      else
        buffer.push ( *cp );
    }

    buffer.push ( '\0' );
    return ( &buffer [ 0 ] );
  }
};
