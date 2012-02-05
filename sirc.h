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

#ifndef __sirc_h
#define __sirc_h

#include "text_protocol.h"
#include "sirc_aux.h"
#include "sirc_user.h"
#include "sirc_channel.h"
#include "sirc_message.h"
#include "map.h"
#include <string>
#include <cstdarg>
#include <cstdlib>
#include <ctime>

//ALGUNAS CARACTERISTICAS SOPORTADAS, Y VERSIÓN ACTUAL
static const char USER_MODES[] = "io";
static const char CHANNEL_MODES[] = "onitl";
static const char SERVER_NAME[] = "SWAD Simple IRC Server";
static const char SERVER_VERSION[] = "0.1";
static const char SERVER_CREATION_DATE[] = "feb 1 2009";

static int defValidatePassword ( const std::string& name, const std::string &pass) { return true; }

class IrcServer : public TextProtocolServer
{
private:
  channels_map_t channels;
  users_map_t    users;
  users_map_t    operators;
  std::string    servername;
  
  time_t time_stamp;  
  time_t ping_interval;  

  int (*ValidatePassword) ( const std::string &name, const std::string &password ) ;  
public:
  IrcServer ()
    : TextProtocolServer(), ValidatePassword(defValidatePassword) {}  
  virtual ~IrcServer ()
  {
    Close();
  }  
  const std::string& GetServerName() const { return servername; } 
  void SetServerName (const std::string& s) { servername = s; }  
  virtual int Open (int port );    
  virtual int Close ();    
  virtual int RunStep ();    
  virtual int Update ();
  virtual int WaitConnection ();
  virtual int CloseConnection ( TextProtocolSession *session, const std::string &message = "" );        
  virtual int GetMessage ( IrcUser *session, IrcMessage& msg );
  virtual int DispatchMessage ( IrcUser *session, IrcMessage& msg );
  virtual int PostProcess ( IrcUser *session, IrcMessage& msg );


  void SetValidatePasswordHandle ( int (*handle) ( const std::string& name, const std::string& pass ) )
  { ValidatePassword = handle; }

 private:
  //MANEJADORES DE LOS COMANDOS IRC  
  virtual int OnNICK ( IrcUser *user, IrcMessage& msg );
  virtual int OnUSER ( IrcUser *user, IrcMessage& msg );        
  virtual int OnQUIT ( IrcUser *user, IrcMessage& msg );  
  virtual int OnJOIN ( IrcUser *user, IrcMessage& msg );
  virtual int OnPRIVMSG ( IrcUser *user, IrcMessage& msg );  
  virtual int OnPONG ( IrcUser *user, IrcMessage& msg );
  virtual int OnPART ( IrcUser *user, IrcMessage& msg );
  virtual int OnKICK ( IrcUser *user, IrcMessage& msg );
  virtual int OnWHO  ( IrcUser *user, IrcMessage& msg );
  virtual int OnMODE ( IrcUser *user, IrcMessage& msg );
  virtual int OnOPER ( IrcUser *user, IrcMessage& msg );
  virtual int OnKILL ( IrcUser *user, IrcMessage& msg );
  virtual int OnWHOIS ( IrcUser *user, IrcMessage& msg );
  virtual int OnTOPIC ( IrcUser *user, IrcMessage& msg );

  virtual ssize_t printf_IRCOP ( const char *fmt, ... );
};


#endif
