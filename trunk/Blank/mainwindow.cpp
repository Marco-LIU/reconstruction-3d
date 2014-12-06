#include "mainwindow.h"
#include "recordWindow.h"
#include "replayWindow.h"
#include "calibrateWindow.h"
#include "stereoCalWindow.h"
#include "manualWindow.h"
#include "MeasureMarkersWindows.h"
#include "markerDefineWindow.h"
#include "trackingWindow.h"

#include "QtWidgets/qgraphicsitem.h"
#include "QtWidgets/qaction.h"
#include "QtWidgets/qstatusbar.h"
#include "QtWidgets/qgraphicsscene.h"
#include "QtWidgets/qmenu.h"
#include "QtWidgets/qmenubar.h"

#include "paras.h"

#include "base/message_loop/message_loop.h"
#include "qt_utils.h"

#include <QtCore/QDir>
#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qlayout.h>
#include <QtWidgets\qlabel.h>
#include <QtWidgets\qslider.h>
#include <QtWidgets\qinputdialog.h>

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

  mMeasureWindow = new MeasureMarkersWindow(&mScene, mLeftCameraPixmap,
                                            mRightCameraPixmap, mStatusBar);
  mCurrentWidget = mMeasureWindow;
  this->setCentralWidget(mMeasureWindow);
}

void MainWindow::showMarkerDefineWindow()
{
  if (mCurrentWidget && mCurrentWidget == mMarkerDefWindow)
    return;

  mMarkerDefWindow = new MarkerDefineWindow(mStatusBar);
  mCurrentWidget = mMarkerDefWindow;
  this->setCentralWidget(mMarkerDefWindow);
}

void MainWindow::showTrackingWindow()
{
  if (mCurrentWidget && mCurrentWidget == mTrackingWindow)
    return;

  mTrackingWindow = new TrackingWindow(&mScene, mLeftCameraPixmap,
                                            mRightCameraPixmap, mStatusBar);
  mCurrentWidget = mTrackingWindow;
  this->setCentralWidget(mTrackingWindow);
}

/*
void MainWindow::setGainExpo()
{
	//todo 有空再写界面吧
	QWidget* sw = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout();
	sw->setLayout(layout);

	QLabel* lg = new QLabel();
	lg->setText(QString::fromWCharArray(L"左摄像头增益"));
	QLabel* le = new QLabel();
	le->setText(QString::fromWCharArray(L"左摄像头曝光"));

	QLabel* rg = new QLabel();
	rg->setText(QString::fromWCharArray(L"右摄像头增益"));
	QLabel* re = new QLabel();
	re->setText(QString::fromWCharArray(L"右摄像头曝光"));

	layout->addWidget(lg);
	layout->addWidget(le);
	layout->addWidget(rg);
	layout->addWidget(re);

	sw->setWindowModality(Qt::WindowModal);
	sw->show();
}
*/

void MainWindow::setLeftCameraGain()
{
	//显示当前值，刘浏加上
	int cur = Paras::getSingletonPtr()->left_gain_;

	//设置新值
	int nv = QInputDialog::getInt(
		0,
		QString::fromWCharArray(L"设置左摄像头增益"),
		QString::fromWCharArray(L"设置左摄像头增益："),
		cur,
		0,
		255);

	//设置左摄像机为新值，刘浏加上
  Paras::getSingletonPtr()->SetLeftGain(nv);
}

void MainWindow::setLeftCameraExpo()
{
	//显示当前值，刘浏加上
  int cur = Paras::getSingletonPtr()->left_expo_;

	//设置新值
	int nv = QInputDialog::getInt(
		0,
		QString::fromWCharArray(L"设置左摄像头曝光"),
		QString::fromWCharArray(L"设置左摄像头曝光："),
		cur,
		0,
		4000);

	//设置左摄像机为新值，刘浏加上
  Paras::getSingletonPtr()->SetLeftExpo(nv);
}

void MainWindow::setRightCameraGain()
{
	//显示当前值，刘浏加上
  int cur = Paras::getSingletonPtr()->right_gain_;

	//设置新值
	int nv = QInputDialog::getInt(
		0,
		QString::fromWCharArray(L"设置右摄像头增益"),
		QString::fromWCharArray(L"设置右摄像头增益："),
		cur,
		0,
		255);

	//设置左摄像机为新值，刘浏加上
  Paras::getSingletonPtr()->SetRightGain(nv);
}

void MainWindow::setRightCameraExpo()
{
	//显示当前值，刘浏加上
  int cur = Paras::getSingletonPtr()->right_expo_;

	//设置新值
	int nv = QInputDialog::getInt(
		0,
		QString::fromWCharArray(L"设置右摄像头曝光"),
		QString::fromWCharArray(L"设置右摄像头曝光："),
		cur,
		0,
		4000);

	//设置左摄像机为新值，刘浏加上
  Paras::getSingletonPtr()->SetRightExpo(nv);
}
void MainWindow::iniParas() {
  //设置窗口，初始位置和最小大小
  setGeometry(50, 50, 800, 600);
  setMinimumHeight(480);
  setMinimumWidth(640);
}

void MainWindow::createAction() {
  //文件
  maLG = new QAction(QString::fromWCharArray(L"设置左摄像头增益"), this);
  connect(maLG, SIGNAL(triggered()), this, SLOT(setLeftCameraGain()));
  maLE = new QAction(QString::fromWCharArray(L"设置左摄像头曝光"), this);
  connect(maLE, SIGNAL(triggered()), this, SLOT(setLeftCameraExpo()));	
  maRG = new QAction(QString::fromWCharArray(L"设置右摄像头增益"), this);
  connect(maRG, SIGNAL(triggered()), this, SLOT(setRightCameraGain()));	
  maRE = new QAction(QString::fromWCharArray(L"设置右摄像头曝光"), this);
  connect(maRE, SIGNAL(triggered()), this, SLOT(setRightCameraExpo()));	
 
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
  connect(maDefaultRT, SIGNAL(triggered()), this, SLOT(setAsDefault()));
  maManual = new QAction(QString::fromWCharArray(L"手动测量"), this);
  connect(maManual, SIGNAL(triggered()), this, SLOT(showManualWindow()));
  maMeasure = new QAction(QString::fromWCharArray(L"自动测量"), this);
  connect(maMeasure, SIGNAL(triggered()), this, SLOT(showAutoMeasureWindow()));
  
  //跟踪
  maDefaultSpec = new QAction(QString::fromWCharArray(L"设置跟踪点"), this);
  connect(maDefaultSpec, SIGNAL(triggered()), this, SLOT(showMarkerDefineWindow()));
  maTrack = new QAction(QString::fromWCharArray(L"跟踪"), this);
  connect(maTrack, SIGNAL(triggered()), this, SLOT(showTrackingWindow()));
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
  fileMenu->addAction(maLG);
  fileMenu->addAction(maLE);
  fileMenu->addAction(maRG);
  fileMenu->addAction(maRE);

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

void MainWindow::setAsDefault(){
	QString destPath("reconstruct\\");
	QString srcPath("defaultRT\\");
	QDir curDir(destPath);

	//删除reconstruct文件夹下的所有文件
	QFileInfoList allFileInfos = curDir.entryInfoList();
	for(int i=0;i<allFileInfos.size();i++)
	{
		QFile::remove(destPath + allFileInfos[i].fileName());
	}
	//从default拷贝
	curDir = srcPath;
	allFileInfos = curDir.entryInfoList();
	for(int i=0;i<allFileInfos.size();i++)
	{
		QFile::copy(srcPath + allFileInfos[i].fileName(), destPath + allFileInfos[i].fileName());
	}

	QMessageBox::about(
		0,
		QString::fromWCharArray(L"消息框"),
		QString::fromWCharArray(L"已将外参数设置成默认参数！")
	);
}