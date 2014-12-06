#include "runtime_context.h"
#include "base/logging_win.h"
#include "base/command_line.h"
#include "base/strings/sys_string_conversions.h"
#include "base/memory/singleton.h"
#include "win_utils.h"
#include <initguid.h>
#include <Shlobj.h>
//#include "product_defines.h"

namespace LLX
{
  //----------------------------------------------------------------------------
  namespace {
    // {226E61EA-4C09-4726-9D24-74FF12D1F928}
    DEFINE_GUID(kLogProvider,
                0x226e61ea, 0x4c09, 0x4726,
                0x9d, 0x24, 0x74, 0xff, 0x12, 0xd1, 0xf9, 0x28);

  
    bool IsLoggingEnabled()
    {
#ifdef _DEBUG
      bool result = true;
#else
      bool result = false;
#endif

      CommandLine* cmd_line = CommandLine::ForCurrentProcess();
      if (cmd_line && cmd_line->HasSwitch("enable-log")) {
        result =  true;
      }

      if (result) {
        ::logging::LogEventProvider::Initialize(kLogProvider);
        ::logging::LoggingSettings settings;
        settings.logging_dest = ::logging::LOG_TO_ALL;

        wchar_t lpPath[MAX_PATH];
        
        swprintf_s(lpPath, MAX_PATH, L"%s\\debug.log",
                   GetCurrentModuleDir().value().c_str());
        settings.log_file = lpPath;
        ::logging::InitLogging(settings);
      }

      return result;
    }
  }
  //----------------------------------------------------------------------------
  // static
  RuntimeContext* RuntimeContext::GetInstance() {
    return Singleton<RuntimeContext>::get();
  }
  //----------------------------------------------------------------------------
  RuntimeContext::RuntimeContext()
    : running_in_service_(true)
    , logging_enabled_(false)
  {

    running_in_service_ = IsRunningInService() ? true : false;
    logging_enabled_ = IsLoggingEnabled();
  }
  //----------------------------------------------------------------------------
  RuntimeContext::~RuntimeContext()
  {
  }
  //----------------------------------------------------------------------------
  bool RuntimeContext::InService() const
  {
    return running_in_service_;
  }
  //----------------------------------------------------------------------------
  bool RuntimeContext::LoggingEnabled() const
  {
    return logging_enabled_;
  }
  //----------------------------------------------------------------------------
}