#ifndef _RECORD_WINDOW_H_
#define _RECORD_WINDOW_H_

//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

#include "QtCore/qobjectdefs.h"
#include "QtGui/qimage.h"
#include "QtWidgets/qwidget.h"
#include "OgreTimer.h"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QTimer;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

class UsbCamera;
class MyCameraView;
class MyDetailView;

/*
  这个类，主要用于录制视频的界面
  */
class RecordWindow : public QWidget
{
  Q_OBJECT
public:
  RecordWindow(QGraphicsScene* scene,
               QGraphicsPixmapItem* left,
               QGraphicsPixmapItem* right,
               QStatusBar* status);

  ~RecordWindow();
  //把灰度数据转换为QImage格式
  QImage convertToQImage(unsigned char* buffer);
  //更新场景图像
  void updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer);
signals:

public slots :
  //1录制视频，4按钮的处理函数
  //预览视频拍摄的画面
  void preview();
  //选择保存位置
  void selectSaveFolder();
  //录制视频
  void recordVedio();
  //结束录制视频
  void stopRecordVedio();
  //关闭视频录制界面
  void closeRecordWindow();
  //更新一帧场景图像
  void updateOneFrame();
  //设置详细视图为左视图
  void setLeftDetailView();
  //设置详细视图为右视图
  void setRightDetailView();
private:
  //创建窗口布局
  void createLayout();

  //创建按钮
  void createWidget();
protected:
  //属性
  QTimer*		mTimer;		//定时器触发
  Timer		mRecTimer;	//用于录制的定时器
  Timer		mProTimer;	//程序的计时器
  UsbCamera*	mCameras;	//摄像头

  //窗口布局
  QWidget*		mCenterWidget;			//中心窗口(即本窗口,this)
  QVBoxLayout*	mMainLayout;			//主布局
  QHBoxLayout*	mVedioLayout;			//上部的视频布局
  QHBoxLayout*	mTimeLineLayout;		//下部的时间线控制布局
  QVBoxLayout*	mStereoLayout;			//左边双视频布局
  QVBoxLayout*	mDetailLayout;			//右边的细节控制布局
  QHBoxLayout*	mCtrlLayout;			//上边的控制面板布局
  MyCameraView*	mLeftCameraView;		//左摄像机看到的场景
  MyCameraView*	mRightCameraView;		//右摄像机看到的场景
  MyDetailView*	mZoomView;				//缩放视图
  bool			mbLeftFocused;			//true表示左摄像头屏幕获得焦点

  //按钮
  QPushButton*	mPlay;			//预览画面,播放暂停按钮
  bool			mbPlay;			//true表示正在预览
  QPushButton*	mRecord;		//录制,暂停按钮
  bool			mbRecord;		//true表示正在录制，false表示暂停录制
  QPushButton*	mStop;			//停止录制按钮
  bool			mbStop;			//true表示已经停止了录制
  QPushButton*	mRecordFolder;	//保存录制文件夹的按钮
  int				mRecordCnt;		//当前录制的帧数，每次录制从1开始
  int				mRecordStart;	//开始录制时的全局时间

  //从外部传入
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//左摄像拍摄的画面（场景图像项）
  QGraphicsPixmapItem*	mRightCameraPixmap;	//右摄像拍摄的画面（场景图像项）
  QStatusBar*				mStatusBar;			//状态栏
  QGraphicsScene*			mScene;				//场景指针
};

#endif