/*
 *    Copyright (C) 2025  Mason Sanders
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __RULE_H__
#define __RULE_H__

#include <vector>
#include <string>

class Symbol
{
public:
	bool isTerminal;
	std::string name;
};

class Rule
{
public:
	std::string lhs;
	std::vector<std::vector<Symbol>> rhs;
};

#endif
