﻿#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

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
protected:
  //场景
  QGraphicsScene			mScene;				//场景对象
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//左摄像拍摄的画面（场景图像项）
  QGraphicsPixmapItem*	mRightCameraPixmap;	//右摄像拍摄的画面（场景图像项）

  //菜单
  QAction*		maRecord;		//录像菜单，id=1
  QAction*		maPlay;			//播放菜单，id=2
  QAction*		maLeftFXY;		//标定内参菜单，id=3
  QAction*		maRightFXY;		//标定内参菜单，id=4
  QAction*		maRT;			//标定外参菜单，id=5
  QAction*		maDefaultRT;	//重置为默认的外参
  QAction*		maManual;		//手动测量菜单,id=6
  QAction*		maDefaultSpec;	//设置默认跟踪点
  QAction*		maTrack;		//跟踪窗口
  QAction*		ma3DShow;		//显示3d窗口
  QAction*		ma2DShow;		//显示2d窗口

  int				maCurFun;		//当前的界面id
  QAction*		maTodo;
  //工具

  //状态栏
  QStatusBar*		mStatusBar;		//状态栏

  //窗口
  RecordWindow*		mRecordWindow;		//录制窗口
  ReplayWindow*		mReplayWindow;		//播放窗口
  CalibrateWindow*	mCalibrateWindow;	//单目标定窗口
  StereoCalibrationWindow* mStereoWindow;	//双目标定窗口
  ManualWindow*		mManualWindow;		//手动测量窗口
  MeasureMarkersWindow* mMeasureWindow;
};

#endif