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

#include "sockets_cpp.h"
#include <algorithm>
#include <errno.h>
#include <iostream>

SocketServerNonBlock::SocketServerNonBlock () 
: listener (-1), max_descriptor (0), static_tbl(), dynamic_tbl(), sessions()
{

}

SocketServerNonBlock::~SocketServerNonBlock ()
{
  Close();
}

SocketServerNonBlock::SocketServerNonBlock ( int port, unsigned max_connections, unsigned max_queue, time_t _timeout)
: listener (-1), max_descriptor (0), static_tbl(), dynamic_tbl(), sessions()
{
  Open ( port, max_connections, max_queue, timeout);
}

SocketServerNonBlock::SocketServerNonBlock ( const SocketServerNonBlock &src )
{
  listener = src.listener;
  max_descriptor = src.max_descriptor;
  static_tbl = src.static_tbl;
  dynamic_tbl = src.dynamic_tbl;
  sessions = src.sessions;
  
  sessions.reserve (src.sessions.capacity());
}
  
SocketServerNonBlock& SocketServerNonBlock::operator= ( const SocketServerNonBlock &src)
{
  if (&src != this)
  {
    listener = src.listener;
    max_descriptor = src.max_descriptor;
    static_tbl = src.static_tbl;
    dynamic_tbl = src.dynamic_tbl;
    sessions = src.sessions;
  
    sessions.reserve (src.sessions.capacity());    
  }
  return *this;
}
  
int SocketServerNonBlock::Open  ( int port, unsigned max_connections, unsigned max_queue, time_t timeout_ )
{
  Close ();

  sockaddr_in address;

  listener = socket ( AF_INET, SOCK_STREAM, 0);
  if (listener == -1)
    return E_FAIL;

  int optval = 1;
  if ( setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    return E_FAIL;
  
  address.sin_family = AF_INET;
  address.sin_port = htons (port);
  address.sin_addr.s_addr = INADDR_ANY;
  
  if ( bind (listener, reinterpret_cast<sockaddr*>(&address), sizeof (address)) == -1)
    return E_FAIL;

  if ( listen (listener, max_queue) == -1)
    return E_FAIL;

  sessions.reserve (max_connections);
  
  FD_ZERO ( &static_tbl );
  FD_ZERO ( &dynamic_tbl );  
  FD_SET  ( listener, &static_tbl );
  max_descriptor = listener;
  timeout = timeout_;
  
  return listener;	 
}
   

int SocketServerNonBlock::Close ( )
{
  while (!sessions.empty())
  {
    SocketSessionNonBlock *session = dynamic_cast<SocketSessionNonBlock*> (sessions.back ());
    if (session->socket != -1)
    {
      if (session->socket >= 0)
        close ( session->socket );
      session->socket = -1;
      delete session;
      sessions.pop_back();      
    }  
  }
  if (listener >= 0)
    close ( listener );
  listener = -1;
 
  FD_ZERO ( &static_tbl );
  FD_ZERO ( &dynamic_tbl );  
 
  return S_OK;
}
           
int SocketServerNonBlock::SetMaxConnections ( unsigned max_connections )
{
  sessions.reserve (max_connections );  
  return S_OK;
}
      
int SocketServerNonBlock::Update ( unsigned usec )
{ 
  timeval tv;
  int res = 0;
  
  tv.tv_sec =  usec / 1000000;
  tv.tv_usec = usec % 1000000;

  FD_ZERO ( &dynamic_tbl );
  dynamic_tbl = static_tbl;  
  res = select(max_descriptor + 1, &dynamic_tbl, 0,  0, &tv);

  //Timeouts
  std::vector < SocketSessionNonBlock* >::iterator it;
  time_t cur_time = time(0);
  for (it = sessions.begin(); it != sessions.end(); ++it)
  {
    if (timeout && ((*it)->GetTimeout() < cur_time) )   
    {
      close ( (*it)->socket );
      (*it)->is_timeout = true;
    }     
  }   
  //
    

  if (res < 0)
    return E_FAIL;  
  return res;
}

SocketSession* SocketServerNonBlock::WaitConnection ( )
{
  SocketSessionNonBlock *session = new SocketSessionNonBlock(timeout);
  
  if ( FD_ISSET ( listener, &dynamic_tbl ) )
  {
    session->socket = accept ( listener, 0, 0);
    if (session->socket <  0)
    {
      delete session;  
      return 0;      
    }
    //session->socket_stream = fdopen ( session->socket, "r+");          
    if (sessions.size() < sessions.capacity())
    {
      session->server = this;
      session->idx = sessions.size();
      sessions.push_back (session);
    
      
      FD_SET ( session->socket, &static_tbl ) ;
      max_descriptor = std::max<int>( max_descriptor, session->socket )  ;  
      return session;    
    }

    //Demasiadas conexiones
    session->Close ();    
  }
  delete session;
  return 0;
}
      
size_t SocketServerNonBlock::Broadcast ( const char *msg, size_t size )
{
  return 0;
}
  
int SocketServerNonBlock::CloseSession ( SocketSessionNonBlock* session )
{
  if (!session)
    return E_FAIL;
    
  //SocketSessionNonBlock *session = reinterpret_cast<SocketSessionNonBlock*> (_session);
  
  if (session->socket != -1 )
  {
    FD_CLR ( session->socket, &static_tbl );
    FD_ZERO ( &dynamic_tbl );
    
    //close ( session->socket );
    /* if ( session->socket_stream > 0 )
      fclose ( session->socket_stream ); */
    if (session->socket >= 0 && !session->is_timeout)
      close ( session->socket );
    session->socket = -1;   
    //session->socket_stream = 0;
    std::swap ( sessions[ session->idx ], sessions.back() );
    //reinterpret_cast<SocketSessionNonBlock*> (sessions[session->idx])->idx = session->idx;
    sessions[session->idx]->idx = session->idx;    
    sessions.pop_back();
    
    if (max_descriptor == session->socket ) //calcular el nuevo maximo
    {
      max_descriptor = -1;
      for (unsigned i = 0; i < sessions.size(); ++i)
      {
        //session = reinterpret_cast<SocketSessionNonBlock*> ( sessions[i] );
        //max_descriptor = std::max<int> (max_descriptor, session->socket );
        max_descriptor = std::max<int> (max_descriptor, sessions[i]->socket );
      }
    }
  }
       
  return S_OK;
}

  
ssize_t SocketSessionNonBlock::Read ( char *buf, size_t len )
{
  if (socket == -1  )
    return E_RW_ERROR;
  else if ( is_timeout )
    return E_TIMEOUT;          //se habra cerrado la conexion
        
  if (FD_ISSET ( socket, &server->dynamic_tbl ) )
  {
    ssize_t res = recv ( socket, buf, len, 0 );
    if (res < 0)
      return E_RW_ERROR;            //error
    else if (res == 0)
      return S_CONNECTION_CLOSED;   //conexion cerrada
    else
    {
      next_timeout = time(0) + timeout;      
      return res; 
    }
  }
  return 0;
}

ssize_t SocketSessionNonBlock::Write (const char *buf, size_t len )
{
  if ( socket < 0 )
    return E_RW_ERROR;
    
  const char *ptr = buf;
  const char *end = buf + len;
  while ( ptr < end )
  {
    ssize_t res = send (socket, ptr, len, MSG_NOSIGNAL | MSG_DONTWAIT );
    if ((res < 0) && (res != EAGAIN))
      return E_RW_ERROR;
    else if (res == EAGAIN)
      return E_RW_RETRY;
    else if (res > 0)
    {
      ptr += res;            
      len -= res;
    }
  }        
  return 0;
}


int SocketSessionNonBlock::Close ()
{
  if ( server )
  {
    server->CloseSession (this);
  }
  else //La sesion no se introdujo en la lista del servidor
  {
    if (socket>= 0)
      close(socket);
    socket = -1;  
  }
  return S_OK;
}
  
SocketSessionNonBlock::SocketSessionNonBlock ( time_t timeout_)
: socket(-1), idx(-1), server(0), is_timeout(false)
{
  timeout = timeout_;
  next_timeout = time(0) + timeout;
}

SocketSessionNonBlock::~SocketSessionNonBlock ()
{
  Close();
}


int SocketClient::Connect ( const char *server_name, int port )
{
  sockaddr_in serv_addr;

  hostent* host = gethostbyname (server_name);
  if (host == NULL)
    return E_FAIL;
  sckt = socket (AF_INET, SOCK_STREAM, 0);
  if (sckt == -1)
    return E_FAIL;

  memset ( &serv_addr, 0, sizeof(serv_addr));
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
  
  if (connect(sckt, (sockaddr*) &serv_addr, sizeof(serv_addr) ) < 0) 
    return E_FAIL;

  FD_ZERO(&select_flag);
  FD_SET(sckt, &select_flag);
  
  return S_OK;
}

int SocketClient::Close ()
{
  if (sckt >= 0)
  {
    FD_CLR ( sckt, &select_flag);
    close (sckt);
    sckt = -1;
  } 
  return S_OK;
}

ssize_t SocketClient::Read ( char *buf, size_t len, unsigned usec )
{
  if (sckt == -1  )
    return E_RW_ERROR;
  
  timeval tv;  
  tv.tv_sec =  usec / 1000000;
  tv.tv_usec = usec % 1000000;

  FD_SET(sckt, &select_flag);
  if ( select(sckt + 1, &select_flag, 0,  0, &tv) < 0 )
    return 0;

  ssize_t res = 0;
  if (FD_ISSET ( sckt, &select_flag ) )
    {
      res = recv(sckt, buf, len,0);
     
      if (res < 0)
	return E_RW_ERROR;
      
      if (res == 0)
	return S_CONNECTION_CLOSED;      
    }  
  return res;
}

ssize_t SocketClient::Write ( const char *buf, size_t len )
{
  if ( sckt < 0 )
    return E_RW_ERROR;
    
  const char *ptr = buf;
  const char *end = buf + len;
  while ( ptr < end )
  {
    ssize_t res = send (sckt, ptr, len, MSG_NOSIGNAL | MSG_DONTWAIT );
    if ((res < 0) && (res != EAGAIN))
      return E_RW_ERROR;
    else if (res == EAGAIN)
      return E_RW_RETRY;
    else if (res > 0)
    {
      ptr += res;            
      len -= res;
    }
  }        
  
  return 0;
}

SocketClient::SocketClient () : sckt(-1) {}

SocketClient::~SocketClient ()
{
  Close();
}

    
//ssize_t SocketSessionNonBlock::printf ( const char *fmt, ... )
//{
 // va_list list;

  //va_start( list, fmt );
  
  //ssize_t res = vfprintf ( socket_stream, fmt, list );
   
  //va_end( list );

  //fsync ( socket);
  //return res;
//}

//ssize_t SocketSessionNonBlock::vprintf ( const char *fmt, va_list arg )
//{
  //ssize_t res = vfprintf ( socket_stream, fmt, arg );
  //fsync ( socket );
//  return res;
//}

