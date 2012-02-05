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
#include <string>
using namespace std;


int IrcServer::OnNICK ( IrcUser *user, IrcMessage& msg )
{
  string& nick = msg.GetParameters()[0];  
  users_map_t::iterator ptr;
  
  //Comprobacion de errores
  if ( msg.GetParameters().empty() || nick.empty() )
  {
    user->printf (":%s 431 %s :No nickname given\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }  
  //if ( nick.length () > max_nick_length )
  //{
  //  user->SendResponse (ERR_ERRONEUSNICKNAME, nick.c_str() );
  //  return S_OK;  
  //}  
  ptr = users.find ( nick );
  if ( ptr != users.end() && ptr->second != user )
  {
    //se ha encontrado el nick, pero no es de este usuario  
    user->printf (":%s 433 %s :Nickname is already in use\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;    
  }  
  
  //eliminar el nick actual de la lista y añadir el nuevo
  users.erase ( user );    
  string old_nick = user->GetNick();    
  user->SetNick ( nick );
  users.insert(nick, user);  
  if (!user->GetRealName().empty())
    user->SetConnectionSuccessful ( true );
  
  //Recorrer canales, informando de que ha cambiado su nick
  channels_map_t::iterator it;  
  for ( it = user->GetChannels().begin(); it != user->GetChannels().end(); ++it)
  {
    it->second->printf (":%s!%s@%s NICK :%s\r\n", 
                        old_nick.c_str(), user->GetNick().c_str(), 
                        user->GetHostName().c_str(), user->GetNick().c_str());       
  }


  //Informar a los IRCops
  printf_IRCOP (":%s!%s@%s NICK :%s\r\n", 
		old_nick.c_str(), user->GetNick().c_str(), 
		user->GetHostName().c_str(), user->GetNick().c_str()); 
  
  return S_OK;
}


/***********************************************************************
*
*
*
*
************************************************************************/
int IrcServer::OnUSER ( IrcUser *user, IrcMessage& msg )
{
  //Comprobar errores!
  if ( user->IsRegistered () )
  {
    user->printf (":%s 462 %s :Unauthorized command (already registered)\r\n", 
                  servername.c_str(), user->GetNick().c_str() );
    return S_OK;       
  }  
  if ( msg.GetParameters().size () < 4 )
  {
    user->printf (":%s 461 %s :Not enough parameters\r\n",
                  servername.c_str(), user->GetNick().c_str());
    return S_OK;     
  }      
  //string& username = msg.GetParameters()[0];
  string& hostname = msg.GetParameters()[1];    
  string& realname = msg.GetParameters()[3];      
  user->SetHostName ( hostname );
  user->SetServerName ( servername );    
  user->SetRealName ( realname );  
  
  if (!user->GetNick().empty() )
    user->SetConnectionSuccessful ( true );
   
  return S_OK;
}
 
 
/***********************************************************************
*
*
*
*
************************************************************************/
int IrcServer::OnQUIT ( IrcUser *user, IrcMessage& msg )
{
  string message_text = "";  
  if ( !msg.GetParameters().empty() )
    message_text = msg.GetParameters()[0];
  user->printf ("ERROR :Closing Link: %s - %s\r\n", user->GetNick().c_str(), message_text.c_str());            
  CloseConnection ( user, message_text );  
  return S_CONNECTION_CLOSED;
}


/***********************************************************************
*
*
*
*
************************************************************************/
int IrcServer::OnPART ( IrcUser *user, IrcMessage& msg )
{
  channels_map_t::iterator it;
  
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }   
    
  if ( msg.GetParameters().empty() )
  {
    user->printf (":%s 461 %s :Not enough parameters\r\n", servername.c_str(), user->GetNick().c_str());
    return S_OK;
  }
  
  std::string &channel_name = msg.GetParameters()[0];  
  it = user->GetChannels().find( channel_name );  
  if (it == user->GetChannels().end() )  
  {
    user->printf (":%s 442 %s %s :You're not on that channel\r\n", user->GetServerName().c_str(),
		  user->GetNick().c_str(), channel_name.c_str());
    return S_OK;
  }
     
  //Eliminar al usuario y borrar el canal si está vacío
  string part_msg = (msg.GetParameters().size() == 1 ? "" : msg.GetParameters()[1]);
  it->second->PartUser ( user, part_msg ); 
  if (it->second->GetUsers().empty())
  {
    channels.erase ( it->second );
    delete it->second;
  }
  user->GetChannels().erase(it);

  //Informar a los IRCOP
  printf_IRCOP(":%s!%s@%s PART %s :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(),
	       user->GetHostName().c_str(), channel_name.c_str(), part_msg.c_str() );

  
  return S_OK;
   
}


/***********************************************************************
*
*
*
*
************************************************************************/  
int IrcServer::OnJOIN ( IrcUser *user, IrcMessage& msg )
{
  IrcChannel *channel;
  channels_map_t::iterator it;
  
  //Comprobacion errores  
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }
            
  if ( msg.GetParameters().empty() )
  {
    user->printf (":%s 461 %s :Not enough parameters\r\n", servername.c_str(), user->GetNick().c_str());
    return S_OK;
  }  
  std::string &channel_name = msg.GetParameters()[0];  
  if ( channel_name[0] != '#' && channel_name[0] != '+' )
  {
    user->printf (":%s 403 %s %s :No such channel\r\n", servername.c_str(), user->GetNick().c_str(), channel_name.c_str() );  
    return S_OK;
  }
      
  if ( channel_name[0] == '0' )
  {
    //TODO: Quitar de todos los canales 
    return S_OK;
  }
  
  it = channels.find ( channel_name ); 
 
  if (it == channels.end())   //No se ha encontrado el canal
  { 
    //Crear canal
    channel = new IrcChannel( channel_name, "" );
    channels.insert(channel_name, channel); 
    channel->GetOperators().insert( user->GetNick(), user);  //soy operador   
  }
  else
  {
    channel = it->second;    
    if ( channel->GetUsers().find(user) != channel->GetUsers().end() )
      return S_OK;
    /*if ( channel->IsModeSet ( CHANNEL_MODE_i ) )
    {
      user->SendResponse ( ERR_INVITEONLYCHAN, channel_name.c_str());
      return S_OK;
    }    
    if ( (channel->GetMaxUsers() > 0 && channel->GetMaxUsers() >= channel->GetUsers().size() ) ||
         (channel->GetMaxUsers() > max_channelusers))
    {
      user->SendResponse ( ERR_CHANNELISFULL, channel_name.c_str());
      return S_OK;
    } */  
  }
         
  //Añadirse al canal   
  channel->GetUsers().insert(user->GetNick(), user);
  user->GetChannels().insert ( channel_name, channel );  
  channel->printf (":%s!%s@%s JOIN :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(), 
                   user->GetHostName().c_str(), channel_name.c_str() );
  //Enviar comando NAME LIST al usuario
  channel->SendNamesList ( user );
  //if ( channel->GetTopic().length() )
  channel->SendTopic ( user );

  //INFORMAR A TODOS LOS IRCOPS
  printf_IRCOP (":%s!%s@%s JOIN :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(), 
                   user->GetHostName().c_str(), channel_name.c_str() );
    
  
  return S_OK;
}

/***********************************************************************
*
*
*
*
************************************************************************/
int IrcServer::OnPRIVMSG ( IrcUser *user, IrcMessage& msg )
{
  if ( user->IsRegistered () == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }  
  //ERR_NORECIPIENT
  if ( msg.GetParameters().size() == 0)
  {
    user->printf(":%s 411 %s :No recipient given (PRIVMSG)\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }
 
  string& nick = msg.GetParameters()[0];  
  //ERR_NOTEXTTOSEND
  if ( msg.GetParameters().size() == 1)
  {
    user->printf (":%s 412 %s :No text to send\r\n", servername.c_str(), user->GetNick().c_str());
    return S_OK;
  }
  
  nick = msg.GetParameters()[0];
  if ( nick[0] == '#' || nick[0] == '+' ) //canal
  {
    channels_map_t::iterator it = channels.find ( nick );
    IrcChannel *channel;    
    //ERR_NOSUCHNICK
    if ( it == channels.end())
    {
      user->printf("%s 401 %s %s :No such nick/channel\r\n", servername.c_str(), user->GetNick().c_str(), nick.c_str());
      return S_OK;
    }    
    channel = it->second;
    
    //ERR_CANNOTSENDTOCHAN  
    /*if ( channel->IsModeSet (CHANNEL_MODE_n) && 
         channel->GetUsers().find(user ) == channel->GetUsers().end() )
    {
      user->SendResponse ( ERR_CANNOTSENDTOCHAN, nick.c_str() );
      return S_OK;
    } */   
    channel->SendMessage ( user, msg.GetParameters()[1] );    
    return S_OK;
  }
  else
  {
    users_map_t::iterator it = users.find ( nick );
    IrcUser *dst_user;    
    //ERR_NOSUCHNICK
    if ( it == users.end())
    {
      user->printf("%s 401 %s %s :No such nick/channel\r\n", servername.c_str(), user->GetNick().c_str(), nick.c_str());     
      return S_OK;
    }    
    dst_user = it->second;
    dst_user->printf (":%s!%s@%s PRIVMSG %s :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(), user->GetHostName().c_str(),
                      user->GetNick().c_str(), msg.GetParameters()[1].c_str() );
    return S_OK;  
  }
  return S_OK;
}

/***********************************************************************
*
*
*
*
************************************************************************/
int IrcServer::OnPONG ( IrcUser *user, IrcMessage& msg )
{
  if (msg.GetParameters().size() == 0)
    return S_OK;

  if ( !user->IsRegistered())
  {
    //modo por defecto
    //for (unsigned i = 0; i < def_user_mode.length(); ++i)
    //  user->SetMode (def_user_mode[i]);     
    user->printf(":%s 1 %s :Welcome to the Internet Relay Network %s!%s@%s\r\n", 
                 servername.c_str(), user->GetNick().c_str(), user->GetNick().c_str(), user->GetNick().c_str(), user->GetHostName().c_str() );
    user->printf(":%s 2 %s :Your host is %s, runing version %s\r\n", servername.c_str(), user->GetNick().c_str(),
                 user->GetServerName().c_str(), SERVER_VERSION );
    user->printf(":%s 3 %s :This server was created %s\r\n", servername.c_str(), user->GetNick().c_str(), SERVER_CREATION_DATE );
    user->printf(":%s 4  %s %s %s %s\r\n", servername.c_str(), user->GetNick().c_str(), user->GetServerName().c_str(), 
                 SERVER_VERSION, USER_MODES, CHANNEL_MODES );
    user->printf(":%s 375 %s :- Message of the day -\r\n"
                 ":%s 372 %s :-\r\n"
                 ":%s 376 %s :End of MOTD command\r\n", servername.c_str(), user->GetNick().c_str(), 
                 servername.c_str(), user->GetNick().c_str(), servername.c_str(), user->GetNick().c_str());       
    user->SetRegistered(true);
  }  
    
  time_t response = strtol (msg.GetParameters()[0].c_str(), 0, 16 );  
  if (response == user->GetNextPingTime() )
    user->SetConnectionAlive ( true );      
    
  return S_OK;
}


/***********************************************************************
*
*
*
*
************************************************************************/
int IrcServer::OnKICK ( IrcUser *user, IrcMessage& msg )
{
  channels_map_t::iterator it;  
  users_map_t::iterator it2;
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }   
  
  if ( msg.GetParameters().size() < 2 )
  {
    user->printf (":%s 461 %s :Not enough parameters\r\n", user->GetServerName().c_str(), user->GetNick().c_str());
    return S_OK;
  }    
  std::string &channel_name = msg.GetParameters()[0];
  std::string &nick_name = msg.GetParameters()[1];
  
  it = user->GetChannels().find( channel_name );  
  if (it == user->GetChannels().end() )  
  {
    user->printf (":%s 442 %s %s :You're not on that channel\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), channel_name.c_str());
    return S_OK;
  }
  
  if (it->second->GetOperators().find ( user ) == it->second->GetOperators().end())
  {
    user->printf (":%s 482 %s %s :You're not channel operator\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), channel_name.c_str());
    return S_OK;
  }
   
  it2 = it->second->GetUsers().find ( nick_name );
  if (it2 == it->second->GetUsers().end())
  {
    user->printf (":%s 441 %s %s %s :They aren't on that channel\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), nick_name.c_str(), channel_name.c_str());
    return S_OK;
  }
  
  it->second->printf (":%s!%s@%s KICK %s %s\r\n", user->GetNick().c_str(), user->GetNick().c_str(), user->GetHostName().c_str(),
                      channel_name.c_str(), nick_name.c_str());  
  it->second->GetUsers().erase (it2->second);
  it->second->GetOperators().erase (it2->second);

  //INFORMAR A TODOS LOS IRCOPS
  printf_IRCOP (":%s!%s@%s KICK %s %s\r\n", user->GetNick().c_str(), user->GetNick().c_str(), user->GetHostName().c_str(),
		channel_name.c_str(), nick_name.c_str());  
  
  return S_OK;
}

int IrcServer::OnWHO  ( IrcUser *user, IrcMessage& msg )
{
  string name = "";
    
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }      
  //Actualmente, solo aceptamos WHO #canal
  if (!msg.GetParameters().empty())
  {
    name = msg.GetParameters()[0];    
    if (name[0] == '#' || name[0] == '+') //canal
    {
      channels_map_t::iterator it = channels.find(name);
      if (it != channels.end() && (it->second->GetUsers().find(user) != it->second->GetUsers().end() ) )
      {
        users_map_t::iterator it2;
        for ( it2 = it->second->GetUsers().begin(); it2 != it->second->GetUsers().end(); ++it2)
        {
          char flags[5] = "H"; 
          if (it->second->GetOperators().find ( user ) == it->second->GetOperators().end())        
            strcat (flags, "@");        
    
          user->printf(":%s 352 %s %s %s %s %s %s %s :%d %s\n",
                user->GetServerName().c_str(), user->GetNick().c_str(), name.c_str(), it2->second->GetNick().c_str(), it2->second->GetHostName().c_str(),
                it2->second->GetServerName().c_str(), it2->second->GetNick().c_str(), flags, 0, it2->second->GetRealName().c_str());
        }
      }
    }
  }
  user->printf (":%s 315 %s %s :End of WHO list\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), name.c_str() );
  
  return S_OK;
}



int IrcServer::OnWHOIS  ( IrcUser *user, IrcMessage& msg )
{
  string name = "";
  //Comprobacion errores  
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }        
  //Actualmente, solo aceptamos WHOIS #usuario
  if (!msg.GetParameters().empty())
  {
    name = msg.GetParameters()[0];    
    users_map_t::iterator it = users.find(name);
    if (it != users.end() )
    {
      user->printf(":%s 311 %s %s %s %s * :%s\n",
              user->GetServerName().c_str(), user->GetNick().c_str(), name.c_str(), it->second->GetHostName().c_str(), it->second->GetNick().c_str(),
              it->second->GetRealName().c_str());
              
      channels_map_t::iterator it2;
      string channel_names = "";
      //Obtener los canales en los que está el usuario, y generar respuestas RPL_WHOISCHANNELS
      for (it2 = it->second->GetChannels().begin(); it2 != it->second->GetChannels().end(); ++it2)
      {
        if ( it2->second->GetOperators().find ( it->second ) != it2->second->GetOperators().end() )
          channel_names += "@";           
        channel_names += (it2->second->GetName() + " ");
        
        if (channel_names.length() > 100)
        {
          user->printf(":%s 319 %s %s :%s\n",
            user->GetServerName().c_str(), user->GetNick().c_str(), it->second->GetNick().c_str(), channel_names.c_str());
          channel_names = "";
        }
      }
      
      if (channel_names.length())
       user->printf(":%s 319 %s %s :%s\n",
            user->GetServerName().c_str(), user->GetNick().c_str(), it->second->GetNick().c_str(), channel_names.c_str());
      user->printf (":%s 318 %s %s :End of WHOIS list\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), name.c_str() );
    }
    else
    {
    /*
      ::printf("WHOIS: %s. NO SUCH NICK/CHANNEL\r\n", name.c_str());
      for (int i = 0; i < users.size(); ++i)
      {
        ::printf("%s ", users[i].second->GetNick().c_str());
      }
      ::printf("\n"); */
      user->printf("%s 401 %s %s :No such nick/channel\r\n", servername.c_str(), user->GetNick().c_str(), name.c_str());  
    }
  }
  else
      user->printf (":%s 431 %s :No nickname given\r\n", servername.c_str(), user->GetNick().c_str() );
  
  return S_OK;
}


int IrcServer::OnMODE ( IrcUser *user, IrcMessage& msg )
{
  return S_OK;
}

int IrcServer::OnOPER ( IrcUser *user, IrcMessage& msg )
{
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }  
  if ( msg.GetParameters().size() < 2 )
    {
      user->printf(":%s 461 %s :Not enough parameters\r\n",
		   servername.c_str(), user->GetNick().c_str());
      return S_OK;
    }


  std::string& name = msg.GetParameters()[0];
  std::string& password = msg.GetParameters()[1];

  if ( ValidatePassword( name, password) == true )
    {
      //OK
      operators.insert ( user->GetNick(), user );
      user->SetIrcOperator (true);
      user->printf (":%s 381 %s :You are now an IRC operator\r\n", servername.c_str(), user->GetNick().c_str() );
    }
  else
    {
      user->printf(":%s 464 %s :Password incorrect\r\n",
		   servername.c_str(), user->GetNick().c_str());
      return S_OK;
    }

  return S_OK;
}

int IrcServer::OnKILL ( IrcUser *user, IrcMessage& msg )
{
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }
  
  if ( !user->IsIrcOperator() )
    {
      user->printf(":%s 481 %s :Permission Denied- You're not an IRC operator\r\n", servername.c_str(),
		   user->GetNick().c_str() );
      return S_OK;
    }
 
  if ( msg.GetParameters().size() < 2 )
    {
      user->printf(":%s 461 %s :Not enough parameters\r\n",
		   servername.c_str(), user->GetNick().c_str());
      return S_OK;
    }

  std::string& nick_name = msg.GetParameters()[0];
  std::string& kill_message = msg.GetParameters()[1];

  users_map_t::iterator ptr;
  ptr = users.find(nick_name);
  if (ptr == users.end())
    {
      user->printf("%s 401 %s %s :No such nick/channel\r\n", servername.c_str(), user->GetNick().c_str(), nick_name.c_str());
      return S_OK;
    }

  //Matar al cliente informándolo de quien lo ha hecho (y por qué)
  //Al resto de los usuarios, ocultaremos la identidad del operador
  ptr->second->printf ("ERROR :Killed by %s: %s\r\n", user->GetNick().c_str(), kill_message.c_str());            
  CloseConnection ( ptr->second, kill_message );  
  return S_CONNECTION_CLOSED;
}


int IrcServer::OnTOPIC ( IrcUser *user, IrcMessage& msg )
{
  //Comprobacion errores  
  if ( user->IsRegistered() == false )
  {
    user->printf(":%s 451 %s :You have not registered\r\n", servername.c_str(), user->GetNick().c_str() );
    return S_OK;
  }
        
  if (msg.GetParameters().empty())
  {
    user->printf (":%s 461 %s :Not enough parameters\r\n",
                  servername.c_str(), user->GetNick().c_str());
    return S_OK;     
  }
  
  string& channel_name = msg.GetParameters()[0];
  channels_map_t::iterator it = user->GetChannels().find( channel_name );  
  if (it == user->GetChannels().end())
  {
    user->printf (":%s 442 %s %s :You're not on that channel\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), channel_name.c_str());
    return S_OK;
  }
  
  IrcChannel *channel = it->second;
  if (msg.GetParameters().size() < 2) //desea conocer el topic del canal
  {
    if ( channel->GetTopic().empty())
    {
      user->printf (":%s 331 %s %s :No topic is set\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), channel_name.c_str());
      return S_OK;
    } 
    user->printf (":%s 332 %s %s :%s\r\n", user->GetServerName().c_str(), user->GetNick().c_str(), channel_name.c_str(), channel->GetTopic().c_str());
  }
  else //cambiar el topic
  {
    string& topic = msg.GetParameters()[1];
    channel->SetTopic ( topic.c_str() );
    channel->printf (":%s!%s@%s TOPIC %s :%s\r\n", user->GetNick().c_str(), user->GetNick().c_str(), 
                        user->GetHostName().c_str(), channel_name.c_str(), topic.c_str());
  }
  return S_OK;
}

