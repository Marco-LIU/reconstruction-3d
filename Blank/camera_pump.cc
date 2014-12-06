#include "camera_pump.h"

#include <vector>

#include "base/logging.h"
#include "base/atomicops.h"
#include "base/time/time.h"

#include "usb_camera.h"
#include "camera_frame.h"
#include "camera_pump_delegate.h"
#include "runtime_context.h"

//------------------------------------------------------------------------------
class CameraPump::Context
    : public base::RefCountedThreadSafe<CameraPump::Context> {
public:
  Context(scoped_refptr<UsbCamera> camera,
          CameraPumpDelegate* delagate);
  ~Context();
  bool IsPumping() const;
  bool Start();
  void Stop();
private:
  static DWORD WINAPI PumpingThread(void* p);

  void PumpRunLoop();
  bool NeedStop();
public:
  HANDLE thread_handle_;

  // set in main thread, used in background thread
  volatile base::subtle::Atomic32 stop_event_;

  CameraPumpDelegate* delagate_;
  scoped_refptr<UsbCamera> camera_;

  int camera_id_;
  unsigned int buffer_length_;
  CameraFrame frame_;
};
//------------------------------------------------------------------------------
CameraPump::Context::Context(scoped_refptr<UsbCamera> camera,
                             CameraPumpDelegate* delagate)
    : camera_(camera)
    , delagate_(delagate)
    , thread_handle_(NULL) {
  DCHECK(camera_.get());
  DCHECK(delagate_);
  // 初始化
  camera_id_ = camera_->id();
  buffer_length_ = camera_->buffer_length();
  frame_.width = camera_->width();
  frame_.height = camera_->height();
  frame_.bpp = 8;
  frame_.frame_seq = 0;
}
//------------------------------------------------------------------------------
CameraPump::Context::~Context() {
  Stop();

  if (thread_handle_) ::CloseHandle(thread_handle_);
}
//------------------------------------------------------------------------------
bool CameraPump::Context::IsPumping() const {
  // 是否正在运行
  if (!thread_handle_) return false;
  return base::subtle::NoBarrier_Load(&stop_event_) == 0;
}
//------------------------------------------------------------------------------
bool CameraPump::Context::Start() {
  DCHECK(camera_.get());

  // 初始化停止事件
  base::subtle::NoBarrier_Store(&stop_event_, 0);
  // balanced at end of thread
  AddRef();
  thread_handle_ = ::CreateThread(NULL, 0,
                                &Context::PumpingThread, this, 0, NULL);
  if (!thread_handle_) {
    Release();
  }

  return thread_handle_ != NULL;
}
//------------------------------------------------------------------------------
void CameraPump::Context::Stop() {
  // 通知子线程停止
  base::subtle::NoBarrier_Store(&stop_event_, 1);
  if (thread_handle_) {
    // 等待子线程退出
    DWORD wait_result = ::WaitForSingleObject(thread_handle_, 2000);
    if (WAIT_OBJECT_0 != wait_result) {
      ::TerminateThread(thread_handle_, 255);
    }

    ::CloseHandle(thread_handle_);
    thread_handle_ = NULL;
  }
}
//------------------------------------------------------------------------------
// static
DWORD CameraPump::Context::PumpingThread(void* p) {
  Context* context = (Context*)p;
  context->PumpRunLoop();
  context->Release();
  return 0;
}
//------------------------------------------------------------------------------
void CameraPump::Context::PumpRunLoop() {
  bool start_seq = frame_.frame_seq;
  while (true) {
    //base::TimeTicks ts = base::TimeTicks::Now();

    if (NeedStop()) break;
    // 一帧即将开始
    if (delagate_ && !delagate_->BeforeFrame(camera_id_)) break;

    if (NeedStop()) break;

    scoped_refptr<base::RefCountedBytes> buf;
    // 即将读取数据
    if (delagate_) buf = delagate_->WillRead(camera_id_, buffer_length_);
    if (!buf.get()) {
      buf = CameraPump::NewBuffer(buffer_length_);
    }

    if (NeedStop()) break;

    unsigned char* b = const_cast<unsigned char*>(buf->front());
    // 读取数据
    bool succ = camera_->CaptureFrameSync(true, b);
    frame_.time_stamp = base::TimeTicks::Now();

    if (NeedStop()) break;

    // 采集了一帧的数据
    if (delagate_) {
      frame_.data = succ ? buf : scoped_refptr<base::RefCountedBytes>();
      delagate_->OnFrame(camera_id_, frame_);
    }

    frame_.frame_seq++;

    if (NeedStop()) break;

    // 一帧采集完成
    if (delagate_ && !delagate_->AfterFrame(camera_id_)) break;

    if (NeedStop()) break;

    // base::TimeDelta td = base::TimeTicks::Now() - ts;
    // LLX_INFO() << td.InMilliseconds();
  }

  if (delagate_) delagate_->OnDone(camera_id_, frame_.frame_seq - start_seq);
}
//------------------------------------------------------------------------------
bool CameraPump::Context::NeedStop() {
  return 0 != base::subtle::NoBarrier_Load(&stop_event_);
}
//------------------------------------------------------------------------------
CameraPump::CameraPump(scoped_refptr<UsbCamera> camera,
                       CameraPumpDelegate* delegate)
    : context_(new Context(camera, delegate)) {
}
//------------------------------------------------------------------------------
CameraPump::~CameraPump() {
  StopPumping();
}
//------------------------------------------------------------------------------
bool CameraPump::IsPumping() const {
  if (context_.get()) return context_->IsPumping();
  return false;
}
//------------------------------------------------------------------------------
bool CameraPump::StartPumping() {
  if (IsPumping()) {
    DCHECK(false);
    return false;
  }

  return context_->Start();
}
//------------------------------------------------------------------------------
void CameraPump::StopPumping() {
  if (IsPumping() && context_.get()) {
    context_->Stop();
  }
}
//------------------------------------------------------------------------------
// static
scoped_refptr<base::RefCountedBytes> CameraPump::NewBuffer(unsigned int len) {
  std::vector<unsigned char> buf_vec;
  buf_vec.resize(len);
  scoped_refptr<base::RefCountedBytes> buf =
      base::RefCountedBytes::TakeVector(&buf_vec);
  return buf;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------