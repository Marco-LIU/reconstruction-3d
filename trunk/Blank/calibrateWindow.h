#ifndef _CALIBRATE_WINDOW_H_
#define _CALIBRATE_WINDOW_H_

//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include "QtCore/qobjectdefs.h"
#include "QtGui/qimage.h"
#include "QtWidgets/qwidget.h"

#include "opencv2/core/core.hpp"
#include "base/memory/ref_counted.h"

#include "OgreTimer.h"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;
class QPushButton;
class QSlider;
class QLabel;

class MyCameraView;
class MyDetailView;
// class UsbCameras;
class UsbCameraGroup;

class CalibrateWindow : public QWidget
{
    Q_OBJECT
public:
  CalibrateWindow(QGraphicsScene* scene,QGraphicsPixmapItem* left,QGraphicsPixmapItem* right,
    QStatusBar* status,QString imgFolder = "./leftCamera/",bool bl = true);
  ~CalibrateWindow();
  //�ѻҶ�����ת��ΪQImage��ʽ
  QImage convertToQImage(unsigned char* buffer);
  //���³���ͼ��
  void updatePixmap(unsigned char* buffer);
  void updatePixmap(QImage& li);
signals:

public slots:
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
  //����x����Ľǵ����
  void setSizeX();
  //����y����Ľǵ����
  void setSizeY();
  //�������̸�ĳ���
  void setLength();
  //���б궨
  void calibrate();
  //����������µ�ǰѡ���
  void updateCurrent(int cs);
private:
  //�ѽǵ㱣�浽�ļ���,ǰ��3����xCorners,yCorners,Length,����������ռ����꣬������ͶӰͼ������
  void saveCornerPoints(std::string filename,std::vector<cv::Point2f> pts);

  //�������еĽǵ�����
  void loadCornerPoints(std::string filename,std::vector<cv::Point2f>& pts,std::vector<cv::Point3f>& opts);
  //�������ڲ���
  void createLayout();

  //������ť
  void createWidget();

  //���ݵ�ǰ�����ͼ�񣬸��¸�����ť��״̬
  void updateButtonState();
protected:
  //���ڲ���
  QWidget*        mCenterWidget;        //���Ĵ���(��������,this)
  QVBoxLayout*    mMainLayout;          //������
  QHBoxLayout*    mVedioLayout;         //�ϲ�����Ƶ����
  QHBoxLayout*    mTimeLineLayout;      //�²���ʱ���߿��Ʋ���
  QVBoxLayout*    mStereoLayout;        //���˫��Ƶ����
  QVBoxLayout*    mDetailLayout;        //�ұߵ�ϸ�ڿ��Ʋ���
  QHBoxLayout*    mCtrlLayout;          //�ϱߵĿ�����岼��
  MyCameraView*   mLeftCameraView;      //������������ĳ���
  MyCameraView*   mRightCameraView;     //������������ĳ���
  MyDetailView*   mZoomView;            //������ͼ

  QTimer*			mTimer;					//��ʱ������
  scoped_refptr<UsbCameraGroup>   mCameras;     //����ͷ
  int				mCameraId;				//0��ʾ������ͷ��1��ʾ������ͷ
  bool			mbLeftFocused;			//true��ʾ������ͷ��Ļ��ý���
  Timer			mProTimer;				//����ļ�ʱ��
  QImage			mLastCaptureImg;		//��һ�β����ͼ��
  int				mXCorners;				//x����Ľǵ����
  int				mYCorners;				//y����Ľǵ����
  int				mSquareLength;			//���̸�ĳ���
  std::vector<int> mImgs;					//�洢���е�ͼƬ
  QString			mImgFolders;			//�洢ͼ����ļ���
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