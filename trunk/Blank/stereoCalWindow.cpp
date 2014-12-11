#include "stereoCalWindow.h"

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
#include "usb_camera_group.h"
#include "camera_frame.h"

StereoCalibrationWindow::StereoCalibrationWindow(
    QGraphicsScene* scene, QGraphicsPixmapItem* left, QGraphicsPixmapItem* right,
    QStatusBar* status, QString imgFolder, QString lc, QString rc) {
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;

  mbPlay = false;
  mXCorners = 9;
  mYCorners = 9;
  mSquareLength = 100;
  mImgFolders = imgFolder;
  mLeftCameraFolders = lc;
  mRightCameraFolders = rc;
  mStereoParasFolder = "./reconstruct/";		//����˫Ŀ���������ļ���

  //��������
  createLayout();

  //����Ĭ�ϳ���
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
StereoCalibrationWindow::~StereoCalibrationWindow() {
  //�ر�����ͷ
  if (mbPlay)
    preview();
}
//�ѻҶ�����ת��ΪQImage��ʽ
QImage StereoCalibrationWindow::convertToQImage(unsigned char* buffer) {
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
QImage StereoCalibrationWindow::convertToQImage(cv::Mat img) {
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
void StereoCalibrationWindow::updatePixmap(unsigned char* leftBuffer,
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

void StereoCalibrationWindow::updatePixmap(QImage& li, QImage& ri) {
  mLastLeft = li;
  mLastRight = ri;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));
  //����ͼ��
  update();
}

//����һ֡����ͼ��
void StereoCalibrationWindow::updateOneFrame() {
  static int FrameCount = 0;
  static int StartTime = mProTimer.getMilliseconds();

  CameraFrames frames;
  mCameras->GetFrames(frames);
  if (frames.size() == 2) {
    FrameCount++;
    updatePixmap(frames[0].ToQImage(), frames[1].ToQImage());

    float fps = mCameras->FrameRate();
    //����״̬����ʾ
    QString show;
    show.sprintf("��ǰ��֡��Ϊ��%f", fps);
    mStatusBar->showMessage(show);
  }

  //ÿ20֡��ͳ����֡��
  /*if (FrameCount == 20) {
    int endTime = mProTimer.getMilliseconds();
    float fps = (float)FrameCount * 1000 / (endTime - StartTime);

    //����״̬����ʾ
    QString show;
    show.sprintf("��ǰ��֡��Ϊ��%f", fps);
    mStatusBar->showMessage(show);

    //��0
    FrameCount = 0;
    StartTime = mProTimer.getMilliseconds();
  }*/
}
//������ϸ��ͼΪ����ͼ
void StereoCalibrationWindow::setLeftDetailView() {
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
void StereoCalibrationWindow::setRightDetailView() {
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
void StereoCalibrationWindow::preview() {
  //���û����������ͷ������������ʼԤ��
  if (mbPlay == false) {
    mCameras = new UsbCameraGroup();
    bool succ = mCameras->Init("para.json");

    //�������ʧ�ܣ���ʾ�����û�����Ӻ�
    if (!succ || mCameras->camera_count() != 2) {
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"δ����"));
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
void StereoCalibrationWindow::capture() {
  //��ֹͣʱ��
  mTimer->stop();

  static int COUNT = 0;
  COUNT++;
  QString name;
  name.sprintf("%d", COUNT);

  //�ȱ���ͼ��
  QString leftname = mImgFolders + "left_" + name;
  QString rightname = mImgFolders + "right_" + name;
  mLastLeft.save(leftname + ".jpg");
  mLastRight.save(rightname + ".jpg");
  mImgWidth = mLastLeft.width();
  mImgHeight = mLastLeft.height();

  //��ȡͼ�񣬼���ǵ�
  cv::Mat li = cv::imread(leftname.toStdString() + ".jpg");
  cv::Mat ri = cv::imread(rightname.toStdString() + ".jpg");
  cv::Mat liGrey = li.clone(), riGrey = ri.clone();
  cv::cvtColor(li, liGrey, cv::COLOR_BGR2GRAY);
  cv::cvtColor(ri, riGrey, cv::COLOR_BGR2GRAY);
  cv::vector<cv::Point2f> leftcorners = CameraCalibration::findCorners(liGrey, mXCorners, mYCorners);
  cv::vector<cv::Point2f> rightcorners = CameraCalibration::findCorners(riGrey, mXCorners, mYCorners);
  //������ҵ��˽ǵ�
  if (!leftcorners.empty() && !rightcorners.empty()) {
    //����ͣ����ͷ
    preview();

    //�ȹ��򻯽ǵ�
    cv::vector<cv::Point2f> nlc, nrc;
    nlc = Utilities::normalizeCorners(leftcorners, mXCorners, mYCorners);
    nrc = Utilities::normalizeCorners(rightcorners, mXCorners, mYCorners);

    //����ǵ�
    std::string corname = mImgFolders.toStdString() + name.toStdString() + ".txt";
    saveCornerPoints(corname, nlc, nrc);

    //���ƽǵ�
    cv::drawChessboardCorners(li, cv::Size(mXCorners, mYCorners), leftcorners, true);
    cv::circle(li, leftcorners[0], 15, cv::Scalar(255, 0, 0), 3);
    cv::drawChessboardCorners(ri, cv::Size(mXCorners, mYCorners), rightcorners, true);
    cv::circle(ri, rightcorners[0], 15, cv::Scalar(255, 0, 0), 3);

    //������ƵĽǵ�ͼ��
    cv::imwrite(leftname.toStdString() + ".bmp", li);
    cv::imwrite(rightname.toStdString() + ".bmp", ri);

    //��¼��ǰͼ��
    mImgs.push_back(COUNT);

    //��ʾ���ƽǵ��ͼ��
    QImage lp = convertToQImage(li);
    QImage rp = convertToQImage(ri);
    mLeftCameraPixmap->setPixmap(QPixmap::fromImage(lp));
    mRightCameraPixmap->setPixmap(QPixmap::fromImage(rp));

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
    QFile::remove(leftname + ".jpg");
    QFile::remove(rightname + ".jpg");

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
void StereoCalibrationWindow::deleteImg() {
  if (mImgs.size() > 0) {
    //�ҵ���ǰѡ����ļ���
    int index = mSlider->value();
    int name = mImgs[index - 1];

    //ɾ����Ӧ���ļ�
    QString n;
    n.sprintf("%d", name);
    QFile::remove(mImgFolders + "left_" + n + ".jpg");
    QFile::remove(mImgFolders + "right_" + n + ".jpg");
    QFile::remove(mImgFolders + "left_" + n + ".bmp");
    QFile::remove(mImgFolders + "right_" + n + ".bmp");
    QFile::remove(mImgFolders + n + ".txt");

    mImgs.erase(mImgs.begin() + index - 1);

    updateButtonState();
	  if(index>1)
		updateCurrent(index-1);
	  else
		updateCurrent(index);
	  }
  }
}

//����������µ�ǰѡ���
void StereoCalibrationWindow::updateCurrent(int cs) {
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
  iu.load(mImgFolders + "left_" + n + ".bmp");
  QPixmap id;
  id.load(mImgFolders + "right_" + n + ".bmp");
  mLeftCameraPixmap->setPixmap(iu);
  mRightCameraPixmap->setPixmap(id);
}
//����x����Ľǵ����
void StereoCalibrationWindow::setSizeX() {
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
void StereoCalibrationWindow::setSizeY() {
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
void StereoCalibrationWindow::setLength() {
  if (mbPlay)
    mTimer->stop();

  bool ok = false;
  int tx = QInputDialog::getInt(
    0,
    QString::fromWCharArray(L"�������̸�ĳ���"),
    QString::fromWCharArray(L"�������̸�ĳ��ȣ���λΪ����"),
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
void StereoCalibrationWindow::calibrate() {
  //С��5�������б궨
  if (mImgs.size() < 1) {
    QMessageBox::StandardButton btn2 = QMessageBox::warning(
      0,						//������
      QString::fromWCharArray(L"���ܽ��б궨"),			//������
      QString::fromWCharArray(L"��ǰ�����ͼ��̫�٣����ܽ��б궨�����ٲ���5��ͼ�񣬲��ܽ��б궨"));		//�ı�����
  } else {
    //todo ����һ���ȴ��Ի���
    mStatusBar->showMessage(QString::fromWCharArray(L"���ڱ궨��Σ������ĵȴ�������"));

    //�����ڲ� todo ��Ӵ����ж�
    cv::Mat lm(3, 3, CV_64FC1);
    cv::Mat ld(1, 5, CV_64FC1);
    cv::Mat rm(3, 3, CV_64FC1);
    cv::Mat rd(1, 5, CV_64FC1);
    Utilities::importMat((mLeftCameraFolders.toStdString() + "matrix.txt").c_str(), lm);
    Utilities::importMat((mLeftCameraFolders.toStdString() + "distortion.txt").c_str(), ld);
    Utilities::importMat((mRightCameraFolders.toStdString() + "matrix.txt").c_str(), rm);
    Utilities::importMat((mRightCameraFolders.toStdString() + "distortion.txt").c_str(), rd);

    //�����Ӧ��ͶӰ��
    std::vector<std::vector<cv::Point3f>> objCorners;	//��¼����㣨��������ϵ��
    std::vector<std::vector<cv::Point2f>> imgPoints1;	//�������ƥ���
    std::vector<std::vector<cv::Point2f>> imgPoints2;	//�������ƥ���
    for (int i = 0; i < mImgs.size(); i++) {
      int n = mImgs[i];
      QString curName;
      curName.sprintf("%d.txt", n);
      std::vector<cv::Point3f> opts;
      std::vector<cv::Point2f> lpts, rpts;
      loadCornerPoints(mImgFolders.toStdString() + curName.toStdString(), opts, lpts, rpts);

      objCorners.push_back(opts);
      imgPoints1.push_back(lpts);
      imgPoints2.push_back(rpts);
    }

    //˫Ŀ����
    cv::Mat E, F, R, T;
    double error = cv::stereoCalibrate(
      objCorners,
      imgPoints1,
      imgPoints2,
      lm,
      ld,
      rm,
      rd,
      cv::Size(mImgWidth, mImgHeight),
      R, T,
      E, F,
      cv::TermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 100, 1e-5), CV_CALIB_FIX_INTRINSIC);

    //����ʹ����Щ�ļ��궨���ڲ�
    std::ofstream of(mImgFolders.toStdString() + "info.txt");
    for (int i = 0; i < mImgs.size(); i++) {
      of << mImgs[i] << std::endl;
    }
    of.close();

    //��¼������
    std::string fd = mStereoParasFolder.toStdString();
    Utilities::exportMat((fd + "R.txt").c_str(), R);
    Utilities::exportMat((fd + "T.txt").c_str(), T);
    Utilities::exportMat((fd + "F.txt").c_str(), F);

    Utilities::exportMat((fd + "LeftMatrix.txt").c_str(), lm);
    Utilities::exportMat((fd + "LeftDistortion.txt").c_str(), ld);
    Utilities::exportMat((fd + "RightMatrix.txt").c_str(), rm);
    Utilities::exportMat((fd + "RightDistortion.txt").c_str(), rd);

    cv::Mat angle;
    cv::Rodrigues(R, angle);
    double x = angle.at<double>(0, 0);
    double y = angle.at<double>(1, 0);
    double z = angle.at<double>(2, 0);
    double length = std::sqrt(x*x + y*y + z*z);
    float degree = length * 180 / 3.1415926;
    std::ofstream A((fd + "Rv.txt").c_str());
    A << degree << " " << x / length << " " << y / length << " " << z / length << std::endl;
    A.close();

    QString msg;
    msg.sprintf("%f", error);
    QMessageBox::about(
      0,				//������
      QString::fromWCharArray(L"�궨���"),		//������
      QString::fromWCharArray(L"��α궨��ɣ�ƽ���������Ϊ��") + msg);	//�ı�����
  }
}
//�ѽǵ㱣�浽�ļ���,ǰ��3����xCorners,yCorners,Length,����������ռ����꣬����������ͶӰͼ������
void StereoCalibrationWindow::saveCornerPoints(std::string filename,
                                               std::vector<cv::Point2f>& l,
                                               std::vector<cv::Point2f>& r) {
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

  //д����ͶӰͼ������
  for (int i = 0; i < l.size(); i++) {
    obj << l[i].x << '\t' << l[i].y << std::endl;
  }

  //д����ͶӰͼ������
  for (int i = 0; i < r.size(); i++) {
    obj << r[i].x << '\t' << r[i].y << std::endl;
  }

  obj.close();
}

//�������еĽǵ�����
void StereoCalibrationWindow::loadCornerPoints(std::string filename,
                                               std::vector<cv::Point3f>& opts,
                                               std::vector<cv::Point2f>& lpts,
                                               std::vector<cv::Point2f>& rpts) {
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

  //��ȡ��ͼ��ͶӰ����
  for (int i = 0; i < x*y; i++) {
    float ix, iy;
    obj >> ix;
    obj >> iy;
    lpts.push_back(cv::Point2f(ix, iy));
  }

  //��ȡ��ͼ��ͶӰ����
  for (int i = 0; i < x*y; i++) {
    float ix, iy;
    obj >> ix;
    obj >> iy;
    rpts.push_back(cv::Point2f(ix, iy));
  }
  obj.close();
}
//�������ڲ���
void StereoCalibrationWindow::createLayout() {
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
void StereoCalibrationWindow::createWidget() {
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
void StereoCalibrationWindow::updateButtonState() {
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