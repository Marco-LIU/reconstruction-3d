#include "mainwindow.h"
#include "recordWindow.h"
#include "replayWindow.h"
#include "calibrateWindow.h"
#include "stereoCalWindow.h"
#include "manualWindow.h"
#include "MeasureMarkersWindows.h"

#include "QtWidgets/qgraphicsitem.h"
#include "QtWidgets/qaction.h"
#include "QtWidgets/qstatusbar.h"
#include "QtWidgets/qgraphicsscene.h"
#include "QtWidgets/qmenu.h"
#include "QtWidgets/qmenubar.h"

#include "paras.h"

#include "base/message_loop/message_loop.h"
#include "qt_utils.h"

MainWindow::MainWindow()
    : mCurrentWidget(NULL)
    , mRecordWindow(NULL)
    , mReplayWindow(NULL)
    , mLeftCalibrateWindow(NULL)
    , mRightCalibrateWindow(NULL)
    , mStereoWindow(NULL)
    , mManualWindow(NULL) {
  //创建场景
  createScene();

  //初始化参数
  iniParas();

  //创建状态栏
  createStatusBar();

  //创建动作
  createAction();

  //创建菜单
  createMenu();

  //设置初始显示的菜单
  showRecordWindow();
}

MainWindow::~MainWindow() {
  //to fix, 场景removeItem时，有点问题
}

void MainWindow::showRecordWindow() {
  if (mCurrentWidget && mCurrentWidget == mRecordWindow)
    return;

  mRecordWindow = new RecordWindow(&mScene, mLeftCameraPixmap,
                                   mRightCameraPixmap, mStatusBar);
  mCurrentWidget = mRecordWindow;
  this->setCentralWidget(mRecordWindow);
}

void MainWindow::showReplayWindow() {
  if (mCurrentWidget && mCurrentWidget == mReplayWindow)
    return;

  mReplayWindow = new ReplayWindow(&mScene, mLeftCameraPixmap,
                                   mRightCameraPixmap, mStatusBar);
  mCurrentWidget = mReplayWindow;
  this->setCentralWidget(mReplayWindow);
}

void MainWindow::showCalibrateLeft() {
  if (mCurrentWidget && mCurrentWidget == mLeftCalibrateWindow)
    return;

  mLeftCalibrateWindow = new CalibrateWindow(&mScene, mLeftCameraPixmap,
                                             mRightCameraPixmap, mStatusBar,
                                             "./leftCamera/", true);
  mCurrentWidget = mLeftCalibrateWindow;
  this->setCentralWidget(mLeftCalibrateWindow);
}

void MainWindow::showCalibrateRight() {
  if (mCurrentWidget && mCurrentWidget == mRightCalibrateWindow)
    return;

  mRightCalibrateWindow = new CalibrateWindow(&mScene, mLeftCameraPixmap,
                                              mRightCameraPixmap, mStatusBar,
                                              "./rightCamera/", false);
  mCurrentWidget = mRightCalibrateWindow;
  this->setCentralWidget(mRightCalibrateWindow);
}

void MainWindow::showCalibrateRT() {
  if (mCurrentWidget && mCurrentWidget == mStereoWindow)
    return;
  mStereoWindow = new StereoCalibrationWindow(&mScene, mLeftCameraPixmap,
                                              mRightCameraPixmap, mStatusBar);
  mCurrentWidget = mStereoWindow;
  this->setCentralWidget(mStereoWindow);
}

void MainWindow::showManualWindow() {
  if (mCurrentWidget && mCurrentWidget == mManualWindow)
    return;

  mManualWindow = new ManualWindow(&mScene, mLeftCameraPixmap,
                                 mRightCameraPixmap, mStatusBar);
  mCurrentWidget = mManualWindow;
  this->setCentralWidget(mManualWindow);
}

void MainWindow::showAutoMeasureWindow() {
  if (mCurrentWidget && mCurrentWidget == mMeasureWindow)
    return;

  //mManualWindow = new ManualWindow(&mScene, mLeftCameraPixmap,
  //                                 mRightCameraPixmap, mStatusBar);
  //this->setCentralWidget(mManualWindow);
  mMeasureWindow = new MeasureMarkersWindow(&mScene, mLeftCameraPixmap,
                                            mRightCameraPixmap, mStatusBar);
  mCurrentWidget = mMeasureWindow;
  this->setCentralWidget(mMeasureWindow);
}

void MainWindow::iniParas() {
  //设置窗口，初始位置和最小大小
  setGeometry(50, 50, 800, 600);
  setMinimumHeight(480);
  setMinimumWidth(640);
}

void MainWindow::createAction() {
  //录播
  maRecord = new QAction(QString::fromWCharArray(L"录像"), this);
  connect(maRecord, SIGNAL(triggered()), this, SLOT(showRecordWindow()));
  maPlay = new QAction(QString::fromWCharArray(L"播放"), this);

  //标定
  connect(maPlay, SIGNAL(triggered()), this, SLOT(showReplayWindow()));
  maLeftFXY = new QAction(QString::fromWCharArray(L"标定左摄像机"), this);
  connect(maLeftFXY, SIGNAL(triggered()), this, SLOT(showCalibrateLeft()));
  maRightFXY = new QAction(QString::fromWCharArray(L"标定右摄像机"), this);
  connect(maRightFXY, SIGNAL(triggered()), this, SLOT(showCalibrateRight()));
  maRT = new QAction(QString::fromWCharArray(L"标定外参"), this);
  connect(maRT, SIGNAL(triggered()), this, SLOT(showCalibrateRT()));
  maDefaultRT = new QAction(QString::fromWCharArray(L"重置为默认"), this);

  maManual = new QAction(QString::fromWCharArray(L"手动测量"), this);
  connect(maManual, SIGNAL(triggered()), this, SLOT(showManualWindow()));
  maMeasure = new QAction(QString::fromWCharArray(L"自动测量"), this);
  connect(maMeasure, SIGNAL(triggered()), this, SLOT(showAutoMeasureWindow()));

  //跟踪
  maDefaultSpec = new QAction(QString::fromWCharArray(L"设置跟踪点"), this);
  maTrack = new QAction(QString::fromWCharArray(L"跟踪"), this);
  ma3DShow = new QAction(QString::fromWCharArray(L"二维显示"), this);
  ma2DShow = new QAction(QString::fromWCharArray(L"三维显示"), this);
  
  //todo
  maTodo = new QAction(QString::fromWCharArray(L"todo"), this);
}

void MainWindow::createMenu() {
  QMenu* fileMenu = menuBar()->addMenu(QString::fromWCharArray(L"文件"));
  QMenu* recPlayMenu = menuBar()->addMenu(QString::fromWCharArray(L"录播"));
  QMenu* calMenu = menuBar()->addMenu(QString::fromWCharArray(L"标定"));
  QMenu* trackMenu = menuBar()->addMenu(QString::fromWCharArray(L"跟踪"));
  QMenu* helpMenu = menuBar()->addMenu(QString::fromWCharArray(L"帮助"));

  //todo
  fileMenu->addAction(maTodo);
  helpMenu->addAction(maTodo);

  recPlayMenu->addAction(maRecord);
  recPlayMenu->addAction(maPlay);

  calMenu->addAction(maLeftFXY);
  calMenu->addAction(maRightFXY);
  calMenu->addAction(maRT);
  calMenu->addAction(maDefaultRT);
  calMenu->addAction(maManual);
  calMenu->addAction(maMeasure);

  trackMenu->addAction(maDefaultSpec);
  trackMenu->addAction(maTrack);
  trackMenu->addAction(ma3DShow);
  trackMenu->addAction(ma2DShow);
}

void MainWindow::createStatusBar() {
  mStatusBar = statusBar();
  mStatusBar->showMessage("ICT inside", 10);
}

void MainWindow::createScene() {
  int mWidth = Paras::getSingleton().width;
  int mHeight = Paras::getSingleton().height;
  //创建场景
  mScene.setSceneRect(0, 0, mWidth * 3, mHeight);

  //在场景中添加2个图像
  mLeftCameraPixmap = new QGraphicsPixmapItem();
  mLeftCameraPixmap->setPos(0, 0);
  mRightCameraPixmap = new QGraphicsPixmapItem();
  mRightCameraPixmap->setPos(mWidth * 2, 0);
  mScene.addItem(mLeftCameraPixmap);
  mScene.addItem(mRightCameraPixmap);
}

void MainWindow::closeEvent(QCloseEvent* event) {
  QMainWindow::closeEvent(event);
  if (base::MessageLoop::current())
    base::MessageLoop::current()->Quit();
}