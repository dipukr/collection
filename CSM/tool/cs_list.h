/*
*
* cs_list.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

/*
*
* This code derived from kplib
* ( Keith Pomakis, http://www.pomakis.com/~pomakis/kplib/ )
*
*/

#ifndef __cs_LIST_H
#define __cs_LIST_H

#include <iostream.h>
#include <stdlib.h>
#include "cs_basic.h"

namespace tool
{
  template <class element> class const_iterator;
  template <class element> class iterator;

#pragma warning(disable: 4284)

  // assumes element has a default constructor and operator= ().
  template <class element>
  class list
  {
    friend class const_iterator<element>;
    friend class iterator<element>;

  public:

    list ();
    list ( const list<element> &the_list );
    list ( const element &element );
    virtual ~list ();
    list<element> operator+ ( const list<element> &the_list ) const;
    list<element> operator+ ( const element &element ) const;
    void operator+= ( const list<element> &the_list );
    void operator+= ( const element &element );
    list<element> &operator= ( const list<element> &the_list );
    list<element> &operator= ( const element &element );
    element &head ();
    element &tail ();
    const element &head () const;
    const element &tail () const;
    void add_to_head ( const element &element );
    void add_to_tail ( const element &element );
    void add_to_tail ();
    void remove_head ();
    void remove_tail ();
    element &operator[] ( int index );
    const element &operator[] ( int index ) const;
    int size () const;
    bool is_empty () const;
    void clear ();
    list<element> all_such_that ( bool ( *f ) ( const element & ) ) const;

  protected:
    struct node
    {
      element _element;
      node *	_previous;
      node *	_next;
    };
    //static void error(const char *msg);
    int my_size;
    unsigned int my_iterator_count;
    node *my_head;
    node *my_tail;
  };

  // a list keeps track of the number of iterators iterating over it.  rules:
  //
  //    - if a list has no iterators, one can add to and remove from that
  //      list without restriction.
  //
  //    - if a list has one iterator, one can add to and remove elements from
  //      the list through the iterator without restriction, although one
  //      cannot remove elements from the list directly.
  //
  //    - if a list has more than one iterator, one cannot remove elements
  //      from the list at all.
  //
  // as a result, if a list goes out of scope while there is still an
  // iterator attached to it, this is an error.  usually the declaration of
  // an iterator appears after the declaration of the list it is to iterate
  // over (or it is in a more local scope), so this is usually not a problem
  // (since it's destructor is called first, detaching it implicitly).  to
  // detach an iterator from a list explicitly, call iterate_over().

  // use const_iterator if you wish to iterate over a const list or if you
  // do not intend to modify the list through the iterator.

  template <class element>
  class const_iterator
  {
  public:
    enum error_type
    {
      no_current_el = 1,
      already_deleted = 2,
      more_iterators_exist = 3
    };
    class error
    {
    protected:
      error_type    _type;
    public:
      error ( error_type et ) : _type ( et )
      {
      }
    };

  public:
    const_iterator ();
    const_iterator ( const list<element> &the_list );
    const_iterator ( const const_iterator<element> &the_iterator );
    ~const_iterator ();
    const_iterator<element> &iterate_over ( const list<element> &the_list );
    const_iterator<element> &iterate_over ();
    const_iterator<element> &
                   operator= ( const const_iterator<element> &the_iterator);
    const element &current () const;
    const element *ptr () const;
    const_iterator<element> &beginning ();
    const_iterator<element> &end ();
    const_iterator<element> &operator++ ();  // prefix
    const_iterator<element> &operator-- ();  // prefix
    void operator++ ( int );                 // postfix
    void operator-- ( int );                 // postfix
    const element &operator* () const;
    const element *operator-> () const;
    bool at_beginning () const;
    bool at_end () const;
    int size () const;
    bool is_empty () const;
    const list<element>& the_list () const;
  protected:
    const list<element> *my_list;
    const list<element>::node *my_current;
  };

  // the rules defining what happens when odd combinations of element
  // removals and insertions are performed are fairly intuitive.  an element
  // can be deleted while the list is being parsed, and the parse can
  // continue.  "previous" and "next" retain their meanings.  just don't try
  // to access an element after it has been deleted!

  template <class element>
  class iterator
  {
  public:
    enum error_type
    {
      no_current_el = 1,
      already_deleted = 2,
      more_iterators_exist = 3
    };
    class error
    {
    protected:
      error_type    _type;
    public:
      error ( error_type et ) : _type ( et )
      {
      }
    };

  public:
    iterator ();
    iterator ( list<element> &the_list );
    iterator ( const iterator<element> &the_iterator );

    ~iterator ();

    iterator<element> &iterate_over ( list<element> &the_list );
    iterator<element> &iterate_over ();
    iterator<element> &operator= ( const iterator<element> &the_iterator );
    iterator<element> &insert_before_current ( const element &element );
    iterator<element> &insert_after_current ( const element &element );
    iterator<element> &replace_current_with ( const element &element );

    element &current ();
    element *ptr ();
    void remove_current ();
    iterator<element> &beginning ();
    iterator<element> &end ();
    iterator<element> &operator++ ();  // prefix
    iterator<element> &operator-- ();  // prefix
    void operator++ ( int );           // postfix
    void operator-- ( int );           // postfix

    element &operator* ();
    element *operator-> ();

    bool at_beginning () const;
    bool at_end () const;

    int size () const;
    bool is_empty () const;
    list<element>& the_list ();
  protected:
    //static void error(const char *msg);
    list<element> *my_list;
    list<element>::node my_deleted;
    list<element>::node *my_current;
  };

  // the following macro is defined as a convenience.  here is an example of
  // its usage:
  //
  //     sortable_list<int> intlist;
  //     intlist += 42;
  //     intlist += 11;
  //     intlist += 76;
  //     intlist += 9;
  //     intlist.sort();
  //
  //     const_iterator<int> iter(intlist);
  //     foreach(iter) cout << *iter << '\n';

#define foreach(iter) for ( iter.beginning(); iter.ptr(); iter++ )
#define foreach_backward(iter) for ( iter.end(); iter.ptr(); iter-- )


  /****************************************************************************/
  template <class element>
  inline
    list<element>::list ()
  {
    my_size = my_iterator_count = 0;
    my_head = my_tail = NULL;
  }


  /****************************************************************************/
  template <class element>
  inline
    list<element>::list ( const list<element> &the_list )
  {
    my_size = my_iterator_count = 0;
    my_head = my_tail = NULL;
    *this = the_list;
  }


  /****************************************************************************/
  template <class element>
  inline
    list<element>::list ( const element &the_element )
  {
    my_size = my_iterator_count = 0;
    my_head = my_tail = NULL;
    *this = the_element;
  }


  /****************************************************************************/
  template <class element>
  inline
    list<element>::~list ()
  {
    assert ( my_iterator_count == 0 );
    //cannot destroy, iterators present
    clear ();
  }


  /****************************************************************************/
  template <class element>
  inline list<element>
    list<element>::operator+ ( const list<element> &the_list ) const
  {
    list<element> new_list ( *this );
    new_list += the_list;
    return new_list;
  }


  /****************************************************************************/
  template <class element>
  inline list<element>
    list<element>::operator+ ( const element &the_element ) const
  {
    list<element> new_list ( *this );
    new_list += the_element;
    return new_list;
  }

  /****************************************************************************/

  template <class element>
  void
    list<element>::operator+= ( const list<element> &the_list )
  {
    register node *n;
    int sz = the_list.my_size;
    int i;
    // must use size as stopping condition in case *this == list.
    for ( n = the_list.my_head, i=0; i < sz; n = n->_next, i++ )
      *this += n->_element;
  }


  /****************************************************************************/
  template <class element>
  void
    list<element>::operator+= ( const element &the_element )
  {
    node *newnode = new node;
    check_mem ( newnode );
    newnode->_next = NULL;
    newnode->_element = the_element;
    if ( my_size++ == 0 )
    {
      my_head = newnode;
      newnode->_previous = NULL;
    }
    else
    {
      my_tail->_next = newnode;
      newnode->_previous = my_tail;
    }
    my_tail = newnode;
  }


  /****************************************************************************/
  template <class element>
  list<element> &
    list<element>::operator= ( const list<element> &the_list )
  {
    if ( this == &the_list )
      return *this;

    assert ( my_iterator_count == 0 );
    //operator=() - cannot reassign, iterators present

    register node *the_node;
    node *newnode, *prevnode;

    clear ();

    if ( !( the_node = the_list.my_head ) )
      return *this;

    newnode = new node;
    check_mem ( newnode );
    newnode->_previous = NULL;
    newnode->_element = the_node->_element;
    my_head = prevnode = newnode;

    for ( the_node = the_node->_next; the_node; the_node = the_node->_next )
    {
      newnode = new node;
      check_mem ( newnode );
      newnode->_element = the_node->_element;
      prevnode->_next = newnode;
      newnode->_previous = prevnode;
      prevnode = newnode;
    }
    newnode->_next = NULL;
    my_tail = newnode;
    my_size = the_list.my_size;

    return *this;
  }


  /****************************************************************************/
  template <class element>
  list<element> &
    list<element>::operator= ( const element &the_element )
  {
    assert ( my_iterator_count == 0 );
    //operator=() - cannot reassign, iterators present

    clear ();

    node *newnode = new node;
    check_mem ( newnode );
    newnode->_element = the_element;
    newnode->_previous = newnode->_next = NULL;
    my_head = my_tail = newnode;
    my_size = 1;

    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline element &
    list<element>::head ()
  {
    assert ( my_head );
    //head() - list is empty;

    return my_head->_element;
  }

  /****************************************************************************/

  template <class element>
  inline element &
    list<element>::tail ()
  {
    assert ( my_tail );
    // tail() - list is empty

    return my_tail->_element;
  }


  /****************************************************************************/
  template <class element>
  inline const element &
    list<element>::head () const
  {
    assert ( my_head );
    //head() - list is empty

    return my_head->_element;
  }


  /****************************************************************************/
  template <class element>
  inline const element &
    list<element>::tail () const
  {
    assert ( my_tail );
    // tail() - list is empty

    return my_tail->_element;
  }


  /****************************************************************************/
  template <class element>
  void
    list<element>::add_to_head ( const element &the_element )
  {
    node *newnode = new node;
    check_mem ( newnode );
    newnode->_element = the_element;
    newnode->_previous = NULL;
    newnode->_next = my_head;

    if ( my_size++ )
      my_head->_previous = newnode;
    else
      my_tail = newnode;
    my_head = newnode;
  }


  /****************************************************************************/
  template <class element>
  void
    list<element>::add_to_tail ( const element &the_element )
  {
    node *newnode = new node;
    check_mem ( newnode );
    newnode->_element = the_element;
    newnode->_previous = my_tail;
    newnode->_next = NULL;

    if ( my_size++ )
      my_tail->_next = newnode;
    else
      my_head = newnode;
    my_tail = newnode;
  }


  /****************************************************************************/
  template <class element>
  void
    list<element>::add_to_tail ()
  {
    node *newnode = new node;
    check_mem ( newnode );
    newnode->_previous = my_tail;
    newnode->_next = NULL;
    if ( my_size++ )
      my_tail->_next = newnode;
    else
      my_head = newnode;
    my_tail = newnode;
  }


  /****************************************************************************/
  template <class element>
  void
    list<element>::remove_head ()
  {
    assert ( my_iterator_count == 0 );
    //error - iterators_present

    node *old_head = my_head;
    if ( old_head )
    {
      if ( old_head->_next )
      {
        old_head->_next->_previous = NULL;
        my_head = old_head->_next;
      }
      else
        my_head = my_tail = NULL;

      delete old_head;
      my_size--;
    }
  }


  /****************************************************************************/
  template <class element>
  void
    list<element>::remove_tail ()
  {
    assert ( my_iterator_count == 0 );
    //error - iterators_present

    node *old_tail = my_tail;
    if ( old_tail )
    {
      if ( old_tail->_previous )
      {
        old_tail->_previous->_next = NULL;
        my_tail = old_tail->_previous;
      }
      else
        my_head = my_tail = NULL;

      delete old_tail;
      my_size--;
    }
  }


  /****************************************************************************/
  template <class element>
  element &
    list<element>::operator[] ( int index )
  {
    assert ( index >= 0 && index < my_size ); //error - invalid_index;

    register node *n = my_head;
    for ( register int i = 0; i < index; i++ )
      n = n->_next;

    return n->_element;
  }


  /****************************************************************************/
  template <class element>
  const element &
    list<element>::operator[] ( int index ) const
  {
    assert ( index >= 0 && index < my_size ); //error - invalid_index;

    register node *n = my_head;
    for ( register int i = 0; i < index; i++ )
      n = n->_next;

    return n->_element;
  }


  /****************************************************************************/
  template <class element>
  inline int
    list<element>::size () const
  {
    return my_size;
  }

  /****************************************************************************/

  template <class element>
  inline bool
    list<element>::is_empty () const
  {
    return ( my_size == 0 );
  }


  /****************************************************************************/
  template <class element>
  void
    list<element>::clear ()
  {
    register node *nextnode;

    assert ( my_iterator_count == 0 );
    //clear() - cannot clear, iterators present

    for ( register node *n = my_head; n; n = nextnode )
    {
      nextnode = n->_next;
      delete n;
    }
    my_size = 0;
    my_head = my_tail = NULL;
  }


  /****************************************************************************/
  template <class element>
  list<element>
    list<element>::all_such_that ( bool ( *f ) ( const element & ) ) const
  {
    list<element> new_list;

    for ( register const node *n = my_head; n; n = n->_next )
      if  ( f ( n->_element ) )
        new_list += n->_element;

    return new_list;
  }


  /****************************************************************************/
  /*
  template <class element>
  void
    list<element>::error ( const char *msg )
  {
    cerr << "list: " << msg << '\n';
    assert ( 0 );
  }
  */


  /****************************************************************************/
  template <class element>
  inline
    const_iterator<element>::const_iterator ()
  {
    my_list = NULL;
    my_current = NULL;
  }

  /****************************************************************************/

  template <class element>
  inline
    const_iterator<element>::const_iterator ( const list<element> &the_list )
  {
    my_list = &the_list;
    // even though list is a const, update its iterator count.
    ( (list<element> &) the_list ).my_iterator_count++;
    my_current = the_list.my_head;
  }


  /****************************************************************************/
  template <class element>
  inline
    const_iterator<element>::const_iterator (
          const const_iterator<element> &the_iterator )
  : my_list ( the_iterator.my_list )
  {
    if ( my_list )
      ( (list<element> *) my_list )->my_iterator_count++;
    my_current = the_iterator.my_current;
  }


  /****************************************************************************/
  template <class element>
  inline
    const_iterator<element>::~const_iterator ()
  {
    if ( my_list )
      ( (list<element> *) my_list )->my_iterator_count--;
  }


  /****************************************************************************/
  template <class element>
  inline const_iterator<element>&
    const_iterator<element>::iterate_over ( const list<element> &the_list )
  {
    if ( my_list )
      ( (list<element> *) my_list )->my_iterator_count--;
    my_list = &the_list;
    ( (list<element> *) my_list )->my_iterator_count++;
    my_current = my_list->my_head;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline const_iterator<element> &
    const_iterator<element>::iterate_over ()
  {
    if ( my_list )
    {
      ( (list<element> *) my_list )->my_iterator_count--;
      my_list = NULL;
      my_current = NULL;
    }

    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline const_iterator<element> &
    const_iterator<element>::operator= (
          const const_iterator<element> &the_iterator )
  {
    if ( my_list )
      ( (list<element> *) my_list )->my_iterator_count--;
    my_list = the_iterator.my_list;
    if ( my_list )
      ( (list<element> *) my_list )->my_iterator_count++;
    my_current = the_iterator.my_current;

    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline const element &
    const_iterator<element>::current () const
  {
    assert ( my_current );
    //error - no_current_el;
    return my_current->_element;
  }


  /****************************************************************************/
  template <class element>
  inline const element *
  const_iterator<element>::ptr () const
  {
    if ( my_current )
      return &my_current->_element;
    else
      return NULL;
  }


  /****************************************************************************/
  template <class element>
  inline const_iterator<element> &
    const_iterator<element>::beginning ()
  {
    if ( my_list )
      my_current = my_list->my_head;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline const_iterator<element> &
    const_iterator<element>::end ()
  {
    if ( my_list )
      my_current = my_list->my_tail;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline const_iterator<element> &
    const_iterator<element>::operator++ ()  // prefix
  {
    assert ( my_current );
    //error - no_current_el;

    my_current = my_current->_next;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline const_iterator<element> &
    const_iterator<element>::operator-- ()  // prefix
  {
    assert ( my_current );
    //error - no_current_el;

    my_current = my_current->_previous;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline void
  const_iterator<element>::operator++ (int)  // postfix
  {
    assert ( my_current );
    //error - no_current_el;

    my_current = my_current->_next;
  }


  /****************************************************************************/
  template <class element>
  inline void
    const_iterator<element>::operator-- (int)  // postfix
  {
    assert ( my_current );
    //error - no_current_el;

    my_current = my_current->_previous;
  }


  /****************************************************************************/
  template <class element>
  inline const element &
    const_iterator<element>::operator* () const
  {
    assert ( my_current );
    //error - no_current_el;

    return my_current->_element;
  }


  /****************************************************************************/
  template <class element>
  inline const element *
  const_iterator<element>::operator-> () const
  {
    assert ( my_current );
    //error - no_current_el;

    return &my_current->_element;
  }

  /****************************************************************************/
  template <class element>
  inline bool
    const_iterator<element>::at_beginning () const
  {
    return ( my_current && !my_current->_previous );
  }


  /****************************************************************************/
  template <class element>
  inline bool
    const_iterator<element>::at_end () const
  {
    return ( my_current && !my_current->_next );
  }


  /****************************************************************************/
  template <class element>
  inline int
    const_iterator<element>::size () const
  {
    return my_list->size ();
  }


  /****************************************************************************/
  template <class element>
  inline bool
    const_iterator<element>::is_empty () const
  {
    return my_list->is_empty ();
  }


  /****************************************************************************/
  template <class element>
  inline const list<element> &
    const_iterator<element>::the_list () const
  {
    assert ( my_list );
    //error - not_initialized;

    return *my_list;
  }


  /****************************************************************************/
  template <class element>
  inline
    iterator<element>::iterator ()
  {
    my_list = NULL;
    my_current = NULL;
  }


  /****************************************************************************/
  template <class element>
  inline
    iterator<element>::iterator ( list<element> &the_list )
  {
    my_list = &the_list;
    the_list.my_iterator_count++;
    my_current = the_list.my_head;
  }


  /****************************************************************************/
  template <class element>
  inline
    iterator<element>::iterator ( const iterator<element> &the_iterator )
  : my_list ( the_iterator.my_list )
  {
    if ( my_list )
      my_list->my_iterator_count++;
    my_current = the_iterator.my_current;
  }


  /****************************************************************************/
  template <class element>
  inline
    iterator<element>::~iterator ()
  {
    if ( my_list )
      my_list->my_iterator_count--;
  }


  /****************************************************************************/
  template <class element>
  inline iterator<element> &
    iterator<element>::iterate_over ( list<element> &the_list )
  {
    if ( my_list )
      my_list->my_iterator_count--;
    my_list = &the_list;
    my_list->my_iterator_count++;
    my_current = my_list->my_head;

    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline iterator<element> &
    iterator<element>::iterate_over ()
  {
    if ( my_list )
    {
      my_list->my_iterator_count--;
      my_list = NULL;
      my_current = NULL;
    }

    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline iterator<element> &
    iterator<element>::operator= ( const iterator<element> &the_iterator )
  {
    if ( my_list )
      my_list->my_iterator_count--;
    my_list = the_iterator.my_list;
    if ( my_list )
      my_list->my_iterator_count++;
    my_current = the_iterator.my_current;

    return *this;
  }


  /****************************************************************************/
  template <class element>
  iterator<element> &
    iterator<element>::insert_before_current ( const element &the_element )
  {
    assert ( my_current );

    list<element>::node *newnode = new list<element>::node;
    check_mem ( newnode );
    newnode->_element = the_element;

    if ( my_current->_previous )
    {

      if (my_current == &my_deleted)
      {
        newnode->_previous = my_current->_previous;
        newnode->_next = my_current->_next;
        my_current->_previous->_next = newnode;
        if ( my_current->_next )
          my_current->_next->_previous = newnode;
        else
          my_list->my_tail = newnode;
        my_current->_previous = newnode;
      }
      else
      {
        newnode->_previous = my_current->_previous;
        newnode->_next = my_current;
        my_current->_previous->_next = newnode;
        my_current->_previous = newnode;
      }
    }
    else
    {
      my_list->my_head->_previous = newnode;
      my_current->_previous = newnode; // in case my_current == my_deleted
      newnode->_next = my_list->my_head;
      newnode->_previous = NULL;
      my_list->my_head = newnode;
    }

    my_list->my_size++;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  iterator<element> &
    iterator<element>::insert_after_current ( const element &the_element )
  {
    assert ( my_current );

    list<element>::node *newnode = new list<element>::node;
    check_mem ( newnode );
    newnode->_element = the_element;

    if ( my_current->_next )
    {
      if ( my_current == &my_deleted )
      {
        newnode->_next = my_current->_next;
        newnode->_previous = my_current->_previous;
        my_current->_next->_previous = newnode;
        if ( my_current->_previous )
          my_current->_previous->_next = newnode;
        else
          my_list->my_head = newnode;
        my_current->_next = newnode;
      }
      else
      {
        newnode->_next = my_current->_next;
        newnode->_previous = my_current;
        my_current->_next->_previous = newnode;
        my_current->_next = newnode;
      }
    }
    else
    {
      my_list->my_tail->_next = newnode;
      my_current->_next = newnode;  // in case my_current == my_deleted
      newnode->_previous = my_list->my_tail;
      newnode->_next = NULL;
      my_list->my_tail = newnode;
    }

    my_list->my_size++;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  iterator<element> &
    iterator<element>::replace_current_with ( const element &the_element )
  {
    assert ( my_current );
    assert ( my_current != &my_deleted );
    my_current->_element = the_element;

    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline element &
    iterator<element>::current ()
  {
    assert ( my_current );
    //current() - no current element
    return my_current->_element;
  }


  /****************************************************************************/
  template <class element>
  inline element *
    iterator<element>::ptr ()
  {
    if ( my_current )
      return &my_current->_element;
    else
      return NULL;
  }


  /****************************************************************************/
  template <class element>
  void
    iterator<element>::remove_current ()
  {
    assert ( my_current );
    //remove_current() - no current element

    assert ( my_current != &my_deleted );
    //remove_current() - current element has already been removed

    assert ( my_list->my_iterator_count == 1 );

    my_deleted._previous = my_current->_previous;
    my_deleted._next = my_current->_next;
    delete my_current;
    my_current = &my_deleted;

    if ( my_deleted._previous )
      my_deleted._previous->_next = my_deleted._next;
    else
      my_list->my_head = my_deleted._next;

    if ( my_deleted._next )
      my_deleted._next->_previous = my_deleted._previous;
    else
      my_list->my_tail = my_deleted._previous;

    my_list->my_size--;
  }


  /****************************************************************************/
  template <class element>
  inline iterator<element> &
    iterator<element>::beginning ()
  {
    if ( my_list )
      my_current = my_list->my_head;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline iterator<element> &
    iterator<element>::end ()
  {
    if ( my_list )
      my_current = my_list->my_tail;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline iterator<element> &
    iterator<element>::operator++ ()  // prefix
  {
    assert ( my_current );

    my_current = my_current->_next;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline iterator<element> &
    iterator<element>::operator-- ()  // prefix
  {
    assert ( my_current );

    my_current = my_current->_previous;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline void
    iterator<element>::operator++ ( int )  // postfix
  {
    assert ( my_current );
    //operator++() - no next element

    my_current = my_current->_next;
  }


  /****************************************************************************/
  template <class element>
  inline void
    iterator<element>::operator-- ( int )  // postfix
  {
    assert ( my_current );
    //operator--() - no next element

    my_current = my_current->_previous;
  }


  /****************************************************************************/
  template <class element>
  inline element &
    iterator<element>::operator* ()
  {
    assert ( my_current );
    return my_current->_element;
  }


  /****************************************************************************/
  template <class element>
  inline element *
    iterator<element>::operator-> ()
  {
    assert ( my_current );
    return &my_current->_element;
  }


  /****************************************************************************/
  template <class element>
  inline bool
    iterator<element>::at_beginning () const
  {
    return ( my_current && !my_current->_previous );
  }


  /****************************************************************************/
  template <class element>
  inline bool
    iterator<element>::at_end () const
  {
    return ( my_current && !my_current->_next );
  }


  /****************************************************************************/
  template <class element>
  inline int
    iterator<element>::size () const
  {
    return my_list->size ();
  }


  /****************************************************************************/
  template <class element>
  inline bool
    iterator<element>::is_empty () const
  {
    return my_list->is_empty ();
  }


  /****************************************************************************/
  template <class element>
  inline list<element> &
    iterator<element>::the_list ()
  {
    assert ( my_list );

    return *my_list;
  }
  /****************************************************************************/
};

#endif
