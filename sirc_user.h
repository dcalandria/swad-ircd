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

#ifndef __sirc_user_h
#define __sirc_user_h

#include "sirc_aux.h"
#include "text_protocol.h"
#include <string>

/*
* la sesion en el IRC la podemos identificar con el usuario
*/
class IrcUser : public TextProtocolSession
{
protected:
  std::string nick;
  std::string name;   
  std::string server;
  std::string host;
  channels_map_t channels;       
  
  bool registered;
  bool connection_alive;
  bool connection_successful;
  time_t next_ping_time;  
  bool ircop;
  
public:
  const std::string& GetNick () const { return nick; }
  const std::string& GetRealName () const { return name; }
  const std::string& GetServerName () const { return server; }
  const std::string& GetHostName () const { return host; }  
  void SetNick ( const std::string &s ) { nick = s; }       
  void SetRealName ( const std::string &s ) { name = s; }    
  void SetServerName ( const std::string &s ) { server = s; }       
  void SetHostName ( const std::string &s ) { host = s; }   
  channels_map_t& GetChannels () { return channels; }
  const channels_map_t& GetChannels () const { return channels; }  
  void SetChannels (channels_map_t &s) { channels = s; }    
          
  bool IsConnectionSuccessful () { return connection_successful; }
  void SetConnectionSuccessful ( bool b ) { connection_successful = b; }	      
  bool IsRegistered ( ) const { return registered; }
  void SetRegistered ( bool b ) { registered = b; }  
  bool IsConnectionAlive () const { return connection_alive; }
  void SetConnectionAlive ( bool b ) { connection_alive = b; }
  time_t GetNextPingTime () const { return next_ping_time; }
  void SetNextPingTime ( time_t t ) { next_ping_time = t; }
  bool IsIrcOperator () const { return ircop; }
  void SetIrcOperator ( bool b ) { ircop = b; }
  
  IrcUser ()
    : TextProtocolSession(), nick(), name(), server(), host(), channels(), registered(false), 
      connection_alive(true), connection_successful(false), next_ping_time(0) {}    
  IrcUser ( SocketSessionNonBlock *socket_session, size_t max_msglength, size_t buffer_size )
  : TextProtocolSession (socket_session, max_msglength, buffer_size),
    nick(), name(), server(), host(), channels(), registered(false), 
    connection_alive(true), connection_successful(false), next_ping_time(0) {}      
  virtual ~IrcUser () {}  
};
typedef IrcUser IrcSession;

#endif

