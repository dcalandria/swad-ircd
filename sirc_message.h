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

#ifndef __sirc_message
#define __sirc_message

#include "sirc_user.h"

class IrcMessage
{
protected:
  IrcUser *user;
  std::string prefix;
  std::string name; 
  std::vector < std::string > parameters;
  std::string prefix_nick;
  std::string prefix_user;
  std::string prefix_host;
  
  int response;
  
public:
  IrcMessage ()
    : user(0), prefix(), name(), parameters(), prefix_nick(), prefix_user(), prefix_host(), response(0) { }
  IrcMessage ( IrcUser *user, const char *msg_string, bool parse_prefix = false );
  
  std::vector < std::string >& GetParameters () { return parameters; }
  const std::vector < std::string >& GetParameters () const { return parameters; }
  std::string& GetPrefix () { return prefix; }
  const std::string& GetPrefix () const { return prefix; }
  std::string& GetName () { return name; }
  const std::string& GetName () const { return name; }      
  IrcUser* GetUser () { return user; }
  const IrcUser* GetUser() const { return user; }
  bool IsResponse () const { return response != -1; }
  int GetResponse () const { return response; }
  void ParsePrefix (); 

  std::string& GetPrefixNick () { return prefix_nick; }
  const std::string& GetPrefixNick () const { return prefix_nick; }   
  std::string& GetPrefixUser () { return prefix_user; }
  const std::string& GetPrefixUser () const { return prefix_user; }
  std::string& GetPrefixHost () { return prefix_host; }
  const std::string& GetPrefixHost () const { return prefix_host; }   
};


#endif
