/*******************************************************************************
   This file is part of libTMCG.

 Copyright (C) 2002-2004 Heiko Stamer, <stamer@gaos.org>

   libTMCG is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*******************************************************************************/

#include "parse_helper.hh"

// simple methods for parsing
bool cm
	(std::string &s, const std::string &c, char p)
{
	size_t ei;
	if ((ei = s.find(p, 0)) != s.npos)
	{
		if (s.substr(0, ei) != c)
			return false;
		else
			s = s.substr(ei + 1, s.length() - ei - 1);
	}
	else
		return false;
	return true;
}

bool nx
	(std::string &s, char p)
{
	size_t ei;
	if ((ei = s.find(p, 0)) != s.npos)
		s = s.substr(ei + 1, s.length() - ei - 1);
	else
		return false;
	return true;
}

const char *gs
	(const std::string &s, char p)
{
	size_t ei;
	if ((ei = s.find(p, 0)) != s.npos)
		return (s.substr(0, ei)).c_str();
	else
		return NULL;
}