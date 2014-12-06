#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

#include "base/compiler_specific.h"
#include "QtWidgets/qmainwindow.h"
#include "QtWidgets/qgraphicsscene.h"

class QGraphicsPixmapItem;
class QAction;
class QMenu;
class QStatusBar;

class RecordWindow;
class ReplayWindow;
class CalibrateWindow;
class StereoCalibrationWindow;
class ManualWindow;
class MeasureMarkersWindow;
class MarkerDefineWindow;
class TrackingWindow;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow();
  ~MainWindow();

signals:

public slots :
  //1、几个主界面切换
  //显示录制界面
  void showRecordWindow();
  //显示播放界面
  void showReplayWindow();
  //显示标定左摄像机的界面
  void showCalibrateLeft();
  //显示标定右摄像机的界面
  void showCalibrateRight();
  //显示标定外参的界面
  void showCalibrateRT();
  //显示手动测量窗口
  void showManualWindow();
  //显示自动测量窗口
  void showAutoMeasureWindow();

  //重置为默认
  void setAsDefault();

  //显示预定义点窗口
  void showMarkerDefineWindow();
  //显示跟踪窗口
  void showTrackingWindow();

  //显示设置左右摄像机增益和曝光的窗口
  void setLeftCameraGain();
  void setLeftCameraExpo();
  void setRightCameraGain();
  void setRightCameraExpo();
protected:
  //初始化
  void iniParas();

  //所有动作
  void createAction();

  //创建菜单
  void createMenu();

  //创建工具栏

  //创建状态栏
  void createStatusBar();

  //创建场景（左|空|右）
  void createScene();

  // 处理窗口关闭，这里需要退出我们的消息循环
  virtual void closeEvent(QCloseEvent* event) OVERRIDE;
protected:
  //场景
  QGraphicsScene		mScene;				//场景对象
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//左摄像拍摄的画面（场景图像项）
  QGraphicsPixmapItem*	mRightCameraPixmap;	//右摄像拍摄的画面（场景图像项）

  //菜单
  QAction*		maLG;			//左摄像头增益
  QAction*		maLE;			//左摄像头曝光
  QAction*		maRG;			//右摄像头增益
  QAction*		maRE;			//右摄像头曝光

  QAction*		maGainExpo;		//设置增益和曝光

  QAction*		maRecord;		//录像菜单
  QAction*		maPlay;			//播放菜单

  QAction*		maLeftFXY;		//标定左摄像机内参菜单
  QAction*		maRightFXY;		//标定右摄像机内参菜单
  QAction*		maRT;			//标定外参菜单
  QAction*		maDefaultRT;	//重置为默认的外参
  QAction*		maManual;		//手动测量菜单
  QAction*		maMeasure;		//自动测量菜单

  QAction*		maDefaultSpec;	//设置默认跟踪点
  QAction*		maTrack;		//跟踪窗口
  QAction*		ma3DShow;		//显示3d窗口
  QAction*		ma2DShow;		//显示2d窗口

  QAction*		maTodo;
  //工具

  //状态栏
  QStatusBar*		mStatusBar;		//状态栏

  //窗口
  RecordWindow*				mRecordWindow;			//录制窗口
  ReplayWindow*				mReplayWindow;			//播放窗口
  CalibrateWindow*			mLeftCalibrateWindow;	//左单目标定窗口
  CalibrateWindow*			mRightCalibrateWindow;	//右单目标定窗口
  StereoCalibrationWindow*	mStereoWindow;			//双目标定窗口
  ManualWindow*				mManualWindow;			//手动打标记点测量窗口
  MeasureMarkersWindow*		mMeasureWindow;			//自动打标记点测量窗口
  MarkerDefineWindow*		mMarkerDefWindow;		//设置跟踪点的窗口
  TrackingWindow*			mTrackingWindow;		//设置跟踪窗口
  QWidget*					mCurrentWidget;
};

#endif