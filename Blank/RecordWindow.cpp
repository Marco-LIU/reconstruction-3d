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

#include "usb_camera_group.h"
#include "myCameraView.h"
#include "marker.h"
#include "paras.h"

RecordWindow::RecordWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
                           QGraphicsPixmapItem* right, QStatusBar* status) {
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
void RecordWindow::updatePixmap(const unsigned char* leftBuffer, const unsigned char* rightBuffer) {
  QImage li = convertToQImage(leftBuffer);
  QImage ri = convertToQImage(rightBuffer);

  if (mbRecord) {
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
  }

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
    mCameras->Init("para.config");

    //�������ʧ�ܣ���ʾ�����û�����Ӻ�
    if (mCameras->camera_count() != 2) {
      delete mCameras;
      mCameras = NULL;
      mPlay->setText("ֹͣ");
      QMessageBox::critical(
        0,							//������
        "�Ҳ������õ�����ͷ",		//������
        "�Ҳ������õ�����ͷ����鿴����ͷ�Ƿ��Ѿ����ӵ����ԣ����Ѿ����ӣ������²��USB�ӿ�");		//�ı�����
    }
    //�ɹ�����
    else {

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
      delete mCameras;
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
        "��Ƶ�ļ��Ѿ����ڡ�\nѡ��yes��ϵͳ�����ǵ�ǰ�ļ���\nѡ��no������������ѡ�񱣴�λ��");		//�ı�����
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
//����¼����Ƶ
void RecordWindow::stopRecordVedio() {
  mbStop = true;

  mbRecord = false;
  mRecord->setText("¼��");
  mStop->setDisabled(true);
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
  static int FrameCount = 0;
  static int StartTime = mProTimer.getMilliseconds();

  if (mCameras->CaptureFrame(true)) {
    FrameCount++;

    updatePixmap(mCameras->camera_buffer(0), mCameras->camera_buffer(1));
  }

  //ÿ20֡��ͳ����֡��
  if (FrameCount == 20) {
    int endTime = mProTimer.getMilliseconds();
    float fps = (float)FrameCount * 1000 / (endTime - StartTime);

    //����״̬����ʾ
    QString show;
    show.sprintf("��ǰ��֡��Ϊ��%f", fps);
    mStatusBar->showMessage(show);

    //��0
    FrameCount = 0;
    StartTime = mProTimer.getMilliseconds();
  }
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