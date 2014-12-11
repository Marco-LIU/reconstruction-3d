#ifndef _STEREO_CAL_WINDOW_H_
#define _STEREO_CAL_WINDOW_H_

//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"

#include "QtGui/qimage.h"

#include "QtWidgets/qwidget.h"

#include <vector>

#include "opencv2/core/core.hpp"

#include "OgreTimer.h"
#include "base/memory/ref_counted.h"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;
class QPushButton;
class QLabel;
class QSlider;

//class UsbCameras;
class MyDetailView;
class MyCameraView;
class  UsbCameraGroup;

class StereoCalibrationWindow : public QWidget
{
  Q_OBJECT
public:
  StereoCalibrationWindow(QGraphicsScene* scene,
                          QGraphicsPixmapItem* left,
                          QGraphicsPixmapItem* right,
                          QStatusBar* status,
                          QString imgFolder = "./stereo/",
                          QString lc = "./leftCamera/",
                          QString rc = "./rightCamera/");
  ~StereoCalibrationWindow();
  //�ѻҶ�����ת��ΪQImage��ʽ
  QImage convertToQImage(unsigned char* buffer);
  QImage convertToQImage(cv::Mat img);
  //���³���ͼ��
  void updatePixmap(unsigned char* leftBuffer, unsigned char* rightBuffer);
  void updatePixmap(QImage& li, QImage& ri);
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
  //����x����Ľǵ����
  void setSizeX();
  //����y����Ľǵ����
  void setSizeY();
  //�������̸�ĳ���
  void setLength();
  //���б궨
  void calibrate();
private:
  //�ѽǵ㱣�浽�ļ���,ǰ��3����xCorners,yCorners,Length,����������ռ����꣬����������ͶӰͼ������
  void saveCornerPoints(std::string filename,
                        std::vector<cv::Point2f>& l,
                        std::vector<cv::Point2f>& r);

  //�������еĽǵ�����
  void loadCornerPoints(std::string filename,
                        std::vector<cv::Point3f>& opts,
                        std::vector<cv::Point2f>& lpts,
                        std::vector<cv::Point2f>& rpts);
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
  scoped_refptr<UsbCameraGroup>   mCameras;     //����ͷ
  int				mCameraId;				//0��ʾ������ͷ��1��ʾ������ͷ
  bool			mbLeftFocused;			//true��ʾ������ͷ��Ļ��ý���
  Timer			mProTimer;				//����ļ�ʱ��
  QImage			mLastLeft;				//��һ�β����ͼ��
  QImage			mLastRight;				//��һ�β����ͼ��
  int				mXCorners;				//x����Ľǵ����
  int				mYCorners;				//y����Ľǵ����
  int				mSquareLength;			//���̸�ĳ���
  std::vector<int> mImgs;					//�洢���е�ͼƬ
  QString			mImgFolders;			//�洢ͼ����ļ���
  QString			mLeftCameraFolders;		//������������ڲε��ļ���
  QString			mRightCameraFolders;	//������������ڲε��ļ���
  QString			mStereoParasFolder;		//����˫Ŀ���������ļ���
  int				mImgWidth;				//�����ͼ����
  int				mImgHeight;				//�����ͼ��߶�

  //��ť
  QPushButton*	mPlay;			//Ԥ������,������ͣ��ť
  bool			mbPlay;			//true��ʾ����Ԥ��
  QLabel*			mCaps;			//��ʾ������������͵�ǰ�ۿ���ͼ��
  QPushButton*	mCapture;		//����ť
  QPushButton*	mDelete;		//ɾ����ť
  QLabel*			mSizeXLabel;	//X����Ľǵ����
  QPushButton*	mSizeX;			//����X����ǵ�����İ�ť
  QLabel*			mSizeYLabel;	//Y����Ľǵ����
  QPushButton*	mSizeY;			//����Y����ǵ�����İ�ť
  QLabel*			mLengthLabel;	//���̸�ĳ��ȣ���λmm
  QPushButton*	mLength;		//�������̸�ĳ���
  QSlider*		mSlider;		//�����飬��ʾ֡����Ϣ 
  QPushButton*	mCalibrate;		//�궨��ť

  //���ⲿ����
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//����������Ļ��棨����ͼ���
  QGraphicsPixmapItem*	mRightCameraPixmap;	//����������Ļ��棨����ͼ���
  QStatusBar*				mStatusBar;			//״̬��
  QGraphicsScene*			mScene;				//����ָ��
};
#endif