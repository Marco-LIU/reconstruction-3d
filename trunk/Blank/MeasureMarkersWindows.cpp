#include "MeasureMarkersWindows.h"

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
#include <QtWidgets\qslider.h>
#include <QtWidgets\qinputdialog.h>

#include <QtCore\qtextstream.h>
#include <QtCore\qtextcodec.h>

#include <QtGui\qtransform.h>
#include <QtGui\qevent.h>
#include <QtGui\qpainter.h>

#include "UsbCameras.h"
#include "myCameraView.h"
#include "marker.h"
#include "paras.h"
#include "CameraCalibration.h"


MeasureMarkersWindow::MeasureMarkersWindow(QGraphicsScene* scene,
                                           QGraphicsPixmapItem* left,
                                           QGraphicsPixmapItem* right,
                                           QStatusBar* status,
                                           QString reconstructFolder /* = "./reconstruct/" */,
                                           QString measureFolder /* = "./measure/" */) {
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;

  mbPlay = false;

  mReconstructFolder = reconstructFolder;
  mMeasureFolder = measureFolder;

  //创建布局
  createLayout();

  //设置默认场景
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}

MeasureMarkersWindow::~MeasureMarkersWindow() {
  //关闭摄像头
  if (mbPlay)
    preview();
}

//把灰度数据转换为QImage格式
QImage MeasureMarkersWindow::convertToQImage(unsigned char* buffer) {
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
QImage MeasureMarkersWindow::convertToQImage(cv::Mat img) {
  int w = img.cols;
  int h = img.rows;
  QImage temp(w, h, QImage::Format_ARGB32);
  for (int i = 0; i < h; i++) {
    unsigned char* pCurData = temp.scanLine(i);
    for (int j = 0; j < w; j++) {
      cv::Vec3b clr = img.at<cv::Vec3b>(i, j);

      *(pCurData + j * 4) = clr[0];		//b
      *(pCurData + j * 4 + 1) = clr[1];	//g
      *(pCurData + j * 4 + 2) = clr[2];	//r
      *(pCurData + j * 4 + 3) = 255;	//a
    }
  }
  return temp;
}
//更新场景图像
void MeasureMarkersWindow::updatePixmap(unsigned char* leftBuffer,
                                        unsigned char* rightBuffer) {
  QImage li = convertToQImage(leftBuffer);
  QImage ri = convertToQImage(rightBuffer);
  mLastLeft = li;
  mLastRight = ri;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));
  //更新图像
  update();
}

//更新一帧场景图像
void MeasureMarkersWindow::updateOneFrame() {
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
void MeasureMarkersWindow::setLeftDetailView() {
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
void MeasureMarkersWindow::setRightDetailView() {
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

//预览视频拍摄的画面
void MeasureMarkersWindow::preview() {
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
      mPlay->setText(QString::fromWCharArray(L"停止"));

      //设置定时触发
      mTimer = new QTimer(this);
      connect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->start(5);
      mCapture->setEnabled(true);
      mSlider->setDisabled(true);
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
      mCapture->setDisabled(true);
      mSlider->setEnabled(true);
    }
  }
}

const double PI = 3.1415926;

std::vector<cv::Point2f> circleDetect(cv::Mat img_gray) {
  cv::Mat img_binary;
  cv::threshold(img_gray, img_binary, 60, 255, cv::THRESH_BINARY);
  cv::imwrite("./measure/binary.jpg", img_binary);

  std::vector<cv::Point2f> corners, result;
  cv::goodFeaturesToTrack(img_gray, corners, 200, 0.01, 10);

  std::cout << corners.size() << std::endl;
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
  /*
  cv::imshow("corners", img_rgb);
  cvWaitKey(0);
  */

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
  //加一个直线的判据

  return result;
}


//捕获一张图像
void MeasureMarkersWindow::capture() {
  //先停止时钟
  mTimer->stop();

  static int COUNT = 0;
  COUNT++;
  QString name;
  name.sprintf("%d", COUNT);

  //先保存图像
  QString leftname = mMeasureFolder + "left_" + name;
  QString rightname = mMeasureFolder + "right_" + name;
  mLastLeft.save(leftname + ".jpg");
  mLastRight.save(rightname + ".jpg");
  mImgWidth = mLastLeft.width();
  mImgHeight = mLastLeft.height();

  //读取图像，计算角点
  cv::Mat li = cv::imread(leftname.toStdString() + ".jpg", CV_LOAD_IMAGE_UNCHANGED);
  cv::Mat ri = cv::imread(rightname.toStdString() + ".jpg", CV_LOAD_IMAGE_UNCHANGED);
  cv::Mat liGrey, riGrey;
  cv::cvtColor(li, liGrey, cv::COLOR_BGR2GRAY);
  cv::cvtColor(ri, riGrey, cv::COLOR_BGR2GRAY);

  std::vector<cv::Point2f> left_markers = circleDetect(liGrey);
  std::vector<cv::Point2f> right_markers = circleDetect(riGrey);

  for (int i = 0; i < left_markers.size(); i++)
    cv::circle(li, left_markers[i], 4, cv::Scalar(0, 0, 255), 3, 8, 0);
  for (int i = 0; i < right_markers.size(); i++)
    cv::circle(ri, right_markers[i], 4, cv::Scalar(0, 0, 255), 3, 8, 0);

  //保存绘制的角点图像
  cv::imwrite(leftname.toStdString() + ".bmp", li);
  cv::imwrite(rightname.toStdString() + ".bmp", ri);

  if (left_markers.size() == 2 && right_markers.size() == 2) {
    //先暂停摄像头
    preview();

    //记录当前图像
    mImgs.push_back(COUNT);

    //显示绘制角点的图像
    QImage lp = convertToQImage(li);
    QImage rp = convertToQImage(ri);
    mLeftCameraPixmap->setPixmap(QPixmap::fromImage(lp));
    mRightCameraPixmap->setPixmap(QPixmap::fromImage(rp));

    //提示找到角点
    mStatusBar->showMessage(QString::fromWCharArray(L"成功找到标记点"));

    int all = mImgs.size();
    QString ta;
    ta.sprintf(" %d/%d", all, all);
    mCaps->setText(ta);

    mSlider->setMaximum(mImgs.size());
    mSlider->setValue(mImgs.size());
    updateButtonState();
    return;
  } else {
    //删除保存的图像
    QFile::remove(leftname + ".jpg");
    QFile::remove(rightname + ".jpg");

    //计数回到为初始值
    COUNT--;

    //弹出对话框，提示没有找到角点
    mStatusBar->showMessage(QString::fromWCharArray(L"没有找到标记点"));
    QMessageBox::StandardButton btn2 = QMessageBox::warning(
      0,						//父窗口
      QString::fromWCharArray(L"没有找到标记点"),			//标题栏
      QString::fromWCharArray(L"在当前的图像中，没有找到对应的标记点"));		//文本内容

    //重新开始
    mTimer->start();
  }
}

//删除当前的图像
void MeasureMarkersWindow::deleteImg() {
  if (mImgs.size() > 0) {
    //找到当前选择的文件名
    int index = mSlider->value();
    int name = mImgs[index - 1];

    //删除对应的文件
    QString n;
    n.sprintf("%d", name);
    QFile::remove(mMeasureFolder + "left_" + n + ".jpg");
    QFile::remove(mMeasureFolder + "right_" + n + ".jpg");
    QFile::remove(mMeasureFolder + "left_" + n + ".bmp");
    QFile::remove(mMeasureFolder + "right_" + n + ".bmp");
    QFile::remove(mMeasureFolder + n + ".txt");

    mImgs.erase(mImgs.begin() + index - 1);

    updateButtonState();
    updateCurrent(index);
  }
}

//滑动滑块更新当前选择的
void MeasureMarkersWindow::updateCurrent(int cs) {
  //更新状态信息
  int all = mImgs.size();
  QString ta;
  ta.sprintf(" %d/%d", cs, all);
  mCaps->setText(ta);

  //更新图片显示
  int name = mImgs[cs - 1];
  QString n;
  n.sprintf("%d", name);
  QPixmap iu;
  iu.load(mMeasureFolder + "left_" + n + ".bmp");
  QPixmap id;
  id.load(mMeasureFolder + "right_" + n + ".bmp");
  mLeftCameraPixmap->setPixmap(iu);
  mRightCameraPixmap->setPixmap(id);
}

//创建窗口布局
void MeasureMarkersWindow::createLayout() {
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

  //添加一些功能按钮（上面的布局代码几乎不用动）
  createWidget();
}

//创建按钮
void MeasureMarkersWindow::createWidget() {
  //预览按钮
  mPlay = new QPushButton();
  mPlay->setText(QString::fromWCharArray(L"预览"));
  connect(mPlay, SIGNAL(pressed()), this, SLOT(preview()));

  //标签
  mCaps = new QLabel();
  mCaps->setText(" 0/0");
  mCaps->setAlignment(Qt::AlignRight | Qt::AlignCenter);

  //捕获按钮
  mCapture = new QPushButton();
  mCapture->setText(QString::fromWCharArray(L"捕获"));
  connect(mCapture, SIGNAL(pressed()), this, SLOT(capture()));
  mCapture->setDisabled(true);

  //删除按钮
  mDelete = new QPushButton();
  mDelete->setText(QString::fromWCharArray(L"删除"));
  connect(mDelete, SIGNAL(pressed()), this, SLOT(deleteImg()));
  mDelete->setDisabled(true);

  //滑块
  mSlider = new QSlider(Qt::Horizontal);
  mSlider->setTickPosition(QSlider::TicksBelow);
  mSlider->setMinimum(1);
  mSlider->setMaximum(1);
  mSlider->setTickInterval(1);
  mSlider->setValue(1);
  mSlider->setTracking(true);
  mSlider->setDisabled(true);
  connect(mSlider, SIGNAL(valueChanged(int)), this, SLOT(updateCurrent(int)));

  mCtrlLayout->addWidget(mPlay);
  mCtrlLayout->addWidget(mCaps);
  mCtrlLayout->addWidget(mCapture);
  mCtrlLayout->addWidget(mDelete);

  mTimeLineLayout->addWidget(mSlider);
}

//根据当前捕获的图像，更新各个按钮的状态
void MeasureMarkersWindow::updateButtonState() {
  //更新删除按钮状态
  if (mImgs.size() > 0) {
    mDelete->setEnabled(true);
  } else {
    mDelete->setDisabled(true);
  }
  mSlider->setMinimum(1);
  mSlider->setMaximum(mImgs.size());
  //更新滑块的状态
  if (mImgs.size() > 1) {
    mSlider->setEnabled(true);
  } else {
    mSlider->setDisabled(true);
  }

  //更新标定按钮状态
  //始终可以，提示>5时，才能标定
}