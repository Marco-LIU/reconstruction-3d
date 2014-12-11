#include "calibrateWindow.h"
#include <QtWidgets\qapplication.h>
#include <QtWidgets\qmainwindow.h>
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

#include <QtCore\qtimer.h>
#include <QtCore\qtextstream.h>
#include <QtCore\qtextcodec.h>

#include <QtGui\qtransform.h>
#include <QtGui\qevent.h>
#include <QtGui\qpainter.h>
#include <QtGui\qimagewriter.h>

#include <vector>

#include "UsbCameras.h"
#include "myCameraView.h"
#include "marker.h"
#include "paras.h"
#include "CameraCalibration.h"
#include "usb_camera_group.h"
#include "camera_frame.h"

CalibrateWindow::CalibrateWindow(QGraphicsScene* scene,
                                 QGraphicsPixmapItem* left,
                                 QGraphicsPixmapItem* right,
                                 QStatusBar* status,
                                 QString imgFolder,
                                 bool bl) {
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;
  if (bl)
    mCameraId = 0;	//默认启用左摄像头
  else
    mCameraId = 1;
  mbPlay = false;
  mXCorners = 9;
  mYCorners = 9;
  mSquareLength = 91;
  mImgFolders = imgFolder;

  //创建布局
  createLayout();

  //设置默认场景
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
CalibrateWindow::~CalibrateWindow() {
  //关闭摄像头
  if (mbPlay)
    preview();
}
//把灰度数据转换为QImage格式
QImage CalibrateWindow::convertToQImage(unsigned char* buffer) {
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
void CalibrateWindow::updatePixmap(unsigned char* buffer) {
  QImage img = convertToQImage(buffer);
  mLastCaptureImg = img;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(img));
  //更新图像
  update();
}

void CalibrateWindow::updatePixmap(QImage& li) {
  mLastCaptureImg = li;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  //更新图像
  update();
}

//更新一帧场景图像
void CalibrateWindow::updateOneFrame() {
  static int FrameCount = 0;
  static int StartTime = mProTimer.getMilliseconds();

  CameraFrames frames;
  mCameras->GetFrames(frames);
  if (frames.size() == 2) {
    FrameCount++;

    FrameCount++;
    updatePixmap(frames[0].ToQImage());

    float fps = mCameras->FrameRate();
    //更新状态栏显示
    QString show;
    show.sprintf("当前的帧率为：%f", fps);
    mStatusBar->showMessage(show);
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
void CalibrateWindow::setLeftDetailView() {
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
void CalibrateWindow::setRightDetailView() {
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
void CalibrateWindow::preview() {
  //如果没有启动摄像头，启动，并开始预览
  if (mbPlay == false) {
    mCameras = new UsbCameraGroup();
    bool succ = mCameras->Init("para.json");

    //如果启动失败，提示摄像机没有连接好
    if (!succ || mCameras->camera_count() != 2) {
      mCameras = NULL;
      mPlay->setText("未连接");
    
      QMessageBox::critical(
        0,							//父窗口
        QString::fromWCharArray(L"找不到可用的摄像头"),		//标题栏
        QString::fromWCharArray(L"找不到可用的摄像头，请查看摄像头是否已经连接到电脑，如已经连接，请重新插拔USB接口"));		//文本内容
    
    }
    //成功启动
    else {
      mCameras->StartAll();
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
      mCameras->StopAll();
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"预览"));
      mCapture->setDisabled(true);
      mSlider->setEnabled(true);
    }
  }
}
//捕获一张图像
void CalibrateWindow::capture() {
  //先停止时钟
  mTimer->stop();

  static int COUNT = 0;
  COUNT++;
  QString name;
  name.sprintf("%d", COUNT);

  //先保存图像
  bool succ = mLastCaptureImg.save(mImgFolders + name + ".jpg");
  mImgWidth = mLastCaptureImg.width();
  mImgHeight = mLastCaptureImg.height();

  //读取图像，计算角点
  std::string imgname = mImgFolders.toStdString() + name.toStdString() + ".jpg";
  cv::Mat ti = cv::imread(imgname, CV_LOAD_IMAGE_UNCHANGED);
  cv::Mat clrImg = ti.clone();
  cv::cvtColor(ti, ti, cv::COLOR_BGR2GRAY);

  cv::vector<cv::Point2f> corners = CameraCalibration::findCorners(ti, mXCorners, mYCorners);

  //如果找到了角点
  if (!corners.empty()) {
    //先暂停摄像头
    preview();

    //保存角点
    std::string corname = mImgFolders.toStdString() + name.toStdString() + ".txt";
    saveCornerPoints(corname, corners);

    //绘制角点
    cv::drawChessboardCorners(clrImg, cv::Size(mXCorners, mYCorners), corners, true);
    cv::circle(clrImg, corners[0], 15, cv::Scalar(255, 0, 0), 3);

    //保存绘制的角点图像
    std::string corImgName = mImgFolders.toStdString() + name.toStdString() + ".bmp";
    cv::imwrite(corImgName, clrImg);

    //记录当前图像
    mImgs.push_back(COUNT);

    //显示绘制角点的图像
    QPixmap tp;
    tp.load(mImgFolders + name + ".bmp");
    mRightCameraPixmap->setPixmap(tp);
    setRightDetailView();

    //提示找到角点
    mStatusBar->showMessage(QString::fromWCharArray(L"成功找到角点"));

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
    QFile::remove(mImgFolders + name + ".jpg");

    //计数回到为初始值
    COUNT--;

    //弹出对话框，提示没有找到角点
    mStatusBar->showMessage(QString::fromWCharArray(L"没有找到角点"));
    QMessageBox::StandardButton btn2 = QMessageBox::warning(
      0,						//父窗口
      QString::fromWCharArray(L"没有找到角点"),			//标题栏
      QString::fromWCharArray(L"在当前的图像中，没有找到对应的棋盘格角点"));		//文本内容

    //重新开始
    mTimer->start();
  }
}
//删除当前的图像
void CalibrateWindow::deleteImg() {
  if (mImgs.size() > 0) {
    //找到当前选择的文件名
    int index = mSlider->value();
    int name = mImgs[index - 1];

    //删除对应的文件
    QString n;
    n.sprintf("%d", name);
    QFile::remove(mImgFolders + n + ".jpg");
    QFile::remove(mImgFolders + n + ".bmp");
    QFile::remove(mImgFolders + n + ".txt");

    mImgs.erase(mImgs.begin() + index - 1);

    updateButtonState();
  if(index>1)
    updateCurrent(index-1);
  else
    updateCurrent(index);
  }
}
//设置x方向的角点个数
void CalibrateWindow::setSizeX() {
  if (mbPlay)
    mTimer->stop();

  bool ok = false;
  int tx = QInputDialog::getInt(
    0,
    QString::fromWCharArray(L"设置X方向的角点个数"),
    QString::fromWCharArray(L"输入X方向的角点个数"),
    mXCorners,
    2,
    25,
    1,
    &ok);
  if (ok) {
    mXCorners = tx;
  }

  //更新显示
  QString t;
  t.sprintf("%d", mXCorners);
  mSizeX->setText(t);

  if (mbPlay)
    mTimer->start();
}
//设置y方向的角点个数
void CalibrateWindow::setSizeY() {
  if (mbPlay)
    mTimer->stop();

  bool ok = false;
  int tx = QInputDialog::getInt(
    0,
    QString::fromWCharArray(L"设置Y方向的角点个数"),
    QString::fromWCharArray(L"输入Y方向的角点个数"),
    mYCorners,
    2,
    25,
    1,
    &ok);
  if (ok) {
    mYCorners = tx;
  }

  //更新显示
  QString t;
  t.sprintf("%d", mYCorners);
  mSizeY->setText(t);

  if (mbPlay)
    mTimer->start();
}
//设置棋盘格的长度
void CalibrateWindow::setLength() {
  if (mbPlay)
    mTimer->stop();

  bool ok = false;
  int tx = QInputDialog::getInt(
    0,
    "设置棋盘格的长度",
    "输入棋盘格的长度，单位为毫米",
    mSquareLength,
    5,
    250,
    1,
    &ok);
  if (ok) {
    mSquareLength = tx;
  }

  //更新显示
  QString t;
  t.sprintf("%d", mSquareLength);
  mLength->setText(t);

  if (mbPlay)
    mTimer->start();
}
//进行标定
void CalibrateWindow::calibrate() {
  //小于5，不进行标定
  if (mImgs.size() < 5) {
    QMessageBox::StandardButton btn2 = QMessageBox::warning(
      0,						//父窗口
      QString::fromWCharArray(L"不能进行标定"),			//标题栏
      QString::fromWCharArray(L"当前捕获的图像太少，不能进行标定。至少捕获5张图像，才能进行标定"));		//文本内容
  } else {
    //todo 弹出一个等待对话框
    mStatusBar->showMessage(QString::fromWCharArray(L"正在标定内参，请耐心等待几分钟"));

    std::vector<std::vector<cv::Point2f>> CamCorners;		//摄像机图像角点坐标
    std::vector<std::vector<cv::Point3f>> ObjCorners;		//记录定标点（世界坐标系）
    cv::vector<cv::Mat> camRotationVectors;
    cv::vector<cv::Mat> camTranslationVectors;

    //载入所有的角点坐标
    for (int i = 0; i < mImgs.size(); i++) {
      int n = mImgs[i];
      QString name;
      name.sprintf("%d.txt", n);

      std::vector<cv::Point2f> ipts;
      std::vector<cv::Point3f> opts;
      loadCornerPoints(mImgFolders.toStdString() + name.toStdString(), ipts, opts);
      ObjCorners.push_back(opts);
      CamCorners.push_back(ipts);
    }

    cv::Mat camMatrix;			//内参,3x3矩阵
    cv::Mat distortion;			//畸变系数,1x5矩阵

    //目前的镜头最好使用k1,k2,p1和p2
    double error = cv::calibrateCamera(
      ObjCorners,
      CamCorners,
      cv::Size(mImgWidth, mImgHeight),
      camMatrix,
      distortion,
      camRotationVectors, camTranslationVectors,
      CV_CALIB_FIX_ASPECT_RATIO + CV_CALIB_FIX_K3,
      cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 100, DBL_EPSILON));

    //保存内参
    std::string matrixName = mImgFolders.toStdString() + "matrix.txt";
    Utilities::exportMat(matrixName.c_str(), camMatrix);
    std::string distortionName = mImgFolders.toStdString() + "distortion.txt";
    Utilities::exportMat(distortionName.c_str(), distortion);
    //保存使用哪些文件标定的内参
    std::ofstream of(mImgFolders.toStdString() + "info.txt");
    for (int i = 0; i < mImgs.size(); i++) {
      of << mImgs[i] << std::endl;
    }
    of.close();

    QString msg;
    msg.sprintf("%f", error);
    QMessageBox::about(
      0,				//父窗口
      QString::fromWCharArray(L"标定内参"),		//标题栏
      QString::fromWCharArray(L"内参标定完成，平均像素误差为：") + msg	//文本内容
    );
  }
}
//滑动滑块更新当前选择的
void CalibrateWindow::updateCurrent(int cs) {
  //更新状态信息
  int all = mImgs.size();
  if(all == 0)
  {
    QString ta;
    ta.sprintf(" %d/%d", 0, 0);
    mCaps->setText(ta);
    return;
  }
  QString ta;
  ta.sprintf(" %d/%d", cs, all);
  mCaps->setText(ta);

  //更新图片显示
  int name = mImgs[cs - 1];
  QString n;
  n.sprintf("%d", name);
  QPixmap iu;
  iu.load(mImgFolders + n + ".jpg");
  QPixmap id;
  id.load(mImgFolders + n + ".bmp");
  mLeftCameraPixmap->setPixmap(iu);
  mRightCameraPixmap->setPixmap(id);
}

//把角点保存到文件中,前面3行是xCorners,yCorners,Length,接着是物体空间坐标，接着是投影图像坐标
void CalibrateWindow::saveCornerPoints(std::string filename, std::vector<cv::Point2f> pts) {
  std::ofstream obj(filename);

  //写入角点信息
  obj << mXCorners << std::endl;
  obj << mYCorners << std::endl;
  obj << mSquareLength << std::endl;

  //写入物体空间坐标
  int z = 0;
  for (int i = 0; i < mYCorners; i++) {
    int y = i*mSquareLength;
    for (int j = 0; j < mXCorners; j++) {
      int x = j*mSquareLength;
      obj << x << '\t' << y << '\t' << z << std::endl;
    }
  }

  //写入投影图像坐标
  for (int i = 0; i < pts.size(); i++) {
    obj << pts[i].x << '\t' << pts[i].y << std::endl;
  }

  obj.close();
}

//载入所有的角点坐标
void CalibrateWindow::loadCornerPoints(std::string filename, std::vector<cv::Point2f>& pts, std::vector<cv::Point3f>& opts) {
  std::ifstream obj(filename);

  //读取角点信息
  int x, y, l;
  obj >> x;
  obj >> y;
  obj >> l;

  //读取物体空间坐标
  for (int i = 0; i < x*y; i++) {
    int ox, oy, oz;
    obj >> ox;
    obj >> oy;
    obj >> oz;
    opts.push_back(cv::Point3f(ox, oy, oz));
  }

  //读取图像投影坐标
  for (int i = 0; i < x*y; i++) {
    float ix, iy;
    obj >> ix;
    obj >> iy;
    pts.push_back(cv::Point2f(ix, iy));
  }

  obj.close();
}
//创建窗口布局
void CalibrateWindow::createLayout() {
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
void CalibrateWindow::createWidget() {
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

  //设置大小
  mSizeXLabel = new QLabel();
  mSizeXLabel->setText(" X:");
  mSizeXLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
  mSizeX = new QPushButton();
  mSizeX->setMaximumWidth(30);
  QString t;
  t.sprintf("%d", mXCorners);
  mSizeX->setText(t);
  connect(mSizeX, SIGNAL(pressed()), this, SLOT(setSizeX()));

  mSizeYLabel = new QLabel();
  mSizeYLabel->setText(" Y:");
  mSizeYLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
  mSizeY = new QPushButton();
  t.sprintf("%d", mYCorners);
  mSizeY->setText(t);
  connect(mSizeY, SIGNAL(pressed()), this, SLOT(setSizeY()));
  mSizeY->setMaximumWidth(30);

  mLengthLabel = new QLabel();
  mLengthLabel->setText(" L:");
  mLengthLabel->setAlignment(Qt::AlignRight | Qt::AlignCenter);
  mLength = new QPushButton();
  t.sprintf("%d", mSquareLength);
  mLength->setText(t);
  connect(mLength, SIGNAL(pressed()), this, SLOT(setLength()));
  mLength->setMaximumWidth(40);

  //标定
  mCalibrate = new QPushButton();
  mCalibrate->setText(QString::fromWCharArray(L"标定"));
  connect(mCalibrate, SIGNAL(pressed()), this, SLOT(calibrate()));

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
  mCtrlLayout->addWidget(mSizeXLabel);
  mCtrlLayout->addWidget(mSizeX);
  mCtrlLayout->addWidget(mSizeYLabel);
  mCtrlLayout->addWidget(mSizeY);
  mCtrlLayout->addWidget(mLengthLabel);
  mCtrlLayout->addWidget(mLength);
  mCtrlLayout->addWidget(mCalibrate);

  mTimeLineLayout->addWidget(mSlider);
}

//根据当前捕获的图像，更新各个按钮的状态
void CalibrateWindow::updateButtonState() {
  //更新删除按钮状态
  if (mImgs.size() > 0) {
    mDelete->setEnabled(true);
  } else {
    mDelete->setDisabled(true);
  }

  //更新滑块的状态
  if (mImgs.size() > 1) {
    mSlider->setEnabled(true);
  } else {
    mSlider->setDisabled(true);
  }

  if(mImgs.size()!=0)
  {
  mSlider->setMinimum(1);
  mSlider->setMaximum(mImgs.size());
  }
  //更新标定按钮状态
  //始终可以，提示>5时，才能标定
}