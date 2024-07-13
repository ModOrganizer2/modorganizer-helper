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

#include "privileges.h"

#include <shellapi.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <filesystem>


#pragma comment(linker, "/manifestDependency:\"name='dlls' processorArchitecture='x86' version='1.0.0.0' type='win32' \"")


static bool createMODirectory(const std::filesystem::path& directory, const std::wstring& accountName)
{
  if (!exists(directory)) {
    if (!create_directory(directory)) {
      error(L"Failed to create \"{}\", do you have the necessary access rights to the installation folder?", directory.native());
      return false;
    }
  }

  if (!is_directory(directory)) {
    error(L"\"{}\" seems to be a regular file", directory.native());
    return false;
  }

  if (!SetOwner(directory.c_str(), accountName.c_str())) {
    error(L"failed to set owner of \"{}\" to \"{}\"", directory.native(), accountName.c_str());
    return false;
  }

  return true;
}


static bool init(const std::filesystem::path& mopath, const std::wstring& accountName)
{
  if (!SetOwner(mopath.c_str(), accountName.c_str())) {
    error(L"failed to set owner of \"{}\" to \"{}\"", mopath.native(), accountName.c_str());
    return false;
  }

  if (!createMODirectory(mopath / "profiles", accountName) ||
    !createMODirectory(mopath / "mods", accountName) ||
    !createMODirectory(mopath / "downloads", accountName)) {
    return false;
  }
  return true;
}


static bool backdateBSAs(const std::filesystem::path& dataPath)
{
  for (auto& entry : std::filesystem::directory_iterator{ dataPath }) {
    const auto& path = entry.path();
    if (path.extension() != ".bsa") {
      continue;
    }

    HANDLE file = ::CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
      0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
      error(L"failed to open {}: errorcode {}", path.c_str(), ::GetLastError());
      return false;
    }

    ULONGLONG temp = 0;
    temp = (145731ULL) * 24 * 60 * 60 * 10000000ULL;

    FILETIME newWriteTime;

    newWriteTime.dwLowDateTime = (DWORD)(temp & 0xFFFFFFFF);
    newWriteTime.dwHighDateTime = (DWORD)(temp >> 32);

    if (!::SetFileTime(file, nullptr, nullptr, &newWriteTime)) {
      error(L"failed to change date for {}: errorcode {}", path.c_str(), ::GetLastError());
      return false;
    }

    CloseHandle(file);
  }
  return true;
}


static bool adminLaunch(DWORD processID, const std::filesystem::path& executable, const std::filesystem::path& workingDir)
{
#define TIMEOUT_S 30

  HANDLE processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
  if (processHandle == NULL) {
    error(L"Failed to get process handle for pid {}: errorcode {}", processID, ::GetLastError());
    return false;
  }

  DWORD exitCode;
  time_t t_start = time(nullptr);
  while (t_start + TIMEOUT_S > time(nullptr)) {
    if (!GetExitCodeProcess(processHandle, &exitCode)) {
      error(L"Failed to get process exit code for pid {}: errorcode {}", processID, ::GetLastError());
      return false;
    }

    if (exitCode != STILL_ACTIVE) {
      if (!::ShellExecuteW(nullptr, L"runas",
        executable.c_str(),
        L"",
        workingDir.c_str(),
        SW_SHOWNORMAL)) {
        error(L"Failed to launch {}: errorcode {}", executable.c_str(), ::GetLastError());
        return false;
      }
      return true;
    }
    Sleep(500);
  }

  return false;
}


int mainDelegate(int argc, wchar_t** argv)
{
  if (argc < 2) {
    error(L"Invalid number of parameters");
    return -1;
  }

  debug(L"action: {}", argv[1]);

  if (wcscmp(argv[1], L"init") == 0) {
    if (argc < 4) {
      error(L"Invalid number of parameters");
      return -1;
    }
    debug(L"init: {} - {}", argv[2], argv[3]);
    // set up mod organizer directory
    if (!init(argv[2], argv[3])) {
      return -2;
    }
  }
  else if (wcscmp(argv[1], L"backdateBSA") == 0) {
    debug(L"backdate bsas in {}", argv[2]);
    if (!backdateBSAs(argv[2])) {
      return -2;
    }
  }
  else if (wcscmp(argv[1], L"adminLaunch") == 0) {
    if (argc < 5) {
      error(L"Invalid number of parameters");
      return -3;
    }
    debug(L"adminLaunch {} {} {}", argv[2], argv[3], argv[4]);
    if (!adminLaunch(std::stoi(argv[2]), argv[3], argv[4])) {
      return -3;
    }
  }
  else {
    return -1;
  }

  return 0;
}


int main(int, char* [])
{
  int ws_argc = 0;
  wchar_t** ws_argv = ::CommandLineToArgvW(GetCommandLineW(), &ws_argc);

  if (!ws_argv) {
    error(L"CommandLineToArgvW() failed");
    return 1;
  }

  int res = mainDelegate(ws_argc, ws_argv);
  if (res != 0) {
    error(L"{}", res);
    getchar();
  }

  LocalFree(ws_argv);

  return res;
}
