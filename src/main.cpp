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
#include "shellapi.h"

#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QDir>
#include <QDirIterator>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


#pragma comment(linker, "/manifestDependency:\"name='dlls' processorArchitecture='x86' version='1.0.0.0' type='win32' \"")


std::wstring ToWString(const QString &source)
{
  wchar_t *buffer = new wchar_t[source.size() + 1];
  source.toWCharArray(buffer);
  buffer[source.size()] = L'\0';
  std::wstring result(buffer);
  delete [] buffer;

  return result;
}

QString ToQString(const std::wstring &source)
{
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  return QString::fromWCharArray(source.c_str());
#else
  return QString::fromUtf16(source.c_str());
#endif
}


static bool createMODirectory(const QString &directoryName, const std::wstring &accountName)
{
  if (!QDir(directoryName).exists()) {
    if (!QDir().mkdir(directoryName)) {
      qCritical("Failed to create \"%s\", do you have the necessary access rights to the installation folder?", qUtf8Printable(directoryName));
      return false;
    }
  }

  if (!QFileInfo(directoryName).isDir()) {
    qCritical("\"%s\" seems to be a regular file", qUtf8Printable(directoryName));
    return false;
  }

  if (!SetOwner(ToWString(directoryName).c_str(), accountName.c_str())) {
    qCritical("failed to set owner of \"%s\" to \"%ls\"", qUtf8Printable(directoryName), accountName.c_str());
    return false;
  }

  return true;
}


static bool init(const QString &mopath, const std::wstring &accountName)
{
  if (!SetOwner(ToWString(mopath).c_str(), accountName.c_str())) {
    qCritical("failed to set owner of \"%s\" to \"%ls\"",
              qUtf8Printable(mopath), accountName.c_str());
    return false;
  }

  if (!createMODirectory(mopath.mid(0).append("\\profiles"), accountName) ||
      !createMODirectory(mopath.mid(0).append("\\mods"), accountName) ||
      !createMODirectory(mopath.mid(0).append("\\downloads"), accountName)) {
    return false;
  }
  return true;
}


static bool backdateBSAs(const QString &dataPath)
{
  QStringList bsaFilter("*.bsa");
  QDirIterator iter(dataPath, bsaFilter, QDir::Files);
  while (iter.hasNext()) {
    iter.next();

    HANDLE file = ::CreateFileW(ToWString(iter.filePath()).c_str(), GENERIC_READ | GENERIC_WRITE,
                               0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
      qCritical("failed to open %s: errorcode %d", iter.filePath().toUtf8().constData(), ::GetLastError());
      return false;
    }

    ULONGLONG temp = 0;
    temp = (145731ULL) * 24 * 60 * 60 * 10000000ULL;

    FILETIME newWriteTime;

    newWriteTime.dwLowDateTime  = (DWORD)(temp & 0xFFFFFFFF);
    newWriteTime.dwHighDateTime = (DWORD)(temp >> 32);

    if (!::SetFileTime(file, nullptr, nullptr, &newWriteTime)) {
      qCritical("failed to change date for %s: errorcode %d", iter.filePath().toUtf8().constData(), ::GetLastError());
      return false;
    }

    CloseHandle(file);
  }
  return true;
}


static bool adminLaunch(const QString &pid, const QString &executable, const QString &workingDir)
{
  #define TIMEOUT_S 30

  DWORD processID = pid.toInt();
  HANDLE processHandle = ::OpenProcess(PROCESS_QUERY_INFORMATION , FALSE, processID);
  if (processHandle == NULL) {
    qCritical("Failed to get process handle for pid %d: errorcode %d", processID, ::GetLastError());
    return false;
  }

  DWORD exitCode;
  time_t t_start = time(nullptr);
  while (t_start + TIMEOUT_S > time(nullptr)) {
    if (!GetExitCodeProcess(processHandle, &exitCode)) {
      qCritical("Failed to get process exit code for pid %d: errorcode %d", processID, ::GetLastError());
      return false;
    }

    if (exitCode != STILL_ACTIVE) {
      if (!::ShellExecuteW(nullptr, L"runas",
              ToWString(executable).c_str(),
              L"",
              ToWString(workingDir).c_str(),
              SW_SHOWNORMAL)) {
        qCritical("Failed to launch %s: errorcode %d", executable.toLocal8Bit(), ::GetLastError());
        return false;
      }
      return true;
    }
    Sleep(500);
  }

  return false;
}


int mainDelegate(int argc, wchar_t **argv)
{
  if (argc < 2) {
    qCritical("Invalid number of parameters");
    return -1;
  }

  qDebug("action: %ls", argv[1]);

  if (wcscmp(argv[1], L"init") == 0) {
    if (argc < 4) {
      qCritical("Invalid number of parameters");
      return -1;
    }
    qDebug("init: %ls - %ls", argv[2], argv[3]);
    // set up mod organizer directory
    if (!init(ToQString(argv[2]), argv[3])) {
      return -2;
    }
  } else if (wcscmp(argv[1], L"backdateBSA") == 0) {
    qDebug("backdate bsas in %ls", argv[2]);
    if (!backdateBSAs(ToQString(argv[2]))) {
      return -2;
    }
  } else if (wcscmp(argv[1], L"adminLaunch") == 0) {
    if (argc < 5) {
      qCritical("Invalid number of parameters");
      return -3;
    }
    qDebug("adminLaunch %ls %ls %ls", argv[2], argv[3], argv[4]);
    if (!adminLaunch(
          ToQString(argv[2]),
          ToQString(argv[3]),
          ToQString(argv[4])
          )) {
      return -3;
    }
  } else {
    return -1;
  }

  return 0;
}


int main()
{
  int ws_argc = 0;
  wchar_t** ws_argv = ::CommandLineToArgvW(GetCommandLineW(), &ws_argc);

  if (!ws_argv) {
    qDebug("CommandLineToArgvW() failed");
    return 1;
  }

  int res = mainDelegate(ws_argc, ws_argv);
  if (res != 0) {
    qDebug("%d", res);
    getchar();
  }

  LocalFree(ws_argv);

  return res;
}
