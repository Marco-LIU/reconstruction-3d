#ifndef _MESSAGE_SERVICE_THREAD_MANAGER_H__
#define _MESSAGE_SERVICE_THREAD_MANAGER_H__
#include "working_thread.h"

namespace LLX
{
  class ThreadManager
  {
  public:
    static ThreadManager& GetInstance();

    virtual void Initialize() = 0;

    virtual void Shuttingdown() = 0;

    virtual bool PostTask(WorkingThread::ID identifier,
      const tracked_objects::Location& from_here,
      const base::Closure& task) = 0;

    virtual bool PostDelayedTask(WorkingThread::ID identifier,
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay) = 0;

    virtual bool CurrentlyOn(WorkingThread::ID identifier) = 0;

    virtual scoped_refptr<base::MessageLoopProxy> GetMessageLoopProxyForThread(
      WorkingThread::ID identifier) = 0;
  };
}
#endif