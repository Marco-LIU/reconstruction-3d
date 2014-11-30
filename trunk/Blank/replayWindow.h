#ifndef _REPLAY_WINDOW_H_
#define _REPLAY_WINDOW_H_

//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"

#include "QtGui/qimage.h"

#include "QtWidgets/qwidget.h"

#include <vector>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;
class QPushButton;
class QLabel;
class QSlider;

class MyDetailView;
class MyCameraView;

class ReplayWindow : public QWidget
{
  Q_OBJECT
public:
  ReplayWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
               QGraphicsPixmapItem* right, QStatusBar* status);
signals:

public slots :
  //设置详细视图为左视图
  void setLeftDetailView();
  //设置详细视图为右视图
  void setRightDetailView();
  //新的位置
  void updateImage(int value);
  //播放视频
  void playVedio();

  //打开视频文件
  void open();
  //播放视频
  void play();
  //上一帧
  void next();
  //下一帧
  void before();
private:
  //载入索引所指的图像
  void loadImage(int i);

  //创建窗口布局
  void createLayout();

  //创建按钮
  void createWidget();
protected:
  //属性参数
  QString			mLeftImagesFoler;	//左图像保存的位置
  QString			mRightImagesFoler;	//右图像保存的位置
  QString			mImagesFoler;		//总文件夹
  int				mStartIndex;		//起始帧的图片索引
  int				mEndIndex;			//结束帧的图片索引
  int				mStartTime;			//起始帧的时间(单位毫秒)
  int				mEndTime;			//结束帧的时间(单位毫秒)
  bool			mbOpen;				//true表示打开了视频文件
  bool			mbPlay;				//true表示播放，false表示暂停
  QTimer*			mTimer;				//定时器触发
  std::vector<int> mAllIndex;			//记录所有照片的索引
  std::vector<int> mAllTimes;			//记录所有照片的拍摄时间

  //按钮
  QLabel*			mTime;				//显示时间
  QPushButton*	mPlay;				//播放暂停按钮
  QPushButton*	mNext;				//下一帧
  QPushButton*	mBefore;			//上一帧
  QPushButton*	mOpen;				//打开视频按钮
  QSlider*		mSlider;			//滑动块，显示帧率信息 

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

  //从外部传入
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//左摄像拍摄的画面（场景图像项）
  QGraphicsPixmapItem*	mRightCameraPixmap;	//右摄像拍摄的画面（场景图像项）
  QStatusBar*				mStatusBar;			//状态栏
  QGraphicsScene*			mScene;				//场景指针
};
#endif