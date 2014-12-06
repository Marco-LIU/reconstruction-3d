#include "recordWindow.h"
#include <QtWidgets\qapplication.h>
#include <QtWidgets\qmainwindow.h>
#include <QtWidgets\qdesktopwidget.h>
#include <QtWidgets\qsplashscreen.h>
#include <QtWidgets\qaction.h>
#include <QtWidgets\qactiongroup.h>
#include <QtWidgets\qmenubar.h>
#include <QtWidgets\qtoolbar.h>
#include <QtWidgets\qstatusbar.h>
#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qlabel.h>
#include <QtWidgets\qboxlayout.h>
#include <QtWidgets\qpushbutton.h>
#include <QtWidgets\qfiledialog.h>
#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qcolordialog.h>
#include <QtWidgets\qprogressdialog.h>
#include <QtWidgets\qgraphicsscene.h>
#include <QtWidgets\qgraphicsitem.h>
#include <QtWidgets\qgraphicsview.h>
#include <QtWidgets\qboxlayout.h>
#include <QtWidgets\qabstractscrollarea.h>
#include <QtWidgets\qscrollbar.h>

#include <QtCore\qtimer.h>
#include <QtCore\qtextstream.h>
#include <QtCore\qtextcodec.h>

#include <QtGui\qtransform.h>
#include <QtGui\qevent.h>
#include <QtGui\qpainter.h>

#include <vector>

#include "base/callback.h"
#include "base/bind.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/sys_string_conversions.h"
#include "base/files/file_util.h"
#include "base/message_loop/message_loop_proxy.h"

#include "runtime_context.h"
#include "working_thread.h"
#include "usb_camera_group.h"
#include "camera_frame.h"
#include "myCameraView.h"
#include "marker.h"
#include "paras.h"

using LLX::WorkingThread;

RecordWindow::RecordWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
                           QGraphicsPixmapItem* right, QStatusBar* status)
    : weak_factory_(this) {
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;

  mCameras = NULL;	//摄像头为空
  mbPlay = false;		//true表示正在预览
  mbRecord = false;	//true表示正在录制，false表示暂停录制
  mbStop = true;		//true表示已经停止了录制

  //创建布局
  createLayout();

  //设置默认场景
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}

RecordWindow::~RecordWindow() {
  closeRecordWindow();
}

//把灰度数据转换为QImage格式
QImage RecordWindow::convertToQImage(const unsigned char* buffer) {
  int w = Paras::getSingleton().width;
  int h = Paras::getSingleton().height;
  QImage temp(w, h, QImage::Format_ARGB32);
  for (int i = 0; i < h; i++) {
    unsigned char* pCurData = temp.scanLine(i);
    for (int j = 0; j < w; j++) {
      unsigned char data = buffer[i*w + j];

      *(pCurData + j * 4) = data;
      *(pCurData + j * 4 + 1) = data;
      *(pCurData + j * 4 + 2) = data;
      *(pCurData + j * 4 + 3) = 255;
    }
  }
  return temp;
}
//更新场景图像
void RecordWindow::updatePixmap(const unsigned char* leftBuffer,
                                const unsigned char* rightBuffer) {
  QImage li = convertToQImage(leftBuffer);
  QImage ri = convertToQImage(rightBuffer);

  /*if (mbRecord) {
    //如果录制
    mRecordCnt++;

    //保存左右图像
    QString name;
    name.sprintf("%d.jpg", mRecordCnt);
    li.save(Paras::getSingleton().LeftImagesFoler + name);
    ri.save(Paras::getSingleton().RightImagesFoler + name);
    //记录文件信息，添加的方式
    QFile file(Paras::getSingleton().ImagesFoler + "info.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);
    ts << mRecordCnt << " " << mProTimer.getMilliseconds() - mRecordStart << "\r\n";
    ts.flush();
    file.close();

    //显示信息
    mStatusBar->showMessage("图像" + name + "保存完毕");
  }*/

  updatePixmap(li, ri);
}

void RecordWindow::updatePixmap(QImage& li, QImage& ri) {
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));
  //更新图像
  update();
}

//1录制视频，4按钮的处理函数
//预览视频拍摄的画面
void RecordWindow::preview() {
  //如果没有启动摄像头，启动，并开始预览
  if (mbPlay == false) {
    mCameras = new UsbCameraGroup();
    bool succ = mCameras->Init("para.json");

    //如果启动失败，提示摄像机没有连接好
    if (!succ || mCameras->camera_count() != 2) {
      mCameras = NULL;
      mPlay->setText("停止");
      QMessageBox::critical(
        0,							//父窗口
        "找不到可用的摄像头",		//标题栏
        "找不到可用的摄像头，请查看摄像头是否已经连接到电脑，"
        "如已经连接，请重新插拔USB接口");		//文本内容
    }
    //成功启动
    else {
      mCameras->StartAll();

      mCameras->SetFrameCallback(base::Bind(&RecordWindow::RecordFrame,
                                            base::Unretained(this)));
      //切换其它按钮状态
      mbPlay = true;
      mRecord->setEnabled(true);
      mPlay->setText("停止");

      //设置定时触发
      mTimer = new QTimer(this);
      connect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->start(10);
    }
  }
  //如果成功启动摄像头，则关闭它
  else {
    if (mCameras != NULL) {
      mbPlay = false;
      disconnect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->stop();
      delete mTimer;
      mTimer = NULL;
      mCameras->SetFrameCallback(UsbCameraGroup::FrameCallback());
      mCameras->StopAll();
      mCameras = NULL;
      mPlay->setText("预览");

      mRecord->setDisabled(true);
    }
  }
}
//选择保存位置
void RecordWindow::selectSaveFolder() {
  //选择一个文件夹
  QString dirName = QFileDialog::getExistingDirectory(
    this,							//父窗口
    "请选择保存视频文件的文件夹",	//标题栏
    "");							//默认为当前文件夹

  //如果选择了一个文件夹
  if (!dirName.isEmpty()) {
    Paras::getSingleton().ImagesFoler = dirName;
    Paras::getSingleton().LeftImagesFoler = dirName + "/left/";
    Paras::getSingleton().RightImagesFoler = dirName + "/right/";

    //如果文件夹存在，询问删除，还是选择别的地方
    QDir imagesFolder(dirName);
    if (imagesFolder.exists("left") || imagesFolder.exists("right")) {
      //question对话框
      QMessageBox::StandardButton btn = QMessageBox::question(
        0,						//父窗口
        "视频文件已经存在",		//标题栏
        "视频文件已经存在。\n"
        "选择yes，系统将覆盖当前文件。\n"
        "选择no，您可以重新选择保存位置");		//文本内容
      if (btn == QMessageBox::Yes) {
        //先删除，在创建新的
        if (imagesFolder.exists("left")) {
          QDir tleft(Paras::getSingleton().LeftImagesFoler);
          tleft.removeRecursively();
        }
        imagesFolder.mkdir("left");
        if (imagesFolder.exists("right")) {
          QDir tright(Paras::getSingleton().RightImagesFoler);
          tright.removeRecursively();
        }
        imagesFolder.mkdir("right");
        mStatusBar->showMessage(dirName);

        QMessageBox::about(
          0,						//父窗口
          "成功覆盖当前文件",		//标题栏
          "视频文件成功被覆盖");	//文本内容
      }
      if (btn == QMessageBox::No) {
        selectSaveFolder();
      }
    }
    //不存在，直接创建
    else {
      imagesFolder.mkdir("left");
      imagesFolder.mkdir("right");
    }
  }
}
//录制视频
void RecordWindow::recordVedio() {
  //如果是第一次点击录制按钮
  if (mbRecord == false && mbStop == true) {
    //设置为录制状态
    mbStop = false;
    mStop->setEnabled(true);
    mPlay->setDisabled(true);
    mRecordFolder->setDisabled(true);
    mRecordCnt = 0;	//录制的帧数从1开始
    mRecordStart = mProTimer.getMilliseconds();

    //todo 如果有文件需不需要提示覆盖文件
    if (QFile::exists(Paras::getSingleton().ImagesFoler + "info.txt")) {
      QFile::remove(Paras::getSingleton().ImagesFoler + "info.txt");
    }

    //创建文件
    QFile file(Paras::getSingleton().ImagesFoler + "info.txt");
    file.open(QIODevice::ReadWrite);
    file.write("/left/0_img_list.txt\r\n");
    file.write("/right/0_img_list.txt\r\n");
    file.close();
  }

  //切换到正在录制状态
  if (mbRecord == false) {
    mbRecord = true;
    mRecord->setText("暂停");

    mStatusBar->showMessage("正在录制");
  }
  //切换到暂停状态
  else {
    mbRecord = false;
    mRecord->setText("录制");

    mStatusBar->showMessage("暂停录制");
  }
}

namespace
{
  void RunInFileThread(scoped_refptr<base::MessageLoopProxy> ml,
                       base::Closure cb) {
    ml->PostTask(FROM_HERE, cb);
  }
}

void RecordWindow::OnRecorderSaveDone() {
  mRecord->setText("录制");
  mRecord->setEnabled(true);
  mPlay->setEnabled(true);
  mRecordFolder->setEnabled(true);

  //提示录像已经成功保存
  QDir imagesFolder(Paras::getSingleton().ImagesFoler);
  QMessageBox::about(
    0,						//父窗口
    "视频已成功保存",		//标题栏
    "视频文件成功保存，保存位置为:\n" + imagesFolder.absolutePath());	//文本内容

  mStatusBar->showMessage("正在预览");
}

//结束录制视频
void RecordWindow::stopRecordVedio() {
  mbStop = true;

  mbRecord = false;
  mStop->setDisabled(true);
  mRecord->setDisabled(true);

  base::Closure callback = base::Bind(&RecordWindow::OnRecorderSaveDone,
                                      weak_factory_.GetWeakPtr());

  WorkingThread::PostTask(WorkingThread::FILE, FROM_HERE,
                          base::Bind(&RunInFileThread,
                                     base::MessageLoopProxy::current(),
                                     callback));
  
  //mCameras->StopRecord();
}
//关闭视频录制界面
void RecordWindow::closeRecordWindow() {
  //如果在录像，先停止录像
  if (mbStop == false) {
    stopRecordVedio();
  }

  //如果在预览，关闭预览
  if (mbPlay == true) {
    preview();
  }
}

//更新一帧场景图像
void RecordWindow::updateOneFrame() {
  if (mbRecord) return;

  static int FrameCount = 0;
  static int StartTime = mProTimer.getMilliseconds();

  CameraFrames frames;
  mCameras->GetFrames(frames);
  if (frames.size() == 2) {
    FrameCount++;
    updatePixmap(frames[0].ToQImage(), frames[1].ToQImage());

    float fps = mCameras->FrameRate();
    //更新状态栏显示
    QString show;
    show.sprintf("当前的帧率为：%f", fps);
    mStatusBar->showMessage(show);
  }

  //每20帧，统计下帧率
  //if (FrameCount == 20) {
  //  int endTime = mProTimer.getMilliseconds();
  //  float fps = (float)FrameCount * 1000 / (endTime - StartTime);

    //置0
    //FrameCount = 0;
    //StartTime = mProTimer.getMilliseconds();
  //}
}
//设置详细视图为左视图
void RecordWindow::setLeftDetailView() {
  //如果已经是左视图，直接返回
  if (mbLeftFocused) {
    return;
  }
  //否则从右视图切换
  else {
    mbLeftFocused = true;
    //保存右视图当前的状态
    mRightCameraView->saveZoomViewState();
    //切换到左视图
    mLeftCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mLeftCameraView->getSavedState());
  }
}
//设置详细视图为右视图
void RecordWindow::setRightDetailView() {
  //如果是左视图，切换到右视图
  if (mbLeftFocused) {
    mbLeftFocused = false;
    //保存左视图当前的状态
    mLeftCameraView->saveZoomViewState();
    //切换到右视图
    mRightCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mRightCameraView->getSavedState());
  } else {
    return;
  }
}

//创建窗口布局
void RecordWindow::createLayout() {
  mCenterWidget = this;

  mMainLayout = new QVBoxLayout();
  mCenterWidget->setLayout(mMainLayout);

  mVedioLayout = new QHBoxLayout();
  mTimeLineLayout = new QHBoxLayout();
  mMainLayout->addLayout(mVedioLayout);
  mMainLayout->addLayout(mTimeLineLayout);

  mStereoLayout = new QVBoxLayout();
  mDetailLayout = new QVBoxLayout();
  mVedioLayout->addLayout(mStereoLayout, 9);
  mVedioLayout->addLayout(mDetailLayout, 16);

  //设置场景视图
  //左摄像机视图
  int mWidth = Paras::getSingleton().width;
  int mHeight = Paras::getSingleton().height;
  mLeftCameraView = new MyCameraView(mScene, QRect(0, 0, mWidth, mHeight));
  //右摄像机视图
  mRightCameraView = new MyCameraView(mScene, QRect(mWidth * 2, 0, mWidth, mHeight));
  //获得焦点时，切换左右视图
  connect(mLeftCameraView, SIGNAL(getFocusSignal()), this, SLOT(setLeftDetailView()));
  connect(mRightCameraView, SIGNAL(getFocusSignal()), this, SLOT(setRightDetailView()));
  mStereoLayout->addWidget(mLeftCameraView);
  mStereoLayout->addWidget(mRightCameraView);

  //创建按钮
  mCtrlLayout = new QHBoxLayout();
  createWidget();
  mDetailLayout->addLayout(mCtrlLayout);

  //创建缩放视图，默认初始化给左视图
  mZoomView = new MyDetailView(mScene, QRect(0, 0, mWidth, mHeight));
  mLeftCameraView->setZoomView(mZoomView);
  mbLeftFocused = true;
  mDetailLayout->addWidget(mZoomView);
}

//创建按钮
void RecordWindow::createWidget() {
  mPlay = new QPushButton();
  mPlay->setText(QString::fromWCharArray(L"预览"));
  connect(mPlay, SIGNAL(pressed()), this, SLOT(preview()));

  mRecord = new QPushButton();
  mRecord->setText(QString::fromWCharArray(L"录制"));
  mRecord->setDisabled(true);
  connect(mRecord, SIGNAL(pressed()), this, SLOT(recordVedio()));

  mStop = new QPushButton();
  mStop->setText(QString::fromWCharArray(L"停止"));
  mStop->setDisabled(true);
  connect(mStop, SIGNAL(pressed()), this, SLOT(stopRecordVedio()));

  mRecordFolder = new QPushButton();
  mRecordFolder->setText(QString::fromWCharArray(L"保存位置"));
  connect(mRecordFolder, SIGNAL(pressed()), this, SLOT(selectSaveFolder()));

  mCtrlLayout->addWidget(mPlay);
  mCtrlLayout->addWidget(mRecord);
  mCtrlLayout->addWidget(mStop);
  mCtrlLayout->addWidget(mRecordFolder);
}

namespace {
  static void SaveImage(int index,
                        const CameraFrame& frame,
                        const base::FilePath& path) {
    base::FilePath p = MakeAbsoluteFilePath(path);
    base::FilePath dir = p.DirName();
    base::FilePath list_path = dir.Append(L"0_img_list.txt");
    if (!base::PathExists(list_path)) {
      base::WriteFile(list_path, "", 0);
    }

    std::string base_name = base::IntToString(index) +
      " " + base::Int64ToString(frame.time_stamp.ToInternalValue() / 1000) +
      " " + base::Int64ToString(frame.sync_stamp.ToInternalValue() / 1000) +
      "\r\n";

    base::AppendToFile(list_path, base_name.c_str(), base_name.length());

    QImage image = frame.ToQImage();
    bool s = image.save(QString::fromWCharArray(p.value().c_str()));
    LLX_INFO() << "Save image to " << p.value().c_str()
      << " " << (s ? "DONE" : "FAIL");
  }
}

void RecordWindow::RecordFrame(CameraFrames& frames) {
  if (!mbRecord || frames.size() < 2) return;

  base::FilePath dir(Paras::getSingleton().ImagesFoler.toStdWString());
  ++mRecordCnt;
  CameraFrames::iterator it = frames.begin();
  while(it != frames.end()) {
    base::FilePath d = dir;
    if (it->first == 1)
      d = dir.Append(L"right");
    else if (it->first == 0)
      d = dir.Append(L"left");
    else continue;

    std::wstring filename = base::UintToString16(mRecordCnt) + L"_"
        + base::Int64ToString16(it->second.time_stamp.ToInternalValue() / 1000)
        + L".jpg";

    d = d.Append(filename);

    WorkingThread::PostTask(LLX::WorkingThread::FILE, FROM_HERE,
                            base::Bind(&SaveImage, mRecordCnt, it->second, d));

    ++it;
  }
}