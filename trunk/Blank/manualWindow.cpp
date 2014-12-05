#include "manualWindow.h"
#include <QtWidgets\qapplication.h>
#include <QtWidgets\qmainwindow.h>
#include <QtWidgets\qwidget.h>
#include <QtWidgets\qdesktopwidget.h>
#include <QtWidgets\qsplashscreen.h>
#include <QtWidgets\qaction.h>
#include <QtWidgets\qactiongroup.h>
#include <QtWidgets\qmenubar.h>
#include <QtWidgets\qtoolbar.h>
#include <QtWidgets\qstatusbar.h>
#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qlabel.h>
#include <QtWidgets\qboxlayout.h>
#include <QtWidgets\qpushbutton.h>
#include <QtWidgets\qfiledialog.h>
#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qcolordialog.h>
#include <QtWidgets\qprogressdialog.h>
#include <QtWidgets\qgraphicsscene.h>
#include <QtWidgets\qgraphicsitem.h>
#include <QtWidgets\qgraphicsview.h>
#include <QtWidgets\qboxlayout.h>
#include <QtWidgets\qabstractscrollarea.h>
#include <QtWidgets\qscrollbar.h>

#include <QtCore\qtimer.h>
#include <QtCore\qtextstream.h>
#include <QtCore\qtextcodec.h>

#include <QtGui\qtransform.h>
#include <QtGui\qevent.h>
#include <QtGui\qpainter.h>

#include "UsbCameras.h"
#include "myCameraView.h"
#include "marker.h"
#include "paras.h"
#include "VirtualCamera.h"

ManualWindow::ManualWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
                           QGraphicsPixmapItem* right, QStatusBar* status) {
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;

  mCameras = NULL;	//摄像头为空
  mbPlay = false;		//true表示正在预览
  mbCapture = true;
  mTempFolder = "./temp/";

  //创建布局
  createLayout();

  //设置默认场景
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
ManualWindow::~ManualWindow() {
  //如果在预览，关闭预览
  if (mbPlay == true) {
    preview();
  }
}

//把灰度数据转换为QImage格式
QImage ManualWindow::convertToQImage(unsigned char* buffer) {
  int w = Paras::getSingleton().width;
  int h = Paras::getSingleton().height;
  QImage temp(w, h, QImage::Format_ARGB32);
  for (int i = 0; i < h; i++) {
    unsigned char* pCurData = temp.scanLine(i);
    for (int j = 0; j < w; j++) {
      unsigned char data = buffer[i*w + j];

      *(pCurData + j * 4) = data;
      *(pCurData + j * 4 + 1) = data;
      *(pCurData + j * 4 + 2) = data;
      *(pCurData + j * 4 + 3) = 255;
    }
  }
  return temp;
}
//更新场景图像
void ManualWindow::updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer) {
  QImage li = convertToQImage(leftBuffer);
  QImage ri = convertToQImage(rightBuffer);
  mLeftImg = li;
  mRightImg = ri;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));
  //更新图像
  update();
}
//预览视频拍摄的画面
void ManualWindow::preview() {
  //如果没有启动摄像头，启动，并开始预览
  if (mbPlay == false) {
    mCameras = new UsbCameras("para.config");

    //如果启动失败，提示摄像机没有连接好
    if (mCameras->getCameraCount() != 2) {
      delete mCameras;
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"停止"));
      QMessageBox::critical(
        0,							//父窗口
        QString::fromWCharArray(L"找不到可用的摄像头"),		//标题栏
        QString::fromWCharArray(L"找不到可用的摄像头，请查看摄像头是否已经连接到电脑，如已经连接，请重新插拔USB接口"));		//文本内容
    }
    //成功启动
    else {
      mCameras->setTriggerMode(true);

      //切换其它按钮状态
      mbPlay = true;
      mPlay->setText(QString::fromWCharArray(L"拍照"));

      //设置定时触发
      mTimer = new QTimer(this);
      connect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->start(10);
    }
  }
  //如果成功启动摄像头，则关闭它
  else {
    if (mCameras != NULL) {
      mbPlay = false;
      disconnect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->stop();
      delete mTimer;
      mTimer = NULL;
      delete mCameras;
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"预览"));

      //保存图像
      mLeftImg.save(mTempFolder + "left.jpg");
      mRightImg.save(mTempFolder + "right.jpg");
      mLeftCameraPixmap->setPixmap(QPixmap::fromImage(mLeftImg));
      mRightCameraPixmap->setPixmap(QPixmap::fromImage(mRightImg));
      //更新图像
      update();
      mbCapture = true;
    }
  }
}

static std::vector<cv::Point2f> circleDetect(cv::Mat img_gray) {
  cv::Mat img_binary;
  cv::threshold(img_gray, img_binary, 60, 255, cv::THRESH_BINARY);
  cv::imwrite("./measure/binary.jpg", img_binary);

  std::vector<cv::Point2f> corners, result;
  cv::goodFeaturesToTrack(img_gray, corners, 200, 0.01, 10);

  //对角点进行聚类

  int threshold_group = 10;
  std::vector<std::vector<cv::Point2f> > group;
  for (size_t i = 0; i != corners.size(); ++i) {
    bool flag = true;
    for (size_t j = 0; j != group.size(); ++j) {
      if (sqrt((double)abs((corners[i].x - group[j][0].x)*(corners[i].x - group[j][0].x) + (corners[i].y - group[j][0].y)*(corners[i].y - group[j][0].y))) <= threshold_group) {
        group[j].push_back(corners[i]);
        flag = false;
        break;
      }
    }
    if (flag) {
      group.push_back(std::vector<cv::Point2f>(1, corners[i]));
    }
  }
  corners.clear();
  for (size_t i = 0; i != group.size(); ++i) {
    int x = 0, y = 0;
    for (size_t j = 0; j != group[i].size(); ++j) {
      x += group[i][j].x;
      y += group[i][j].y;
    }
    corners.push_back(cv::Point2f(x / group[i].size(), y / group[i].size()));
  }

  //求亚像素角点
  cv::cornerSubPix(img_gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 30, 0.1));
  //test code for corners
  cv::Mat img_rgb;
  cv::cvtColor(img_gray, img_rgb, CV_GRAY2RGB);
  for (int i = 0; i<corners.size(); i++)
    cv::circle(img_rgb, corners[i], 2, cv::Scalar(0, 0, 255), -1, 8, 0);
  cv::imwrite("./measure/corners.jpg", img_rgb);
 
  for (size_t i = 0; i != corners.size(); ++i) {
    std::vector<int> mean;	//圆周的归一化值
    int x = corners[i].x, y = corners[i].y, judge = 1;

    for (int k = 5; k<10 && judge; k++) {
      //num：圆周的像素点数，考虑到图像像素数离散的，所以只取了2×R个点，R是直径
      //sum：圆周的像素二值化值加和，255或者0
      //reverse_time：统计圆周像素变换，对角标志的话会变换4次，所以小于4次全都淘汰
      int sum = 0, reverse_time1 = 0, reverse_time2 = 0, flag = -1, blacknum2 = 0, blacknum1 = 0, num = 0;

      for (int xx = x - k; xx <= x + k; xx++) {
        if (xx<0 || xx >= img_gray.cols) {
          judge = 0;
          break;
        }
        int yy = y - static_cast<int>(sqrt(static_cast<double>(k*k - (xx - x)*(xx - x))) + 0.5);
        if (yy<0 || yy >= img_gray.rows) {
          judge = 0;
          break;
        }

        int val = img_binary.at<uchar>(yy, xx);
        if (flag == -1)
          flag = val;
        else if (flag != val) {
          flag = val;
          reverse_time1++;
        }
        if (val == 0)
          blacknum1++;
        sum += val;
        num++;
      }
      for (int xx = x + k; xx >= x - k; xx--) {
        if (xx<0 || xx >= img_gray.cols) {
          judge = 0;
          break;
        }
        int yy = y + static_cast<int>(sqrt(static_cast<double>(k*k - (xx - x)*(xx - x))) + 0.5);
        if (yy<0 || yy >= img_gray.rows) {
          judge = 0;
          break;
        }

        int val = img_binary.at<uchar>(yy, xx);
        if (flag == -1)
          flag = val;
        else if (flag != val) {
          flag = val;
          reverse_time2++;
        }
        if (val == 0)
          blacknum2++;

        sum += val;
        num++;
      }
      if (judge == 0 || reverse_time1 == 0 || reverse_time2 == 0 || reverse_time1 + reverse_time2<4 || (double)blacknum1 * 2 / num <= 0.2 || (double)blacknum2 * 2 / num >= 0.8 || abs(blacknum1 - blacknum2)>5) {
        break;
      }
      mean.push_back(sum / num);
    }

    if (judge == 0 || mean.size()<5) {
      mean.clear();
      continue;
    }

    int avarage = 0;
    for (int j = 0; j<mean.size(); j++)
      avarage += mean[j];
    avarage /= 5;

    int threshold_val = 30;
    for (int j = 0; j<5; j++) {
      //std::cout << mean[j] << " ";
      if (abs(mean[j] - mean[0])>threshold_val) {
        judge = 0;
        break;
      }
    }

    if (judge) {
      //std::cout << i << std::endl;
      result.push_back(corners[i]);
    }
    mean.clear();

  }
  return result;
}

//用于自动检测标记点
void ManualWindow::autoDetectMarkers(){
	//读取图像，计算角点
	cv::Mat li = cv::imread((mTempFolder + "left.jpg").toStdString(), CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat ri = cv::imread((mTempFolder + "right.jpg").toStdString(), CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat liGrey, riGrey;
	cv::cvtColor(li, liGrey, cv::COLOR_BGR2GRAY);
	cv::cvtColor(ri, riGrey, cv::COLOR_BGR2GRAY);

	std::vector<cv::Point2f> left_markers = circleDetect(liGrey);
	std::vector<cv::Point2f> right_markers = circleDetect(riGrey);

	if(left_markers.size()==2 && right_markers.size()==2){
		//sort(left_markers.begin(), left_markers.end());
		//sort(right_markers.begin(), right_markers.end());

		for(int i=0; i<2; i++){
			Marker* plm = new Marker(QPoint(left_markers[i].x, left_markers[i].y), QString::number(i+1), mScene);
			mLeftMarkers.push_back(plm);
			Marker* prm = new Marker(QPoint(left_markers[i].x, left_markers[i].y), QString::number(i+1), mScene);
			mRightMarkers.push_back(prm);
		}
	}
	else{
		QMessageBox::about(
		0,				//父窗口
		QString::fromWCharArray(L"消息框"),		//标题栏
		QString::fromWCharArray(L"找不到标记点，请手动选择"));	//文本内容
	}
}

//显示左标记点的编辑界面
void ManualWindow::showLeftMarkers() {
	mbCapture = true;
	setLeftDetailView();	//switch to left view
  QMessageBox::about(
    0,				//父窗口
    QString::fromWCharArray(L"消息框"),		//标题栏
    QString::fromWCharArray(L"选择，或批量删除左标记点"));	//文本内容

  


}
void ManualWindow::showRightMarkers() {
	mbCapture = true;
	setRightDetailView();	//switch to right view
  QMessageBox::about(
    0,				//父窗口
    QString::fromWCharArray(L"消息框"),		//标题栏
    QString::fromWCharArray(L"选择，或批量删除右标记点"));	//文本内容


}

//计算3d坐标
//taokelu@gmail.com
void ManualWindow::calculate3dPoints() {
  QMessageBox::about(
    0,				//父窗口
    QString::fromWCharArray(L"消息框"),		//标题栏
    QString::fromWCharArray(L"计算3d坐标"));	//文本内容

  
  //把标记点一一对应记录到left.txt和right.txt中，传给reconstructor
  if(mLeftMarkers.size()!=mRightMarkers.size()){
	QMessageBox::about(
		0,
		QString::fromWCharArray(L"消息框"),
		QString::fromWCharArray(L"错误：左右标记点数目不相等！")
		);
	return;
  }
  std::vector<cv::Point2f> left_markers, right_markers;
  for(std::list<Marker*>::iterator iter1=mLeftMarkers.begin(); iter1!=mLeftMarkers.end(); ++iter1){
	  left_markers.push_back(cv::Point((*iter1)->getPosition().x(),(*iter1)->getPosition().y()));
	  //right_markers.push_back(cv::Point((*iter2)->getPositon().x(),(*iter2)->getPositon().y()));
	  bool flag=true;
	  for(std::list<Marker*>::iterator iter2=mRightMarkers.begin(); iter2!=mRightMarkers.end(); ++iter2){
		if((*iter1)->getText()==(*iter2)->getText()){
			right_markers.push_back(cv::Point((*iter2)->getPosition().x()-Paras::getSingleton().width*2,(*iter2)->getPosition().y()));
			flag = false;
			break;
		}
	  }
	  if(flag){
		  QMessageBox::about(
			0,
			QString::fromWCharArray(L"消息框"),
			QString::fromWCharArray(L"错误：左右标记点名称不匹配，请确保名称匹配！")
			);
		  return;
	  }
  }

  /*
  for(std::vector<cv::Point2f>::iterator iter1=left_markers.begin(),iter2=right_markers.begin(); iter1!=left_markers.end(); ++iter1,++iter2){
	  std::cout << (*iter1).x << " " << (*iter1).y << std::endl;
	  std::cout << (*iter2).x << " " << (*iter2).y << std::endl;
  }*/
  //
  std::vector<StereoReconstructor::RestructPoint> ret = restructPoints(left_markers, right_markers);

  double distance = sqrt((ret[0].point.x-ret[1].point.x)*(ret[0].point.x-ret[1].point.x)+(ret[0].point.y-ret[1].point.y)*(ret[0].point.y-ret[1].point.y)+(ret[0].point.z-ret[1].point.z)*(ret[0].point.z-ret[1].point.z));
  std::cout << distance << std::endl;
}

//更新一帧场景图像
void ManualWindow::updateOneFrame() {
  static int FrameCount = 0;
  static int StartTime = mProTimer.getMilliseconds();

  if (mCameras->captureTwoFrameSyncSoftControl(0, 1)) {
    FrameCount++;

    updatePixmap(mCameras->getBuffer(0), mCameras->getBuffer(1));
  }

  //每20帧，统计下帧率
  if (FrameCount == 20) {
    int endTime = mProTimer.getMilliseconds();
    float fps = (float)FrameCount * 1000 / (endTime - StartTime);

    //更新状态栏显示
    QString show;
    show.sprintf("当前的帧率为：%f", fps);
    mStatusBar->showMessage(show);

    //置0
    FrameCount = 0;
    StartTime = mProTimer.getMilliseconds();
  }
}
//设置详细视图为左视图
void ManualWindow::setLeftDetailView() {
  //如果已经是左视图，直接返回
  if (mbLeftFocused) {
    return;
  }
  //否则从右视图切换
  else {
    mbLeftFocused = true;
    //保存右视图当前的状态
    mRightCameraView->saveZoomViewState();
    //切换到左视图
    mLeftCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mLeftCameraView->getSavedState());
  }
}
//设置详细视图为右视图
void ManualWindow::setRightDetailView() {
  //如果是左视图，切换到右视图
  if (mbLeftFocused) {
    mbLeftFocused = false;
    //保存左视图当前的状态
    mLeftCameraView->saveZoomViewState();
    //切换到右视图
    mRightCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mRightCameraView->getSavedState());
  } else {
    return;
  }
}

//处理创建信号
void ManualWindow::dealwithCreatSignal(QPoint sp) {
  //只有成功捕获了才处理
  if (mbCapture) {
    if (mbLeftFocused) {
      static int LC = 0;
      LC++;
      QString name;
      name.sprintf("%d", LC);
      Marker* plm = new Marker(sp, name, mScene);
      mLeftMarkers.push_back(plm);
    } else {
      static int RC = 0;
      RC++;
      QString name;
      name.sprintf("%d", RC);
      Marker* prm = new Marker(sp, name, mScene);
      mRightMarkers.push_back(prm);
    }
  }
}
//处理按键删除命令
void ManualWindow::dealwithDeleteSignal() {
  std::vector<Marker*> toDelete;
  std::list<Marker*>::iterator i = mLeftMarkers.begin();
  while (i != mLeftMarkers.end()) {
    //如果被选择了，就删除
    if ((*i)->getSelectedState()) {
      toDelete.push_back(*i);
      i = mLeftMarkers.erase(i);
    } else {
      i++;
    }
  }

  std::list<Marker*>::iterator j = mRightMarkers.begin();
  while (j != mRightMarkers.end()) {
    //如果被选择了，就删除
    if ((*j)->getSelectedState()) {
      toDelete.push_back(*j);
      j = mRightMarkers.erase(j);
    } else {
      j++;
    }
  }

  for (int i = 0; i < toDelete.size(); i++) {
    delete toDelete[i];
  }
}
  //从左右匹配点重建三维坐标
std::vector<StereoReconstructor::RestructPoint> ManualWindow::restructPoints(
    std::vector<cv::Point2f>& lpts, std::vector<cv::Point2f>& rpts) {
  //载入2个虚拟摄像机
  VirtualCamera vc1;
  VirtualCamera vc2;
  //载入内参
  vc1.loadCameraMatrix("reconstruct\\LeftMatrix.txt");
  vc1.loadDistortion("reconstruct\\LeftDistortion.txt");
  vc1.rotationMatrix = cv::Mat::eye(3, 3, CV_32FC1);
  vc1.translationVector = cv::Mat::zeros(3, 1, CV_32FC1);
  vc2.loadCameraMatrix("reconstruct\\RightMatrix.txt");
  vc2.loadDistortion("reconstruct\\RightDistortion.txt");
  vc2.loadRotationMatrix("reconstruct\\R.txt");
  vc2.loadTranslationVector("reconstruct\\T.txt");

  //设置左右匹配点
  vc1.setKeyPoints(lpts);
  vc2.setKeyPoints(rpts);

  //重建3d点
  std::vector<StereoReconstructor::RestructPoint> ret;
  ret = StereoReconstructor::triangulation(vc1, vc2, true);	//true进行畸变矫正,false表示不进行

  return ret;
}

//创建窗口布局
void ManualWindow::createLayout() {
  mCenterWidget = this;

  mMainLayout = new QVBoxLayout();
  mCenterWidget->setLayout(mMainLayout);

  mVedioLayout = new QHBoxLayout();
  mTimeLineLayout = new QHBoxLayout();
  mMainLayout->addLayout(mVedioLayout);
  mMainLayout->addLayout(mTimeLineLayout);

  mStereoLayout = new QVBoxLayout();
  mDetailLayout = new QVBoxLayout();
  mVedioLayout->addLayout(mStereoLayout, 9);
  mVedioLayout->addLayout(mDetailLayout, 16);

  //设置场景视图
  //左摄像机视图
  int mWidth = Paras::getSingleton().width;
  int mHeight = Paras::getSingleton().height;
  mLeftCameraView = new MyCameraView(mScene, QRect(0, 0, mWidth, mHeight));
  //右摄像机视图
  mRightCameraView = new MyCameraView(mScene, QRect(mWidth * 2, 0, mWidth, mHeight));
  //获得焦点时，切换左右视图
  connect(mLeftCameraView, SIGNAL(getFocusSignal()), this, SLOT(setLeftDetailView()));
  connect(mRightCameraView, SIGNAL(getFocusSignal()), this, SLOT(setRightDetailView()));
  mStereoLayout->addWidget(mLeftCameraView);
  mStereoLayout->addWidget(mRightCameraView);

  //创建按钮
  mCtrlLayout = new QHBoxLayout();
  mDetailLayout->addLayout(mCtrlLayout);

  //创建缩放视图，默认初始化给左视图
  mZoomView = new MyDetailView(mScene, QRect(0, 0, mWidth, mHeight));
  mLeftCameraView->setZoomView(mZoomView);
  mbLeftFocused = true;
  mDetailLayout->addWidget(mZoomView);

  createWidget();
}
//创建按钮
void ManualWindow::createWidget() {
  mPlay = new QPushButton();
  mPlay->setText(QString::fromWCharArray(L"预览"));
  connect(mPlay, SIGNAL(pressed()), this, SLOT(preview()));

  autoDetect = new QPushButton();
  autoDetect->setText(QString::fromWCharArray(L"自动检测"));
  connect(autoDetect, SIGNAL(pressed()), this, SLOT(autoDetectMarkers()));

  mLM = new QPushButton();
  mLM->setText(QString::fromWCharArray(L"手动点左标记点"));
  connect(mLM, SIGNAL(pressed()), this, SLOT(showLeftMarkers()));

  mRM = new QPushButton();
  mRM->setText(QString::fromWCharArray(L"手动点右标记点"));
  connect(mRM, SIGNAL(pressed()), this, SLOT(showRightMarkers()));

  mCalculate = new QPushButton();
  mCalculate->setText(QString::fromWCharArray(L"计算3D坐标"));
  connect(mCalculate, SIGNAL(pressed()), this, SLOT(calculate3dPoints()));

  //添加双击创建信号的响应
  connect(mZoomView, SIGNAL(createMarker(QPoint)), this, SLOT(dealwithCreatSignal(QPoint)));
  connect(mZoomView, SIGNAL(deleteMarker()), this, SLOT(dealwithDeleteSignal()));

  mCtrlLayout->addWidget(mPlay);
  mCtrlLayout->addWidget(autoDetect);
  mCtrlLayout->addWidget(mLM);
  mCtrlLayout->addWidget(mRM);
  mCtrlLayout->addWidget(mCalculate);
}