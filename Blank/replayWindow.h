#ifndef _REPLAY_WINDOW_H_
#define _REPLAY_WINDOW_H_

//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"

#include "QtGui/qimage.h"

#include "QtWidgets/qwidget.h"

#include <vector>

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QTimer;
class QPushButton;
class QLabel;
class QSlider;

class MyDetailView;
class MyCameraView;

class ReplayWindow : public QWidget
{
  Q_OBJECT
public:
  ReplayWindow(QGraphicsScene* scene, QGraphicsPixmapItem* left,
               QGraphicsPixmapItem* right, QStatusBar* status);
signals:

public slots :
  //������ϸ��ͼΪ����ͼ
  void setLeftDetailView();
  //������ϸ��ͼΪ����ͼ
  void setRightDetailView();
  //�µ�λ��
  void updateImage(int value);
  //������Ƶ
  void playVedio();

  //����Ƶ�ļ�
  void open();
  //������Ƶ
  void play();
  //��һ֡
  void next();
  //��һ֡
  void before();
private:
  //����������ָ��ͼ��
  void loadImage(int i);

  //�������ڲ���
  void createLayout();

  //������ť
  void createWidget();
protected:
  //���Բ���
  QString			mLeftImagesFoler;	//��ͼ�񱣴��λ��
  QString			mRightImagesFoler;	//��ͼ�񱣴��λ��
  QString			mImagesFoler;		//���ļ���
  int				mStartIndex;		//��ʼ֡��ͼƬ����
  int				mEndIndex;			//����֡��ͼƬ����
  int				mStartTime;			//��ʼ֡��ʱ��(��λ����)
  int				mEndTime;			//����֡��ʱ��(��λ����)
  bool			mbOpen;				//true��ʾ������Ƶ�ļ�
  bool			mbPlay;				//true��ʾ���ţ�false��ʾ��ͣ
  QTimer*			mTimer;				//��ʱ������
  std::vector<int> mAllIndex;			//��¼������Ƭ������
  std::vector<int> mAllTimes;			//��¼������Ƭ������ʱ��

  //��ť
  QLabel*			mTime;				//��ʾʱ��
  QPushButton*	mPlay;				//������ͣ��ť
  QPushButton*	mNext;				//��һ֡
  QPushButton*	mBefore;			//��һ֡
  QPushButton*	mOpen;				//����Ƶ��ť
  QSlider*		mSlider;			//�����飬��ʾ֡����Ϣ 

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

  //���ⲿ����
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//����������Ļ��棨����ͼ���
  QGraphicsPixmapItem*	mRightCameraPixmap;	//����������Ļ��棨����ͼ���
  QStatusBar*				mStatusBar;			//״̬��
  QGraphicsScene*			mScene;				//����ָ��
};
#endif