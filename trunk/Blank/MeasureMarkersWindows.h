#pragma once
#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"

#include "QtGui/qimage.h"

#include "QtWidgets/qwidget.h"

#include <vector>

#include "opencv2/core/core.hpp"

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

class UsbCameras;
class MyDetailView;
class MyCameraView;

/*
�Զ������ǵ㣬�ֶ������޸�
*/
class MeasureMarkersWindow : public QWidget
{
  Q_OBJECT
public:
  MeasureMarkersWindow(QGraphicsScene* scene,
                       QGraphicsPixmapItem* left,
                       QGraphicsPixmapItem* right,
                       QStatusBar* status,
                       QString reconstructFolder = "./reconstruct/",
                       QString measureFolder = "./measure/");
  ~MeasureMarkersWindow();
  //�ѻҶ�����ת��ΪQImage��ʽ
  QImage convertToQImage(unsigned char* buffer);
  QImage convertToQImage(cv::Mat img);
  //���³���ͼ��
  void updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer);
signals:

public slots :
  //����һ֡����ͼ��
  void updateOneFrame();
  //������ϸ��ͼΪ����ͼ
  void setLeftDetailView();
  //������ϸ��ͼΪ����ͼ
  void setRightDetailView();

  //Ԥ����Ƶ����Ļ���
  void preview();
  //����һ��ͼ��
  void capture();
  //ɾ����ǰ��ͼ��
  void deleteImg();

  //����������µ�ǰѡ���
  void updateCurrent(int cs);
private:
  //�������ڲ���
  void createLayout();

  //������ť
  void createWidget();

  //���ݵ�ǰ�����ͼ�񣬸��¸�����ť��״̬
  void updateButtonState();
protected:
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

  QTimer*			mTimer;					//��ʱ������
  UsbCameras*		mCameras;				//����ͷ
  int				mCameraId;				//0��ʾ������ͷ��1��ʾ������ͷ
  bool			mbLeftFocused;			//true��ʾ������ͷ��Ļ��ý���
  Timer			mProTimer;				//����ļ�ʱ��
  QImage			mLastLeft;				//��һ�β����ͼ��
  QImage			mLastRight;				//��һ�β����ͼ��
  
  std::vector<int> mImgs;					//�洢���е�ͼƬ
  QString			mReconstructFolder;			//�洢ͼ����ļ���
  QString			mMeasureFolder;		//������������ڲε��ļ���
  int				mImgWidth;				//�����ͼ����
  int				mImgHeight;				//�����ͼ��߶�

  //��ť
  QPushButton*	mPlay;			//Ԥ������,������ͣ��ť
  bool			mbPlay;			//true��ʾ����Ԥ��
  QLabel*			mCaps;			//��ʾ������������͵�ǰ�ۿ���ͼ��
  QPushButton*	mCapture;		//����ť
  QPushButton*	mDelete;		//ɾ����ť
  QSlider*		mSlider;		//�����飬��ʾ֡����Ϣ 

  //���ⲿ����
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//����������Ļ��棨����ͼ���
  QGraphicsPixmapItem*	mRightCameraPixmap;	//����������Ļ��棨����ͼ���
  QStatusBar*				mStatusBar;			//״̬��
  QGraphicsScene*			mScene;				//����ָ��
};