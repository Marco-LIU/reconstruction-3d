#ifndef _CALIBRATE_WINDOW_H_
#define _CALIBRATE_WINDOW_H_

//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

#include "QtCore/qobjectdefs.h"
#include "QtGui/qimage.h"
#include "QtWidgets/qwidget.h"

#include "opencv2/core/core.hpp"
#include "base/memory/ref_counted.h"

#include "OgreTimer.h"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;
class QPushButton;
class QSlider;
class QLabel;

class MyCameraView;
class MyDetailView;
// class UsbCameras;
class UsbCameraGroup;

class CalibrateWindow : public QWidget
{
    Q_OBJECT
public:
  CalibrateWindow(QGraphicsScene* scene,QGraphicsPixmapItem* left,QGraphicsPixmapItem* right,
    QStatusBar* status,QString imgFolder = "./leftCamera/",bool bl = true);
  ~CalibrateWindow();
  //把灰度数据转换为QImage格式
  QImage convertToQImage(unsigned char* buffer);
  //更新场景图像
  void updatePixmap(unsigned char* buffer);
  void updatePixmap(QImage& li);
signals:

public slots:
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
  //设置x方向的角点个数
  void setSizeX();
  //设置y方向的角点个数
  void setSizeY();
  //设置棋盘格的长度
  void setLength();
  //进行标定
  void calibrate();
  //滑动滑块更新当前选择的
  void updateCurrent(int cs);
private:
  //把角点保存到文件中,前面3行是xCorners,yCorners,Length,接着是物体空间坐标，接着是投影图像坐标
  void saveCornerPoints(std::string filename,std::vector<cv::Point2f> pts);

  //载入所有的角点坐标
  void loadCornerPoints(std::string filename,std::vector<cv::Point2f>& pts,std::vector<cv::Point3f>& opts);
  //创建窗口布局
  void createLayout();

  //创建按钮
  void createWidget();

  //根据当前捕获的图像，更新各个按钮的状态
  void updateButtonState();
protected:
  //窗口布局
  QWidget*        mCenterWidget;        //中心窗口(即本窗口,this)
  QVBoxLayout*    mMainLayout;          //主布局
  QHBoxLayout*    mVedioLayout;         //上部的视频布局
  QHBoxLayout*    mTimeLineLayout;      //下部的时间线控制布局
  QVBoxLayout*    mStereoLayout;        //左边双视频布局
  QVBoxLayout*    mDetailLayout;        //右边的细节控制布局
  QHBoxLayout*    mCtrlLayout;          //上边的控制面板布局
  MyCameraView*   mLeftCameraView;      //左摄像机看到的场景
  MyCameraView*   mRightCameraView;     //右摄像机看到的场景
  MyDetailView*   mZoomView;            //缩放视图

  QTimer*			mTimer;					//定时器触发
  scoped_refptr<UsbCameraGroup>   mCameras;     //摄像头
  int				mCameraId;				//0表示左摄像头，1表示右摄像头
  bool			mbLeftFocused;			//true表示左摄像头屏幕获得焦点
  Timer			mProTimer;				//程序的计时器
  QImage			mLastCaptureImg;		//上一次捕获的图像
  int				mXCorners;				//x方向的角点个数
  int				mYCorners;				//y方向的角点个数
  int				mSquareLength;			//棋盘格的长度
  std::vector<int> mImgs;					//存储所有的图片
  QString			mImgFolders;			//存储图像的文件夹
  int				mImgWidth;				//捕获的图像宽度
  int				mImgHeight;				//捕获的图像高度

  //按钮
  QPushButton*	mPlay;			//预览画面,播放暂停按钮
  bool			mbPlay;			//true表示正在预览
  QLabel*			mCaps;			//显示捕获的数量，和当前观看的图像
  QPushButton*	mCapture;		//捕获按钮
  QPushButton*	mDelete;		//删除按钮
  QLabel*			mSizeXLabel;	//X方向的角点个数
  QPushButton*	mSizeX;			//设置X方向角点个数的按钮
  QLabel*			mSizeYLabel;	//Y方向的角点个数
  QPushButton*	mSizeY;			//设置Y方向角点个数的按钮
  QLabel*			mLengthLabel;	//棋盘格的长度，单位mm
  QPushButton*	mLength;		//设置棋盘格的长度
  QSlider*		mSlider;		//滑动块，显示帧率信息 
  QPushButton*	mCalibrate;		//标定按钮

//从外部传入
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//左摄像拍摄的画面（场景图像项）
  QGraphicsPixmapItem*	mRightCameraPixmap;	//右摄像拍摄的画面（场景图像项）
  QStatusBar*				mStatusBar;			//状态栏
  QGraphicsScene*			mScene;				//场景指针
};
#endif