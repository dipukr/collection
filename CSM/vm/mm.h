/*
*
* mm.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __mm_h
#define __mm_h
#include <malloc.h>
#include "task.h"

extern bool memory_active;

namespace c_smile
{
#define MARK	1
#define ALLOCSIZE(x)  ( ( ( x ) + sizeof ( MEMORY::AUNIT ) - 1 ) / sizeof ( MEMORY::AUNIT ) )

  class VM;

  class MEMORY //memory manager
  {
    friend class VM;
  private:

    struct block
    {
      struct block *  next;	      // next block
      unsigned char   data [ 1 ];	// block data
    };

    block*            blocks;

    sal::event        notification;
    sal::mutex        guard;
    sal::mutex        gc_guard;
    volatile size_t   allocated;

    unsigned int      gc_threshold;

  public:

    volatile bool     gc_active;

    MEMORY () : blocks ( 0 ), gc_active ( false ), gc_threshold ( 0xFFFFFF )
    {
    }

    ~MEMORY  ()
    {
    }

    void *			alloc_thing ( size_t size );
    size_t      gc ();

    void        not_enough_memory  ()
    {
    }

  };

  extern MEMORY memory;

};

extern size_t other_allocs;

#endif
