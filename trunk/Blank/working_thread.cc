#include "working_thread.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/lazy_instance.h"
#include "thread_manager.h"

namespace LLX {
#ifdef _DEBUG
  static ThreadManager* s_thread_manager = NULL;
  void WorkingThread::SetThreadManagerForTesting(ThreadManager* thread_manager)
  {
    s_thread_manager = thread_manager;
  }
#endif
    bool WorkingThread::PostTask(ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task)
  {
#ifdef _DEBUG
    if(s_thread_manager)
      return s_thread_manager->PostTask(identifier, from_here, task);
#endif
    return ThreadManager::GetInstance().PostTask(identifier, from_here, task);
  }
  bool WorkingThread::PostDelayedTask(ID identifier,
    const tracked_objects::Location& from_here,
    const base::Closure& task,
    base::TimeDelta delay)
  {
#ifdef _DEBUG
    if(s_thread_manager) {
      return s_thread_manager->PostDelayedTask(identifier,
        from_here, task, delay);
    }
#endif
    return ThreadManager::GetInstance().
      PostDelayedTask(identifier, from_here, task, delay);
  }

  bool WorkingThread::CurrentlyOn(ID identifier)
  {
#ifdef _DEBUG
    if(s_thread_manager)
      return s_thread_manager->CurrentlyOn(identifier);
#endif
    return ThreadManager::GetInstance().CurrentlyOn(identifier);
  }

  scoped_refptr<base::MessageLoopProxy>
    WorkingThread::GetMessageLoopProxyForThread(ID identifier)
  {
#ifdef _DEBUG
    if(s_thread_manager)
      return s_thread_manager->GetMessageLoopProxyForThread(identifier);
#endif
    return ThreadManager::GetInstance().GetMessageLoopProxyForThread(identifier);
  }

  void AssureWorkingThreadStartup()
  {
#ifdef _DEBUG
    if(s_thread_manager)
      s_thread_manager->Initialize();
#endif
    ThreadManager::GetInstance().Initialize();
  }

  void NotifyWorkingThreadShutdown()
  {
#ifdef _DEBUG
    if(s_thread_manager) {
      s_thread_manager->Shuttingdown();
      s_thread_manager = NULL;
    }
#endif
    ThreadManager::GetInstance().Shuttingdown();
  }
} // namespace message_push
