/*
*
* cs_url.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_url_h
#define __cs_url_h

#include "cs_string.h"
#include "cs_dictionary.h"

namespace tool
{
  class url
  {
    /*
    * protocol://username:password@hostname:port/filename#anchor
    */
  public:
    string protocol;
    string hostname;
    int    port;
    string filename;
    string anchor;

    string                     method;
    dictionary<string, string> attributes;

    string data_type;
    string auth_type;

    string username;
    string password;

  public:
    url () : port ( 0 )
    {
    }

    ~url ()
    {
    }

    bool parse ( const char * src );
    static string escape ( const char *url, bool space_to_plus = true );
    static string unescape ( const char *src );

  };

};

#endif
