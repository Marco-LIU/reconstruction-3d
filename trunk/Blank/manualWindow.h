#ifndef _MANUAL_WINDOW_H_
#define _MANUAL_WINDOW_H_

//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"

#include "QtGui/qimage.h"

#include "QtWidgets/qwidget.h"

#include <vector>
#include <list>

#include "opencv2/core/core.hpp"
#include "StereoReconstructor.h"
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

class UsbCamera;
class MyDetailView;
class MyCameraView;
class Marker;

/*
  ����࣬��Ҫ�����ֶ����ǵ㣬�����в���
  1��˫��������ǵ�
  2��deleteɾ��ѡ��ı�ǵ�
  */
class ManualWindow : public QWidget
{
  Q_OBJECT
public:
  ManualWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
               QGraphicsPixmapItem* right, QStatusBar* status);
  ~ManualWindow();

  //�ѻҶ�����ת��ΪQImage��ʽ
  QImage convertToQImage(unsigned char* buffer);
  //���³���ͼ��
  void updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer);
signals:

public slots :
  //Ԥ����Ƶ����Ļ���
  void preview();
  //��ʾ���ǵ�ı༭����
  void showLeftMarkers();
  void showRightMarkers();
  //����3d����
  void calculate3dPoints();

  //����һ֡����ͼ��
  void updateOneFrame();
  //������ϸ��ͼΪ����ͼ
  void setLeftDetailView();
  //������ϸ��ͼΪ����ͼ
  void setRightDetailView();

  //���������ź�
  void dealwithCreatSignal(QPoint sp);
  //��������ɾ������
  void dealwithDeleteSignal();
private:
  //������ƥ����ؽ���ά����
  std::vector<StereoReconstructor::RestructPoint> restructPoints(
    std::vector<cv::Point2f>& lpts, std::vector<cv::Point2f>& rpts);

  //�������ڲ���
  void createLayout();
  //������ť
  void createWidget();
protected:
  //����
  QTimer*		mTimer;		//��ʱ������
  Timer		mRecTimer;	//����¼�ƵĶ�ʱ��
  Timer		mProTimer;	//����ļ�ʱ��
  UsbCamera*	mCameras;	//����ͷ

  //���ڲ���
  QWidget*		mCenterWidget;			//���Ĵ���(��������,this)
  QVBoxLayout*	mMainLayout;			//������
  QHBoxLayout*	mVedioLayout;			//�ϲ�����Ƶ����
  QHBoxLayout*	mTimeLineLayout;		//�²���ʱ���߿��Ʋ���
  QVBoxLayout*	mStereoLayout;			//���˫��Ƶ����
  QVBoxLayout*	mDetailLayout;			//�ұߵ�ϸ�ڿ��Ʋ���
  QHBoxLayout*	mCtrlLayout;			//�ϱߵĿ�����岼��
  MyCameraView*	mLeftCameraView;		//������������ĳ���
  MyCameraView*	mRightCameraView;		//������������ĳ���
  MyDetailView*	mZoomView;				//������ͼ
  bool			mbLeftFocused;			//true��ʾ������ͷ��Ļ��ý���

  //��ť
  QPushButton*			mPlay;			//Ԥ������,������ͣ��ť
  bool					mbPlay;			//true��ʾ����Ԥ��
  QPushButton*			mLM;			//�������ѡ�񣬻�����ɾ�����ǵ�
  QPushButton*			mRM;			//�������ѡ�񣬻�����ɾ���ұ�ǵ�
  QPushButton*			mCalculate;		//�������ѡ�񣬻�����ɾ���ұ�ǵ�
  QImage					mLeftImg;		//�������ͼ��
  QImage					mRightImg;		//�������ͼ��
  QString					mTempFolder;	//��ʱ�ļ���
  bool					mbCapture;		//true��ʾ�ɹ�������
  std::list<Marker*>		mLeftMarkers;	//��������������б�ǵ�
  std::list<Marker*>		mRightMarkers;	//��������������б�ǵ�

  //���ⲿ����
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//����������Ļ��棨����ͼ���
  QGraphicsPixmapItem*	mRightCameraPixmap;	//����������Ļ��棨����ͼ���
  QStatusBar*				mStatusBar;			//״̬��
  QGraphicsScene*			mScene;				//����ָ��
};

#endif