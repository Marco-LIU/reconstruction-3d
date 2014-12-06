#include "win_utils.h"
#include <tlhelp32.h>
#include <Accctrl.h>
#include <Aclapi.h>
#include <tchar.h>
#include <shellapi.h>
#include <objbase.h>
#include <CommCtrl.h>
#include "base/logging.h"
#include "base/file_util.h"
#include "base/win/windows_version.h"
#include "base/file_version_info_win.h"
#include "base/strings/sys_string_conversions.h"

#if _M_IX86
extern "C"
{
  struct TBBUTTONINFOX64
  {
    UINT      cbSize;
    DWORD     dwMask;
    int       idCommand;
    int       iImage;
    BYTE      fsState;
    BYTE      fsStyle;
    WORD      cx;
    __int64   lParam;
    __int64   pszText;
    int       cchText;
  };

  struct TBBUTTONX64 {
    int iBitmap;
    int idCommand;
    BYTE fsState;
    BYTE fsStyle;
    BYTE bReserved[6];          // padding for alignment
    __int64 dwData;
    __int64 iString;
  };
}
#endif


extern "C" IMAGE_DOS_HEADER __ImageBase;
typedef BOOL (__stdcall *IsWow64Process_PROC)(HANDLE InProc, BOOL* OutResult);
typedef void (__stdcall *GetNativeSystemInfo_PROC)(LPSYSTEM_INFO OutSysInfo);

namespace LLX
{
  typedef enum _SYSTEM_TRAY_ICON_TYPE
  {
    SYS_TRAYICON_CLOCK,
    SYS_TRAYICON_VOLUME,
    SYS_TRAYICON_NETWORK,
    SYS_TRAYICON_POWER,
    SYS_TRAYICON_ACTIONCENTER,
    SYS_TRAYICON_INPUTFLAG
  } SYSTEM_TRAY_ICON_TYPE, *PSYSTEM_TRAY_ICON_TYPE;

  typedef struct _SYSTEM_TRAY_ICON_STATE
  {
    BOOL bVisible;
    GUID guidIcon;
  } SYSTEM_TRAY_ICON_STATE, *PSYSTEM_TRAY_ICON_STATE;

  struct TRAYDATA
  {
    HWND hWnd;
    UINT uID;
    UINT uCallbackMessage;
    DWORD Reserved1[2];
    HICON hIcon;
    DWORD Reserved2[3];
    TCHAR szExePath[MAX_PATH];
    TCHAR szTip[128];
  };

  bool IsX64Process(DWORD prorcess_id) {
    HANDLE process_handle =
        OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, prorcess_id);
    // open target process
    if (NULL == process_handle) {
      if (GetLastError() == ERROR_ACCESS_DENIED) {

      } else {

      }
      return false;
    }

    bool result = IsX64Process(process_handle);
    ::CloseHandle(process_handle);
    return result;
  }

  bool IsX64Process(HANDLE process_handle) {
    IsWow64Process_PROC pIsWow64Process;
    bool result = false;

#ifndef _M_X64
    GetNativeSystemInfo_PROC pGetNativeSystemInfo;
    SYSTEM_INFO SysInfo;
#endif

    do 
    {
      HMODULE kernel32_module = GetModuleHandleW(L"kernel32.dll");

      if (NULL == kernel32_module) break;

      // if WOW64 is not available then target must be 32-bit
      pIsWow64Process = (IsWow64Process_PROC)GetProcAddress(
          kernel32_module, "IsWow64Process");
#ifdef _M_X64
      BOOL is_wow64 = FALSE;
      BOOL r = pIsWow64Process(process_handle, &is_wow64);
      if (r != 0) {
        result = (0 == is_wow64);
      }
#else
      // 不存在IsWow64Process函数，肯定为32位系统
      if (pIsWow64Process != NULL) {
        pGetNativeSystemInfo = (GetNativeSystemInfo_PROC)GetProcAddress(
            kernel32_module, "GetNativeSystemInfo");

        if (pGetNativeSystemInfo != NULL) {
          pGetNativeSystemInfo(&SysInfo);

          if (SysInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL) {
            result = true;
          }
        }
      }
#endif
    } while (false);
    return result;
  }

  DWORD GetProcessIdByName(const TCHAR* lpName) {
    //根据进程名获取进程ID,失败时返回0(System Idle Process) 
    DWORD dwProcessId;
    HANDLE hSnapshot;
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
      PROCESSENTRY32 ppe;
      ppe.dwSize = sizeof(PROCESSENTRY32);
      if (Process32First(hSnapshot, &ppe)) {
        if (_tcsicmp(lpName, ppe.szExeFile) == 0) {
          dwProcessId = ppe.th32ProcessID;
          CloseHandle(hSnapshot);
          return dwProcessId;
        }
        while (Process32Next(hSnapshot, &ppe)) {
          if (_tcsicmp(lpName, ppe.szExeFile) == 0) {
            dwProcessId = ppe.th32ProcessID;
            CloseHandle(hSnapshot);
            return dwProcessId;
          }
        }
      }
      CloseHandle(hSnapshot);
    }
    return 0;
  }

  base::FilePath GetCurrentModuleDir() {
    return GetCurrentModulePath().DirName();
  }

  base::FilePath GetCurrentModulePath() {
    WCHAR swzModuleName[MAX_PATH] = { 0 };
    ::GetModuleFileNameW((HMODULE)&__ImageBase, swzModuleName, MAX_PATH);
    // 当前模块所在目录中
    return MakeAbsoluteFilePath(base::FilePath(swzModuleName));
  }

  base::FilePath GetCurrentProcessPath() {
    WCHAR swzEXEName[MAX_PATH] = { 0 };
    ::GetModuleFileNameW(NULL, swzEXEName, MAX_PATH);
    return MakeAbsoluteFilePath(base::FilePath(swzEXEName));
  }

  base::FilePath GetSystem32Dir() {
    WCHAR swzDirName[MAX_PATH] = { 0 };
    ::GetSystemDirectoryW(swzDirName, MAX_PATH);

    return MakeAbsoluteFilePath(base::FilePath(swzDirName));
  }

  __forceinline void MakeSysTrayIconGUID(
    PSYSTEM_TRAY_ICON_STATE pState,
    SYSTEM_TRAY_ICON_TYPE eType) {
    switch (eType) {
    case SYS_TRAYICON_CLOCK:
      IIDFromString(
          L"{7820AE72-23E3-4229-82C1-E41CB67D5B9C}", &pState->guidIcon);
      break;
    case SYS_TRAYICON_VOLUME:
      IIDFromString(
          L"{7820AE73-23E3-4229-82C1-E41CB67D5B9C}", &pState->guidIcon);
      break;
    case SYS_TRAYICON_NETWORK:
      IIDFromString(
          L"{7820AE74-23E3-4229-82C1-E41CB67D5B9C}", &pState->guidIcon);
      break;
    case SYS_TRAYICON_POWER:
      IIDFromString(
          L"{7820AE75-23E3-4229-82C1-E41CB67D5B9C}", &pState->guidIcon);
      break;
    case SYS_TRAYICON_ACTIONCENTER:
      IIDFromString(
          L"{7820AE76-23E3-4229-82C1-E41CB67D5B9C}", &pState->guidIcon);
      break;
    case SYS_TRAYICON_INPUTFLAG:
      IIDFromString(
          L"{A59B00B9-F6CD-4FED-A1DC-0F4064A12831}", &pState->guidIcon);
      break;
    }
  }

  BOOL WINAPI SetSysTrayIconVisible(
      BOOL bVisible, SYSTEM_TRAY_ICON_TYPE eType) {
    BOOL result = FALSE;
    COPYDATASTRUCT cds;
    SYSTEM_TRAY_ICON_STATE sts;
    cds.dwData = 4;
    cds.lpData = &sts;
    cds.cbData = sizeof(SYSTEM_TRAY_ICON_STATE);
    sts.bVisible = bVisible;
    MakeSysTrayIconGUID(&sts, eType);
    HWND hTrayWnd = FindWindow(_T("Shell_TrayWnd"), NULL);
    LRESULT l = 2;
    if (IsWindow(hTrayWnd))
      l = SendMessageW(hTrayWnd, WM_COPYDATA, NULL, (LPARAM)&cds);

    if (0 == l || 1 == l)
      result = TRUE;

    return result;
  }

  BOOL FindSystemVolumeTrayData(TRAYDATA* data) {
    return FALSE;
  }

  bool IsSystemVolumeTrayTextXP(LPCWSTR lpszText, int uiLen) {
    return (wcsncmp(lpszText, L"音量",
                    std::min(uiLen, (int)wcslen(L"音量"))) == 0);
  }

  /*bool HideSystemVolumeTrayXPX64(HANDLE hExplorer, bool hide) {

  }*/

  bool HideSystemVolumeTrayXP(HANDLE hExplorer, bool hide) {
    WCHAR buf[512];
    TBBUTTON btn;
    HWND tray_wnd = FindSysTrayWnd();
    LONG button_count = SendMessage(tray_wnd, TB_BUTTONCOUNT, 0, 0);
    void* pbuf = VirtualAllocEx(hExplorer,
                                NULL,
                                1024,
                                MEM_COMMIT,
                                PAGE_READWRITE);
    BOOL S = FALSE;

    for (int i = 0; i < button_count; i++) {
      S = SendMessage(tray_wnd, TB_GETBUTTON, i, (UINT_PTR)pbuf);

      if(S == 0) continue;

      // 64位和32位的Explorer.exe中TBBUTTON结构是不一样的
      // 但是他不会校验TBBUTTON的大小，而我们只需要idCommand
      // 可以无视这些差异，但是分配内存必须大于64位下该结构的大小
      S = ReadProcessMemory(hExplorer, pbuf, &btn, sizeof(TBBUTTON), NULL);

      if(S == 0) continue;

      int x = SendMessage(tray_wnd,
                          TB_GETBUTTONTEXT,
                          btn.idCommand,
                          NULL);

      if (x != -1 && x < 512) {
        x = SendMessage(tray_wnd,
                        TB_GETBUTTONTEXT,
                        btn.idCommand,
                        (UINT_PTR)pbuf);
        if (x != -1) {
          S = ReadProcessMemory(hExplorer, pbuf, buf, x * sizeof(WCHAR), NULL);
          buf[x] = TEXT('\0');

          if (S != 0 && IsSystemVolumeTrayTextXP(buf, x)) {
            S = SendMessage(tray_wnd, TB_HIDEBUTTON, btn.idCommand,
                            MAKELONG((hide ? TRUE : FALSE), 0));
            return S != 0;
          }
        }
      }
    }
    return false;
  }

  bool HideSystemVolumeTrayXP(bool hide) {
    HANDLE hExplorer = OpenProcessWithEnoughGrant(
      GetProcessIdByName(TEXT("explorer.exe")),
      PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION |
      PROCESS_VM_WRITE | PROCESS_VM_READ);

    if (hExplorer == NULL || hExplorer == INVALID_HANDLE_VALUE)
      return false;

    return HideSystemVolumeTrayXP(hExplorer, hide);
  }

  bool HideSystemVolumeTray(bool hide) {
    base::win::OSInfo* os_info = base::win::OSInfo::GetInstance();
    if (os_info->version() >= base::win::VERSION_VISTA) {
      return 0 !=
          SetSysTrayIconVisible(hide ? FALSE : TRUE, SYS_TRAYICON_VOLUME);
    }

    return HideSystemVolumeTrayXP(hide);
  }

  bool IsWindowsVistaOrHigher() {
    base::win::OSInfo* os_info = base::win::OSInfo::GetInstance();
    if (os_info->version() >= base::win::VERSION_VISTA)
      return true;
    return false;
  }

  HWND FindSysTrayWnd() {
    HWND hShell_TrayWnd = ::FindWindow(TEXT("Shell_TrayWnd"), NULL);

    if (hShell_TrayWnd == NULL) return NULL;

    HWND hTrayNotifyWnd =
        ::FindWindowEx(hShell_TrayWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
    if (hTrayNotifyWnd == NULL) return NULL;

    HWND hSysPager =
        ::FindWindowEx(hTrayNotifyWnd, NULL, TEXT("SysPager"), NULL);

    if (hSysPager == NULL) return NULL;

    HWND hToolbarWindow32 =
        ::FindWindowEx(hSysPager, NULL, TEXT("ToolbarWindow32"), NULL);
    return hToolbarWindow32;
  }

  DWORD GetWndProcess(HWND hWnd) {
    DWORD dwProcessId = 0;
    DWORD dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);
    return dwProcessId;
  }

  bool EnablePrivilege(HANDLE hToken,
                        LPCTSTR lpName,
                        TOKEN_PRIVILEGES &OldPrivilege) {
    //打开访问环(Access Token)中指定权限 
    BOOL IsSuc = FALSE;
    if (hToken != NULL) {
      TOKEN_PRIVILEGES tkpNewPrivilege;
      tkpNewPrivilege.PrivilegeCount = 1;
      tkpNewPrivilege.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      if (LookupPrivilegeValue(NULL, lpName,
        &tkpNewPrivilege.Privileges[0].Luid)) {
        DWORD dwBufferLen;
        dwBufferLen = sizeof(TOKEN_PRIVILEGES);
        IsSuc = AdjustTokenPrivileges(hToken,
                                      false,
                                      &tkpNewPrivilege,
                                      dwBufferLen,
                                      &OldPrivilege,
                                      &dwBufferLen);
        IsSuc = (IsSuc && (GetLastError() == ERROR_SUCCESS));
      }
    }
    return IsSuc != FALSE;
  }
  bool RestorePrivilege(HANDLE hToken, TOKEN_PRIVILEGES &OldPrivilege) {
    //恢复访问环(Access Token)原来的权限 
    BOOL IsSuc = FALSE;
    IsSuc = AdjustTokenPrivileges(hToken, false, &OldPrivilege, 0, NULL, NULL);
    IsSuc = (IsSuc && (GetLastError() == ERROR_SUCCESS));
    return IsSuc != FALSE;
  }
  bool AdjustDacl(HANDLE hProcess, DWORD dwDesiredAccess) {
    //修改访问控制列表(DACL),打开指定的权限 
    bool IsSuc = false;
    SID sid = {SID_REVISION, 1, SECURITY_WORLD_SID_AUTHORITY, 0};
    EXPLICIT_ACCESS ea = {
      dwDesiredAccess,
      SET_ACCESS,
      NO_INHERITANCE,
      {
        NULL,
        NO_MULTIPLE_TRUSTEE,
        TRUSTEE_IS_SID,
        TRUSTEE_IS_USER,
        reinterpret_cast<LPTSTR>(&sid)
      }
    };
    PACL pAcl;
    if (SetEntriesInAcl(1, &ea, NULL, &pAcl) == ERROR_SUCCESS) {
      DWORD dwResult = SetSecurityInfo(hProcess,
                                        SE_KERNEL_OBJECT,
                                        DACL_SECURITY_INFORMATION,
                                        NULL,
                                        NULL,
                                        pAcl,
                                        NULL);
      IsSuc = dwResult == ERROR_SUCCESS;
      LocalFree(pAcl);
    }
    return IsSuc;
  }
  HANDLE OpenProcessWithEnoughGrant(DWORD dwProcessId, DWORD dwDesiredAccess) {
    //以指定的权限打开指定ID号的进程,成功时返回进程句柄,失败返回NULL 
    HANDLE hProcess;
    BOOL s;
    hProcess = OpenProcess(dwDesiredAccess, FALSE, dwProcessId);
    if (hProcess == NULL) {
      // 以普通方式打开进程失败,尝试以WRITE_DAC的方式打开,
      // 以获得WRITE_DAC权限，如果以WRITE_DAC的方式打开失败,
      // 则尝试以WRITE_OWNER方式打开,以便通过修改安全描述符所有者
      // (security descriptor owner)的方式来获得WRITE_DAC权限 
      // 只要获得了WRITE_DAC权限，就可以修改进程的访问控制列表(dacl)
      // 从设置dwDesiredAccess指定的权限 
      HANDLE hWriteDAC;
      hWriteDAC = OpenProcess(WRITE_DAC, FALSE, dwProcessId);
      if (hWriteDAC == NULL) {
        //第一次尝试失败,无法以WRITE_DAC的方式打开 
        //则以WRITE_OWNER的方式打开,进行第二次尝试 
        HANDLE hWriteOwner;
        hWriteOwner = OpenProcess(WRITE_OWNER, FALSE, dwProcessId);
        if (hWriteOwner != NULL) {
          //根据当前进程环中的用户Sid修改安全描述符号所有者 
          HANDLE hToken;
          s = OpenProcessToken(GetCurrentProcess(),
                               TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                               &hToken);
          if (0 != s) {
            TOKEN_PRIVILEGES OldPrivilege;
            if (EnablePrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, OldPrivilege)) {
              BYTE TokenInfo[256];
              DWORD dwLen = sizeof(TokenInfo);
              GetTokenInformation(hToken, TokenUser, TokenInfo, dwLen, &dwLen);
              PSID sid = reinterpret_cast<PTOKEN_USER>(TokenInfo)->User.Sid;
              DWORD dwErr = SetSecurityInfo(hWriteOwner,
                                            SE_KERNEL_OBJECT,
                                            OWNER_SECURITY_INFORMATION,
                                            sid,
                                            NULL,
                                            NULL,
                                            NULL);
              if (dwErr = ERROR_SUCCESS) {
                //修改安全描述符所有者成功，句柄复制,检测是否有WRITE_DAC权限
                s = DuplicateHandle(GetCurrentProcess(),
                                    hWriteOwner,
                                    GetCurrentProcess(),
                                    &hWriteDAC,
                                    WRITE_DAC, FALSE,
                                    0);
                if (0 == s)
                  hWriteDAC = NULL;  //无WRITE_DAC权限,失败 
              }
              RestorePrivilege(hToken, OldPrivilege);
            }
            CloseHandle(hToken);
          }
          CloseHandle(hWriteOwner);
        }
      }
      if (hWriteDAC != NULL) {
        if (AdjustDacl(hWriteDAC, dwDesiredAccess)) {
          s = DuplicateHandle(GetCurrentProcess(),
                              hWriteDAC,
                              GetCurrentProcess(),
                              &hProcess,
                              dwDesiredAccess,
                              FALSE,
                              0);
          if (0 == s)
            hProcess = NULL;
        }
        CloseHandle(hWriteDAC);
      }
    }
    return hProcess;
  }

  SIZE GetIconSize(HICON hIcon) {
    SIZE sz;
    sz.cx = sz.cy = 0;
    ICONINFO icon_info = {0};
    BOOL suc = GetIconInfo(hIcon, &icon_info);

    if (!suc) return sz;

    if (icon_info.hbmColor) {
      return GetBitmapSize(icon_info.hbmColor);
    } else if (icon_info.hbmMask) {
      sz = GetBitmapSize(icon_info.hbmMask);
      sz.cy /= 2;
    }
    return sz;
  }

  SIZE GetBitmapSize(HBITMAP hBitmap) {
    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);
    SIZE sz;
    sz.cx = bitmap.bmWidth;
    sz.cy = bitmap.bmHeight;
    return sz;
  }


  bool IsRunningInService()
  {
    LPENUM_SERVICE_STATUS_PROCESS  lpServices = NULL;
    DWORD cbBytesNeeded = NULL;
    DWORD ServicesReturned = NULL;
    DWORD i;
    DWORD dwResumeHandle = 0;
    DWORD dwBufferSize = 0;
    PBYTE pbyBuffer = NULL;
    BOOL bRetCode = FALSE;
    BOOL bResult = FALSE;

    SC_HANDLE hManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (hManager == NULL) {
      return FALSE;
    }

    bRetCode = EnumServicesStatusExW(hManager,
                                     SC_ENUM_PROCESS_INFO,
                                     SERVICE_WIN32,
                                     SERVICE_ACTIVE,
                                     NULL,
                                     0,
                                     &cbBytesNeeded,
                                     &ServicesReturned,
                                     &dwResumeHandle,
                                     NULL);
    dwBufferSize = cbBytesNeeded ? (cbBytesNeeded + 16) : 8 * 1024;
    pbyBuffer = (PBYTE)malloc(dwBufferSize);
    if (NULL == pbyBuffer) {
      CloseServiceHandle(hManager);
      return FALSE;
    }
    bRetCode = EnumServicesStatusExW(hManager,
                                     SC_ENUM_PROCESS_INFO,
                                     SERVICE_WIN32,
                                     SERVICE_ACTIVE,
                                     pbyBuffer,
                                     dwBufferSize,
                                     &cbBytesNeeded,
                                     &ServicesReturned,
                                     &dwResumeHandle,
                                     NULL);
    if (!bRetCode) {
      free(pbyBuffer);
      CloseServiceHandle(hManager);
      return FALSE;
    }
    lpServices = (LPENUM_SERVICE_STATUS_PROCESS)pbyBuffer;
    for (i = 0; i < ServicesReturned; i++) {
      if (lpServices[i].ServiceStatusProcess.dwProcessId == GetCurrentProcessId()) {
        bResult = TRUE;
        break;
      }
    }
    free(pbyBuffer);
    CloseServiceHandle(hManager);
    return (bResult != 0);
  }

  HMODULE GetCurrentModule() {
    HMODULE current_module;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)GetCurrentModule,
                       &current_module);
    return current_module;
  }
  
  //
  //   FUNCTION: IsUserInAdminGroup()
  //
  //   PURPOSE: The function checks whether the primary access token of the 
  //   process belongs to user account that is a member of the local 
  //   Administrators group, even if it currently is not elevated.
  //
  //   RETURN VALUE: Returns TRUE if the primary access token of the process 
  //   belongs to user account that is a member of the local Administrators 
  //   group. Returns FALSE if the token does not.
  //
  //   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
  //   contains the Win32 error code of the failure.
  //
  //   EXAMPLE CALL:
  //     try 
  //     {
  //         if (IsUserInAdminGroup())
  //             wprintf (L"User is a member of the Administrators group\n");
  //         else
  //             wprintf (L"User is not a member of the Administrators group\n");
  //     }
  //     catch (DWORD dwError)
  //     {
  //         wprintf(L"IsUserInAdminGroup failed w/err %lu\n", dwError);
  //     }
  //
  DWORD IsUserInAdminGroup(PBOOL result)
  {
    BOOL fInAdminGroup = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    HANDLE hTokenToCheck = NULL;
    DWORD cbSize = 0;
    OSVERSIONINFO osver = { sizeof(osver) };

    // Open the primary access token of the process for query and duplicate.
    if (!OpenProcessToken(GetCurrentProcess(),
        TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
      dwError = GetLastError();
      goto Cleanup;
    }

    // Determine whether system is running Windows Vista or later operating 
    // systems (major version >= 6) because they support linked tokens, but 
    // previous versions (major version < 6) do not.
    if (!GetVersionEx(&osver)) {
      dwError = GetLastError();
      goto Cleanup;
    }

    if (osver.dwMajorVersion >= 6) {
      // Running Windows Vista or later (major version >= 6). 
      // Determine token type: limited, elevated, or default. 
      TOKEN_ELEVATION_TYPE elevType;
      if (!GetTokenInformation(hToken, TokenElevationType, &elevType,
        sizeof(elevType), &cbSize)) {
        dwError = GetLastError();
        goto Cleanup;
      }

      // If limited, get the linked elevated token for further check.
      if (TokenElevationTypeLimited == elevType) {
        if (!GetTokenInformation(hToken, TokenLinkedToken, &hTokenToCheck,
          sizeof(hTokenToCheck), &cbSize)) {
          dwError = GetLastError();
          goto Cleanup;
        }
      }
    }

    // CheckTokenMembership requires an impersonation token. If we just got a 
    // linked token, it already is an impersonation token.  If we did not get 
    // a linked token, duplicate the original into an impersonation token for 
    // CheckTokenMembership.
    if (!hTokenToCheck) {
      if (!DuplicateToken(hToken, SecurityIdentification, &hTokenToCheck)) {
        dwError = GetLastError();
        goto Cleanup;
      }
    }

    // Create the SID corresponding to the Administrators group.
    BYTE adminSID[SECURITY_MAX_SID_SIZE];
    cbSize = sizeof(adminSID);
    if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, &adminSID,
      &cbSize)) {
      dwError = GetLastError();
      goto Cleanup;
    }

    // Check if the token to be checked contains admin SID.
    // http://msdn.microsoft.com/en-us/library/aa379596(VS.85).aspx:
    // To determine whether a SID is enabled in a token, that is, whether it 
    // has the SE_GROUP_ENABLED attribute, call CheckTokenMembership.
    if (!CheckTokenMembership(hTokenToCheck, &adminSID, &fInAdminGroup)) {
      dwError = GetLastError();
      goto Cleanup;
    }

  Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
      CloseHandle(hToken);
      hToken = NULL;
    }
    if (hTokenToCheck) {
      CloseHandle(hTokenToCheck);
      hTokenToCheck = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS == dwError && result) {
      *result = fInAdminGroup;
    }

    return dwError;
  }


  // 
  //   FUNCTION: IsRunAsAdmin()
  //
  //   PURPOSE: The function checks whether the current process is run as 
  //   administrator. In other words, it dictates whether the primary access 
  //   token of the process belongs to user account that is a member of the 
  //   local Administrators group and it is elevated.
  //
  //   RETURN VALUE: Returns TRUE if the primary access token of the process 
  //   belongs to user account that is a member of the local Administrators 
  //   group and it is elevated. Returns FALSE if the token does not.
  //
  //   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
  //   contains the Win32 error code of the failure.
  //
  //   EXAMPLE CALL:
  //     try 
  //     {
  //         if (IsRunAsAdmin())
  //             wprintf (L"Process is run as administrator\n");
  //         else
  //             wprintf (L"Process is not run as administrator\n");
  //     }
  //     catch (DWORD dwError)
  //     {
  //         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
  //     }
  //
  DWORD IsRunAsAdmin(PBOOL result)
  {
    BOOL fIsRunAsAdmin = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    PSID pAdministratorsGroup = NULL;

    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsGroup)) {
      dwError = GetLastError();
      goto Cleanup;
    }

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin)) {
      dwError = GetLastError();
      goto Cleanup;
    }

  Cleanup:
    // Centralized cleanup for all allocated resources.
    if (pAdministratorsGroup) {
      FreeSid(pAdministratorsGroup);
      pAdministratorsGroup = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS == dwError && result) {
      *result = fIsRunAsAdmin;
    }

    return dwError;
  }


  //
  //   FUNCTION: IsProcessElevated()
  //
  //   PURPOSE: The function gets the elevation information of the current 
  //   process. It dictates whether the process is elevated or not. Token 
  //   elevation is only available on Windows Vista and newer operating 
  //   systems, thus IsProcessElevated throws a C++ exception if it is called 
  //   on systems prior to Windows Vista. It is not appropriate to use this 
  //   function to determine whether a process is run as administartor.
  //
  //   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if 
  //   it is not.
  //
  //   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
  //   which contains the Win32 error code of the failure. For example, if 
  //   IsProcessElevated is called on systems prior to Windows Vista, the error 
  //   code will be ERROR_INVALID_PARAMETER.
  //
  //   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the 
  //   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
  //   TokenElevationTypeFull) of the process. It is different from 
  //   TokenElevation in that, when UAC is turned off, elevation type always 
  //   returns TokenElevationTypeDefault even though the process is elevated 
  //   (Integrity Level == High). In other words, it is not safe to say if the 
  //   process is elevated based on elevation type. Instead, we should use 
  //   TokenElevation.
  //
  //   EXAMPLE CALL:
  //     try 
  //     {
  //         if (IsProcessElevated())
  //             wprintf (L"Process is elevated\n");
  //         else
  //             wprintf (L"Process is not elevated\n");
  //     }
  //     catch (DWORD dwError)
  //     {
  //         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
  //     }
  //
  DWORD IsProcessElevated(PBOOL result)
  {
    BOOL fIsElevated = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
      dwError = GetLastError();
      goto Cleanup;
    }

    // Retrieve token elevation information.
    TOKEN_ELEVATION elevation;
    DWORD dwSize;
    if (!GetTokenInformation(hToken, TokenElevation, &elevation,
      sizeof(elevation), &dwSize)) {
      // When the process is run on operating systems prior to Windows 
      // Vista, GetTokenInformation returns FALSE with the 
      // ERROR_INVALID_PARAMETER error code because TokenElevation is 
      // not supported on those operating systems.
      dwError = GetLastError();
      goto Cleanup;
    }

    fIsElevated = elevation.TokenIsElevated;

  Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
      CloseHandle(hToken);
      hToken = NULL;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS == dwError && result) {
      *result = fIsElevated;
    }

    return dwError;
  }


  //
  //   FUNCTION: GetProcessIntegrityLevel()
  //
  //   PURPOSE: The function gets the integrity level of the current process. 
  //   Integrity level is only available on Windows Vista and newer operating 
  //   systems, thus GetProcessIntegrityLevel throws a C++ exception if it is 
  //   called on systems prior to Windows Vista.
  //
  //   RETURN VALUE: Returns the integrity level of the current process. It is 
  //   usually one of these values:
  //
  //     SECURITY_MANDATORY_UNTRUSTED_RID (SID: S-1-16-0x0)
  //     Means untrusted level. It is used by processes started by the 
  //     Anonymous group. Blocks most write access. 
  //
  //     SECURITY_MANDATORY_LOW_RID (SID: S-1-16-0x1000)
  //     Means low integrity level. It is used by Protected Mode Internet 
  //     Explorer. Blocks write acess to most objects (such as files and 
  //     registry keys) on the system. 
  //
  //     SECURITY_MANDATORY_MEDIUM_RID (SID: S-1-16-0x2000)
  //     Means medium integrity level. It is used by normal applications 
  //     being launched while UAC is enabled. 
  //
  //     SECURITY_MANDATORY_HIGH_RID (SID: S-1-16-0x3000)
  //     Means high integrity level. It is used by administrative applications 
  //     launched through elevation when UAC is enabled, or normal 
  //     applications if UAC is disabled and the user is an administrator. 
  //
  //     SECURITY_MANDATORY_SYSTEM_RID (SID: S-1-16-0x4000)
  //     Means system integrity level. It is used by services and other 
  //     system-level applications (such as Wininit, Winlogon, Smss, etc.)  
  //
  //   EXCEPTION: If this function fails, it throws a C++ DWORD exception 
  //   which contains the Win32 error code of the failure. For example, if 
  //   GetProcessIntegrityLevel is called on systems prior to Windows Vista, 
  //   the error code will be ERROR_INVALID_PARAMETER.
  //
  //   EXAMPLE CALL:
  //     try 
  //     {
  //         DWORD dwIntegrityLevel = GetProcessIntegrityLevel();
  //     }
  //     catch (DWORD dwError)
  //     {
  //         wprintf(L"GetProcessIntegrityLevel failed w/err %lu\n", dwError);
  //     }
  //
  DWORD GetProcessIntegrityLevel(PDWORD result)
  {
    DWORD dwIntegrityLevel = 0;
    DWORD dwError = ERROR_SUCCESS;
    HANDLE hToken = NULL;
    DWORD cbTokenIL = 0;
    PTOKEN_MANDATORY_LABEL pTokenIL = NULL;

    // Open the primary access token of the process with TOKEN_QUERY.
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
      dwError = GetLastError();
      goto Cleanup;
    }

    // Query the size of the token integrity level information. Note that 
    // we expect a FALSE result and the last error ERROR_INSUFFICIENT_BUFFER
    // from GetTokenInformation because we have given it a NULL buffer. On 
    // exit cbTokenIL will tell the size of the integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &cbTokenIL)) {
      if (ERROR_INSUFFICIENT_BUFFER != GetLastError()) {
        // When the process is run on operating systems prior to Windows 
        // Vista, GetTokenInformation returns FALSE with the 
        // ERROR_INVALID_PARAMETER error code because TokenElevation 
        // is not supported on those operating systems.
        dwError = GetLastError();
        goto Cleanup;
      }
    }

    // Now we allocate a buffer for the integrity level information.
    pTokenIL = (TOKEN_MANDATORY_LABEL *)LocalAlloc(LPTR, cbTokenIL);
    if (pTokenIL == NULL) {
      dwError = GetLastError();
      goto Cleanup;
    }

    // Retrieve token integrity level information.
    if (!GetTokenInformation(hToken, TokenIntegrityLevel, pTokenIL,
      cbTokenIL, &cbTokenIL)) {
      dwError = GetLastError();
      goto Cleanup;
    }

    // Integrity Level SIDs are in the form of S-1-16-0xXXXX. (e.g. 
    // S-1-16-0x1000 stands for low integrity level SID). There is one and 
    // only one subauthority.
    dwIntegrityLevel = *GetSidSubAuthority(pTokenIL->Label.Sid, 0);

  Cleanup:
    // Centralized cleanup for all allocated resources.
    if (hToken) {
      CloseHandle(hToken);
      hToken = NULL;
    }
    if (pTokenIL) {
      LocalFree(pTokenIL);
      pTokenIL = NULL;
      cbTokenIL = 0;
    }

    // Throw the error if something failed in the function.
    if (ERROR_SUCCESS == dwError && result) {
      *result = dwIntegrityLevel;
    }

    return dwError;
  }

  bool GetFileVersion(FileVersionInfoWin* ver_info, Version& version) {
    VS_FIXEDFILEINFO* info = ver_info->fixed_file_info();
    if (info) {
      version.major = HIWORD(info->dwFileVersionMS);
      version.minor = LOWORD(info->dwFileVersionMS);
      version.mantenance = HIWORD(info->dwFileVersionLS);
      version.build = LOWORD(info->dwFileVersionLS);
      return true;
    }
    return false;
  }

  bool GetProductVersion(FileVersionInfoWin* ver_info, Version& version) {
    VS_FIXEDFILEINFO* info = ver_info->fixed_file_info();
    if (info) {
      version.major = HIWORD(info->dwProductVersionMS);
      version.minor = LOWORD(info->dwProductVersionMS);
      version.mantenance = HIWORD(info->dwProductVersionLS);
      version.build = LOWORD(info->dwProductVersionLS);
      return true;
    }
    return false;
  }

  bool GetFileVersion(HMODULE module, Version& version) {
    FileVersionInfoWin* ver_info = (FileVersionInfoWin*)
        FileVersionInfo::CreateFileVersionInfoForModule(module);

    if (ver_info) {
      return GetFileVersion(ver_info, version);
    }
    return false;
  }

  bool GetFileVersion(const base::FilePath& file, Version& version) {
    FileVersionInfoWin* ver_info = (FileVersionInfoWin*)
        FileVersionInfo::CreateFileVersionInfo(file);
    if (ver_info) {
      return GetFileVersion(ver_info, version);
    }
    return false;
  }

  bool GetProductVersion(const base::FilePath& file, Version& version) {
    FileVersionInfoWin* ver_info = (FileVersionInfoWin*)
        FileVersionInfo::CreateFileVersionInfo(file);
    if (ver_info) {
      return GetProductVersion(ver_info, version);
    }
    return false;
  }
  bool GetProductVersion(HMODULE module, Version& version) {
    FileVersionInfoWin* ver_info = (FileVersionInfoWin*)
      FileVersionInfo::CreateFileVersionInfoForModule(module);

    if (ver_info) {
      return GetProductVersion(ver_info, version);
    }
    return false;
  }

  bool operator<(const Version& l, const Version& r) {
    if (l.major < r.major) return true;
    if (l.major == r.major && l.minor < r.minor) return true;
    if (l.major == r.major && l.minor == r.minor &&
        l.mantenance < r.mantenance) return true;
    if (l.major == r.major && l.minor == r.minor &&
        l.mantenance == r.mantenance && l.build < r.build) return true;
    return false;
  }

  bool operator==(const Version& l, const Version& r) {
    if (l.minor == r.major && l.minor == r.minor &&
        l.mantenance == r.mantenance && l.build == r.build) return true;
    return false;
  }

  bool operator!=(const Version& l, const Version& r) {
    return !operator==(l, r);
  }

  std::string VersionString(const Version& v) {
    char buf[32];
    sprintf_s(buf, 32, "%d.%d.%d.%d", v.major, v.minor, v.mantenance, v.build);
    return std::string(buf);
  }

  bool ParseVersion(const std::string ver, Version& v) {
    int x = sscanf_s(ver.c_str(), "%hd.%hd.%hd.%hd",
                     &v.major, &v.minor, &v.mantenance, &v.build);
    return x == 4;
  }

  std::string NewGuidString() {
    GUID guid;
    CoCreateGuid(&guid);

    OLECHAR* bstrGuid;
    StringFromCLSID(guid, &bstrGuid);

    std::string result= base::SysWideToUTF8(bstrGuid);
    ::CoTaskMemFree(bstrGuid);

    return result;
  }
}