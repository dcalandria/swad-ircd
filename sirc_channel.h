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

#ifndef __sirc_channel_h
#define __sirc_channel_h

#include <string>

class IrcUser;

/*
* Canal del IRC
*/
class IrcChannel
{
  std::string name;  
  std::string topic;
  std::string key;       
  users_map_t users;
  users_map_t operators;      
public:  
  IrcChannel ()
    : name (), topic (), key(), users(), operators() {}  
  ~IrcChannel () {}  
  IrcChannel ( const std::string& _name, const std::string& _topic, const std::string& _key = std::string() )
    : name (_name), topic (_topic), key(), users(), operators() {}      
  const std::string& GetName () const  {return name;  }  
  const std::string& GetTopic () const  { return topic; }     
  void SetName (const std::string& _name) { name = _name;   }  
  void SetTopic (const std::string& _topic)  {  topic = _topic;  }   
  users_map_t& GetUsers () { return users; }
  const users_map_t& GetUsers () const { return users; }  
  users_map_t& GetOperators () { return operators; }
  const users_map_t& GetOperators () const { return operators; }

  bool IsOperator ( IrcUser *usr );
  ssize_t SendNamesList (  IrcUser *user );
  ssize_t SendMessage ( IrcUser *user, const std::string& message );
  ssize_t SendTopic ( IrcUser *user );
  
  void PartUser ( IrcUser *user, const std::string& part_msg );
  
  virtual ssize_t printf ( const char *fmt, ... );      
  
};

#endif
