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

#include "sirc_client.h"
#include <string>

using namespace std;


int IrcClient::Open ( const char *server_name, int port )
{
  this->server_name = server_name;
  return socket.Open ( server_name, port );
}

int IrcClient::Close ()
{
  return socket.Close ();
}

int IrcClient::RunStep ()
{
  char buf[1024];
  IrcMessage msg;
  
  //Obtener un mensaje y procesar
  int res = socket.ReadMessage ( buf );
  if (res > 0)
    {
      std::printf ("IRC MESSAGE: %s\n", buf );
      msg = IrcMessage ( 0, buf, true );
      return DispatchMessage ( msg );
    }

  return res;
}
  
void IrcClient::SetEventsHandler ( Events* handler )
{
  if (events)
    delete events;
  
  events = handler;
}

void IrcClient::RegisterUser ( const std::string& user, const std::string& host, const std::string& realname )
{
  socket.printf ("USER %s %s %s :%s\r\n", user.c_str(), host.c_str(), server_name.c_str(),
		 realname.c_str() );

  if (!is_registered && reg_step == 1)
    {
      while (!is_registered && reg_step == 1)
	if (WaitResponse (376) != 376)
	  break;	
    }
  reg_step = 1;
}


void IrcClient::ChangeNick ( const std::string& nick )
{
  socket.printf ("NICK %s\r\n", nick.c_str());
  
  if (!is_registered && reg_step == 1)
    {
      //esperar registro
      while (!is_registered && reg_step == 1)
	if (WaitResponse (376) != 376)
	  break;
    }
  reg_step = 1;
}

#define IRC_DISPATCH_MESSAGE(msg, name) if (msg.GetName() == #name) { return On##name ( msg ); }

int IrcClient::DispatchMessage ( const IrcMessage& msg )
{

  IRC_DISPATCH_MESSAGE(msg, PRIVMSG);  
  IRC_DISPATCH_MESSAGE(msg, PING);   
  IRC_DISPATCH_MESSAGE(msg, JOIN);   
  IRC_DISPATCH_MESSAGE(msg, NICK);
  IRC_DISPATCH_MESSAGE(msg, QUIT);    
  IRC_DISPATCH_MESSAGE(msg, PART);
  IRC_DISPATCH_MESSAGE(msg, KICK);
  IRC_DISPATCH_MESSAGE(msg, KILL);

  if (msg.IsResponse () )
    return DispatchResponse ( msg );
  
  return S_OK;
}


int IrcClient::DispatchResponse ( const IrcMessage &msg )
{
  switch (msg.GetResponse ())
    {
    case 376: //End of MOTD command
      is_registered = true;
      if (events)
	events->OnRegistered ();
      break;
    }

  if (events)
    events->OnResponse ( msg );
  
  return S_OK;
}

int IrcClient::OnNICK ( const IrcMessage& msg )
{
  if (events)
    events->OnNick (  msg.GetParameters()[0].c_str(), msg.GetPrefixNick().c_str() ); 
  
  return S_OK;
}

int IrcClient::OnQUIT ( const IrcMessage& msg )
{
  if (events )
    events->OnQuit ( msg.GetPrefixNick().c_str() );
  return S_OK;
}

int IrcClient::OnJOIN ( const IrcMessage& msg )
{
  if (events)
    events->OnJoin ( msg.GetParameters()[0].c_str(),  msg.GetPrefixNick().c_str());
  return S_OK;
}

int IrcClient::OnPRIVMSG ( const IrcMessage& msg )
{
  if (events)
    events->OnPrivmsg ( msg.GetPrefixNick().c_str(), msg.GetParameters()[0].c_str(),
		       msg.GetParameters()[1].c_str() );
  return S_OK;
}

int IrcClient::OnPING ( const IrcMessage& msg )
{
  socket.printf("PONG %s\r\n", msg.GetParameters()[0].c_str() );
  return S_OK;
}

int IrcClient::OnPART ( const IrcMessage& msg )
{
  if (events)
    events->OnPart (  msg.GetParameters()[0].c_str(), msg.GetPrefixNick().c_str());
  return S_OK;
}

int IrcClient::OnKICK ( const IrcMessage& msg )
{
  if (events)
    events->OnKick ( msg.GetParameters()[0].c_str(), msg.GetPrefixNick().c_str(), msg.GetParameters()[1].c_str(),
		    msg.GetParameters().size() > 2 ? msg.GetParameters()[2].c_str() : "" );
  return S_OK;
}

int IrcClient::OnKILL ( const IrcMessage& msg )
{
  if (events)
    events->OnKill ( msg.GetPrefixNick().c_str(), msg.GetParameters()[0].c_str(), msg.GetParameters()[1].c_str());
  return S_OK;
}

int IrcClient::WaitResponse (int response )
{
  char buf[1024];
  IrcMessage msg;
  
  while (true)
    {
      int res = socket.ReadMessage ( buf );
      if (res > 0)
	{
	  std::printf ("IRC MESSAGE: %s\n", buf );
	  msg = IrcMessage ( 0, buf, true );

	  if ( (res = DispatchMessage ( msg )) != S_OK)
	    return res;
	  		   
	  if ( msg.GetResponse() == response ||
	       msg.GetResponse() >= 400  /* error */ )
	    return msg.GetResponse();
	}
    }
  return S_OK;
}


void IrcClient::JoinChannel ( const std::string& channel )
{
  socket.printf("JOIN %s\r\n", channel.c_str());
}

void IrcClient::SendMessage ( const std::string& to, const std::string& message  )
{
  socket.printf("PRIVMSG %s :%s\r\n", to.c_str(), message.c_str());
}

void IrcClient::PartChannel ( const std::string& channel )
{
  socket.printf("PART %s\r\n", channel.c_str());
}

void IrcClient::KickUser ( const std::string& channel, const std::string& nick )
{
  socket.printf("KICK %s %s\r\n", channel.c_str(), nick.c_str());
}

void IrcClient::KillUser ( const std::string& nick, const std::string& message )
{
  socket.printf("KILL %s :%s\r\n", nick.c_str(), message.c_str());
}

void IrcClient::QueryOperator ( const std::string &user, const std::string &password )
{
  socket.printf("OPER %s :%s\r\n", user.c_str(), password.c_str());
}

void IrcClient::Quit ()
{
  socket.printf("QUIT\r\n");
}
    
