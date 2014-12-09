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
#include "usb_camera_group.h"
#include "camera_frame.h"

ManualWindow::ManualWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
                           QGraphicsPixmapItem* right, QStatusBar* status) {
  mLeftCameraPixmap = left;
  mRightCameraPixmap = right;
  mStatusBar = status;
  mScene = scene;

  mCameras = NULL;	//����ͷΪ��
  mbPlay = false;		//true��ʾ����Ԥ��
  mbCapture = true;
  mTempFolder = "./temp/";

  //��������
  createLayout();

  //����Ĭ�ϳ���
  mLeftCameraPixmap->setPixmap(Paras::getSingleton().LeftBlankImg);
  mRightCameraPixmap->setPixmap(Paras::getSingleton().RightBlankImg);
}
ManualWindow::~ManualWindow() {
  mCameras = NULL;
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

void ManualWindow::updatePixmap(QImage& li, QImage& ri) {
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
    mCameras = new UsbCameraGroup();
    bool succ = mCameras->Init("para.json");

    //�������ʧ�ܣ���ʾ�����û�����Ӻ�
    if (!succ || mCameras->camera_count() != 2) {
      mCameras = NULL;
      mPlay->setText(QString::fromWCharArray(L"ֹͣ"));
      QMessageBox::critical(
        0,							//������
        QString::fromWCharArray(L"�Ҳ������õ�����ͷ"),		//������
        QString::fromWCharArray(L"�Ҳ������õ�����ͷ����鿴����ͷ�Ƿ�"
                                L"�Ѿ����ӵ����ԣ����Ѿ����ӣ������²��USB�ӿ�"));		//�ı�����
    }
    //�ɹ�����
    else {
      mCameras->StartAll();

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
      mCameras->SetFrameCallback(UsbCameraGroup::FrameCallback());
      mCameras->StopAll();
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

static std::vector<cv::Point2f> circleDetect(cv::Mat img_gray) {
  cv::Mat img_binary;
  cv::threshold(img_gray, img_binary, 60, 255, cv::THRESH_BINARY);
  cv::imwrite("./measure/binary.jpg", img_binary);

  std::vector<cv::Point2f> corners, result;
  cv::goodFeaturesToTrack(img_gray, corners, 200, 0.01, 10);

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
 
  for (size_t i = 0; i != corners.size(); ++i) {
    std::vector<int> mean;	//Բ�ܵĹ�һ��ֵ
    int x = corners[i].x, y = corners[i].y, judge = 1;

    for (int k = 5; k<25 && judge; k++) {
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
      if (judge == 0 || reverse_time1 == 0 || reverse_time2 == 0 || reverse_time1 + reverse_time2<4 || (double)blacknum1 * 2 / num <= 0.3 || (double)blacknum2 * 2 / num >= 0.7 || abs(blacknum1 - blacknum2)>3) {
		judge = 0;
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
    avarage /= 20;

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

static std::vector<cv::Point2f> circleDetect1(cv::Mat img_gray) {
  cv::Mat img_binary;
  cv::threshold(img_gray, img_binary, 60, 255, cv::THRESH_BINARY);
  cv::imwrite("./measure/binary.jpg", img_binary);

  std::vector<cv::Point2f> corners, result;
  cv::goodFeaturesToTrack(img_gray, corners, 200, 0.01, 10);

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
 
  for (size_t i = 0; i != corners.size(); ++i) {
    std::vector<int> mean;	//Բ�ܵĹ�һ��ֵ
    int x = corners[i].x, y = corners[i].y, judge = 1;

    for (int k = 5; k<10 && judge; k++) {
      //num��Բ�ܵ����ص��������ǵ�ͼ����������ɢ�ģ�����ֻȡ��2��R���㣬R��ֱ��
      //sum��Բ�ܵ����ض�ֵ��ֵ�Ӻͣ�255����0
      //reverse_time��ͳ��Բ�����ر任���ԽǱ�־�Ļ���任4�Σ�����С��4��ȫ����̭
      int sum = 0, reverse_time1 = 0, reverse_time2 = 0, flag = -1, blacknum2 = 0, blacknum1 = 0, num = 0;
	  std::string str1, str2;

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
        if (val == 0){
          blacknum1++;
		  str1.push_back('0');
		}else{
			str1.push_back('1');
		}
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
        if (val == 0){
          blacknum2++;
		  str2.push_back('0');
		}else{
			str2.push_back('1');
		}
        sum += val;
        num++;
      }
      if (judge == 0 || reverse_time1 == 0 || reverse_time2 == 0 || reverse_time1 + reverse_time2<4 || (double)blacknum1 * 2 / num <= 0.3 || (double)blacknum2 * 2 / num >= 0.7 || abs(blacknum1 - blacknum2)>3) {
		judge = 0;
		  break;
      }
	  int diff=0;
      for(size_t t=0; t!=str1.size(); t++){
		if(str1[t]!=str2[t])
			diff++;
	  }
	  if(diff>4){
		judge=0;
		break;
	  }
		  mean.push_back(sum/num);
    }

    if (judge == 0 || mean.size()<5) {
      mean.clear();
      continue;
    }

    int avarage = 0;
    for (int j = 0; j<mean.size(); j++)
      avarage += mean[j];
    avarage /= mean.size();

    int threshold_val = 30;
    for (int j = 0; j<mean.size(); j++) {
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
//�����Զ�����ǵ�
void ManualWindow::autoDetectMarkers(){
	//��ȡͼ�񣬼���ǵ�
	cv::Mat li = cv::imread((mTempFolder + "left.jpg").toStdString(), CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat ri = cv::imread((mTempFolder + "right.jpg").toStdString(), CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat liGrey, riGrey;
	cv::cvtColor(li, liGrey, cv::COLOR_BGR2GRAY);
	cv::cvtColor(ri, riGrey, cv::COLOR_BGR2GRAY);

	std::vector<cv::Point2f> left_markers = circleDetect1(liGrey);
	std::vector<cv::Point2f> right_markers = circleDetect1(riGrey);

	for (int i = 0; i < left_markers.size(); i++)
		cv::circle(li, left_markers[i], 4, cv::Scalar(0, 0, 255), 3, 8, 0);
	for (int i = 0; i < right_markers.size(); i++)
		cv::circle(ri, right_markers[i], 4, cv::Scalar(0, 0, 255), 3, 8, 0);

	//������ƵĽǵ�ͼ��
	cv::imwrite((mTempFolder + "left_result.jpg").toStdString(), li);
	cv::imwrite((mTempFolder + "right_result.jpg").toStdString(), ri);
	for(int i=0; i<left_markers.size(); i++){
		Marker* plm = new Marker(QPoint(left_markers[i].x, left_markers[i].y), QString::number(i+1), mScene);
		mLeftMarkers.push_back(plm);
	}
	for(int i=0; i<right_markers.size(); i++){
		Marker* prm = new Marker(QPoint(right_markers[i].x+Paras::getSingleton().width*2, right_markers[i].y), QString::number(i+1), mScene);
		mRightMarkers.push_back(prm);
	}
	QMessageBox::about(
	0,				//������
	QString::fromWCharArray(L"��Ϣ��"),		//������
	QString::fromWCharArray(L"��ɾ������Ľǵ㣬�����ȷ�Ľǵ㣬ע������һ��"));	//�ı�����
}

void ManualWindow::autoDetectLights(){
	cv::Mat li = cv::imread((mTempFolder + "left.jpg").toStdString(), 0);
	cv::Mat ri = cv::imread((mTempFolder + "right.jpg").toStdString(), 0);

	double min_val, max_val;
	cv::Point min_loc, max_loc1, max_loc2;
	
	cv::minMaxLoc(li, &min_val, &max_val, &min_loc, &max_loc1);
	cv::minMaxLoc(ri, &min_val, &max_val, &min_loc, &max_loc2);
	
	int sum_x=0, sum_y=0, sum_val=0;
	for(int i=-10; i<=10; i++){
		for(int j=-10; j<=10; j++){
			int val = li.at<uchar>(max_loc1.y+i, max_loc1.x+j);
			sum_x += val*(max_loc1.x+j);
			sum_y += val*(max_loc1.y+i);
			sum_val += val;
		}
	}
	sum_x /= sum_val;
	sum_y /= sum_val;

	Marker* plm = new Marker(QPoint(sum_x, sum_y), QString::number(1), mScene);
	mLeftMarkers.push_back(plm);

	sum_x=0, sum_y=0, sum_val=0;
	for(int i=-10; i<=10; i++){
		for(int j=-10; j<=10; j++){
			int val = ri.at<uchar>(max_loc2.y+i, max_loc2.x+j);
			sum_x += val*(max_loc2.x+j);
			sum_y += val*(max_loc2.y+i);
			sum_val += val;
		}
	}
	sum_x /= sum_val;
	sum_y /= sum_val;

	Marker* prm = new Marker(QPoint(sum_x+2*Paras::getSingleton().width, sum_y), QString::number(1), mScene);
	mRightMarkers.push_back(prm);
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
  if(ret.size() == 0)
  {
	QMessageBox::about(
		0,
		QString::fromWCharArray(L"��Ϣ��"),
		QString::fromWCharArray(L"�������ֶ��������ƥ��㣡")
		);
  }

  double distance = sqrt(ret[0].point.x*ret[0].point.x+ret[0].point.y*ret[0].point.y+ret[0].point.z*ret[0].point.z);
  std::cout << "First point distance:"<<distance << std::endl;
  int id = distance;
  distance = id/(float)1000;
  QString sd;
  sd.sprintf("%.3f",distance);

  if(ret.size()>1)
  {
		double length = sqrt((ret[0].point.x-ret[1].point.x)*(ret[0].point.x-ret[1].point.x)+(ret[0].point.y-ret[1].point.y)*(ret[0].point.y-ret[1].point.y)+(ret[0].point.z-ret[1].point.z)*(ret[0].point.z-ret[1].point.z));
		std::cout << "First and Second points length:"<<length << std::endl;
		int il = length;
		length = il/(float)1000;
		QString sl;
		sl.sprintf("%.3f",length);

		QString tShowMessage = QString::fromWCharArray(L"��������ǵ�1�ľ���Ϊ��")+ sd + QString::fromWCharArray(L"��\r\n");
		tShowMessage +=QString::fromWCharArray(L"1,2��ǵ�֮��ľ���Ϊ��")+ sl + QString::fromWCharArray(L"��\r\n");
		
		QMessageBox::about(
			0,
			QString::fromWCharArray(L"���Ȳ���"),
			tShowMessage
			);
  }
  else
  {
	QMessageBox::about(
		0,
		QString::fromWCharArray(L"���Ȳ���"),
		QString::fromWCharArray(L"���ǵ�1�ľ���Ϊ��")+ sd + QString::fromWCharArray(L"��")
		);
  }



}

//����һ֡����ͼ��
void ManualWindow::updateOneFrame() {
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

  autoLight = new QPushButton();
  autoLight->setText(QString::fromWCharArray(L"�Զ�����źŵ�"));
  connect(autoLight, SIGNAL(pressed()), this, SLOT(autoDetectLights()));

  autoDetect = new QPushButton();
  autoDetect->setText(QString::fromWCharArray(L"�Զ����"));
  connect(autoDetect, SIGNAL(pressed()), this, SLOT(autoDetectMarkers()));

  mLM = new QPushButton();
  mLM->setText(QString::fromWCharArray(L"�ֶ������ǵ�"));
  connect(mLM, SIGNAL(pressed()), this, SLOT(showLeftMarkers()));

  mRM = new QPushButton();
  mRM->setText(QString::fromWCharArray(L"�ֶ����ұ�ǵ�"));
  connect(mRM, SIGNAL(pressed()), this, SLOT(showRightMarkers()));

  mCalculate = new QPushButton();
  mCalculate->setText(QString::fromWCharArray(L"����3D����"));
  connect(mCalculate, SIGNAL(pressed()), this, SLOT(calculate3dPoints()));

  //���˫�������źŵ���Ӧ
  connect(mZoomView, SIGNAL(createMarker(QPoint)), this, SLOT(dealwithCreatSignal(QPoint)));
  connect(mZoomView, SIGNAL(deleteMarker()), this, SLOT(dealwithDeleteSignal()));

  mCtrlLayout->addWidget(mPlay);
  mCtrlLayout->addWidget(autoLight);
  mCtrlLayout->addWidget(autoDetect);
  mCtrlLayout->addWidget(mLM);
  mCtrlLayout->addWidget(mRM);
  mCtrlLayout->addWidget(mCalculate);
}