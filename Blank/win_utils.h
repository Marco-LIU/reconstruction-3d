#ifndef _LLX_UTILS_H__
#define _LLX_UTILS_H__
#include <Windows.h>
#include "base/files/file_path.h"
#include <string>

namespace LLX
{
  typedef struct _VSERSION {
    _VSERSION() : major(0), minor(0), mantenance(0), build(0) {}
    WORD major;
    WORD minor;
    WORD mantenance;
    WORD build;
  } Version;

  bool operator<(const Version& l, const Version& r);
  bool operator==(const Version& l, const Version& r);
  bool operator!=(const Version& l, const Version& r);

  std::string VersionString(const Version& v);
  bool ParseVersion(const std::string ver, Version& v);

  class KSecurityDesAcessByAnyone
  {
   public:
    KSecurityDesAcessByAnyone()
    {
      m_psa = new SECURITY_ATTRIBUTES;
      m_psd = new SECURITY_DESCRIPTOR;
      ::InitializeSecurityDescriptor(m_psd, SECURITY_DESCRIPTOR_REVISION);
      ::SetSecurityDescriptorDacl(m_psd,
                                  TRUE,
                                  NULL,     // Allowing all access to the object
                                  FALSE);
      m_psa->nLength = sizeof(SECURITY_ATTRIBUTES);
      m_psa->bInheritHandle = TRUE;
      m_psa->lpSecurityDescriptor = m_psd;
    }
    operator PSECURITY_ATTRIBUTES()
    {
      return m_psa;
    }

    ~KSecurityDesAcessByAnyone()
    {
      if (m_psa) {
        delete m_psa->lpSecurityDescriptor;
        delete m_psa;
      }
    }
   private:
    PSECURITY_ATTRIBUTES m_psa;
    PSECURITY_DESCRIPTOR m_psd;
  };

  base::FilePath GetCurrentModuleDir();
  base::FilePath GetCurrentModulePath();
  base::FilePath GetCurrentProcessPath();
  base::FilePath GetSystem32Dir();

  bool IsWindowsVistaOrHigher();

  bool IsX64Process(DWORD prorcess_id);
  bool IsX64Process(HANDLE process_handle);
  DWORD GetProcessIdByName(const TCHAR* lpName);

  bool HideSystemVolumeTray(bool hide);
  HWND FindSysTrayWnd();
  HANDLE OpenProcessWithEnoughGrant(DWORD dwProcessId, DWORD dwDesiredAccess);
  DWORD GetWndProcess(HWND hWnd);
  SIZE GetIconSize(HICON hIcon);
  SIZE GetBitmapSize(HBITMAP hBitmap);

  bool IsRunningInService();
  HMODULE GetCurrentModule();

  DWORD IsUserInAdminGroup(PBOOL result);
  DWORD IsRunAsAdmin(PBOOL result);
  DWORD IsProcessElevated(PBOOL result);
  DWORD GetProcessIntegrityLevel(PDWORD result);

  bool GetFileVersion(const base::FilePath& file, Version& version);
  bool GetFileVersion(HMODULE module, Version& version);
  bool GetProductVersion(const base::FilePath& file, Version& version);
  bool GetProductVersion(HMODULE module, Version& version);

  std::string NewGuidString();
}
#endif