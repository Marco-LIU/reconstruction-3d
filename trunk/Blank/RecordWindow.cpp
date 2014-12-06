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

  mCameras = NULL;	//����ͷΪ��
  mbPlay = false;		//true��ʾ����Ԥ��
  mbRecord = false;	//true��ʾ����¼�ƣ�false��ʾ��ͣ¼��
  mbStop = true;		//true��ʾ�Ѿ�ֹͣ��¼��

  //��������
  createLayout();

  //����Ĭ�ϳ���
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}

RecordWindow::~RecordWindow() {
  closeRecordWindow();
}

//�ѻҶ�����ת��ΪQImage��ʽ
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
//���³���ͼ��
void RecordWindow::updatePixmap(const unsigned char* leftBuffer,
                                const unsigned char* rightBuffer) {
  QImage li = convertToQImage(leftBuffer);
  QImage ri = convertToQImage(rightBuffer);

  /*if (mbRecord) {
    //���¼��
    mRecordCnt++;

    //��������ͼ��
    QString name;
    name.sprintf("%d.jpg", mRecordCnt);
    li.save(Paras::getSingleton().LeftImagesFoler + name);
    ri.save(Paras::getSingleton().RightImagesFoler + name);
    //��¼�ļ���Ϣ����ӵķ�ʽ
    QFile file(Paras::getSingleton().ImagesFoler + "info.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);
    ts << mRecordCnt << " " << mProTimer.getMilliseconds() - mRecordStart << "\r\n";
    ts.flush();
    file.close();

    //��ʾ��Ϣ
    mStatusBar->showMessage("ͼ��" + name + "�������");
  }*/

  updatePixmap(li, ri);
}

void RecordWindow::updatePixmap(QImage& li, QImage& ri) {
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));
  //����ͼ��
  update();
}

//1¼����Ƶ��4��ť�Ĵ�����
//Ԥ����Ƶ����Ļ���
void RecordWindow::preview() {
  //���û����������ͷ������������ʼԤ��
  if (mbPlay == false) {
    mCameras = new UsbCameraGroup();
    bool succ = mCameras->Init("para.json");

    //�������ʧ�ܣ���ʾ�����û�����Ӻ�
    if (!succ || mCameras->camera_count() != 2) {
      mCameras = NULL;
      mPlay->setText("ֹͣ");
      QMessageBox::critical(
        0,							//������
        "�Ҳ������õ�����ͷ",		//������
        "�Ҳ������õ�����ͷ����鿴����ͷ�Ƿ��Ѿ����ӵ����ԣ�"
        "���Ѿ����ӣ������²��USB�ӿ�");		//�ı�����
    }
    //�ɹ�����
    else {
      mCameras->StartAll();

      mCameras->SetFrameCallback(base::Bind(&RecordWindow::RecordFrame,
                                            base::Unretained(this)));
      //�л�������ť״̬
      mbPlay = true;
      mRecord->setEnabled(true);
      mPlay->setText("ֹͣ");

      //���ö�ʱ����
      mTimer = new QTimer(this);
      connect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->start(10);
    }
  }
  //����ɹ���������ͷ����ر���
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
      mPlay->setText("Ԥ��");

      mRecord->setDisabled(true);
    }
  }
}
//ѡ�񱣴�λ��
void RecordWindow::selectSaveFolder() {
  //ѡ��һ���ļ���
  QString dirName = QFileDialog::getExistingDirectory(
    this,							//������
    "��ѡ�񱣴���Ƶ�ļ����ļ���",	//������
    "");							//Ĭ��Ϊ��ǰ�ļ���

  //���ѡ����һ���ļ���
  if (!dirName.isEmpty()) {
    Paras::getSingleton().ImagesFoler = dirName;
    Paras::getSingleton().LeftImagesFoler = dirName + "/left/";
    Paras::getSingleton().RightImagesFoler = dirName + "/right/";

    //����ļ��д��ڣ�ѯ��ɾ��������ѡ���ĵط�
    QDir imagesFolder(dirName);
    if (imagesFolder.exists("left") || imagesFolder.exists("right")) {
      //question�Ի���
      QMessageBox::StandardButton btn = QMessageBox::question(
        0,						//������
        "��Ƶ�ļ��Ѿ�����",		//������
        "��Ƶ�ļ��Ѿ����ڡ�\n"
        "ѡ��yes��ϵͳ�����ǵ�ǰ�ļ���\n"
        "ѡ��no������������ѡ�񱣴�λ��");		//�ı�����
      if (btn == QMessageBox::Yes) {
        //��ɾ�����ڴ����µ�
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
          0,						//������
          "�ɹ����ǵ�ǰ�ļ�",		//������
          "��Ƶ�ļ��ɹ�������");	//�ı�����
      }
      if (btn == QMessageBox::No) {
        selectSaveFolder();
      }
    }
    //�����ڣ�ֱ�Ӵ���
    else {
      imagesFolder.mkdir("left");
      imagesFolder.mkdir("right");
    }
  }
}
//¼����Ƶ
void RecordWindow::recordVedio() {
  //����ǵ�һ�ε��¼�ư�ť
  if (mbRecord == false && mbStop == true) {
    //����Ϊ¼��״̬
    mbStop = false;
    mStop->setEnabled(true);
    mPlay->setDisabled(true);
    mRecordFolder->setDisabled(true);
    mRecordCnt = 0;	//¼�Ƶ�֡����1��ʼ
    mRecordStart = mProTimer.getMilliseconds();

    //todo ������ļ��費��Ҫ��ʾ�����ļ�
    if (QFile::exists(Paras::getSingleton().ImagesFoler + "info.txt")) {
      QFile::remove(Paras::getSingleton().ImagesFoler + "info.txt");
    }

    //�����ļ�
    QFile file(Paras::getSingleton().ImagesFoler + "info.txt");
    file.open(QIODevice::ReadWrite);
    file.write("/left/0_img_list.txt\r\n");
    file.write("/right/0_img_list.txt\r\n");
    file.close();
  }

  //�л�������¼��״̬
  if (mbRecord == false) {
    mbRecord = true;
    mRecord->setText("��ͣ");

    mStatusBar->showMessage("����¼��");
  }
  //�л�����ͣ״̬
  else {
    mbRecord = false;
    mRecord->setText("¼��");

    mStatusBar->showMessage("��ͣ¼��");
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
  mRecord->setText("¼��");
  mRecord->setEnabled(true);
  mPlay->setEnabled(true);
  mRecordFolder->setEnabled(true);

  //��ʾ¼���Ѿ��ɹ�����
  QDir imagesFolder(Paras::getSingleton().ImagesFoler);
  QMessageBox::about(
    0,						//������
    "��Ƶ�ѳɹ�����",		//������
    "��Ƶ�ļ��ɹ����棬����λ��Ϊ:\n" + imagesFolder.absolutePath());	//�ı�����

  mStatusBar->showMessage("����Ԥ��");
}

//����¼����Ƶ
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
//�ر���Ƶ¼�ƽ���
void RecordWindow::closeRecordWindow() {
  //�����¼����ֹͣ¼��
  if (mbStop == false) {
    stopRecordVedio();
  }

  //�����Ԥ�����ر�Ԥ��
  if (mbPlay == true) {
    preview();
  }
}

//����һ֡����ͼ��
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
    //����״̬����ʾ
    QString show;
    show.sprintf("��ǰ��֡��Ϊ��%f", fps);
    mStatusBar->showMessage(show);
  }

  //ÿ20֡��ͳ����֡��
  //if (FrameCount == 20) {
  //  int endTime = mProTimer.getMilliseconds();
  //  float fps = (float)FrameCount * 1000 / (endTime - StartTime);

    //��0
    //FrameCount = 0;
    //StartTime = mProTimer.getMilliseconds();
  //}
}
//������ϸ��ͼΪ����ͼ
void RecordWindow::setLeftDetailView() {
  //����Ѿ�������ͼ��ֱ�ӷ���
  if (mbLeftFocused) {
    return;
  }
  //���������ͼ�л�
  else {
    mbLeftFocused = true;
    //��������ͼ��ǰ��״̬
    mRightCameraView->saveZoomViewState();
    //�л�������ͼ
    mLeftCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mLeftCameraView->getSavedState());
  }
}
//������ϸ��ͼΪ����ͼ
void RecordWindow::setRightDetailView() {
  //���������ͼ���л�������ͼ
  if (mbLeftFocused) {
    mbLeftFocused = false;
    //��������ͼ��ǰ��״̬
    mLeftCameraView->saveZoomViewState();
    //�л�������ͼ
    mRightCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mRightCameraView->getSavedState());
  } else {
    return;
  }
}

//�������ڲ���
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

  //���ó�����ͼ
  //���������ͼ
  int mWidth = Paras::getSingleton().width;
  int mHeight = Paras::getSingleton().height;
  mLeftCameraView = new MyCameraView(mScene, QRect(0, 0, mWidth, mHeight));
  //���������ͼ
  mRightCameraView = new MyCameraView(mScene, QRect(mWidth * 2, 0, mWidth, mHeight));
  //��ý���ʱ���л�������ͼ
  connect(mLeftCameraView, SIGNAL(getFocusSignal()), this, SLOT(setLeftDetailView()));
  connect(mRightCameraView, SIGNAL(getFocusSignal()), this, SLOT(setRightDetailView()));
  mStereoLayout->addWidget(mLeftCameraView);
  mStereoLayout->addWidget(mRightCameraView);

  //������ť
  mCtrlLayout = new QHBoxLayout();
  createWidget();
  mDetailLayout->addLayout(mCtrlLayout);

  //����������ͼ��Ĭ�ϳ�ʼ��������ͼ
  mZoomView = new MyDetailView(mScene, QRect(0, 0, mWidth, mHeight));
  mLeftCameraView->setZoomView(mZoomView);
  mbLeftFocused = true;
  mDetailLayout->addWidget(mZoomView);
}

//������ť
void RecordWindow::createWidget() {
  mPlay = new QPushButton();
  mPlay->setText(QString::fromWCharArray(L"Ԥ��"));
  connect(mPlay, SIGNAL(pressed()), this, SLOT(preview()));

  mRecord = new QPushButton();
  mRecord->setText(QString::fromWCharArray(L"¼��"));
  mRecord->setDisabled(true);
  connect(mRecord, SIGNAL(pressed()), this, SLOT(recordVedio()));

  mStop = new QPushButton();
  mStop->setText(QString::fromWCharArray(L"ֹͣ"));
  mStop->setDisabled(true);
  connect(mStop, SIGNAL(pressed()), this, SLOT(stopRecordVedio()));

  mRecordFolder = new QPushButton();
  mRecordFolder->setText(QString::fromWCharArray(L"����λ��"));
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