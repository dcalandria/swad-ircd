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

#ifndef __sirc_plugin_h
#define __sirc_plugin_h
  
struct IrcPlugin
{  
  
};

struct IrcMessageHook
{
  string message;

  //Devuelve:
  //   true: procesar mensaje
  //   false: no procesar mensaje
  int (*handle) ();
}

struct IrcPluginDef
{
  void (*Create) ();
  void (*Destroy) ();
  void (*Init) ();
};

#endif
