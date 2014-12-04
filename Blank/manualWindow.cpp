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

  mCameras = NULL;	//����ͷΪ��
  mbPlay = false;		//true��ʾ����Ԥ��
  mbCapture = false;
  mTempFolder = "./temp/";

  //��������
  createLayout();

  //����Ĭ�ϳ���
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
ManualWindow::~ManualWindow() {
  //�����Ԥ�����ر�Ԥ��
  if (mbPlay == true) {
    preview();
  }
}

//�ѻҶ�����ת��ΪQImage��ʽ
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
//���³���ͼ��
void ManualWindow::updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer) {
  QImage li = convertToQImage(leftBuffer);
  QImage ri = convertToQImage(rightBuffer);
  mLeftImg = li;
  mRightImg = ri;
  mLeftCameraPixmap->setPixmap(QPixmap::fromImage(li));
  mRightCameraPixmap->setPixmap(QPixmap::fromImage(ri));
  //����ͼ��
  update();
}
//Ԥ����Ƶ����Ļ���
void ManualWindow::preview() {
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
      mPlay->setText(QString::fromWCharArray(L"����"));

      //���ö�ʱ����
      mTimer = new QTimer(this);
      connect(mTimer, SIGNAL(timeout()), this, SLOT(updateOneFrame()));
      mTimer->start(10);
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

      //����ͼ��
      mLeftImg.save(mTempFolder + "left.jpg");
      mRightImg.save(mTempFolder + "right.jpg");
      mLeftCameraPixmap->setPixmap(QPixmap::fromImage(mLeftImg));
      mRightCameraPixmap->setPixmap(QPixmap::fromImage(mRightImg));
      //����ͼ��
      update();
      mbCapture = true;
    }
  }
}
//��ʾ���ǵ�ı༭����
void ManualWindow::showLeftMarkers() {
	mbCapture = true;
	setLeftDetailView();	//switch to left view
  QMessageBox::about(
    0,				//������
    QString::fromWCharArray(L"��Ϣ��"),		//������
    QString::fromWCharArray(L"ѡ�񣬻�����ɾ�����ǵ�"));	//�ı�����

  


}
void ManualWindow::showRightMarkers() {
	mbCapture = true;
	setRightDetailView();	//switch to right view
  QMessageBox::about(
    0,				//������
    QString::fromWCharArray(L"��Ϣ��"),		//������
    QString::fromWCharArray(L"ѡ�񣬻�����ɾ���ұ�ǵ�"));	//�ı�����


}

//����3d����
//taokelu@gmail.com
void ManualWindow::calculate3dPoints() {
  QMessageBox::about(
    0,				//������
    QString::fromWCharArray(L"��Ϣ��"),		//������
    QString::fromWCharArray(L"����3d����"));	//�ı�����

  
  //�ѱ�ǵ�һһ��Ӧ��¼��left.txt��right.txt�У�����reconstructor
  if(mLeftMarkers.size()!=mRightMarkers.size()){
	QMessageBox::about(
		0,
		QString::fromWCharArray(L"��Ϣ��"),
		QString::fromWCharArray(L"�������ұ�ǵ���Ŀ����ȣ�")
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
			QString::fromWCharArray(L"��Ϣ��"),
			QString::fromWCharArray(L"�������ұ�ǵ����Ʋ�ƥ�䣬��ȷ������ƥ�䣡")
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

//����һ֡����ͼ��
void ManualWindow::updateOneFrame() {
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
void ManualWindow::setLeftDetailView() {
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
void ManualWindow::setRightDetailView() {
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

//�������ź�
void ManualWindow::dealwithCreatSignal(QPoint sp) {
  //ֻ�гɹ������˲Ŵ���
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
//������ɾ������
void ManualWindow::dealwithDeleteSignal() {
  std::vector<Marker*> toDelete;
  std::list<Marker*>::iterator i = mLeftMarkers.begin();
  while (i != mLeftMarkers.end()) {
    //�����ѡ���ˣ���ɾ��
    if ((*i)->getSelectedState()) {
      toDelete.push_back(*i);
      i = mLeftMarkers.erase(i);
    } else {
      i++;
    }
  }

  std::list<Marker*>::iterator j = mRightMarkers.begin();
  while (j != mRightMarkers.end()) {
    //�����ѡ���ˣ���ɾ��
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
  //������ƥ����ؽ���ά����
std::vector<StereoReconstructor::RestructPoint> ManualWindow::restructPoints(
    std::vector<cv::Point2f>& lpts, std::vector<cv::Point2f>& rpts) {
  //����2�����������
  VirtualCamera vc1;
  VirtualCamera vc2;
  //�����ڲ�
  vc1.loadCameraMatrix("reconstruct\\LeftMatrix.txt");
  vc1.loadDistortion("reconstruct\\LeftDistortion.txt");
  vc1.rotationMatrix = cv::Mat::eye(3, 3, CV_32FC1);
  vc1.translationVector = cv::Mat::zeros(3, 1, CV_32FC1);
  vc2.loadCameraMatrix("reconstruct\\RightMatrix.txt");
  vc2.loadDistortion("reconstruct\\RightDistortion.txt");
  vc2.loadRotationMatrix("reconstruct\\R.txt");
  vc2.loadTranslationVector("reconstruct\\T.txt");

  //��������ƥ���
  vc1.setKeyPoints(lpts);
  vc2.setKeyPoints(rpts);

  //�ؽ�3d��
  std::vector<StereoReconstructor::RestructPoint> ret;
  ret = StereoReconstructor::triangulation(vc1, vc2, true);	//true���л������,false��ʾ������

  return ret;
}

//�������ڲ���
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

  createWidget();
}
//������ť
void ManualWindow::createWidget() {
  mPlay = new QPushButton();
  mPlay->setText(QString::fromWCharArray(L"Ԥ��"));
  connect(mPlay, SIGNAL(pressed()), this, SLOT(preview()));

  mLM = new QPushButton();
  mLM->setText(QString::fromWCharArray(L"���ǵ�"));
  connect(mLM, SIGNAL(pressed()), this, SLOT(showLeftMarkers()));

  mRM = new QPushButton();
  mRM->setText(QString::fromWCharArray(L"�ұ�ǵ�"));
  connect(mRM, SIGNAL(pressed()), this, SLOT(showRightMarkers()));

  mCalculate = new QPushButton();
  mCalculate->setText(QString::fromWCharArray(L"����3D����"));
  connect(mCalculate, SIGNAL(pressed()), this, SLOT(calculate3dPoints()));

  //���˫�������źŵ���Ӧ
  connect(mZoomView, SIGNAL(createMarker(QPoint)), this, SLOT(dealwithCreatSignal(QPoint)));
  connect(mZoomView, SIGNAL(deleteMarker()), this, SLOT(dealwithDeleteSignal()));

  mCtrlLayout->addWidget(mPlay);
  mCtrlLayout->addWidget(mLM);
  mCtrlLayout->addWidget(mRM);
  mCtrlLayout->addWidget(mCalculate);
}