#include "usb_camera_group.h"
#include "usb_camera.h"
#include "JHCap.h"
#include "cv.h"
#include "OgreTimer.h"

#define IO_THREAD_PER_CAMERA 2

class UsbCameraGroup::RecordContext {
public:
  RecordContext() 
      : is_recording_(false)
      , capture_data_(NULL)
      , io_data_(NULL)
      , threads_(NULL) {
    InitializeCriticalSection(&sync_root_);
    stop_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
  }
  ~RecordContext() {
    CloseHandle(stop_event_);
    DeleteCriticalSection(&sync_root_);

    Clean();
  }

  bool Start(const CameraMap& cameras, CameraGroupRecordDelegate* delegate) {
    timer_.reset();
    Clean();
    delegate_ = delegate;
    camera_count_ = cameras.size();
    capture_data_ = new CaptureThreadData[camera_count_];
    io_data_ = new FileIOThreadData[camera_count_ * IO_THREAD_PER_CAMERA];
    threads_ = new HANDLE[camera_count_ * 3 + 1];
    trigger_data_.loop = 0;

    for (int i = 0; i < camera_count_; i++) {
      capture_data_[i].ioBufferReady = false;
    }
    for (int i = 0; i < camera_count_ * IO_THREAD_PER_CAMERA; i++) {
      io_data_[i].state = FileIOThreadData::IDLE;
    }

    threads_[0] = CreateThread(NULL, 0, CameraTriggerThread, this, 0, NULL);
    if (!threads_[0]) return false;
    pending_camera_count_ = 0;

    int i = 0;
    CameraMap::const_iterator cit = cameras.begin();
    while (cit != cameras.end()) {
      std::string dir = ".";
      if (delegate_) dir = delegate_->GetSavingDir(cit->first);
      FileIOThreadParam* p1 = new FileIOThreadParam;
      p1->context = this;
      p1->io_index = i;
      p1->saving_dir = dir;
      threads_[1 + 3 * i] = CreateThread(NULL, 0, FileIOThread, p1, 0, NULL);
      if (!threads_[1 + 3 * i]) {
        delete p1;
        return false;
      }
      FileIOThreadParam* p2 = new FileIOThreadParam;
      p2->context = this;
      p2->io_index = i + IO_THREAD_PER_CAMERA;
      p2->saving_dir = dir;
      threads_[1 + 3 * i + 1] = CreateThread(NULL, 0, FileIOThread, p2, 0, NULL);
      if (!threads_[1 + 3 * i + 1]) {
        delete p1;
        return false;
      }
      CaptureThreadParam* p = new CaptureThreadParam;
      p->context = this;
      p->camera_id = cit->first;
      p->camera = cit->second;
      threads_[1 + 3 * i + 2] = CreateThread(NULL, 0, CaptureThread, p, 0, NULL);
      if (!threads_[1 + 3 * i + 2]) {
        delete p;
        return false;
      }

      ++i;
      ++cit;
    }

    return true;
  }

  void Quit() {
    _InterlockedAnd(reinterpret_cast<volatile LONG*>(&delegate_), 0);
    SetEvent(stop_event_);
  }
  bool WaitQuit(DWORD ms) {
    DWORD r = WaitForMultipleObjects(camera_count_ * 3 + 1, threads_,
                                     TRUE, ms);

    if (r >= WAIT_OBJECT_0 && r <= WAIT_OBJECT_0 + camera_count_ * 3)
      return true;
    return false;
  }
  void ForceQuit() {
    if (!threads_) return;
    for (int i = 0; i < camera_count_ * 3 + 1; ++i) {
      if (threads_[i]) TerminateThread(threads_[i], 255);
    }
  }

private:
  void Clean() {
    for (int i = 0; i < camera_count_ * IO_THREAD_PER_CAMERA; ++i) {
      if (io_data_[i].buffer)
        delete[] io_data_[i].buffer;
    }
    if (capture_data_) delete[] capture_data_;
    if (io_data_) delete[] io_data_;

    if (threads_) {
      for (int i = 0; i < camera_count_ * 3 + 1; ++i) {
        if (threads_[i]) CloseHandle(threads_[i]);
      }
      delete[] threads_;
    }
  }

private:
  struct CaptureThreadData
  {
    unsigned long  preCaptureTime;
    unsigned long  postCaptureTime;
    bool           captureResult;
    volatile bool  ioBufferReady;
  };

  struct TriggerThreadData
  {
    unsigned long loop;
  };

  struct FileIOThreadData
  {
    FileIOThreadData() : buffer(NULL) {}

    enum { IDLE, FILLING, WRITING };

    unsigned char*  buffer;
    int            size; // must be int
    unsigned int   camIndex;
    unsigned long  imgIndex;
    volatile int   state;
  };


private:
  struct CaptureThreadParam {
    RecordContext* context;
    int camera_id;
    UsbCamera* camera;
  };

  static DWORD WINAPI CaptureThread(LPVOID p) {
    CaptureThreadParam* param = (CaptureThreadParam*)p;
    RecordContext* context = param->context;
    int camId = param->camera_id;
    UsbCamera* camera = param->camera;
    delete param;

    CaptureThreadData& d = context->capture_data_[camId];
    int ioThreadIndex = -1;
    while (true) {
      if (WAIT_OBJECT_0 == WaitForSingleObject(context->stop_event_, 0))
        break;
      if (!d.ioBufferReady) {
        for (int i = camId; i < context->camera_count_ * 2; i += context->camera_count_) // 不同camera永远不会竞争同一个I/O线程
        {
          if (context->io_data_[i].state == FileIOThreadData::IDLE) {
            context->io_data_[i].state = FileIOThreadData::FILLING;
            d.ioBufferReady = true;
            ioThreadIndex = i;
            break;
          }
        }
      }

      if (WAIT_OBJECT_0 == WaitForSingleObject(context->stop_event_, 0))
        break;

      if (!d.ioBufferReady || ioThreadIndex == -1) {
        continue;
      }

      while (context->pending_camera_count_ <= 0) Sleep(1);
      d.preCaptureTime = context->timer_.getMicroseconds();

      if (!context->io_data_[ioThreadIndex].buffer) {
        context->io_data_[ioThreadIndex].buffer =
            new unsigned char[camera->buffer_length()];
        context->io_data_[ioThreadIndex].size = camera->buffer_length();
      }

      d.captureResult = camera->CaptureFrameSync(
          true, context->io_data_[ioThreadIndex].buffer);

      if (WAIT_OBJECT_0 == WaitForSingleObject(context->stop_event_, 0))
        break;

      if (context->delegate_) {
        bool r = context->delegate_->OnFrame(
            camId, context->trigger_data_.loop,
            context->io_data_[ioThreadIndex].buffer, camera->buffer_length());

        if (!r) d.captureResult = false;
      }

      if (!d.captureResult) {
        context->io_data_[ioThreadIndex].state = FileIOThreadData::IDLE;
        InterlockedDecrement(&context->pending_camera_count_);
        continue;
      }
      d.postCaptureTime = context->timer_.getMicroseconds();
      d.ioBufferReady = false;
      std::cout << "Trigger io thread: " << ioThreadIndex << std::endl;
      context->io_data_[ioThreadIndex].state = FileIOThreadData::WRITING;
      context->io_data_[ioThreadIndex].camIndex = camId;
      context->io_data_[ioThreadIndex].imgIndex = context->trigger_data_.loop;
      InterlockedDecrement(&context->pending_camera_count_);
    }

    std::cout << "CaptureThread: " << camId << " quit" << std::endl;

    return 0;
  }

  struct FileIOThreadParam {
    RecordContext* context;
    int io_index;
    std::string saving_dir;
  };

  static DWORD WINAPI FileIOThread(LPVOID p) {
    FileIOThreadParam* param = (FileIOThreadParam*)p;
    RecordContext* context = param->context;
    int ioIndex = param->io_index;
    std::string dir = param->saving_dir;
    delete param;
    std::cout << "IO Thread: " << ioIndex << std::endl;

    FileIOThreadData& d = context->io_data_[ioIndex];

    char filename[32];
    while (true) {
      if (WAIT_OBJECT_0 == WaitForSingleObject(context->stop_event_, 0))
        break;
      if (d.state != FileIOThreadData::WRITING) {
        Sleep(1);
        continue;
      }

      // 写入文件
      _snprintf(filename, sizeof(filename), "%s/save/%s/%d.jpg",
                dir.c_str(), d.camIndex ? "right" : "left", d.imgIndex);

      std::cout << ioIndex << " to write file: " << filename << std::endl;

      /*for (int i = d.camIndex * 4; i < d.camIndex * 4 + 4; i++) {
        if (gTimeBoard[i].size() < d.imgIndex) {
          gTimeBoard[i].resize(d.imgIndex);
        }
      }
      gTimeBoard[d.camIndex * 4 + 0][d.imgIndex - 1] = g_captureData[d.camIndex].preCaptureTime;
      gTimeBoard[d.camIndex * 4 + 1][d.imgIndex - 1] = g_captureData[d.camIndex].postCaptureTime;
      gTimeBoard[d.camIndex * 4 + 2][d.imgIndex - 1] = gTimer.getMicroseconds();*/
      FILE* fp = fopen(filename, "w");
      fwrite(d.buffer, d.size, 1, fp);
      fclose(fp);
      if (WAIT_OBJECT_0 == WaitForSingleObject(context->stop_event_, 0))
        break;
      //gTimeBoard[d.camIndex * 4 + 3][d.imgIndex - 1] = gTimer.getMicroseconds();

      /*std::cerr << "file written: " << filename << " with time "
      << gTimeBoard[d.camIndex * 4 + 0][d.imgIndex - 1] << ","
      << gTimeBoard[d.camIndex * 4 + 1][d.imgIndex - 1] << ","
      << gTimeBoard[d.camIndex * 4 + 2][d.imgIndex - 1] << ","
      << gTimeBoard[d.camIndex * 4 + 3][d.imgIndex - 1] << ""
      << std::endl; */
      d.state = FileIOThreadData::IDLE;
    }

    std::cerr << "IO thread finished " << ioIndex << std::endl;
    return 0;
  }

  static DWORD WINAPI CameraTriggerThread(LPVOID p) {
    RecordContext* context = (RecordContext*)p;
    Sleep(10);
    while (true) {
      if(WAIT_OBJECT_0 == WaitForSingleObject(context->stop_event_, 0))
        break;
      if (context->pending_camera_count_ > 0) {
        Sleep(1);
        continue;
      }

      bool allReady = true;
      for (int i = 0; i < context->camera_count_; i++) {
        if (!context->capture_data_[i].ioBufferReady) {
          allReady = false;
        }
      }
      if (!allReady) {
        Sleep(1);
        continue;
      }

      context->trigger_data_.loop++;
      InterlockedExchangeAdd(&context->pending_camera_count_,
                             context->camera_count_);
      Sleep(2);
    }
    std::cout << "CameraTriggerThread quit" << std::endl;
    return 0;
  }
public:
  volatile bool is_recording_;
  HANDLE stop_event_;
  CameraGroupRecordDelegate* delegate_;
  CRITICAL_SECTION sync_root_;

  int camera_count_;
  volatile LONG pending_camera_count_;
  TriggerThreadData trigger_data_;
  CaptureThreadData* capture_data_;
  FileIOThreadData* io_data_;
  Timer timer_;
  HANDLE* threads_;
};

UsbCameraGroup::UsbCameraGroup()
    : record_context_(NULL) {
  record_context_ = new RecordContext();
}

UsbCameraGroup::~UsbCameraGroup() {
  StopAll();

  {
    CameraMap::iterator it = cameras_.begin();
    while (it != cameras_.end()) {
      delete it->second;
      ++it;
    }
    cameras_.clear();
  }
  {
    BufferMap::iterator it = buffers_.begin();
    while (it != buffers_.end()) {
      delete[] it->second;
      ++it;
    }
    buffers_.clear();
  }
  delete record_context_;
}

bool UsbCameraGroup::Init(const std::string& config) {
  int count = 0;
  CameraGetCount(&count);
  bool succ = true;

  std::map<std::string, int>* sn_to_id = NULL;

  if (config != "") {
    sn_to_id = new std::map<std::string, int>();
    //从配置文件中读取摄像头序列号
    cv::FileStorage fs(config, cv::FileStorage::READ);
    for (cv::FileNodeIterator i = fs["Camera"].begin(); i != fs["Camera"].end(); i++) {
      std::string nodeName = (*i).name();
      int id = int(*i);
      (*sn_to_id)[nodeName] = id;
    }
    fs.release();
  }

  for (int i = 0; i < count; i++) {
    UsbCamera* c = new UsbCamera(i);
    if (c->Init()) {
      if (sn_to_id) {
        std::string sn = c->sn();

        std::map<std::string, int>::iterator it = sn_to_id->find(sn);
        if (it != sn_to_id->end()) {
          cameras_[it->second] = c;
          unsigned char* buf = new unsigned char[c->buffer_length()];
          memset(buf, 0, c->buffer_length());
          buffers_[it->second] = buf;
        } else {
          succ = false;
        }
      } else {
        cameras_[i] = c;
        unsigned char* buf = new unsigned char[c->buffer_length()];
        memset(buf, 0, c->buffer_length());
        buffers_[i] = buf;
      }
    } else {
      succ = false;
    }
  }

  if (sn_to_id) delete sn_to_id;

  return succ;
}

bool UsbCameraGroup::StartAll() {
  bool succ = true;

  CameraMap::iterator it = cameras_.begin();
  while (it != cameras_.end()) {
    if (!it->second->Start())
      succ = false;
    ++it;
  }
  return succ;
}

void UsbCameraGroup::StopAll() {
  StopRecord();
  CameraMap::iterator it = cameras_.begin();
  while (it != cameras_.end()) {
    it->second->Stop();
    ++it;
  }
}

unsigned int UsbCameraGroup::camera_count() const {
  return cameras_.size();
}

UsbCamera* UsbCameraGroup::GetCamera(int id) const {
  CameraMap::const_iterator it = cameras_.find(id);
  if (it != cameras_.end()) return it->second;

  return NULL;
}

bool UsbCameraGroup::CaptureFrame(bool use_trigger, const BufferMap* buffers) {
  if (buffers == NULL)
    buffers = &buffers_;

  unsigned int count = buffers->size();
  HANDLE* handles = new HANDLE[count];
  memset(handles, 0, count * sizeof(HANDLE));
  BufferMap::const_iterator cit = buffers_.begin();
  unsigned int index = 0;
  while (cit != buffers_.end()) {
    UsbCamera* camera = GetCamera(cit->first);
    if (camera) {
      handles[index] = camera->CaptureFrameAsync(use_trigger, cit->second);
    }

    if (handles[index] == NULL) {
      for (int i = 0; i < index; i++) {
        CloseHandle(handles[i]);
      }
      return false;
    }

    ++index;
    ++cit;
  }

  DWORD dwResult = WaitForMultipleObjects(count, handles, TRUE, 100);

  for (int i = 0; i < count; i++) {
    CloseHandle(handles[i]);
  }

  if (dwResult >= WAIT_OBJECT_0 && dwResult <= WAIT_OBJECT_0 + count - 1)
    return true;
  return false;
}

const unsigned char* UsbCameraGroup::camera_buffer(int id) const {
  BufferMap::const_iterator cit = buffers_.find(id);
  if (cit != buffers_.end()) return cit->second;
  return NULL;
}

bool UsbCameraGroup::StartRecord(CameraGroupRecordDelegate* delegate) {
  if (record_context_->is_recording_) return false;
  bool result = false;

  result = record_context_->Start(cameras_, delegate);

  if (result)
    record_context_->is_recording_ = true;

  return result;
}

void UsbCameraGroup::StopRecord() {
  record_context_->Quit();
  if (!record_context_->WaitQuit(200)) {
    record_context_->ForceQuit();
  }
  record_context_->is_recording_ = false;
}