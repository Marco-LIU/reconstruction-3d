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

  //��������
  createLayout();

  //����Ĭ�ϳ���
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}

MeasureMarkersWindow::~MeasureMarkersWindow() {
  //�ر�����ͷ
  if (mbPlay)
    preview();
}

//�ѻҶ�����ת��ΪQImage��ʽ
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
//���³���ͼ��
void MeasureMarkersWindow::updatePixmap(unsigned char* leftBuffer,
                                        unsigned char* rightBuffer) {
  QImage li = convertToQImage(leftBuffer);
  QImage ri = convertToQImage(rightBuffer);
  mLastLeft = li;
  mLastRight = ri;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));
  //����ͼ��
  update();
}

//����һ֡����ͼ��
void MeasureMarkersWindow::updateOneFrame() {
  static int FrameCount = 0;
  static int StartTime = mProTimer.getMilliseconds();

  if (mCameras->captureTwoFrameSyncSoftControl(0, 1)) {
    FrameCount++;

    updatePixmap(mCameras->getBuffer(0), mCameras->getBuffer(1));
  }

  //ÿ20֡��ͳ����֡��
  if (FrameCount == 20) {
    int endTime = mProTimer.getMilliseconds();
    float fps = (float)FrameCount * 1000 / (endTime - StartTime);

    //����״̬����ʾ
    QString show;
    show.sprintf("��ǰ��֡��Ϊ��%f", fps);
    mStatusBar->showMessage(show);

    //��0
    FrameCount = 0;
    StartTime = mProTimer.getMilliseconds();
  }
}

//������ϸ��ͼΪ����ͼ
void MeasureMarkersWindow::setLeftDetailView() {
  //����Ѿ�������ͼ��ֱ�ӷ���
  if (mbLeftFocused) {
    return;
  }
  //���������ͼ�л�
  else {
    mbLeftFocused = true;
    //��������ͼ��ǰ��״̬
    mRightCameraView->saveZoomViewState();
    //�л�������ͼ
    mLeftCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mLeftCameraView->getSavedState());
  }
}
//������ϸ��ͼΪ����ͼ
void MeasureMarkersWindow::setRightDetailView() {
  //���������ͼ���л�������ͼ
  if (mbLeftFocused) {
    mbLeftFocused = false;
    //��������ͼ��ǰ��״̬
    mLeftCameraView->saveZoomViewState();
    //�л�������ͼ
    mRightCameraView->setZoomView(mZoomView);
    mZoomView->restoreViewState(mRightCameraView->getSavedState());
  } else {
    return;
  }
}

//Ԥ����Ƶ����Ļ���
void MeasureMarkersWindow::preview() {
  //���û����������ͷ������������ʼԤ��
  if (mbPlay == false) {
    mCameras = new UsbCameras("para.config");

    //�������ʧ�ܣ���ʾ�����û�����Ӻ�
    if (mCameras->getCameraCount() != 2) {
      delete mCameras;
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"ֹͣ"));
      QMessageBox::critical(
        0,							//������
        QString::fromWCharArray(L"�Ҳ������õ�����ͷ"),		//������
        QString::fromWCharArray(L"�Ҳ������õ�����ͷ����鿴����ͷ�Ƿ��Ѿ����ӵ����ԣ����Ѿ����ӣ������²��USB�ӿ�"));		//�ı�����
    }
    //�ɹ�����
    else {
      mCameras->setTriggerMode(true);

      //�л�������ť״̬
      mbPlay = true;
      mPlay->setText(QString::fromWCharArray(L"ֹͣ"));

      //���ö�ʱ����
      mTimer = new QTimer(this);
      connect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->start(5);
      mCapture->setEnabled(true);
      mSlider->setDisabled(true);
    }
  }
  //����ɹ���������ͷ����ر���
  else {
    if (mCameras != NULL) {
      mbPlay = false;
      disconnect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->stop();
      delete mTimer;
      mTimer = NULL;
      delete mCameras;
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"Ԥ��"));
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
  //�Խǵ���о���

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

  //�������ؽǵ�
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
    std::vector<int> mean;	//Բ�ܵĹ�һ��ֵ
    int x = corners[i].x, y = corners[i].y, judge = 1;

    for (int k = 5; k<10 && judge; k++) {
      //num��Բ�ܵ����ص��������ǵ�ͼ����������ɢ�ģ�����ֻȡ��2��R���㣬R��ֱ��
      //sum��Բ�ܵ����ض�ֵ��ֵ�Ӻͣ�255����0
      //reverse_time��ͳ��Բ�����ر任���ԽǱ�־�Ļ���任4�Σ�����С��4��ȫ����̭
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
  //��һ��ֱ�ߵ��о�

  return result;
}


//����һ��ͼ��
void MeasureMarkersWindow::capture() {
  //��ֹͣʱ��
  mTimer->stop();

  static int COUNT = 0;
  COUNT++;
  QString name;
  name.sprintf("%d", COUNT);

  //�ȱ���ͼ��
  QString leftname = mMeasureFolder + "left_" + name;
  QString rightname = mMeasureFolder + "right_" + name;
  mLastLeft.save(leftname + ".jpg");
  mLastRight.save(rightname + ".jpg");
  mImgWidth = mLastLeft.width();
  mImgHeight = mLastLeft.height();

  //��ȡͼ�񣬼���ǵ�
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

  //������ƵĽǵ�ͼ��
  cv::imwrite(leftname.toStdString() + ".bmp", li);
  cv::imwrite(rightname.toStdString() + ".bmp", ri);

  if (left_markers.size() == 2 && right_markers.size() == 2) {
    //����ͣ����ͷ
    preview();

    //��¼��ǰͼ��
    mImgs.push_back(COUNT);

    //��ʾ���ƽǵ��ͼ��
    QImage lp = convertToQImage(li);
    QImage rp = convertToQImage(ri);
    mLeftCameraPixmap->setPixmap(QPixmap::fromImage(lp));
    mRightCameraPixmap->setPixmap(QPixmap::fromImage(rp));

    //��ʾ�ҵ��ǵ�
    mStatusBar->showMessage(QString::fromWCharArray(L"�ɹ��ҵ���ǵ�"));

    int all = mImgs.size();
    QString ta;
    ta.sprintf(" %d/%d", all, all);
    mCaps->setText(ta);

    mSlider->setMaximum(mImgs.size());
    mSlider->setValue(mImgs.size());
    updateButtonState();
    return;
  } else {
    //ɾ�������ͼ��
    QFile::remove(leftname + ".jpg");
    QFile::remove(rightname + ".jpg");

    //�����ص�Ϊ��ʼֵ
    COUNT--;

    //�����Ի�����ʾû���ҵ��ǵ�
    mStatusBar->showMessage(QString::fromWCharArray(L"û���ҵ���ǵ�"));
    QMessageBox::StandardButton btn2 = QMessageBox::warning(
      0,						//������
      QString::fromWCharArray(L"û���ҵ���ǵ�"),			//������
      QString::fromWCharArray(L"�ڵ�ǰ��ͼ���У�û���ҵ���Ӧ�ı�ǵ�"));		//�ı�����

    //���¿�ʼ
    mTimer->start();
  }
}

//ɾ����ǰ��ͼ��
void MeasureMarkersWindow::deleteImg() {
  if (mImgs.size() > 0) {
    //�ҵ���ǰѡ����ļ���
    int index = mSlider->value();
    int name = mImgs[index - 1];

    //ɾ����Ӧ���ļ�
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

//����������µ�ǰѡ���
void MeasureMarkersWindow::updateCurrent(int cs) {
  //����״̬��Ϣ
  int all = mImgs.size();
  QString ta;
  ta.sprintf(" %d/%d", cs, all);
  mCaps->setText(ta);

  //����ͼƬ��ʾ
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

//�������ڲ���
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

  //���ó�����ͼ
  //���������ͼ
  int mWidth = Paras::getSingleton().width;
  int mHeight = Paras::getSingleton().height;
  mLeftCameraView = new MyCameraView(mScene, QRect(0, 0, mWidth, mHeight));
  //���������ͼ
  mRightCameraView = new MyCameraView(mScene, QRect(mWidth * 2, 0, mWidth, mHeight));
  //��ý���ʱ���л�������ͼ
  connect(mLeftCameraView, SIGNAL(getFocusSignal()), this, SLOT(setLeftDetailView()));
  connect(mRightCameraView, SIGNAL(getFocusSignal()), this, SLOT(setRightDetailView()));
  mStereoLayout->addWidget(mLeftCameraView);
  mStereoLayout->addWidget(mRightCameraView);

  //������ť
  mCtrlLayout = new QHBoxLayout();
  mDetailLayout->addLayout(mCtrlLayout);

  //����������ͼ��Ĭ�ϳ�ʼ��������ͼ
  mZoomView = new MyDetailView(mScene, QRect(0, 0, mWidth, mHeight));
  mLeftCameraView->setZoomView(mZoomView);
  mbLeftFocused = true;
  mDetailLayout->addWidget(mZoomView);

  //���һЩ���ܰ�ť������Ĳ��ִ��뼸�����ö���
  createWidget();
}

//������ť
void MeasureMarkersWindow::createWidget() {
  //Ԥ����ť
  mPlay = new QPushButton();
  mPlay->setText(QString::fromWCharArray(L"Ԥ��"));
  connect(mPlay, SIGNAL(pressed()), this, SLOT(preview()));

  //��ǩ
  mCaps = new QLabel();
  mCaps->setText(" 0/0");
  mCaps->setAlignment(Qt::AlignRight | Qt::AlignCenter);

  //����ť
  mCapture = new QPushButton();
  mCapture->setText(QString::fromWCharArray(L"����"));
  connect(mCapture, SIGNAL(pressed()), this, SLOT(capture()));
  mCapture->setDisabled(true);

  //ɾ����ť
  mDelete = new QPushButton();
  mDelete->setText(QString::fromWCharArray(L"ɾ��"));
  connect(mDelete, SIGNAL(pressed()), this, SLOT(deleteImg()));
  mDelete->setDisabled(true);

  //����
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

//���ݵ�ǰ�����ͼ�񣬸��¸�����ť��״̬
void MeasureMarkersWindow::updateButtonState() {
  //����ɾ����ť״̬
  if (mImgs.size() > 0) {
    mDelete->setEnabled(true);
  } else {
    mDelete->setDisabled(true);
  }
  mSlider->setMinimum(1);
  mSlider->setMaximum(mImgs.size());
  //���»����״̬
  if (mImgs.size() > 1) {
    mSlider->setEnabled(true);
  } else {
    mSlider->setDisabled(true);
  }

  //���±궨��ť״̬
  //ʼ�տ��ԣ���ʾ>5ʱ�����ܱ궨
}