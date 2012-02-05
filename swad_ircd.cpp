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
#include <unistd.h>
#include "sirc.h"
#include <locale.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
using namespace std;


int ValidatePassword (const std::string& name, const std::string& pass )
{
  return 1; //por ahora todos son validos
}


int main (int argc, char **argv)
{

#if 0  //ejecutar como demonio
  pid_t pid, sid; 
  pid = fork();
  if (pid < 0)
  {
    exit(EXIT_FAILURE);
  }
  if (pid > 0) 
  {
    exit(EXIT_SUCCESS);
  }
  umask(0);
  sid = setsid();
  if (sid < 0) 
  {
    exit(EXIT_FAILURE);
  }
 
  if ((chdir("/")) < 0) 
  {
    exit(EXIT_FAILURE);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
#endif
  

  //setlocale(LC_ALL, "es_ES.ISO-8859-1");
  IrcServer* server = new IrcServer();   
  char hostname[1024];  
  gethostname ( hostname, 1024 );   
  
  server->Open ( 5000 );    
  server->SetServerName ( "http://swad.ugr.es" );  
  server->SetValidatePasswordHandle ( ValidatePassword );
  while (true)
  {
    server->RunStep ();       
    sched_yield();
  }
  
  return 0;
}
