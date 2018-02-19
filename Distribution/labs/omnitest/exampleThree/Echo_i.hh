// Echo_i.hh --- 
// 
// Filename: Echo_i.hh
// Description: 
// Author: Joseph
// Maintainer: 
// Created: Mon Feb 19 21:03:03 2018 (+0000)
// Version: 
// Package-Requires: ()
// Last-Updated: Mon Feb 19 21:09:07 2018 (+0000)
//           By: Joseph
//     Update #: 3
// URL: 
// Doc URL: 
// Keywords: 
// Compatibility: 
// 
// 

// Commentary: 
// 
// 
// 
// 

// Change Log:
// 
// 
// 
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.
// 
// 

// Code:

#include "Echo.hh"

// This is the object implementation.
//Just inherit from POA_Echo to get all Corba goodies
//Implement the interface as normal
class Echo_i : public POA_Echo
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};




// 
// Echo_i.hh ends here
