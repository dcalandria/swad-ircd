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

#include <iostream>
#include "text_protocol.h"

ssize_t TextProtocolSession::ReadMessage ( char *dst )
{
  //Leer buffer  
  ssize_t res = socket_session->Read ( buffer + buf_ptr, buffer_size - buf_ptr );
 
  if ( res >= 0 )
  {   
    //Copiar al mensaje
    buf_ptr += res;
    for (unsigned i = 0; i < buf_ptr; ++i)
    {
      if (buffer[i] == '\r' || buffer[i] == '\n' ) //ya tenemos el mensaje
      {
        msg[msg_ptr] = '\0';
                        
        std::memcpy ( dst, msg, msg_ptr + 1);
        res = msg_ptr;        
        msg_ptr = 0;
          
        //Desplazar buffer
        ++i;        
        if ((i < buf_ptr) && (buffer[i] == '\r' || buffer[i] == '\n'))
          ++i;          
        for (unsigned j = i, k = 0; j < buf_ptr; ++j, ++k)
          buffer[k] = buffer[j];
        buf_ptr = buf_ptr - i;
        return res;
      }             
 
      //if (buffer[i] > 0 && buffer[i] < 240)
      //{        
        msg[msg_ptr++] = buffer[i];       
        if (msg_ptr == max_msglength)
          msg_ptr = 0;      //DESBORDAMIENTO! NO DEBE PASAR!
      //}
      
	
    }
    buf_ptr = 0;       
    return 0;
  }  
  return res;    
}

ssize_t TextProtocolSession::WriteMessage ( const char *src )
{
  return socket_session->Write ( src, std::strlen ( src ) );
}


ssize_t TextProtocolSession::printf ( const char *fmt, ... )
{
  va_list list;

  va_start( list, fmt );
  //ssize_t res = socket_session->vprintf ( fmt, list );
  
  ssize_t res = vsnprintf ( str_aux, max_msglength + 1, fmt, list );
  res = socket_session->Write ( str_aux, res ); 
  
  va_end( list );

  return res;
}

ssize_t TextProtocolSession::vprintf ( const char *fmt, va_list arg )
{
  //return socket_session->vprintf ( fmt, arg );
  
  ssize_t res = vsnprintf ( str_aux, max_msglength + 1, fmt, arg );
  return socket_session->Write ( str_aux, res ); 
}

int TextProtocolServer::Open ( int port, unsigned max_msg_length, unsigned buffer_size, unsigned max_connections, unsigned max_queue)
{
  Close ();
  
  this->max_msg_length = max_msg_length;
  this->buffer_size = buffer_size;
  
  socket_server = new SocketServerNonBlock ();
  socket_server->Open ( port, max_connections, max_queue );
  
  msg = new char [ max_msg_length + 1];
  std::memset ( msg, 0, max_msg_length + 1);
  return S_OK;  
}


int TextProtocolServer::Close ()
{
  while (! sessions.empty() ) 
  {    
    delete sessions.back();
    sessions.pop_back();
  }
  
  if (socket_server)
  {
    socket_server->Close ();    
    delete socket_server;
    socket_server = 0;
  }
  
  if (msg )
    delete [] msg;
  msg = 0;
  
  return S_OK;
}

int TextProtocolServer::RunStep ()
{
  Update ();
  WaitConnection ();
  
  for (unsigned i = 0; i < sessions.size(); ++i)
  {
    ssize_t res = GetMessage ( sessions[i], msg );
    if (res == S_CONNECTION_CLOSED || res == E_TIMEOUT)
    {
      CloseConnection ( sessions[i] );
      --i;
    }
    else if (res > 0)
    {
      DispatchMessage ( sessions[i], msg);          
      PostProcess ( sessions[i], msg );
    }
  }
    
  return S_OK;
}

int TextProtocolServer::Update ()
{
  socket_server->Update ( 100000 );
  return S_OK;
}

int TextProtocolServer::WaitConnection ()
{  
  SocketSessionNonBlock *new_socket_session;  
  new_socket_session = reinterpret_cast<SocketSessionNonBlock*> (socket_server->WaitConnection());
  
  if (new_socket_session )
  {
    TextProtocolSession *session = new TextProtocolSession(new_socket_session, max_msg_length, buffer_size );
    sessions.push_back ( session );    
  }
  
  return S_OK;
}

int TextProtocolServer::CloseConnection ( TextProtocolSession *session )
{
  for (unsigned i = 0; i < sessions.size(); ++i)
    if (sessions[i] == session)
    {
      std::swap (sessions[i], sessions.back());
      sessions.pop_back();
      break;
    } 
  std::printf (": Connection closed!\r\n" );
  delete session;
  
  return S_OK;
}

int TextProtocolServer::GetMessage ( TextProtocolSession *session, char *msg)
{
  return session->ReadMessage ( msg );
}

int TextProtocolServer::DispatchMessage ( TextProtocolSession *session, char *msg )
{
  std::printf (": %s\r\n", msg);
  
  return S_OK;
}

int TextProtocolServer::PostProcess ( TextProtocolSession *session, char *msg )
{
  return S_OK;
}

int TextProtocolClient::Open ( const char *server_name, int port, unsigned msg_length, unsigned buffer_size )
{
  if (  socket.Connect (server_name, port ) < 0 )
    return E_FAIL;

  max_msglength = msg_length;
  this->buffer_size = buffer_size;

  msg = new char[msg_length+1];
  str_aux = new char[msg_length+1];
  buffer = new char[buffer_size+1];

  std::memset ( msg, 0, msg_length+1 );
  std::memset ( str_aux, 0, msg_length+1);
  std::memset ( buffer, 0, buffer_size+1);

  buf_ptr = 0;
  msg_ptr = 0;
  
  return S_OK;
  
}

int TextProtocolClient::Close ()
{
  if (msg)
    delete [] msg;
  if (buffer)
    delete [] buffer;

  msg = 0;
  buffer = 0;
  str_aux = 0;
  
  socket.Close ();

  return S_OK;
}

ssize_t TextProtocolClient::ReadMessage ( char *dst )
{
  //Leer buffer  
  ssize_t res = socket.Read ( buffer + buf_ptr, buffer_size - buf_ptr );

  dst[0] = '\0';
  if ( res >= 0 )
  {   
    //Copiar al mensaje
    buf_ptr += res;
    for (unsigned i = 0; i < buf_ptr; ++i)
    {
      if (buffer[i] == '\r' || buffer[i] == '\n' ) //ya tenemos el mensaje
      {
        msg[msg_ptr] = '\0';
                        
        std::memcpy ( dst, msg, msg_ptr + 1);
        res = msg_ptr;        
        msg_ptr = 0;
          
        //Desplazar buffer
        ++i;        
        if ((i < buf_ptr) && (buffer[i] == '\r' || buffer[i] == '\n'))
          ++i;          
        for (unsigned j = i, k = 0; j < buf_ptr; ++j, ++k)
          buffer[k] = buffer[j];
        buf_ptr = buf_ptr - i;
        return res;
      }             
      //if (buffer[i] > 0 && buffer[i] < 240)
      //{        
        msg[msg_ptr++] = buffer[i];       
        if (msg_ptr == max_msglength)
          msg_ptr = 0;      //DESBORDAMIENTO! NO DEBE PASAR!
      //}   	
    }
    buf_ptr = 0;       
    return 0;
  }  
  return res;   
}

ssize_t TextProtocolClient::WriteMessage ( const char *src )
{
  return socket.Write ( src, std::strlen(src) );
}

ssize_t TextProtocolClient::printf ( const char *fmt, ... )
{
  va_list list;

  va_start( list, fmt );
  //ssize_t res = socket_session->vprintf ( fmt, list );
  
  ssize_t res = vsnprintf ( str_aux, max_msglength + 1, fmt, list );
  res = socket.Write ( str_aux, res ); 
  
  va_end( list );

  return res;  
}

ssize_t TextProtocolClient::vprintf ( const char *fmt, va_list arg )
{
  ssize_t res = vsnprintf ( str_aux, max_msglength + 1, fmt, arg );
  return socket.Write ( str_aux, res ); 
}
