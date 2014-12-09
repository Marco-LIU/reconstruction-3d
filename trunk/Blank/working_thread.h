#pragma once
#include "base/basictypes.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/time/time.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/memory/ref_counted.h"


namespace LLX {
#ifdef _DEBUG
  class ThreadManager;
#endif

class WorkingThread {
 public:
  enum ID {
    FILE1,
    FILE2,
    IO,
    ID_COUNT
  };

  static bool PostTask(ID identifier,
                       const tracked_objects::Location& from_here,
                       const base::Closure& task);
  static bool PostDelayedTask(ID identifier,
                              const tracked_objects::Location& from_here,
                              const base::Closure& task,
                              base::TimeDelta delay);

  static bool CurrentlyOn(ID identifier);

  static scoped_refptr<base::MessageLoopProxy> GetMessageLoopProxyForThread(
      ID identifier);

  template <class T>
  static bool DeleteSoon(ID identifier,
                         const tracked_objects::Location& from_here,
                         const T* object) {
    return GetMessageLoopProxyForThread(identifier)->DeleteSoon(
        from_here, object);
  }

  template <class T>
  static bool ReleaseSoon(ID identifier,
                          const tracked_objects::Location& from_here,
                          const T* object) {
    return GetMessageLoopProxyForThread(identifier)->ReleaseSoon(
        from_here, object);
  }

  template<ID thread>
  struct DeleteOnThread {
    template<typename T>
    static void Destruct(const T* x) {
      if (CurrentlyOn(thread)) {
        delete x;
      } else {
        if (!DeleteSoon(thread, FROM_HERE, x)) {

        }
      }
    }
  };


  struct DeleteOnIOThread : public DeleteOnThread<IO> {};
  struct DeleteOnFileThread : public DeleteOnThread<FILE1> {};

#ifdef _DEBUG
  static void SetThreadManagerForTesting(ThreadManager* thread_manager);
#endif
};

void AssureWorkingThreadStartup();

void NotifyWorkingThreadShutdown();
}