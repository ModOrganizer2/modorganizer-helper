/*
Copyright (C) 2012 Sebastian Herbord. All rights reserved.

This file is part of Mod Organizer.

Mod Organizer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Mod Organizer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Mod Organizer.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PRIVILEGES_H
#define PRIVILEGES_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <iostream>
#include <format>

template <class... Args>
void error(std::wformat_string<Args... > format, Args&& ...args) {
  std::wcerr << std::format(format, std::forward<Args>(args)...) << L'\n';
}

template <class... Args>
void debug(std::wformat_string<Args... > format, Args&& ...args) {
  std::wcout << std::format(format, std::forward<Args>(args)...) << L'\n';
}

BOOL SetOwner(LPCTSTR filename, LPCTSTR newowner);

#endif // PRIVILEGES_H
