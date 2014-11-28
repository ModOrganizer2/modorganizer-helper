#include "privileges.h"
#include <QDebug>
#include <stdio.h>
#include <accctrl.h>
#include <aclapi.h>
#include <memory>
#include <LMCons.h>


BOOL SetPrivilege(HANDLE token, LPCWSTR privilege, BOOL enable)
{
  TOKEN_PRIVILEGES tokenPriv;
  LUID luid;

  // get the local id of the privilege
  if (!LookupPrivilegeValue(nullptr, privilege, &luid)) {
    qCritical("failed to look up privilege %ls: %lu",
              privilege, ::GetLastError());
    return FALSE;
  }

  // set the privilege to either enabled or disabled
  tokenPriv.PrivilegeCount = 1;
  tokenPriv.Privileges[0].Luid = luid;
  if (enable) {
    tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  } else {
    tokenPriv.Privileges[0].Attributes = 0;
  }

  // change the privilege
  if (!AdjustTokenPrivileges(token, FALSE, &tokenPriv,
         sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)nullptr,
         (PDWORD)nullptr)) {
    qCritical("failed to adjust privilege %ls: %lu",
              privilege, ::GetLastError());
    return FALSE;
  }

  if (::GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
    qCritical("The token does not have the specified privilege %s.",
              privilege);
    return FALSE;
  }

  return TRUE;
}


BOOL SetOwner(LPCTSTR filename, LPCTSTR newOwner)
{
  PSID sid = nullptr;
  BOOL res = TRUE;
  PACL pacl = nullptr;

  // get the SID for the new owner
  TCHAR domainUnused[4096];
  DWORD sidSize = 0;
  DWORD domainBufSize = 4096;
  SID_NAME_USE sidUse;
  // pre-flight to determine required size of the sid
  LookupAccountName(nullptr, newOwner, nullptr, &sidSize, domainUnused, &domainBufSize, &sidUse);
  sid = (PSID)malloc(sidSize);
  // determine sid for account name
  if (!LookupAccountName(nullptr, newOwner, sid, &sidSize, domainUnused, &domainBufSize, &sidUse)) {
    qCritical("failed to look up account name: %ls", newOwner);
    res = FALSE;
  } else {
    EXPLICIT_ACCESS access;
    ZeroMemory(&access, sizeof(EXPLICIT_ACCESS));

    wchar_t ownerTemp[UNLEN + 1];
    wcsncpy(ownerTemp, newOwner, UNLEN + 1);

    // Set full control for Administrators.
    access.grfAccessPermissions = GENERIC_ALL;
    access.grfAccessMode = SET_ACCESS;
    access.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    access.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    access.Trustee.ptstrName = (LPTSTR)sid;

    DWORD secRes = SetEntriesInAcl(1, &access, nullptr, &pacl);
    if (secRes != ERROR_SUCCESS) {
      qCritical("failed to set up acls: %lu", secRes);
      return FALSE;
    }


    // filename parameter for SetNamedSecurityInfo isn't const
    // which is odd since it is documented to be a input parameter...
    TCHAR *fileNameBuf = new TCHAR[32768];
    wcsncpy(fileNameBuf, filename, 32768);
    // Set the owner on the file and give him full access
    secRes = SetNamedSecurityInfo(
        fileNameBuf, SE_FILE_OBJECT,
        OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
        sid, nullptr, pacl, nullptr);

    delete [] fileNameBuf;
    if (secRes != NOERROR) {
      qCritical("failed to set file owner: %d", secRes);
      res = false;
    }
  }

  if (sid != nullptr) {
    free(sid);
  }

  return res;
}
