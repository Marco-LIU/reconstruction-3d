#pragma once
#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"

#include "QtGui/qimage.h"

#include "QtWidgets/qwidget.h"

#include <vector>

#include "opencv2/core/core.hpp"

#include "OgreTimer.h"

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

/*
自动捕获标记点，手动可以修改
*/
class MeasureMarkersWindow : public QWidget
{
  Q_OBJECT
public:
  MeasureMarkersWindow(QGraphicsScene* scene,
                       QGraphicsPixmapItem* left,
                       QGraphicsPixmapItem* right,
                       QStatusBar* status,
                       QString reconstructFolder = "./reconstruct/",
                       QString measureFolder = "./measure/");
  ~MeasureMarkersWindow();
  //把灰度数据转换为QImage格式
  QImage convertToQImage(unsigned char* buffer);
  QImage convertToQImage(cv::Mat img);
  //更新场景图像
  void updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer);
signals:

public slots :
  //更新一帧场景图像
  void updateOneFrame();
  //设置详细视图为左视图
  void setLeftDetailView();
  //设置详细视图为右视图
  void setRightDetailView();

  //预览视频拍摄的画面
  void preview();
  //捕获一张图像
  void capture();
  //删除当前的图像
  void deleteImg();

  //滑动滑块更新当前选择的
  void updateCurrent(int cs);
private:
  //创建窗口布局
  void createLayout();

  //创建按钮
  void createWidget();

  //根据当前捕获的图像，更新各个按钮的状态
  void updateButtonState();
protected:
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

  QTimer*			mTimer;					//定时器触发
  UsbCameras*		mCameras;				//摄像头
  int				mCameraId;				//0表示左摄像头，1表示右摄像头
  bool			mbLeftFocused;			//true表示左摄像头屏幕获得焦点
  Timer			mProTimer;				//程序的计时器
  QImage			mLastLeft;				//上一次捕获的图像
  QImage			mLastRight;				//上一次捕获的图像
  
  std::vector<int> mImgs;					//存储所有的图片
  QString			mReconstructFolder;			//存储图像的文件夹
  QString			mMeasureFolder;		//保存左摄像机内参的文件夹
  int				mImgWidth;				//捕获的图像宽度
  int				mImgHeight;				//捕获的图像高度

  //按钮
  QPushButton*	mPlay;			//预览画面,播放暂停按钮
  bool			mbPlay;			//true表示正在预览
  QLabel*			mCaps;			//显示捕获的数量，和当前观看的图像
  QPushButton*	mCapture;		//捕获按钮
  QPushButton*	mDelete;		//删除按钮
  QSlider*		mSlider;		//滑动块，显示帧率信息 

  //从外部传入
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//左摄像拍摄的画面（场景图像项）
  QGraphicsPixmapItem*	mRightCameraPixmap;	//右摄像拍摄的画面（场景图像项）
  QStatusBar*				mStatusBar;			//状态栏
  QGraphicsScene*			mScene;				//场景指针
};