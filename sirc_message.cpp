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

#include <string>
#include "sirc_message.h"
#include "sirc_user.h"
#include <algorithm>

using namespace std;

IrcMessage::IrcMessage ( IrcUser *_user, const char *msg_string, bool parse_prefix )
  : user(_user), prefix(), name(), parameters()
{
  //Analizar la cadena
  const char *ptr1, *ptr2, *end;
  string parameter;
  
  end = msg_string + strlen(msg_string) ;
  ptr1 = msg_string;
  //quitar espacios a ambos lados
  while (end > msg_string && *(end-1) == ' ' ) --end;  
  
  while (ptr1 < end && *ptr1 == ' ') ++ptr1;
  if (ptr1 == end )
    return;  
    
  if (*ptr1 == ':') //prefijo
  {
    //Buscar el espacio
    ptr2 = ptr1;
    while ( ptr2 < end && *(++ptr2) != ' ');
    prefix = string ( ptr1+1, ptr2 );
    ptr1 = ptr2;      
    while (ptr1 < end && *ptr1 == ' ') ++ptr1;  
    if (ptr1 == end)
      return;     
  }      
    
  //nombre del comando
  ptr2 = ptr1;
  while ( ptr2 < end && *(++ptr2) != ' ');
  name = string ( ptr1, ptr2 );   
  transform(name.begin(), name.end(), name.begin(), static_cast<int(*)(int)>( toupper) );
  
  ptr1 = ptr2;        
  while (ptr1 < end && *ptr1 == ' ') ++ptr1;
  if (ptr1 == end )
    return;  
     
  //parametros  
  while (ptr1 < end)
  {  
    if (*ptr1 == ':')
    {
      //ultimo parametro (con espacios permitidos)
      parameter = string (ptr1+1, end);
      ptr1 = end;
    }
    else
    {        
      ptr2 = ptr1; 
      while ( ptr2 < end && *(++ptr2) != ' ');        
      parameter = string ( ptr1, ptr2 );   
      ptr1 = ptr2;        
      while (ptr1 < end && *ptr1 == ' ') ++ptr1;
    }   
    if (parameter.length())
      parameters.push_back (parameter);
  }
  
  if (name[0] >= '0' && name[0] <= '9')
    response = atoi (name.c_str());
  else
    response = -1;


  if (parse_prefix )
    ParsePrefix();
}



void IrcMessage::ParsePrefix ()
{
  int p = prefix.find ('!');
  prefix_nick = prefix.substr (0, p);
  int q = prefix.find ('@');
  p = p+1;
  prefix_user = prefix.substr (p, q-p);
  prefix_host = prefix.substr (q+1);
}
