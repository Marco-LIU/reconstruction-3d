#ifndef _LLX_RUNTIME_CONTEXT_H__
#define _LLX_RUNTIME_CONTEXT_H__
#include "base/logging.h"

#define ENABLE_LOG 1

namespace LLX
{
  class RuntimeContext
  {
   public:
    static RuntimeContext* GetInstance();

   public:
    RuntimeContext();
    ~RuntimeContext();
    bool InService() const;
    bool LoggingEnabled() const;

   private:
    bool running_in_service_;
    bool logging_enabled_;
  };
}



#define LOGGING_ENABLED() (::LLX::RuntimeContext::GetInstance() ? \
  ::LLX::RuntimeContext::GetInstance()->LoggingEnabled() : false)

#define RUNNING_IN_SERVICE() (::LLX::RuntimeContext::GetInstance() ? \
  ::LLX::RuntimeContext::GetInstance()->InService() : true)


#if ENABLE_LOG
#define LLX_LOG(severity) LOG_IF(severity, LOGGING_ENABLED())
#else
#define LLX_LOG(severity) LOG_IF(severity, false)
#endif

#define LLX_INFO() LLX_LOG(INFO)

#endif