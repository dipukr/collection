/*
*
* mm.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

// memory manager

#include <stdlib.h>
#include <setjmp.h>
#include "c-smile.h"
#include "vm.h"
#include "mm.h"
#include "compiler.h"


bool memory_active = false;

namespace c_smile
{
  MEMORY    memory;
  static int	getblocksize ( THING *hdr );

  void
    VM::mark ()
  {
    // mark current bytecode vectors
    mark_thing ( code );
    mark_thing ( native_code );

    if ( running )
      mark_thing ( thread );

    int i;

    // mark error handlers
    for ( i = 0; i <= eh_pos; ++i )
    {
      mark_thing ( eh [ i ].code );
      eh [ i ].thrown.mark ();
    }

    // mark synchro stack
    for ( i = 0; i <= m_stack_pos; ++i )
      mark_thing ( m_stack [ i ] );

    // mark the stack
    for ( VALUE *p = stkbase; p <= sp; p++ )
      p->mark ();

    t_register.mark ();

    // mark the queue
    queue.mark ();

    mark_thing ( package );
  }


  void
    VM::mark_all ()
  {
    iterator<VM *> it ( all );
    foreach ( it ) it.current()->mark ();

    //|
    //| mark perimeter - all reachable values
    //|
    mark_thing ( std );
    mark_thing ( packages );
    mark_std_classes ();

  }

  // gc - do garbage collect
  size_t
    MEMORY::gc ()
  {
    sal::critical_section cs ( gc_guard );
    if ( gc_active )
      return size_t ( -1 );

    //
    if ( memory.allocated + other_allocs < memory.gc_threshold )
      return size_t ( -1 ); // don't need to

#ifdef SHOW_GC
    VM::info ( "\nGC, started\n" );
#endif
    // wait all writers to complete

    gc_active = true;
    bool not_ready = true;

    while ( not_ready )
    {
      sal::critical_section cs_vms ( VM::all_guard );
      not_ready = false;
      iterator<VM *> it ( VM::all );
      foreach ( it  )
      {
#ifdef SHOW_GC
        VM::info ( "\nGC, waiting for thread to stop allocations %d\n",
                   it.current()->running );
#endif
        if ( !it.current () ->ready_for_gc )
        {
#ifdef SHOW_GC
          VM::info ( "\nGC, before wait..." );
#endif
          notification.wait ();
#ifdef SHOW_GC
          VM::info ( "after\n" );
#endif
          not_ready = true;
          break;
        }
      }
    }
#ifdef SHOW_GC
    VM::info ( "\nGC, step 1\n" );
#endif

    // all clear, proceed with gc
    // mark
    size_t total = 0;
    size_t total_in_use = 0;
    size_t allocated_items = 0;
    size_t items_collected = 0;

    block *bp;
    block **bpp;
    {
      sal::critical_section cs ( guard );
      sal::critical_section cs_vms ( VM::all_guard );

      VM::mark_all ();

      // and sweep
      bp = blocks;
      bpp = &blocks;

      while ( bp )
      {
        allocated_items++;
        if ( ( (THING *) bp->data )->marked () )
        {
          *bpp = bp;
          bpp = &bp->next;
          ( (THING *) bp->data )->clear_mark ();
          total_in_use += ( (THING *) bp->data )->allocated_size ();
          bp = bp->next;
        }
        else
        {
          block *tp = bp;
          bp = bp->next;
          total += ( (THING *) tp->data )->allocated_size ();
          ( (THING *) tp->data )->~THING ();
          free ( tp );
          items_collected ++;
        }
      }
      *bpp = 0;
    }
#ifdef SHOW_GC
    VM::info ( "\nGC, total items %d, items collected %d\n", allocated_items,
                                                             items_collected );
    VM::info ( "GC, total mem in use by c-smile objects %d bytes, mem collected %d bytes\n",
               total_in_use, total );
    VM::info ( "GC, other allocs from last gc %d bytes\n", other_allocs );
#endif
    other_allocs = 0;
    allocated = 0;
    gc_active = false;
    return total; // for a while
  }

  //|
  //| Memory manager functions
  //|

  // allocmemory - allocate a block of memory
  void *
    MEMORY::alloc_thing ( size_t size )
  {
    VM *vm = VM::current ();
    if ( vm )
    {
      if ( vm->ready_for_gc )
      {
        sal::critical_section cs ( gc_guard );
        vm->ready_for_gc = false;
      }
    }
    {
      sal::critical_section cs ( guard );

      block * b = (block*) calloc ( 1, sizeof (struct block *) + size );
      if ( b == NULL )
        printf ( "not enough memory!" );

      allocated += size;

      b->next = blocks;
      blocks = b;
      return b->data;
    }
  }

};
// namespace c_smile

size_t other_allocs = 0;

void *
  operator new ( size_t s )
{
  other_allocs += s;
  return ( malloc ( s ) );

}

void
  operator delete ( void * m )
{
  if ( m )
    free ( m );
}

