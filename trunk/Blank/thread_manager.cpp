#include "thread_manager.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/message_loop/message_loop.h"
#include "base/threading/thread.h"
#include "base/memory/singleton.h"

namespace LLX
{
  static const char* g_working_thread_names[WorkingThread::ID_COUNT] = {
    "LLX::FileThread",
    "LLX::IOThread",
  };

  static base::MessageLoop::Type g_working_thread_type[WorkingThread::ID_COUNT]
    = {
    base::MessageLoop::TYPE_DEFAULT,
    base::MessageLoop::TYPE_IO,
  };

  class ThreadManagerImpl : public ThreadManager
  {
  public:
    static ThreadManagerImpl* GetInstance() {
      return Singleton<ThreadManagerImpl>::get();
    }
  public:
    ThreadManagerImpl() : is_shut_down_(false), is_initialized_(false) {}
    ~ThreadManagerImpl() {
      for (int i = WorkingThread::ID_COUNT - 1; i >= 0; i--) {
        if (threads_[i].get()) {
          threads_[i].release();
        }
      }
    }

    virtual void Initialize() OVERRIDE {
      if (is_initialized_) return;
      base::AutoLock auto_lock(threads_lock_);
      if (is_initialized_) return;
      is_initialized_ = true;
      for (int i = 0; i < WorkingThread::ID_COUNT; i++) {
        threads_[i].reset(new base::Thread(g_working_thread_names[i]));

        threads_[i]->StartWithOptions(
          base::Thread::Options(g_working_thread_type[i], 0));
      }
    }

    virtual void Shuttingdown() OVERRIDE {
      if (is_shut_down_) return;
      base::AutoLock auto_lock(threads_lock_);
      if (is_shut_down_) return;
      is_shut_down_ = true;
      int release_index = -1;
      for (int i = WorkingThread::ID_COUNT - 1; i >= 0; i--) {
        if (threads_[i].get()) {
          if (threads_[i]->message_loop() == base::MessageLoop::current()) {
            release_index = i;
          } else {
            threads_[i]->StopSoon();
            threads_[i].reset();
          }
        }
      }

      if (release_index != -1)
        threads_[release_index].release();
    }

    virtual bool PostTask(WorkingThread::ID identifier,
        const tracked_objects::Location& from_here,
        const base::Closure& task) OVERRIDE {
      base::AutoLock auto_lock(threads_lock_);
      DCHECK(identifier >= 0 && identifier < WorkingThread::ID_COUNT);
      DCHECK(threads_[identifier] != NULL);
      scoped_refptr<base::MessageLoopProxy> message_loop_proxy;
      if (threads_[identifier].get()) {
        message_loop_proxy = threads_[identifier]->message_loop_proxy();
      }
      if (message_loop_proxy.get()) {
        message_loop_proxy->PostTask(from_here, task);
        return true;
      }
      return false;
    }
    
    virtual bool PostDelayedTask(WorkingThread::ID identifier,
        const tracked_objects::Location& from_here,
        const base::Closure& task,
        base::TimeDelta delay) OVERRIDE {
      base::AutoLock auto_lock(threads_lock_);
      DCHECK(identifier >= 0 && identifier < WorkingThread::ID_COUNT);
      DCHECK(threads_[identifier].get());
      scoped_refptr<base::MessageLoopProxy> message_loop_proxy;
      if (threads_[identifier].get()) {
        message_loop_proxy = threads_[identifier]->message_loop_proxy();
      }
      if (message_loop_proxy.get()) {
        message_loop_proxy->PostDelayedTask(from_here, task, delay);
        return true;
      }
      return false;
    }

    virtual bool CurrentlyOn(WorkingThread::ID identifier) OVERRIDE {
      base::AutoLock auto_lock(threads_lock_);
      DCHECK(identifier >= 0 && identifier < WorkingThread::ID_COUNT);
      return threads_[identifier].get() &&
        threads_[identifier]->message_loop() ==
        base::MessageLoop::current();
    }


    virtual scoped_refptr<base::MessageLoopProxy> GetMessageLoopProxyForThread(
      WorkingThread::ID identifier) OVERRIDE {
      base::AutoLock auto_lock(threads_lock_);
      DCHECK(identifier >= 0 && identifier < WorkingThread::ID_COUNT);
      return threads_[identifier].get() ?
        threads_[identifier]->message_loop_proxy() : NULL;
    }

  protected:
    bool is_shut_down_;
    bool is_initialized_;
    base::Lock threads_lock_;
    scoped_ptr<base::Thread> threads_[WorkingThread::ID_COUNT];
  };

  ThreadManager& ThreadManager::GetInstance() {
    return *ThreadManagerImpl::GetInstance();
  }
}