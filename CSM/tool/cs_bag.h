/*
*
* cs_bag.h
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

#ifndef __cs_bag_h
#define __cs_bag_h

#include <stdlib.h>
#include "cs_basic.h"
#include "cs_list.h"

namespace tool
{
  // assumes element has a default constructor, operator=(), operator==(),
  // and operator<().  note that operator<() must place a total ordering on
  // the elements.
  // bag<t>::operator<() places a total ordering on all bag<t> elements,
  // so that set<bag<t> > is possible.

  template <class element>
  class bag
  {
  public:
    bag ();
    bag ( const bag<element> &the_bag );
    bag ( const list<element> &the_list );
    bag ( const element &element, int count = 1 );
    ~bag ();
    operator list<element> () const;

    bag<element> operator+ ( const bag<element> &the_bag ) const;
    bag<element> operator+ ( const element &element ) const;
    bag<element> &operator+= ( const bag<element> &the_bag );
    bag<element> &operator+= ( const element &element );
    bag<element> &add ( const bag<element> &the_bag );
    bag<element> &add ( const element &element, int num = 1 );

    // it is illegal to call the following functions unless the elements
    // are there to be removed!  if unsure, call contains() first.
    bag<element> operator- ( const bag<element> &the_bag ) const;
    bag<element> operator- ( const element &element ) const;
    bag<element> &operator-= ( const bag<element> &the_bag );
    bag<element> &operator-= ( const element &element );
    bag<element> &remove ( const bag<element> &the_bag );
    bag<element> &remove ( const element &element, int num = 1 );
    bag<element> &remove_all ( const element &element );

    // miscellaneous
    bool operator== ( const bag<element> &the_bag ) const;
    bool operator!= ( const bag<element> &the_bag ) const;
    bool operator< ( const bag<element> &the_bag ) const;
    bag<element> &operator= ( const bag<element> &the_bag );
    bag<element> &operator= ( const element &element );
    bag<element> &operator= ( const list<element> &the_list );
    list<element> the_list () const;
    list<element> the_unique_list () const;
    int size () const;        // total number of elements.
    int unique_size () const; // total number of unique elements.
    bool is_empty () const;
    bool contains ( const bag<element> &the_bag ) const;
    bool contains ( const element &element, int num = 1 ) const;
    int occurrences_of ( const element &element ) const;
    element retrieve ();  // returns and removes an element.
    bag<element> &clear ();
  protected:
    class slot
    {
    public:
      element my_element;
      int my_count;
    public:
      slot () : my_count ( 0 )
      {
        /* do nothing */ ;
      }

      slot ( const slot &slot ) : my_count ( slot.my_count )
      {
        my_element = slot.my_element;
      }

      slot ( const element &element, int num ) : my_count ( num )
      {
        my_element = element;
      }

      void operator= ( const slot &slot )
      {
        my_element = slot.my_element;
        my_count = slot.my_count;
      }
    };

    list<slot> my_list;
  };


  /****************************************************************************/
  template <class element>
  inline
    bag<element>::bag () : my_list ()
  {
    /* do nothing */ ;
  }


  /****************************************************************************/
  template <class element>
  inline
    bag<element>::bag ( const bag<element> &the_bag ) : my_list ( bag.my_list )
  {
    /* do nothing */ ;
  }


  /****************************************************************************/
  template <class element>
  inline
    bag<element>::bag ( const list<element> &the_list )
  {
    *this = list;
  }


  /****************************************************************************/
  template <class element>
  inline
    bag<element>::bag ( const element &element, int count ) : my_list ()
  {
    if ( count < 0 )
    {
      cerr << "bag - negative count\n";
      assert ( 0 );
    }
    else if ( count > 0 )
    {
      bag<element>::slot slot ( element, count );
      my_list = slot;
    }
  }


  /****************************************************************************/
  template <class element>
  inline
    bag<element>::~bag ()
  {
    my_list.clear();
  }


  /****************************************************************************/
  template <class element>
    bag<element>::operator list<element> () const
  {
    list<element> list;

    read_only_iterator<bag<element>::slot> iter ( my_list );
    foreach ( iter )
      for ( register int i = 0; i < iter->my_count; i++ )
        list.add_to_tail ( iter->my_element );

    return list;
  }


  /****************************************************************************/
  template <class element>
  inline bag<element>
    bag<element>::operator+ ( const bag<element> &the_bag ) const
  {
    bag<element> new_bag ( *this );
    new_bag += bag;
    return new_bag;
  }

  /****************************************************************************/

  template <class element>
  inline bag<element>
    bag<element>::operator+ ( const element &element ) const
  {
    bag<element> new_bag ( *this );
    new_bag += element;
    return new_bag;
  }

  /****************************************************************************/

  template <class element>
  inline bag<element> &
    bag<element>::operator+= ( const bag<element> &the_bag )
  {
    return add ( bag );
  }

  /****************************************************************************/

  template <class element>
  inline bag<element> &
    bag<element>::operator+= ( const element &element )
  {
    return add ( element, 1 );
  }

  /****************************************************************************/

  template <class element>
  bag<element> &
    bag<element>::add ( const bag<element> &the_bag )
  {
    iterator<bag<element>::slot> a ( my_list );
    read_only_iterator<bag<element>::slot> b ( bag.my_list );

    while ( b.ptr() )
    {
      while ( a.ptr() && a->my_element < b->my_element )
        a++;
      if ( !a.ptr() )
      {
        for ( ; b.ptr(); b++ )
          my_list.add_to_tail ( *b );
      }
      else
      {
        if ( a->my_element == b->my_element )
        {
          a->my_count += b->my_count;
          a++;
        }
        else
          a.insert_before_current ( *b );
        b++;
      }
    }
    return *this;
  }


  /****************************************************************************/
  template <class element>
  bag<element> &
    bag<element>::add(const element &element, int num)
  {
    if ( num < 0 )
    {
      cerr << "bag - can't add a negative number of something\n";
      assert ( 0 );
    }
    else if ( num > 0 )
    {
      iterator<bag<element>::slot> iter ( my_list );

      // start looking from the end of the list, in case the elements are
      // being added in alphabetical order.

      iter.end();
      while ( iter.ptr() && element < iter->my_element )
        --iter;
      if ( !iter.ptr() )
      {
        bag<element>::slot slot ( element, num );
        my_list.add_to_head ( slot );
      }
      else if ( iter->my_element == element )
        iter->my_count += num;
      else
      {
        bag<element>::slot slot ( element, num );
        iter.insert_after_current ( slot );
      }
    }
    return *this;
  }


  /****************************************************************************/
  template <class element>
  bag<element>
    bag<element>::operator- ( const bag<element> &the_bag ) const
  {
    bag<element> new_bag;
    read_only_iterator<bag<element>::slot> a ( my_list ), b ( bag.my_list );

    foreach ( b )
    {
      while ( a.ptr() && a->my_element < b->my_element )
        a++;
      if ( !a.ptr() || b->my_element < a->my_element ||
            b->my_count > a->my_count )
      {
        cerr << "bag - error subtracting more than exists\n";
        assert ( 0 );
      }
      else if ( b->my_count < a->my_count )
      {
        bag<element>::slot slot ( a->my_element, a->my_count - b->my_count );
        new_bag.my_list.add_to_tail ( slot );
      }
      a++;
    }
    return new_bag;
  }


  /****************************************************************************/
  template <class element>
  inline bag<element>
    bag<element>::operator- ( const element &element ) const
  {
    bag<element> new_bag ( *this );
    return new_bag.remove ( element, 1 );
  }


  /****************************************************************************/
  template <class element>
  inline bag<element> &
    bag<element>::operator-= ( const bag<element> &the_bag )
  {
    return remove ( bag );
  }


  /****************************************************************************/
  template <class element>
  inline bag<element> &
    bag<element>::operator-= ( const element &element )
  {
    return remove ( element, 1 );
  }


  /****************************************************************************/
  template <class element>
  bag<element> &
    bag<element>::remove ( const bag &the_bag )
  {
    if ( this == &the_bag )
      my_list.clear();
    else
    {
      iterator<bag<element>::slot> a ( my_list );
      read_only_iterator<bag<element>::slot> b ( bag.my_list );

      foreach ( b )
      {
        while ( a.ptr() && a->my_element < b->my_element )
          a++;
        if ( !a.ptr() || b->my_element < a->my_element ||
              b->my_count > a->my_count )
        {
          cerr << "bag - error subtracting more than exists\n";
          assert ( 0 );
        }
        else if ( b->my_count < a->my_count )
          a->my_count -= b->my_count;
        else
          a.remove_current ();
        a++;
      }
    }
    return *this;
  }


  /****************************************************************************/
  template <class element>
  bag<element> &
    bag<element>::remove ( const element &element, int num )
  {
    iterator<bag<element>::slot> iter ( my_list );

    while ( iter.ptr() && iter->my_element < element )
      iter++;

    if ( !iter.ptr() || element < iter->my_element || num > iter->my_count)
    {
      cerr << "bag - error subtracting more than exists\n";
      assert ( 0 );
    }
    else if ( num < iter->my_count )
      iter->my_count -= num;
    else
      iter.remove_current();

    return *this;
  }


  /****************************************************************************/
  template <class element>
  bag<element> &
    bag<element>::remove_all ( const element &element )
  {
    iterator<bag<element>::slot> iter ( my_list );

    while ( iter.ptr() && iter->my_element < element )
      iter++;
    if ( iter.ptr() && iter->my_element == element )
      iter.remove_current();

    return *this;
  }


  /****************************************************************************/
  template <class element>
  bool
    bag<element>::operator== ( const bag<element> &the_bag ) const
  {
    if ( this == &the_bag )
      return true;

    if ( my_list.size() != bag.my_list.size() )
      return false;

    read_only_iterator<bag<element>::slot> a ( my_list ), b ( bag.my_list );
    while ( a.ptr() )
    {
      if ( !( a->my_count == b->my_count && a->my_element == b->my_element ) )
        return false;
      a++, b++;
    }
    return true;
  }


  /****************************************************************************/
  template <class element>
  inline bool
    bag<element>::operator!= ( const bag<element> &the_bag ) const
  {
    return !operator== ( bag );
  }


  /****************************************************************************/
  template <class element>
  bool
    bag<element>::operator< ( const bag<element> &the_bag ) const
  {
    if ( bag.my_list.size() < my_list.size() )
      return false;
    else if ( my_list.size() < bag.my_list.size() )
      return true;
    else
    {
      read_only_iterator<bag<element>::slot> a ( my_list ), b ( bag.my_list );
      while ( a.ptr() )
      {
        if ( b->my_count < a->my_count )
          return false;
        else if ( a->my_count < b->my_count )
          return true;
        else if ( b->my_element < a->my_element )
          return false;
        else if ( a->my_element < b->my_element )
          return true;
        a++, b++;
      }
    }
    return false;
  }


  /****************************************************************************/
  template <class element>
  inline bag<element> &
    bag<element>::operator= ( const bag<element> &the_bag )
  {
    my_list = bag.my_list;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  inline bag<element> &
    bag<element>::operator= ( const element &element )
  {
    bag<element>::slot slot ( element, 1 );
    my_list += slot;
    return *this;
  }


  /****************************************************************************/
  template <class element>
  bag<element> &
    bag<element>::operator= ( const list<element> &the_list )
  {
    clear();
    read_only_iterator<element> iter ( the_list );
    foreach ( iter )
      add ( *iter );

    return *this;
  }


  /****************************************************************************/
  template <class element>
  list<element>
    bag<element>::the_list () const
  {
    list<element> _list;
    read_only_iterator<bag<element>::slot> iter ( my_list );

    foreach ( iter )
      for ( register int i = 0; i < iter->my_count; i++ )
        _list.add_to_tail ( iter->my_element );

    return _list;
  }


  /****************************************************************************/
  template <class element>
  list<element>
    bag<element>::the_unique_list () const
  {
    list<element> _list;
    read_only_iterator<bag<element>::slot> iter ( my_list );

    foreach ( iter )
      _list.add_to_tail ( iter->my_element );

    return _list;
  }


  /****************************************************************************/
  template <class element>
  int
    bag<element>::size () const
  {
    int total_count = 0;
    read_only_iterator<bag<element>::slot> iter ( my_list );

    foreach ( iter )
      total_count += iter->my_count;

    return total_count;
  }


  /****************************************************************************/
  template <class element>
  inline int
    bag<element>::unique_size() const
  {
    return my_list.size();
  }


  /****************************************************************************/
  template <class element>
  inline bool
    bag<element>::is_empty () const
  {
    return my_list.is_empty ();
  }


  /****************************************************************************/
  template <class element>
  bool
    bag<element>::contains ( const bag<element> &the_bag ) const
  {
    if ( this == &the_bag )
      return true;

    read_only_iterator<bag<element>::slot> a ( my_list ), b ( bag.my_list );

    while ( b.ptr() )
    {
      while ( a.ptr() && a->my_element < b->my_element )
        a++;
      if ( !( a.ptr() && a->my_element == b->my_element &&
            a->my_count >= b->my_count ) )
        return false;
      a++;
      b++;
    }
    return true;
  }


  /****************************************************************************/
  template <class element>
  bool
    bag<element>::contains ( const element &element, int num ) const
  {
    read_only_iterator<bag<element>::slot> iter ( my_list );

    while ( iter.ptr() && iter->my_element < element )
      iter++;

    return ( iter.ptr() && iter->my_element == element &&
             iter->my_count >= num );
  }


  /****************************************************************************/
  template <class element>
  int
    bag<element>::occurrences_of ( const element &element ) const
  {
    read_only_iterator<bag<element>::slot> iter ( my_list );

    while ( iter.ptr() && iter->my_element < element )
      iter++;
    if ( iter.ptr() && iter->my_element == element )
      return iter->my_count;
    else
      return 0;
  }


  /****************************************************************************/
  template <class element>
  element
    bag<element>::retrieve ()
  {
    if ( my_list.is_empty() )
    {
      cerr << "bag: retreive() - bag empty\n";
      assert ( 0 );
    }

    element element = my_list.head ().my_element;

    if ( --my_list.head().my_count == 0 )
      my_list.remove_head ();

    return element;
  }


  /****************************************************************************/
  template <class element>
  inline bag<element> &
    bag<element>::clear ()
  {
    my_list.clear ();

    return *this;
  }


/****************************************************************************/
};

#endif /* bag_defined */
