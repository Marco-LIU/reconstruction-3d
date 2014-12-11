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
    mCameraId = 0;	//Ĭ������������ͷ
  else
    mCameraId = 1;
  mbPlay = false;
  mXCorners = 9;
  mYCorners = 9;
  mSquareLength = 91;
  mImgFolders = imgFolder;

  //��������
  createLayout();

  //����Ĭ�ϳ���
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
CalibrateWindow::~CalibrateWindow() {
  //�ر�����ͷ
  if (mbPlay)
    preview();
}
//�ѻҶ�����ת��ΪQImage��ʽ
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
//���³���ͼ��
void CalibrateWindow::updatePixmap(unsigned char* buffer) {
  QImage img = convertToQImage(buffer);
  mLastCaptureImg = img;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(img));
  //����ͼ��
  update();
}

void CalibrateWindow::updatePixmap(QImage& li) {
  mLastCaptureImg = li;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  //����ͼ��
  update();
}

//����һ֡����ͼ��
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
    //����״̬����ʾ
    QString show;
    show.sprintf("��ǰ��֡��Ϊ��%f", fps);
    mStatusBar->showMessage(show);
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
void CalibrateWindow::setLeftDetailView() {
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
void CalibrateWindow::setRightDetailView() {
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
void CalibrateWindow::preview() {
  //���û����������ͷ������������ʼԤ��
  if (mbPlay == false) {
    mCameras = new UsbCameraGroup();
    bool succ = mCameras->Init("para.json");

    //�������ʧ�ܣ���ʾ�����û�����Ӻ�
    if (!succ || mCameras->camera_count() != 2) {
      mCameras = NULL;
      mPlay->setText("δ����");
    
      QMessageBox::critical(
        0,							//������
        QString::fromWCharArray(L"�Ҳ������õ�����ͷ"),		//������
        QString::fromWCharArray(L"�Ҳ������õ�����ͷ����鿴����ͷ�Ƿ��Ѿ����ӵ����ԣ����Ѿ����ӣ������²��USB�ӿ�"));		//�ı�����
    
    }
    //�ɹ�����
    else {
      mCameras->StartAll();
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
      mCameras->StopAll();
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"Ԥ��"));
      mCapture->setDisabled(true);
      mSlider->setEnabled(true);
    }
  }
}
//����һ��ͼ��
void CalibrateWindow::capture() {
  //��ֹͣʱ��
  mTimer->stop();

  static int COUNT = 0;
  COUNT++;
  QString name;
  name.sprintf("%d", COUNT);

  //�ȱ���ͼ��
  bool succ = mLastCaptureImg.save(mImgFolders + name + ".jpg");
  mImgWidth = mLastCaptureImg.width();
  mImgHeight = mLastCaptureImg.height();

  //��ȡͼ�񣬼���ǵ�
  std::string imgname = mImgFolders.toStdString() + name.toStdString() + ".jpg";
  cv::Mat ti = cv::imread(imgname, CV_LOAD_IMAGE_UNCHANGED);
  cv::Mat clrImg = ti.clone();
  cv::cvtColor(ti, ti, cv::COLOR_BGR2GRAY);

  cv::vector<cv::Point2f> corners = CameraCalibration::findCorners(ti, mXCorners, mYCorners);

  //����ҵ��˽ǵ�
  if (!corners.empty()) {
    //����ͣ����ͷ
    preview();

    //����ǵ�
    std::string corname = mImgFolders.toStdString() + name.toStdString() + ".txt";
    saveCornerPoints(corname, corners);

    //���ƽǵ�
    cv::drawChessboardCorners(clrImg, cv::Size(mXCorners, mYCorners), corners, true);
    cv::circle(clrImg, corners[0], 15, cv::Scalar(255, 0, 0), 3);

    //������ƵĽǵ�ͼ��
    std::string corImgName = mImgFolders.toStdString() + name.toStdString() + ".bmp";
    cv::imwrite(corImgName, clrImg);

    //��¼��ǰͼ��
    mImgs.push_back(COUNT);

    //��ʾ���ƽǵ��ͼ��
    QPixmap tp;
    tp.load(mImgFolders + name + ".bmp");
    mRightCameraPixmap->setPixmap(tp);
    setRightDetailView();

    //��ʾ�ҵ��ǵ�
    mStatusBar->showMessage(QString::fromWCharArray(L"�ɹ��ҵ��ǵ�"));

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
    QFile::remove(mImgFolders + name + ".jpg");

    //�����ص�Ϊ��ʼֵ
    COUNT--;

    //�����Ի�����ʾû���ҵ��ǵ�
    mStatusBar->showMessage(QString::fromWCharArray(L"û���ҵ��ǵ�"));
    QMessageBox::StandardButton btn2 = QMessageBox::warning(
      0,						//������
      QString::fromWCharArray(L"û���ҵ��ǵ�"),			//������
      QString::fromWCharArray(L"�ڵ�ǰ��ͼ���У�û���ҵ���Ӧ�����̸�ǵ�"));		//�ı�����

    //���¿�ʼ
    mTimer->start();
  }
}
//ɾ����ǰ��ͼ��
void CalibrateWindow::deleteImg() {
  if (mImgs.size() > 0) {
    //�ҵ���ǰѡ����ļ���
    int index = mSlider->value();
    int name = mImgs[index - 1];

    //ɾ����Ӧ���ļ�
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
//����x����Ľǵ����
void CalibrateWindow::setSizeX() {
  if (mbPlay)
    mTimer->stop();

  bool ok = false;
  int tx = QInputDialog::getInt(
    0,
    QString::fromWCharArray(L"����X����Ľǵ����"),
    QString::fromWCharArray(L"����X����Ľǵ����"),
    mXCorners,
    2,
    25,
    1,
    &ok);
  if (ok) {
    mXCorners = tx;
  }

  //������ʾ
  QString t;
  t.sprintf("%d", mXCorners);
  mSizeX->setText(t);

  if (mbPlay)
    mTimer->start();
}
//����y����Ľǵ����
void CalibrateWindow::setSizeY() {
  if (mbPlay)
    mTimer->stop();

  bool ok = false;
  int tx = QInputDialog::getInt(
    0,
    QString::fromWCharArray(L"����Y����Ľǵ����"),
    QString::fromWCharArray(L"����Y����Ľǵ����"),
    mYCorners,
    2,
    25,
    1,
    &ok);
  if (ok) {
    mYCorners = tx;
  }

  //������ʾ
  QString t;
  t.sprintf("%d", mYCorners);
  mSizeY->setText(t);

  if (mbPlay)
    mTimer->start();
}
//�������̸�ĳ���
void CalibrateWindow::setLength() {
  if (mbPlay)
    mTimer->stop();

  bool ok = false;
  int tx = QInputDialog::getInt(
    0,
    "�������̸�ĳ���",
    "�������̸�ĳ��ȣ���λΪ����",
    mSquareLength,
    5,
    250,
    1,
    &ok);
  if (ok) {
    mSquareLength = tx;
  }

  //������ʾ
  QString t;
  t.sprintf("%d", mSquareLength);
  mLength->setText(t);

  if (mbPlay)
    mTimer->start();
}
//���б궨
void CalibrateWindow::calibrate() {
  //С��5�������б궨
  if (mImgs.size() < 5) {
    QMessageBox::StandardButton btn2 = QMessageBox::warning(
      0,						//������
      QString::fromWCharArray(L"���ܽ��б궨"),			//������
      QString::fromWCharArray(L"��ǰ�����ͼ��̫�٣����ܽ��б궨�����ٲ���5��ͼ�񣬲��ܽ��б궨"));		//�ı�����
  } else {
    //todo ����һ���ȴ��Ի���
    mStatusBar->showMessage(QString::fromWCharArray(L"���ڱ궨�ڲΣ������ĵȴ�������"));

    std::vector<std::vector<cv::Point2f>> CamCorners;		//�����ͼ��ǵ�����
    std::vector<std::vector<cv::Point3f>> ObjCorners;		//��¼����㣨��������ϵ��
    cv::vector<cv::Mat> camRotationVectors;
    cv::vector<cv::Mat> camTranslationVectors;

    //�������еĽǵ�����
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

    cv::Mat camMatrix;			//�ڲ�,3x3����
    cv::Mat distortion;			//����ϵ��,1x5����

    //Ŀǰ�ľ�ͷ���ʹ��k1,k2,p1��p2
    double error = cv::calibrateCamera(
      ObjCorners,
      CamCorners,
      cv::Size(mImgWidth, mImgHeight),
      camMatrix,
      distortion,
      camRotationVectors, camTranslationVectors,
      CV_CALIB_FIX_ASPECT_RATIO + CV_CALIB_FIX_K3,
      cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 100, DBL_EPSILON));

    //�����ڲ�
    std::string matrixName = mImgFolders.toStdString() + "matrix.txt";
    Utilities::exportMat(matrixName.c_str(), camMatrix);
    std::string distortionName = mImgFolders.toStdString() + "distortion.txt";
    Utilities::exportMat(distortionName.c_str(), distortion);
    //����ʹ����Щ�ļ��궨���ڲ�
    std::ofstream of(mImgFolders.toStdString() + "info.txt");
    for (int i = 0; i < mImgs.size(); i++) {
      of << mImgs[i] << std::endl;
    }
    of.close();

    QString msg;
    msg.sprintf("%f", error);
    QMessageBox::about(
      0,				//������
      QString::fromWCharArray(L"�궨�ڲ�"),		//������
      QString::fromWCharArray(L"�ڲα궨��ɣ�ƽ���������Ϊ��") + msg	//�ı�����
    );
  }
}
//����������µ�ǰѡ���
void CalibrateWindow::updateCurrent(int cs) {
  //����״̬��Ϣ
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

  //����ͼƬ��ʾ
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

//�ѽǵ㱣�浽�ļ���,ǰ��3����xCorners,yCorners,Length,����������ռ����꣬������ͶӰͼ������
void CalibrateWindow::saveCornerPoints(std::string filename, std::vector<cv::Point2f> pts) {
  std::ofstream obj(filename);

  //д��ǵ���Ϣ
  obj << mXCorners << std::endl;
  obj << mYCorners << std::endl;
  obj << mSquareLength << std::endl;

  //д������ռ�����
  int z = 0;
  for (int i = 0; i < mYCorners; i++) {
    int y = i*mSquareLength;
    for (int j = 0; j < mXCorners; j++) {
      int x = j*mSquareLength;
      obj << x << '\t' << y << '\t' << z << std::endl;
    }
  }

  //д��ͶӰͼ������
  for (int i = 0; i < pts.size(); i++) {
    obj << pts[i].x << '\t' << pts[i].y << std::endl;
  }

  obj.close();
}

//�������еĽǵ�����
void CalibrateWindow::loadCornerPoints(std::string filename, std::vector<cv::Point2f>& pts, std::vector<cv::Point3f>& opts) {
  std::ifstream obj(filename);

  //��ȡ�ǵ���Ϣ
  int x, y, l;
  obj >> x;
  obj >> y;
  obj >> l;

  //��ȡ����ռ�����
  for (int i = 0; i < x*y; i++) {
    int ox, oy, oz;
    obj >> ox;
    obj >> oy;
    obj >> oz;
    opts.push_back(cv::Point3f(ox, oy, oz));
  }

  //��ȡͼ��ͶӰ����
  for (int i = 0; i < x*y; i++) {
    float ix, iy;
    obj >> ix;
    obj >> iy;
    pts.push_back(cv::Point2f(ix, iy));
  }

  obj.close();
}
//�������ڲ���
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
void CalibrateWindow::createWidget() {
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

  //���ô�С
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

  //�궨
  mCalibrate = new QPushButton();
  mCalibrate->setText(QString::fromWCharArray(L"�궨"));
  connect(mCalibrate, SIGNAL(pressed()), this, SLOT(calibrate()));

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
  mCtrlLayout->addWidget(mSizeXLabel);
  mCtrlLayout->addWidget(mSizeX);
  mCtrlLayout->addWidget(mSizeYLabel);
  mCtrlLayout->addWidget(mSizeY);
  mCtrlLayout->addWidget(mLengthLabel);
  mCtrlLayout->addWidget(mLength);
  mCtrlLayout->addWidget(mCalibrate);

  mTimeLineLayout->addWidget(mSlider);
}

//���ݵ�ǰ�����ͼ�񣬸��¸�����ť��״̬
void CalibrateWindow::updateButtonState() {
  //����ɾ����ť״̬
  if (mImgs.size() > 0) {
    mDelete->setEnabled(true);
  } else {
    mDelete->setDisabled(true);
  }

  //���»����״̬
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
  //���±궨��ť״̬
  //ʼ�տ��ԣ���ʾ>5ʱ�����ܱ궨
}