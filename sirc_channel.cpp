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

#include "sirc.h"
using namespace std;

ssize_t IrcChannel::printf ( const char *fmt, ... )
{
  va_list list;
  users_map_t::iterator it;
  
  for (it = users.begin(); it != users.end(); ++it)
  {
    va_start( list, fmt );
    it->second->vprintf ( fmt, list );
    va_end( list );  
  }
  return S_OK;
}

bool IrcChannel::IsOperator ( IrcUser *usr )
{
  users_map_t::iterator it;
  for (it = operators.begin(); it != operators.end(); ++it )
    if ( usr == it->second)
      return true;
  return false;
}

void IrcChannel::PartUser ( IrcUser *user, const std::string& part_msg )
{
  printf(":%s!%s@%s PART %s :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(),
         user->GetHostName().c_str(), name.c_str(), part_msg.c_str() );
  GetUsers().erase ( user );
  GetOperators().erase ( user );  
}

ssize_t IrcChannel::SendNamesList (  IrcUser *user )
{
  //IrcUser *src_user;
  users_map_t::iterator it;  
  string nick_list = "";
  
  for (it = users.begin(); it != users.end(); ++it)
  {
    string nick = "";
    if ( IsOperator ( it->second ) )
      nick = "@";
    nick = nick + it->second->GetNick();    
    nick_list += nick + " ";
    
    if ( nick_list.length() > 100 )
    {            
      user->printf(":%s 353 %s = %s :%s\r\n", user->GetServerName().c_str(), user->GetNick().c_str(),
		   name.c_str(), nick_list.c_str() );    
      nick_list = "";
    }
  }
  if (nick_list.length())
    user->printf(":%s 353 %s = %s :%s\r\n", user->GetServerName().c_str(), user->GetNick().c_str(),
		 name.c_str(), nick_list.c_str() );    
  user->printf(":%s 366 %s %s :End of NAMES list\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), name.c_str());
  return S_OK;
}

ssize_t IrcChannel::SendMessage ( IrcUser *user, const string& message )
{
  IrcUser *dst_user;
  users_map_t::iterator it;
    
  for (it = users.begin(); it != users.end(); ++it)
  {
    dst_user = it->second;
    if (dst_user != user)
    {
      dst_user->printf (":%s!%s@%s PRIVMSG %s :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(), 
          user->GetHostName().c_str(), name.c_str(), message.c_str() );        
    }
  }
  return S_OK;
}

ssize_t IrcChannel::SendTopic ( IrcUser *user )
{
  user->printf ( ":%s 332 %s %s :%s\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), name.c_str (), topic.c_str() );
  return S_OK;
}


