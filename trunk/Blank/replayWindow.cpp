#include "replayWindow.h"
#include <QtWidgets\qapplication.h>
#include <QtWidgets\qmainwindow.h>
#include <QtWidgets\qwidget.h>
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
#include <QtWidgets\qslider.h>

#include <QtCore\qtimer.h>
#include <QtCore\qtextstream.h>
#include <QtCore\qtextcodec.h>

#include <QtGui\qtransform.h>
#include <QtGui\qevent.h>
#include <QtGui\qpainter.h>

#include "UsbCameras.h"
#include "myCameraView.h"
#include "marker.h"
#include "paras.h"

ReplayWindow::ReplayWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left, QGraphicsPixmapItem* right, QStatusBar* status) {
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;

  //��������
  createLayout();

  //����Ĭ�ϳ���
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
//������ϸ��ͼΪ����ͼ
void ReplayWindow::setLeftDetailView() {
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
void ReplayWindow::setRightDetailView() {
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
//�µ�λ��
void ReplayWindow::updateImage(int value) {
  //����ʱ��
  int length = (mEndTime - mStartTime) / 1000;
  int hour = length / 3600;
  int minute = (length - hour * 3600) / 60;
  int second = length - hour * 3600 - minute * 60;
  int ct = mAllTimes[value - mStartIndex];
  int cl = (ct - mStartTime) / 1000;
  int hl = cl / 3600;
  int ml = (cl - hl * 3600) / 60;
  int sl = cl - hl * 3600 - ml * 60;
  QString tt;
  tt.sprintf("%d:%d:%d/%d:%d:%d", hl, ml, sl, hour, minute, second);
  mTime->setText(tt);

  //����ͼ��
  loadImage(value);
}
//������Ƶ
void ReplayWindow::playVedio() {
  next();
}

//����Ƶ�ļ�
void ReplayWindow::open() {
  //��ȡһ���ļ�
  QString selectFile = QFileDialog::getOpenFileName(
    0,						//������
    "����Ƶ�ļ�",			//������
    "",						//Ĭ��Ϊ��ǰ�ļ���
    "˫Ŀ��Ƶ (*.txt)");	//������

  //�ɹ���һ���ļ������г�ʼ������
  if (selectFile != "") {
    //todo ��֤�ļ��ĵ���ȷ��

    mAllIndex.clear();
    mAllTimes.clear();

    //��ȡ�ļ�����
    QFile file(selectFile);
    file.open(QIODevice::ReadWrite);
    QTextStream ts(&file);
    int i1, i2;
    ts >> i1 >> i2;
    while (!ts.atEnd()) {
      mAllIndex.push_back(i1);
      mAllTimes.push_back(i2);
      ts >> i1 >> i2;
    }
    QFileInfo tfi(file);
    mImagesFoler = tfi.absolutePath();
    mLeftImagesFoler = mImagesFoler + "/left/";
    mRightImagesFoler = mImagesFoler + "/right/";
    file.close();

    //������ز���
    mStartIndex = mAllIndex.front();
    mEndIndex = mAllIndex.back();
    mStartTime = mAllTimes.front();
    mEndTime = mAllTimes.back();

    mbOpen = true;
    mbPlay = false;

    //�����һ֡��
    int length = (mEndTime - mStartTime) / 1000;	//ת��Ϊ��
    int hour = length / 3600;
    int minute = (length - hour * 3600) / 60;
    int second = length - hour * 3600 - minute * 60;
    QString tt;
    tt.sprintf("0:0:0/%d:%d:%d", hour, minute, second);
    mTime->setText(tt);
    mTime->setEnabled(true);
    mPlay->setEnabled(true);
    mNext->setEnabled(true);
    mBefore->setEnabled(true);
    mSlider->setMinimum(mStartIndex);
    mSlider->setMaximum(mEndIndex);
    mSlider->setValue(mStartIndex);
    mSlider->setTickInterval((mEndIndex - mStartIndex) / 10);
    mSlider->setEnabled(true);
    loadImage(mStartIndex);

    //todo �Ƿ񻺴�ͼ��
  }
}
//������Ƶ
void ReplayWindow::play() {
  //�����ͣ���л�������
  if (mbPlay == false) {
    mbPlay = true;
    mPlay->setText("��ͣ");
    mNext->setDisabled(true);
    mBefore->setDisabled(true);

    //���ö�ʱ����
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(playVedio()));
    mTimer->start(1);
  }
  //������ţ��л�����ͣ
  else {
    mbPlay = false;
    disconnect(mTimer, SIGNAL(timeout()), this, SLOT(playVedio()));
    mTimer->stop();
    delete mTimer;
    mPlay->setText("����");
    mNext->setEnabled(true);
    mBefore->setEnabled(true);
  }
}
//��һ֡
void ReplayWindow::next() {
  int cv = mSlider->value();
  cv = cv + 1;
  if (cv > mEndIndex)
    cv = mStartIndex;
  mSlider->setValue(cv);
}
//��һ֡
void ReplayWindow::before() {
  int cv = mSlider->value();
  cv = cv - 1;
  if (cv < mStartIndex)
    cv = mEndIndex;
  mSlider->setValue(cv);
}
//����������ָ��ͼ��
void ReplayWindow::loadImage(int i) {
  QImage li, ri;
  QString name;
  name.sprintf("%d.jpg", i);
  li.load(mLeftImagesFoler + name);
  ri.load(mRightImagesFoler + name);
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));

  //����ͼ��
  update();
}

//�������ڲ���
void ReplayWindow::createLayout() {
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
  mDetailLayout->addLayout(mCtrlLayout);

  //����������ͼ��Ĭ�ϳ�ʼ��������ͼ
  mZoomView = new MyDetailView(mScene, QRect(0, 0, mWidth, mHeight));
  mLeftCameraView->setZoomView(mZoomView);
  mbLeftFocused = true;
  mDetailLayout->addWidget(mZoomView);

  //����һЩ���ܰ�ť������Ĳ��ִ��뼸�����ö���
  createWidget();
}

//������ť
void ReplayWindow::createWidget() {
  mbOpen = false;
  mLeftImagesFoler = "";
  mRightImagesFoler = "";
  mImagesFoler = "";

  mTime = new QLabel();
  mTime->setText("00:00:00/00:00:00");
  mTime->setDisabled(true);

  mPlay = new QPushButton();
  mPlay->setText("����");
  mPlay->setDisabled(true);
  connect(mPlay, SIGNAL(pressed()), this, SLOT(play()));

  mNext = new QPushButton();
  mNext->setText("��һ֡");
  mNext->setDisabled(true);
  connect(mNext, SIGNAL(pressed()), this, SLOT(next()));

  mBefore = new QPushButton();
  mBefore->setText("��һ֡");
  mBefore->setDisabled(true);
  connect(mBefore, SIGNAL(pressed()), this, SLOT(before()));

  mOpen = new QPushButton();
  mOpen->setText("���ļ�");
  connect(mOpen, SIGNAL(pressed()), this, SLOT(open()));

  mSlider = new QSlider(Qt::Horizontal);
  mSlider->setTickPosition(QSlider::TicksBelow);
  mSlider->setMinimum(1);
  mSlider->setMaximum(3);
  mSlider->setValue(2);
  mSlider->setTracking(true);
  mSlider->setDisabled(true);
  connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateImage(int)));

  mCtrlLayout->addWidget(mTime);
  mCtrlLayout->addWidget(mPlay);
  mCtrlLayout->addWidget(mNext);
  mCtrlLayout->addWidget(mBefore);
  mCtrlLayout->addWidget(mOpen);

  mTimeLineLayout->addWidget(mSlider);
}