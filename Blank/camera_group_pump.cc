#include "camera_group_pump.h"

#include "build_config.h"

#include "base/atomicops.h"
#include "base/memory/linked_ptr.h"
#include "base/synchronization/lock.h"

#include "camera_frame.h"
#include "camera_pump.h"
#include "camera_pump_delegate.h"
#include "camera_group_pump_delegate.h"
#include "usb_camera_group.h"
#include "usb_camera.h"
#include "runtime_context.h"

//------------------------------------------------------------------------------
class CameraGroupPump::Context
    : public CameraPumpDelegate
    , public base::RefCountedThreadSafe<CameraGroupPump::Context> {
public:
  Context(UsbCameraGroup* camera_group, CameraGroupPumpDelegate* delegate);
  ~Context();
  // ��ʼ
  bool Start();
  // ֹͣ
  void Stop();

  bool IsPumping() const;

  // ʵ��CameraPumpDelegate
  virtual bool BeforeFrame(int id) OVERRIDE;

  virtual scoped_refptr<base::RefCountedBytes> WillRead(
    int id, unsigned int len) OVERRIDE;

  virtual void OnFrame(int id, const CameraFrame& frame) OVERRIDE;

  virtual bool AfterFrame(int id) OVERRIDE;

  virtual void OnDone(int id, unsigned int frames) OVERRIDE;

  float FrameRate();

private:
  static DWORD WINAPI PumpingThread(void* p);

  void PumpRunLoop();
  bool NeedStop();

  // int Id2Index(int id) const;

  void NotifyNewFrame();

  // used in main thread
  UsbCameraGroup* camera_group_;
  unsigned int camera_count_;

  // used in backgound thread
  // ÿ5֡ͳ��һ��֡��
  unsigned int frame_spin_;
  base::Time last_tick_;
  volatile float frame_rate_;

  // filled in each CameraPump thread, collected in backgroud thread
  CameraFrames frame_cache_;
  base::Lock lock_;

  // set in main thread, used in backgound thread
  CameraGroupPumpDelegate* delegate_;

  volatile base::subtle::Atomic32 stop_event_;
  volatile base::subtle::Atomic32 free_cameras_;

  HANDLE semaphore_;

  HANDLE pumping_thread_;

  base::hash_map<int, linked_ptr<CameraPump> > pump_map_;
};
//------------------------------------------------------------------------------
CameraGroupPump::Context::Context(UsbCameraGroup* camera_group,
                                  CameraGroupPumpDelegate* delegate)
    : camera_group_(camera_group)
    , delegate_(delegate)
    , camera_count_(camera_group->camera_count())
    , pumping_thread_(NULL)
    , stop_event_(0)
    , free_cameras_(0)
    , semaphore_(NULL) {
  
}
//------------------------------------------------------------------------------
CameraGroupPump::Context::~Context() {
  Stop();

  if (semaphore_) {
    ::CloseHandle(semaphore_);
    semaphore_ = NULL;
  }
}
//------------------------------------------------------------------------------
bool CameraGroupPump::Context::Start() {
  if (!camera_count_) return false;
  do
  {
    // Ϊ��������ͷ����pump
    pump_map_.clear();
    const UsbCameraGroup::CameraMap& cameras = camera_group_->cameras();

    UsbCameraGroup::CameraMap::const_iterator cit = cameras.begin();
    while (cit != cameras.end()) {
      linked_ptr<CameraPump> pump(new CameraPump(cit->second, this));
      pump->StartPumping();
      pump_map_[cit->first] = pump;
      ++cit;
    }

    // ����ͬ���ź���
    semaphore_ = ::CreateSemaphoreW(NULL, 0, camera_count_, NULL);
    if (!semaphore_) break;

    // ��ʼ��
    base::subtle::NoBarrier_Store(&stop_event_, 0);
    // ��������ͷ��Ŀ
    base::subtle::NoBarrier_Store(&free_cameras_, 0);

    // balance at end of thread
    AddRef();

    // �������߳�
    pumping_thread_ = ::CreateThread(NULL, 0,
                                     &Context::PumpingThread, this, 0, NULL);

    if (!pumping_thread_) {
      Release();
      break;
    }

    return true;
  } while (false);

  if (semaphore_) {
    ::CloseHandle(semaphore_);
    semaphore_ = NULL;
  }
  return false;
}
//------------------------------------------------------------------------------
void CameraGroupPump::Context::Stop() {
  // ֹͣ��������ͷ��pump
  base::hash_map<int, linked_ptr<CameraPump> >::iterator it =
      pump_map_.begin();
  while (it != pump_map_.end()) {
    it->second->StopPumping();
    ++it;
  }
  pump_map_.clear();
  // ֪ͨ��̨�߳��˳�
  base::subtle::NoBarrier_Store(&stop_event_, 1);

  // �ȴ���̨�߳��˳����
  if (pumping_thread_) {
    DWORD wait_result = ::WaitForSingleObject(pumping_thread_, 2000);
    if (WAIT_OBJECT_0 != wait_result) {
      ::TerminateThread(pumping_thread_, 255);
    }

    ::CloseHandle(pumping_thread_);
    pumping_thread_ = NULL;
  }
}
//------------------------------------------------------------------------------
float CameraGroupPump::Context::FrameRate() {
  return frame_rate_;
}
//------------------------------------------------------------------------------
bool CameraGroupPump::Context::IsPumping() const {
  // �Ƿ���������
  if (!pumping_thread_) return false;
  return base::subtle::NoBarrier_Load(&stop_event_) == 0;
}
//------------------------------------------------------------------------------
// static
DWORD CameraGroupPump::Context::PumpingThread(void* p) {
  Context* context = (Context*)p;
  context->PumpRunLoop();
  context->Release();
  return 0;
}
//------------------------------------------------------------------------------
void CameraGroupPump::Context::PumpRunLoop() {
  bool need_stop = false;
  frame_spin_ = 0;
  last_tick_ = base::Time::Now();
  frame_rate_ = -1;
  while (!NeedStop()) {
    //camera_group_->SoftTriggerAll();
    // ͬʱ��������camera��ʼ��ȡͼ��
    ::ReleaseSemaphore(semaphore_, camera_count_, NULL);
    // LLX_INFO() << "Waiting all camera done...";
    while (true) {
      // �ȴ�����camera��ȡͼ�����
      if (camera_count_ == base::subtle::NoBarrier_CompareAndSwap(
          &free_cameras_, camera_count_, 0))
        break;
      // �Ƿ���Ҫֹͣ
      if (NeedStop()) {
        need_stop = true;
        break;
      }
      ::Sleep(1);
    }
    // LLX_INFO() << "All camera done...";
    if (need_stop) break;
    // �����������һ֡���ռ�
    NotifyNewFrame();
  }
}
//------------------------------------------------------------------------------
bool CameraGroupPump::Context::BeforeFrame(int id) {
#if ENABLE_CAMERA_PUMP_PROFILE
  base::Time tick = base::Time::Now();
#endif
  while (true) {
    // �ȴ�һ֡�Ŀ�ʼ�ź�
    if (WAIT_OBJECT_0 == WaitForSingleObject(semaphore_, 0)) break;
    if (NeedStop()) return false;
    ::Sleep(1);
  }
#if ENABLE_CAMERA_PUMP_PROFILE
  base::TimeDelta td = base::Time::Now() - tick;
  LLX_INFO() << "Camera " << id << " starting a frame, waiting time: "
      << td.InMilliseconds();
#endif
  return true;
}
//------------------------------------------------------------------------------
scoped_refptr<base::RefCountedBytes> CameraGroupPump::Context::WillRead(
    int id, unsigned int len) {
  // ��������
  return CameraPump::NewBuffer(len);
}
//------------------------------------------------------------------------------
void CameraGroupPump::Context::OnFrame(int id, const CameraFrame& frame) {
  // LLX_INFO() << "Camera " << id << " capture a frame...";
  // ����ͷ��ȡ��һ֡����Ҫ��frame_cache_����
#if ENABLE_CAMERA_PUMP_PROFILE
  base::Time tick = base::Time::Now();
#endif
  base::AutoLock al(lock_);
  frame_cache_[id] = frame;
#if ENABLE_CAMERA_PUMP_PROFILE
  base::TimeDelta td = base::Time::Now() - tick;
  LLX_INFO() << "Camera " << id << " collecting a frame, lock time: "
      << td.InMilliseconds();
#endif
  // LLX_INFO() << "Camera " << id << " capture a frame done..."
  //     << frame_cache_.size();
}
//------------------------------------------------------------------------------
bool CameraGroupPump::Context::AfterFrame(int id) {
  // LLX_INFO() << "Camera " << id << " after a frame...";
  // ��ʾ����ͷ����
  ::Sleep(1);
  base::subtle::NoBarrier_AtomicIncrement(&free_cameras_, 1);
  return true;
}
//------------------------------------------------------------------------------
void CameraGroupPump::Context::OnDone(int id, unsigned int frames) {
  // ignore
}
//------------------------------------------------------------------------------
// int CameraGroupPump::Context::Id2Index(int id) const {
//   return id;
// }
//------------------------------------------------------------------------------
bool CameraGroupPump::Context::NeedStop() {
  return base::subtle::NoBarrier_Load(&stop_event_) != 0;
}
//------------------------------------------------------------------------------
void CameraGroupPump::Context::NotifyNewFrame() {
  base::Time t = base::Time::Now();
  base::TimeDelta td = t - last_tick_;
  if (++frame_spin_ == 5) {
    frame_rate_ = (float)(frame_spin_ * 1000 / (td.InMillisecondsF()));
    frame_spin_ = 0;
    last_tick_ = t;
  } else if(frame_rate_ < 0) {
    frame_rate_ = (float)(frame_spin_ * 1000 / (td.InMillisecondsF()));
  }

  scoped_ptr<CameraFrames> frame_cache(new CameraFrames);
  {
    base::AutoLock al(lock_);
    // �������ܻ�Ϊÿ������ͷ��ȡһ֡
    //DCHECK_EQ(camera_count_, (unsigned int)frame_cache_.size());
    if (camera_count_ != frame_cache_.size()) {
      LLX_INFO() << "Error frame count, frame: " << frame_cache_.size()
        << " cameras: " << camera_count_;
    }
    frame_cache->swap(frame_cache_);
  }
  CameraFrames::iterator it = frame_cache->begin();
  while (it != frame_cache->end()) {
    it->second.sync_stamp = t;
    ++it;
  }

  if (delegate_) delegate_->OnFrame(frame_cache.Pass());
}
//------------------------------------------------------------------------------
CameraGroupPump::CameraGroupPump(UsbCameraGroup* camera_group,
                                 CameraGroupPumpDelegate* delegate)
    : context_(new Context(camera_group, delegate)) {

}
//------------------------------------------------------------------------------
CameraGroupPump::~CameraGroupPump() {
  StopPumping();
}
//------------------------------------------------------------------------------
bool CameraGroupPump::IsPumping() const {
  if (context_.get()) return context_->IsPumping();
  return false;
}
//------------------------------------------------------------------------------
bool CameraGroupPump::StartPumping() {
  if (IsPumping()) {
    DCHECK(false);
    return false;
  }

  return context_->Start();
}
//------------------------------------------------------------------------------
void CameraGroupPump::StopPumping() {
  if (IsPumping() && context_.get()) {
    context_->Stop();
  }
}
//------------------------------------------------------------------------------
float CameraGroupPump::FrameRate() {
  if (!IsPumping() || !context_.get()) return 0.0f;
  return context_->FrameRate();
}
//------------------------------------------------------------------------------