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

#ifndef __swad_socket_h
#define __swad_socket_h

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdarg>
#include <time.h>

#include <vector>

enum
{
  E_RW_ERROR = -2,    
  E_TIMEOUT  = -8,    
  E_RW_RETRY = -1,    
  E_FAIL     = -1,    
};

enum
{
  S_CONNECTION_CLOSED = -1, 
  S_OK = 0,                 
};

const unsigned DEF_MAX_CONNECTIONS = 1000;
const unsigned DEF_MAX_QUEUE = 100;
const unsigned DEF_TIMEOUT   = 100;

class SocketSession
{
public:
  virtual ssize_t Read ( char *buf, size_t len ) = 0;
  virtual ssize_t Write (const char *buf, size_t len ) = 0;    
  virtual int Close () = 0;
  //virtual ssize_t printf ( const char *fmt, ... ) = 0;
  virtual ~SocketSession() {}
};

class SocketServer
{
public:
  virtual int Open ( int port, unsigned max_connections, unsigned max_queue, time_t timeout = DEF_TIMEOUT) = 0;
  virtual int Close () = 0;
  virtual SocketSession* WaitConnection () = 0;        
  
  virtual ~SocketServer() {}
};   


class SocketSessionNonBlock;
  
class SocketServerNonBlock : public SocketServer
{
protected:       
  time_t  timeout;
  int listener;           
  int max_descriptor;
  fd_set static_tbl;
  fd_set dynamic_tbl;    
  std::vector < SocketSessionNonBlock* > sessions;         
public:
  SocketServerNonBlock ();
  virtual ~SocketServerNonBlock ();
  
  SocketServerNonBlock ( int port, unsigned max_connections = DEF_MAX_CONNECTIONS, unsigned max_queue = DEF_MAX_QUEUE,
                         time_t timeout = DEF_TIMEOUT );
  SocketServerNonBlock ( const SocketServerNonBlock &src );
  
  SocketServerNonBlock& operator= ( const SocketServerNonBlock &src);                
public:
  virtual int Open  ( int port, unsigned max_connections = DEF_MAX_CONNECTIONS, unsigned max_queue = DEF_MAX_QUEUE,
                      time_t timeout = DEF_TIMEOUT  );
  virtual int Close ( );      
  virtual int SetMaxConnections ( unsigned max_connections );
  virtual int Update ( unsigned usec = 500000 );
            
  SocketSession* WaitConnection ( );
  size_t Broadcast ( const char *msg, size_t size );

  friend class SocketSessionNonBlock;  
protected:  
  int CloseSession ( SocketSessionNonBlock* session );
};


class SocketSessionNonBlock : public SocketSession
{
protected:
  int  socket;
  int  idx;
  bool selected;  
  time_t timeout; 
  time_t next_timeout;
  SocketServerNonBlock *server;
  bool is_timeout;
public:   

  SocketSessionNonBlock( time_t timeout = 0 );
  virtual ~SocketSessionNonBlock();

  virtual ssize_t Read  (char *buf, size_t len);
  virtual ssize_t Write (const char *buf, size_t len ); 
  virtual int Close ();
      
  virtual time_t GetTimeout() const { return next_timeout; }
  virtual void SetTimeout (time_t timeout_)
  {        
    next_timeout = time(0) + timeout_;  
    timeout = timeout_;
  }
   
  friend class SocketServerNonBlock;
};

/* TODO */
class SocketClient
{
protected:
  int sckt;
  fd_set select_flag;
public:

  SocketClient ();
  virtual ~SocketClient ();

  virtual int Connect ( const char *server_name, int port );
  virtual int Close ();
  
  virtual ssize_t Read (  char *buf, size_t len, unsigned usec = 500000 );
  virtual ssize_t Write ( const char *buf, size_t len );
};
  
#endif

