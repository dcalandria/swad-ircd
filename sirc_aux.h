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

#ifndef __sirc_aux_h
#define __sirc_aux_h

#include <cctype>
#include <string>
#include "map.h"

inline int toupper_irc ( int c )
{
  switch ( c )
  {
  case '{':
    return '[';
  case '}':
    return ']';
  case '|':
    return '\\';    
  case '^':
    return '~';
  default:
    return std::toupper (c);
  }    
}
inline int tolower_irc ( int c )
{
  switch ( c )
  {
  case '[':
    return '{';
  case ']':
    return '}';
  case '\\':
    return '|';    
  case '~':
    return '^';
  default:
    return std::tolower (c);
  }    
}

class irc_str_cmp_func
{
public:
  int operator()(const std::string& s1, const std::string& s2) const
  {   
    unsigned end;
    int     len;    
    if (s1.length() < s2.length())
    {      end = s1.length();      len = -1;     }
    else if (s1.length() > s2.length())
    {      end = s2.length();      len = 1;    }
    else
    {      end = s1.length();      len = 0;    }
    
    for (unsigned i = 0; i < end; ++i)
    {
      int c1 = toupper_irc (s1[i]); 
      int c2 = toupper_irc (s2[i]);
      if ( c1 > c2 )             
        return 1;
      else if ( c1 < c2 )      
        return -1;
    }
    return len;    
  }
};

class IrcUser;
class IrcChannel;

typedef map2 < std::string, IrcUser*,    irc_str_cmp_func > users_map_t;  
typedef map2 < std::string, IrcChannel*, irc_str_cmp_func > channels_map_t;



#endif
