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

#ifndef __text_protocol_h
#define __text_protocol_h

#include "sockets_cpp.h"

class TextProtocolSession
{
protected:
  SocketSessionNonBlock *socket_session;  
  
  size_t buffer_size;
  size_t max_msglength;
  
  char *buffer;
  char *msg;  
  unsigned buf_ptr;
  unsigned msg_ptr;
  char *str_aux;
public:
  
  TextProtocolSession()
  : socket_session(0), buffer_size(0), max_msglength(0), buffer(0), msg(0), buf_ptr (0), msg_ptr (0) { }
  
  virtual ~TextProtocolSession()
  {   
    if (buffer)
      delete [] buffer;
    if (msg)
      delete [] msg;
    if (socket_session) 
      delete socket_session;
    if (str_aux)
      delete [] str_aux;
  }
  
  TextProtocolSession ( SocketSessionNonBlock *socket_session, size_t max_msglength, size_t buffer_size )
  : socket_session(0), buffer_size(0), max_msglength(0), buffer(0), msg(0) , buf_ptr (0), msg_ptr (0)
  {
    this->socket_session = socket_session;
    SetMaxMessageLength (max_msglength);
    SetBufferSize ( buffer_size);
  }

  virtual size_t GetBufferSize () const
  {
    return buffer_size;   
  }
  
  virtual void SetBufferSize (size_t buffer_size )
  {
    this->buffer_size = buffer_size;
    if (buffer)
      delete [] buffer;
    buffer = new char[buffer_size];
    std::memset (buffer, 0, buffer_size);
  }
  
  virtual size_t GetMaxMessageLength () const 
  { 
    return max_msglength; 
  }
    
  virtual void SetMaxMessageLength ( size_t max_msglength )
  {
    this->max_msglength = max_msglength;
    if (msg)
      delete [] msg;
    msg = new char [ max_msglength + 1 ];
    std::memset (msg, 0, max_msglength + 1);
    str_aux = new char [ max_msglength + 1];
    std::memset ( str_aux, 0, max_msglength + 1);
  
  }
    
  virtual int Close ()
  {
    if (buffer)
      delete [] buffer;
    if (msg)
      delete [] msg;
    if (str_aux)
      delete [] str_aux;
    
    str_aux = 0;
    msg  = 0;
    buffer = 0;
    
    if (socket_session)
      delete socket_session;
    return S_OK;
  }
  
  virtual ssize_t ReadMessage ( char *dst ) ;   
  virtual ssize_t WriteMessage ( const char *src );
  virtual SocketSessionNonBlock* GetSocket () { return socket_session; }
  virtual const SocketSessionNonBlock* GetSocket () const { return socket_session; }  
  
  virtual ssize_t printf ( const char *fmt, ... );
  virtual ssize_t vprintf ( const char *fmt, va_list arg );
};


class TextProtocolServer
{
protected:
  SocketServerNonBlock *socket_server;
  std::vector < TextProtocolSession* > sessions;


  unsigned max_msg_length;
  unsigned buffer_size;  
  char *msg;
public:
  TextProtocolServer ()
    : socket_server (0), sessions() { }
  
  virtual ~TextProtocolServer ()
  {
    Close();
  }
  
  virtual int Open ( int port, unsigned msg_length = 1024, unsigned buffer_size = 128, 
                     unsigned max_connections = DEF_MAX_CONNECTIONS, unsigned max_queue = DEF_MAX_QUEUE );
  virtual int Close ();
  virtual int RunStep ();
  
  virtual int Update ();
  virtual int WaitConnection ();
  virtual int CloseConnection ( TextProtocolSession *session );
  virtual int GetMessage ( TextProtocolSession *session, char *msg);
  virtual int DispatchMessage ( TextProtocolSession *session, char *msg );
  virtual int PostProcess ( TextProtocolSession *session, char *msg );            
};

class TextProtocolClient
{
protected:
  SocketClient socket;
  
  size_t buffer_size;
  size_t max_msglength;
  char *buffer;
  char *msg;  
  unsigned buf_ptr;
  unsigned msg_ptr;
  char *str_aux;
public:

  TextProtocolClient () : buffer_size(0), max_msglength(0), buffer(0), msg(0), buf_ptr(0), msg_ptr(0), str_aux(0) {}
  virtual ~TextProtocolClient ()
    {
      if (buffer)
	delete [] buffer;
      if (msg)
	delete [] msg;
      if (str_aux)
	delete [] str_aux;
    }
  
  virtual int Open ( const char *server_name, int port, unsigned msg_length = 1024, unsigned buffer_size = 128 );
  virtual int Close ();
  virtual ssize_t ReadMessage ( char *dst ) ;   
  virtual ssize_t WriteMessage ( const char *src );
  virtual ssize_t printf ( const char *fmt, ... );
  virtual ssize_t vprintf ( const char *fmt, va_list arg );  
};


#endif
