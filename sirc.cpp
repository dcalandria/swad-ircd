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
#include <cstdlib>
using namespace std;

int IrcServer::Open (int port )
{    
  ping_interval = 30;  
  
  Close(); 
  return TextProtocolServer::Open ( port, 512, 128, 1000, 50 );  
}

int IrcServer::Close ()
{
  TextProtocolServer::Close ();
  
  users.clear();  
  for (channels_map_t::iterator it = channels.begin(); it != channels.end(); ++it )    
    delete (*it).second;
  channels.clear();
      
  return S_OK;
}

int IrcServer::WaitConnection ()
{
  SocketSessionNonBlock *new_socket_session;  
  new_socket_session = reinterpret_cast<SocketSessionNonBlock*>(socket_server->WaitConnection());  
  if (new_socket_session )
  {
    IrcUser *session = new IrcUser(new_socket_session, 512, 128 );
    sessions.push_back ( session );    
  }
  return S_OK;
}

int IrcServer::CloseConnection ( TextProtocolSession *session, const string& message )
{
  IrcUser *user = reinterpret_cast<IrcUser*> (session);

  for (unsigned i = 0; i < sessions.size(); ++i)
    if (sessions[i] == user)
    {
      swap (sessions[i], sessions.back());
      sessions.pop_back();
      break;
    } 
  //Analizar los canales en los que esta el usuario
  for (channels_map_t::iterator it = user->GetChannels().begin(); it != user->GetChannels().end(); ++it)
  {
    if (it->second->GetUsers().size() == 1 )
    {
      //Eliminar este canal
      channels.erase ( it->second );      
      delete it->second;
    }
    else
    {
      it->second->GetUsers().erase ( user );
      it->second->GetOperators().erase ( user );
    
      //Informar a los usuarios de los canales
      it->second->printf(":%s!%s@%s QUIT %s :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(),
                        user->GetHostName().c_str(), user->GetNick().c_str(), message.c_str() );     
    }
  }  

  printf_IRCOP (":%s!%s@%s QUIT %s :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(),
		user->GetHostName().c_str(), user->GetNick().c_str(), message.c_str() );
  
  // Eliminar el usuario de la lista
  users.erase ( user );
  operators.erase (user);
  delete user;
  
  return S_OK;
}


int IrcServer::Update ()
{  
  socket_server->Update ( 100000 );  
  time_stamp = time(0);   
   
  return S_OK;
}

int IrcServer::RunStep ()
{
  IrcMessage msg;
  Update ();
  WaitConnection ();
  
  unsigned i = 0;
  while (i < sessions.size())
  {
    IrcUser *user = reinterpret_cast<IrcSession*> ( sessions[i] );
    ssize_t res = GetMessage ( user, msg );
    if (res == S_CONNECTION_CLOSED || res == E_TIMEOUT )
    {
      //printf ("CONEXION CERRADA\n");
      CloseConnection (user);      
      continue;
    }
    else if ( res > 0 )
    {
     if ( DispatchMessage ( user, msg ) == S_CONNECTION_CLOSED )
        continue;
    }
    PostProcess (user, msg);
    ++i;
  }    
  return S_OK;
}

int IrcServer::GetMessage ( IrcUser *user, IrcMessage& msg )
{
  int res = TextProtocolServer::GetMessage ( user, TextProtocolServer::msg );    
  msg = IrcMessage ( user, TextProtocolServer::msg );
  return res;
}

#define IRC_DISPATCH_MESSAGE(session, msg, name) if (msg.GetName() == #name) { return On##name ( session, msg ); }

int IrcServer::DispatchMessage ( IrcUser *user, IrcMessage& msg )
{
  /*
  printf("PREFIX:%s\\ MESSAGE:%s\\ PARAMETERS:", msg.GetPrefix().c_str(), msg.GetName().c_str());
  for (unsigned i= 0; i < msg.GetParameters().size(); ++i)
    printf("%s\\", msg.GetParameters()[i].c_str());
  printf("\\\n");
  */
    
  IRC_DISPATCH_MESSAGE(user, msg, PRIVMSG);  
  IRC_DISPATCH_MESSAGE(user, msg, PONG);   
  IRC_DISPATCH_MESSAGE(user, msg, JOIN);   
  IRC_DISPATCH_MESSAGE(user, msg, NICK);
  IRC_DISPATCH_MESSAGE(user, msg, WHO);    
  IRC_DISPATCH_MESSAGE(user, msg, WHOIS);    
  IRC_DISPATCH_MESSAGE(user, msg, QUIT);    
  IRC_DISPATCH_MESSAGE(user, msg, PART);
  IRC_DISPATCH_MESSAGE(user, msg, KICK);
  IRC_DISPATCH_MESSAGE(user, msg, KILL);
  IRC_DISPATCH_MESSAGE(user, msg, TOPIC);
  IRC_DISPATCH_MESSAGE(user, msg, USER);
  IRC_DISPATCH_MESSAGE(user, msg, MODE);          
  IRC_DISPATCH_MESSAGE(user, msg, OPER);
                   
  return S_OK;
}

int IrcServer::PostProcess ( IrcUser *user, IrcMessage& msg )
{     
  if ( user->IsConnectionSuccessful () )
  {   
    //¿Hay que enviar ping?
    if ( user->GetNextPingTime() < time_stamp )
    {
      if ( user->IsConnectionAlive() )
      {            
        user->SetNextPingTime ( time_stamp + ping_interval );
        user->SetConnectionAlive ( false );
        user->printf ("PING :%X\r\n", user->GetNextPingTime() );
      }
      else
      {
        user->printf ("ERROR :Closing Link: %s (Ping timeout)\r\n", user->GetNick().c_str() );
        CloseConnection ( user );
        return S_CONNECTION_CLOSED;
      }
    }
  }    
  return S_OK;
}


ssize_t IrcServer::printf_IRCOP ( const char *fmt, ... )
{
  va_list list;
  users_map_t::iterator it;
  
  for (it = operators.begin(); it != operators.end(); ++it)
  {
    va_start( list, fmt );
    it->second->vprintf ( fmt, list );
    va_end( list );  
  }
  return S_OK;
}
