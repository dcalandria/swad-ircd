/*
 *
 *  Copyright (C) 2009  Daniel J. Calandria Hernández &
 *                      Antonio Cañas Vargas
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __map_h
#define __map_h


#include <vector>
#include <utility>
#include <iostream>

template <typename Key, typename Data, typename Compare>
class map2
{
public:
  typedef std::pair<Key,Data> map_item;
  typedef std::pair<Key,Data>* iterator;
  typedef const std::pair<Key,Data>* const_iterator;  
private:  
  std::vector < map_item > v;
  Compare cmp_obj;
public:
  map2 () 
  { 
    v.reserve(256);
    v.resize (1);
  }

  iterator begin () { return &v[0]; }
  const_iterator begin() const { return &v[0]; }    
  iterator end () { return &v[v.size()-1]; }
  const_iterator end() const { return &v[v.size()-1]; }
  
  iterator find ( const Data& data )
  {
    for (iterator it = begin(); it != end(); ++it)
      if (it->second == data )
        return it;
    return end();
  }

  const_iterator find ( const Data& data ) const
  {
    for (const_iterator it = begin(); it != end(); ++it)
      if (it->second == data )
        return it;
    return end();
  }
    
  iterator find ( const Key& key )
  {
    int cmp_res;
    
    iterator a = begin();
    iterator b = end()-1; 
            
    while ( b > a )
    {
      iterator p = a + (b - a)/2;
      
      cmp_res = cmp_obj(key, p->first );      
      if (cmp_res == 0)
        return p;
      else if ( cmp_res < 0)
        b = p-1;
      else
        a = p+1;
    }
    
    if ( !cmp_obj ( key, a->first ) )  
      return a;
    else
      return end();
  }
  
  const_iterator find ( const Key& key ) const
  {
    int cmp_res;
    
    const_iterator a = begin();
    const_iterator b = end()-1; 
            
    while ( b > a )
    {
      iterator p = a + (b - a)/2;
      
      cmp_res = cmp_obj(key, p->first );      
      if (cmp_res == 0)
        return p;
      else if ( cmp_res < 0)
        b = p-1;
      else
        a = p+1;
    }
    
    if ( !cmp_obj (key, a->first))  
      return a;
    else
      return end();
  }
   
  void erase ( iterator pos )
  {    
    for ( ++pos ; pos <= end(); ++pos )
      *(pos-1) = *pos;
    v.pop_back();      
  }
  
  void erase ( const Key& key )
  {
    iterator pos = find ( key );
    if (pos != end())
      erase ( pos );
  }  
  
  void erase ( const Data& data )
  {    
    iterator pos = find ( data );
    if (pos != end())
      erase ( pos ); 
  }
  
    
  void insert ( const Key& key, const Data& data)
  {
    if ( v.size() >= v.capacity() - 1 )
      v.reserve  ( v.capacity() * 2 );     
     else if ( v.size() < v.capacity() / 2 )
       v.reserve ( v.size() > 255 ? v.capacity() / 2 : 255 ); 
  
    if ( empty() )
    {
      v.resize ( v.size() + 1 );
      v[0] = map_item(key, data);
    }
    else
    {
      int res = 0;
      iterator pos = find_insert_pos ( key, res );
      if ( res ) 
      {     
        v.resize ( v.size() + 1 );
        if (res > 0) //insertar a la derecha
          pos++;
        for (iterator it = end()-1; it > pos; --it)
          *it = *(it-1);
        *pos = map_item(key,data);
      }     
    }
  }
  
  
  void clear() { v.resize(1); }
  size_t size() { return (v.size()-1); }
  bool empty() { return  (v.size() == 1); }
  
  
  
private:
  iterator find_insert_pos ( const Key& key, int &res )
  {
    int cmp_res;
    
    res = 0;
        
    iterator a = begin();
    iterator b = end()-1;      
    while ( b > a )
    {          
      iterator p = a + (b - a)/2;      
      cmp_res = cmp_obj(key, p->first );     
      if (cmp_res == 0)   
        return end();    //existe!!      
      else if ( cmp_res < 0)
        b = p-1;
      else
        a = p+1;
    }
    //a == b
    res = cmp_obj (key, a->first );
    return a;
  }   
public:  
  map_item& operator[] ( int s ) { return v[s]; }
  const map_item& operator[] (int s ) const { return v[s]; }
};
   
#endif
