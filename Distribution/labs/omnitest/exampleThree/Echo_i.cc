// Echo_i.cc --- 
// 
// Filename: Echo_i.cc
// Description: 
// Author: Joseph
// Maintainer: 
// Created: Mon Feb 19 21:07:14 2018 (+0000)
// Version: 
// Package-Requires: ()
// Last-Updated: Mon Feb 19 21:07:49 2018 (+0000)
//           By: Joseph
//     Update #: 1
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
#include "Echo_i.hh"

char* Echo_i::echoString(const char* mesg)
{
  // Memory management rules say we must return a newly allocated
  // string.
  return CORBA::string_dup(mesg);
}

// 
// Echo_i.cc ends here
