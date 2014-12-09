#ifndef _MANUAL_WINDOW_H_
#define _MANUAL_WINDOW_H_

//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"

#include "QtGui/qimage.h"

#include "QtWidgets/qwidget.h"

#include <vector>
#include <list>

#include "opencv2/core/core.hpp"
#include "StereoReconstructor.h"
#include "OgreTimer.h"

#include "base/memory/ref_counted.h"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;
class QPushButton;
class QLabel;
class QSlider;

class UsbCameras;
class MyDetailView;
class MyCameraView;
class Marker;

class UsbCameraGroup;

/*
  这个类，主要用于手动打标记点，并进行测量
  1、双击创建标记点
  2、delete删除选择的标记点
  */
class ManualWindow : public QWidget
{
  Q_OBJECT
public:
  ManualWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
               QGraphicsPixmapItem* right, QStatusBar* status);
  ~ManualWindow();

  //把灰度数据转换为QImage格式
  QImage convertToQImage(unsigned char* buffer);
  //更新场景图像
  void updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer);
  void updatePixmap(QImage& li, QImage& ri);
signals:

public slots :
  //预览视频拍摄的画面
  void preview();
  //用于自动检查信号灯
  void autoDetectLights();
  //用于自动检测标记点
  void autoDetectMarkers();
  //显示左标记点的编辑界面
  void showLeftMarkers();
  void showRightMarkers();
  //计算3d坐标
  void calculate3dPoints();

  //更新一帧场景图像
  void updateOneFrame();
  //设置详细视图为左视图
  void setLeftDetailView();
  //设置详细视图为右视图
  void setRightDetailView();

  //处理创建信号
  void dealwithCreatSignal(QPoint sp);
  //处理按键删除命令
  void dealwithDeleteSignal();

  static bool cmp(cv::Point2f p1, cv::Point2f p2){
	if(p1.x == p2.x)	return p1.y<p2.y;
	else	return p1.x<p2.x;
  }
private:
  //从左右匹配点重建三维坐标
  std::vector<StereoReconstructor::RestructPoint> restructPoints(
    std::vector<cv::Point2f>& lpts, std::vector<cv::Point2f>& rpts);

  //创建窗口布局
  void createLayout();
  //创建按钮
  void createWidget();
protected:
  //属性
  QTimer*		mTimer;		//定时器触发
  Timer		mRecTimer;	//用于录制的定时器
  Timer		mProTimer;	//程序的计时器
  scoped_refptr<UsbCameraGroup>   mCameras;   //摄像头

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
  QPushButton*			mPlay;			//预览画面,播放暂停按钮
  bool					mbPlay;			//true表示正在预览
  QPushButton*			autoLight;		//自动检测灯
  QPushButton*			autoDetect;		//点击用于自动检测标记点
  QPushButton*			mLM;			//点击用于选择，或批量删除左标记点
  QPushButton*			mRM;			//点击用于选择，或批量删除右标记点
  QPushButton*			mCalculate;		//点击用于选择，或批量删除右标记点
  QImage					mLeftImg;		//左摄像机图像
  QImage					mRightImg;		//右摄像机图像
  QString					mTempFolder;	//临时文件夹
  bool					mbCapture;		//true表示成功捕获了
  std::list<Marker*>		mLeftMarkers;	//保存左摄像的所有标记点
  std::list<Marker*>		mRightMarkers;	//保存右摄像的所有标记点

  int index;		//用于标志编号
  //从外部传入
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//左摄像拍摄的画面（场景图像项）
  QGraphicsPixmapItem*	mRightCameraPixmap;	//右摄像拍摄的画面（场景图像项）
  QStatusBar*				mStatusBar;			//状态栏
  QGraphicsScene*			mScene;				//场景指针

  QPushButton *			mClickMarker;		//手动选择标记点
};

#endif