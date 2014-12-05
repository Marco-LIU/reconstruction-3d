#include "trackingWindow.h"
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

TrackingWindow::TrackingWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left, QGraphicsPixmapItem* right, QStatusBar* status) 
{
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;

  //创建布局
  createLayout();

  //设置默认场景
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
//设置详细视图为左视图
void TrackingWindow::setLeftDetailView() {
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
void TrackingWindow::setRightDetailView() {
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
//新的位置
void TrackingWindow::updateImage(int value) {
  //更新时间
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

  //载入图像
  loadImage(value);
}
//播放视频
void TrackingWindow::playVedio() {
  next();
}

//打开视频文件
void TrackingWindow::open() {
  //读取一个文件
  QString selectFile = QFileDialog::getOpenFileName(
    0,						//父窗口
    "打开视频文件",			//标题栏
    "",						//默认为当前文件夹
    "双目视频 (*.txt)");	//过滤器

  //成功打开一个文件，进行初始化操作
  if (selectFile != "") {
    //todo 验证文件的的正确性

    mAllIndex.clear();
    mAllTimes.clear();

    //读取文件内容
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

    //设置相关参数
    mStartIndex = mAllIndex.front();
    mEndIndex = mAllIndex.back();
    mStartTime = mAllTimes.front();
    mEndTime = mAllTimes.back();

    mbOpen = true;
    mbPlay = false;

    //载入第一帧率
    int length = (mEndTime - mStartTime) / 1000;	//转换为秒
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

    //todo 是否缓存图像
  }
}
//播放视频
void TrackingWindow::play() {
  //如果暂停，切换到播放
  if (mbPlay == false) {
    mbPlay = true;
    mPlay->setText("暂停");
    mNext->setDisabled(true);
    mBefore->setDisabled(true);

    //设置定时触发
    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(playVedio()));
    mTimer->start(1);
  }
  //如果播放，切换到暂停
  else {
    mbPlay = false;
    disconnect(mTimer, SIGNAL(timeout()), this, SLOT(playVedio()));
    mTimer->stop();
    delete mTimer;
    mPlay->setText("播放");
    mNext->setEnabled(true);
    mBefore->setEnabled(true);
  }
}
//上一帧
void TrackingWindow::next() {
  int cv = mSlider->value();
  cv = cv + 1;
  if (cv > mEndIndex)
    cv = mStartIndex;
  mSlider->setValue(cv);
}
//下一帧
void TrackingWindow::before() {
  int cv = mSlider->value();
  cv = cv - 1;
  if (cv < mStartIndex)
    cv = mEndIndex;
  mSlider->setValue(cv);
}
//载入索引所指的图像
void TrackingWindow::loadImage(int i) {
  QImage li, ri;
  QString name;
  name.sprintf("%d.jpg", i);
  li.load(mLeftImagesFoler + name);
  ri.load(mRightImagesFoler + name);
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));

  //更新图像
  update();
}

//创建窗口布局
void TrackingWindow::createLayout() {
  mCenterWidget = this;

  mMainLayout = new QVBoxLayout();
  mCenterWidget->setLayout(mMainLayout);

  mVedioLayout = new QHBoxLayout();
  mTimeLineLayout = new QHBoxLayout();
  mMainLayout->addLayout(mVedioLayout);
  mMainLayout->addLayout(mTimeLineLayout);

  mStereoLayout = new QVBoxLayout();
  mDetailLayout = new QVBoxLayout();
  mTrackerListLayout = new QVBoxLayout();
  mVedioLayout->addLayout(mStereoLayout, 7);
  mVedioLayout->addLayout(mDetailLayout, 13);
  mVedioLayout->addLayout(mTrackerListLayout,5);

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
  mCtrlLayout2 = new QHBoxLayout();
  mDetailLayout->addLayout(mCtrlLayout);
  mDetailLayout->addLayout(mCtrlLayout2);

  //创建缩放视图，默认初始化给左视图
  mZoomView = new MyDetailView(mScene, QRect(0, 0, mWidth, mHeight));
  mLeftCameraView->setZoomView(mZoomView);
  mbLeftFocused = true;
  mDetailLayout->addWidget(mZoomView);

  //添加一些功能按钮（上面的布局代码几乎不用动）
  createWidget();
}

//创建按钮
void TrackingWindow::createWidget() {
  mbOpen = false;
  mLeftImagesFoler = "";
  mRightImagesFoler = "";
  mImagesFoler = "";

  mTime = new QLabel();
  mTime->setText(QString::fromWCharArray(L"00:00:00/00:00:00"));
  mTime->setDisabled(true);

  mPlay = new QPushButton();
  mPlay->setText(QString::fromWCharArray(L"播放"));
  mPlay->setDisabled(true);
  connect(mPlay, SIGNAL(pressed()), this, SLOT(play()));

  mNext = new QPushButton();
  mNext->setText(QString::fromWCharArray(L"下一帧"));
  mNext->setDisabled(true);
  connect(mNext, SIGNAL(pressed()), this, SLOT(next()));

  mBefore = new QPushButton();
  mBefore->setText(QString::fromWCharArray(L"上一帧"));
  mBefore->setDisabled(true);
  connect(mBefore, SIGNAL(pressed()), this, SLOT(before()));

  mOpen = new QPushButton();
  mOpen->setText(QString::fromWCharArray(L"打开文件"));
  connect(mOpen, SIGNAL(pressed()), this, SLOT(open()));

  mReset = new QPushButton();
  mReset->setText(QString::fromWCharArray(L"重新载入"));
  connect(mReset, SIGNAL(pressed()), this, SLOT(reset()));

  mSetBegin = new QPushButton();
  mSetBegin->setText(QString::fromWCharArray(L"设置为起始点"));
  connect(mSetBegin, SIGNAL(pressed()), this, SLOT(setBegin()));

  mSetEnd = new QPushButton();
  mSetEnd->setText(QString::fromWCharArray(L"设置为结束点"));
  connect(mSetEnd, SIGNAL(pressed()), this, SLOT(setEnd()));

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

  mCtrlLayout2->addWidget(mReset);
  mCtrlLayout2->addWidget(mSetBegin);
  mCtrlLayout2->addWidget(mSetEnd);
  mCtrlLayout2->addWidget(mOpen);

  mTimeLineLayout->addWidget(mSlider);

  mTrackPointLable = new QLabel();
  mTrackPointLable->setText(QString::fromWCharArray(L"跟踪点"));
  mTrackLineLable = new QLabel();
  mTrackLineLable->setText(QString::fromWCharArray(L"跟踪线"));
  mTrackPointList = new QListWidget();
  mTrackLineList = new QListWidget();
  mTrackerListLayout->addWidget(mTrackPointLable);
  mTrackerListLayout->addWidget(mTrackPointList);
  mTrackerListLayout->addWidget(mTrackLineLable);
  mTrackerListLayout->addWidget(mTrackLineList);
}