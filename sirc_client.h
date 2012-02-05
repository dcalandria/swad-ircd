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

#ifndef __sirc_client_h
#define __sirc_client_h

#include <string>
#include "text_protocol.h"
#include "map.h"
#include "sirc_message.h"


class IrcClient
{
 public:
  
  class Events
  {
  public:
    virtual void OnPrivmsg ( const std::string& user, const std::string& channel, const std::string& text  ) {}
    virtual void OnRegistered () {}
    virtual void OnJoin ( const std::string& channel, const std::string& user) {}
    virtual void OnNick ( const std::string& user, const std::string& old_nick) {}
    virtual void OnKick ( const std::string& channel, const std::string& user_from, const std::string& user_to,
			  const std::string& text) {}
    virtual void OnPart ( const std::string& channel, const std::string& user ) {}
    virtual void OnQuit ( const std::string& user) {}
    virtual void OnKill ( const std::string& user_from, const std::string& user_to, const std::string& text) {}
	
    virtual void OnResponse ( const IrcMessage& message ) {}
  };


 protected:
  TextProtocolClient socket;
  Events* events;
  std::string server_name;

  bool is_registered;
  int  reg_step;
  
 public:

  IrcClient () : events(0), is_registered(false), reg_step(0) {}
  virtual ~IrcClient () { if ( events ) delete events; } 
  
  virtual int Open ( const char *server_name, int port );
  virtual int Close ();
  virtual int RunStep ();
  
  void SetEventsHandler ( Events* handler );
  void RegisterUser ( const std::string& user, const std::string& host, const std::string& realname );
  void ChangeNick ( const std::string& nick);
  void JoinChannel ( const std::string& channel );
  void SendMessage ( const std::string& to, const std::string& message  );
  void PartChannel ( const std::string& channel );
  void KickUser ( const std::string& channel, const std::string& nick );
  void KillUser ( const std::string& nick, const std::string& message );
  void QueryOperator ( const std::string &user, const std::string &password );
  void Quit ();
  
 protected:

  int DispatchMessage ( const IrcMessage &msg);
  int DispatchResponse ( const IrcMessage &msg );
  int WaitResponse ( int response );
  
  //MANEJADORES DE LOS COMANDOS IRC  
  int OnNICK ( const IrcMessage& msg );        
  int OnQUIT ( const IrcMessage& msg );  
  int OnJOIN ( const IrcMessage& msg );
  int OnPRIVMSG ( const IrcMessage& msg );  
  int OnPING ( const IrcMessage& msg );
  int OnPART ( const IrcMessage& msg );
  int OnKICK ( const IrcMessage& msg );
  int OnKILL ( const IrcMessage& msg );
};

#endif
