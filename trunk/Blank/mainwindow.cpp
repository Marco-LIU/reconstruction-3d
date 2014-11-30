﻿#include "mainwindow.h"
#include "recordWindow.h"
#include "replayWindow.h"
#include "calibrateWindow.h"
#include "stereoCalWindow.h"
#include "manualWindow.h"
#include "measure_markers.h"

#include "QtWidgets/qgraphicsitem.h"
#include "QtWidgets/qaction.h"
#include "QtWidgets/qstatusbar.h"
#include "QtWidgets/qgraphicsscene.h"
#include "QtWidgets/qmenu.h"
#include "QtWidgets/qmenubar.h"

#include "paras.h"

MainWindow::MainWindow()
  : maCurFun(0)
  , mRecordWindow(NULL)
  , mReplayWindow(NULL)
  , mCalibrateWindow(NULL)
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
  if (maCurFun == 1)
    return;

  maCurFun = 1;
  mRecordWindow = new RecordWindow(&mScene, mLeftCameraPixmap,
                                   mRightCameraPixmap, mStatusBar);
  this->setCentralWidget(mRecordWindow);
}

void MainWindow::showReplayWindow() {
  if (maCurFun == 2)
    return;

  maCurFun = 2;
  mReplayWindow = new ReplayWindow(&mScene, mLeftCameraPixmap,
                                   mRightCameraPixmap, mStatusBar);
  this->setCentralWidget(mReplayWindow);
}

void MainWindow::showCalibrateLeft() {
  if (maCurFun == 3)
    return;

  maCurFun = 3;

  mCalibrateWindow = new CalibrateWindow(&mScene, mLeftCameraPixmap,
                                         mRightCameraPixmap, mStatusBar,
                                         "./leftCamera/", true);
  this->setCentralWidget(mCalibrateWindow);
}

void MainWindow::showCalibrateRight() {
  if (maCurFun == 4)
    return;

  maCurFun = 4;

  mCalibrateWindow = new CalibrateWindow(&mScene, mLeftCameraPixmap,
                                         mRightCameraPixmap, mStatusBar,
                                         "./rightCamera/", false);
  this->setCentralWidget(mCalibrateWindow);
}

void MainWindow::showCalibrateRT() {
  if (maCurFun == 5)
    return;

  maCurFun = 5;

  mStereoWindow = new StereoCalibrationWindow(&mScene, mLeftCameraPixmap,
                                              mRightCameraPixmap, mStatusBar);
  this->setCentralWidget(mStereoWindow);
}

void MainWindow::showManualWindow() {
  if (maCurFun == 6)
    return;

  maCurFun = 6;

  //mManualWindow = new ManualWindow(&mScene, mLeftCameraPixmap,
  //                                 mRightCameraPixmap, mStatusBar);
  //this->setCentralWidget(mManualWindow);
  mMeasureWindow = new MeasureMarkersWindow(&mScene, mLeftCameraPixmap,
                                            mRightCameraPixmap, mStatusBar);
  this->setCentralWidget(mMeasureWindow);
}

void MainWindow::iniParas() {
  //设置窗口，初始位置和最小大小
  setGeometry(50, 50, 800, 600);
  setMinimumHeight(480);
  setMinimumWidth(640);
}

void MainWindow::createAction() {
  maRecord = new QAction(QString::fromWCharArray(L"录像"), this);
  connect(maRecord, SIGNAL(triggered()), this, SLOT(showRecordWindow()));
  maPlay = new QAction(QString::fromWCharArray(L"播放"), this);
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