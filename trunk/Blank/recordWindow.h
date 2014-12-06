#ifndef _RECORD_WINDOW_H_
#define _RECORD_WINDOW_H_

//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include "QtCore/qobjectdefs.h"
#include "QtGui/qimage.h"
#include "QtWidgets/qwidget.h"
#include "OgreTimer.h"

#include "base/memory/ref_counted.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"

#include "camera_frame.h"

class QGraphicsScene;
class QGraphicsPixmapItem;
class QStatusBar;
class QTimer;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

class MyCameraView;
class MyDetailView;
class UsbCameraGroup;

/*
  ����࣬��Ҫ����¼����Ƶ�Ľ���
  */
class RecordWindow : public QWidget
{
  Q_OBJECT
public:
  RecordWindow(QGraphicsScene* scene,
               QGraphicsPixmapItem* left,
               QGraphicsPixmapItem* right,
               QStatusBar* status);

  ~RecordWindow();
  //�ѻҶ�����ת��ΪQImage��ʽ
  QImage convertToQImage(const unsigned char* buffer);
  //���³���ͼ��
  void updatePixmap(const unsigned char* leftBuffer,
                    const unsigned char* rightBuffer);
  void updatePixmap(QImage& li, QImage& ri);
signals:

public slots :
  //1¼����Ƶ��4��ť�Ĵ�������
  //Ԥ����Ƶ����Ļ���
  void preview();
  //ѡ�񱣴�λ��
  void selectSaveFolder();
  //¼����Ƶ
  void recordVedio();
  //����¼����Ƶ
  void stopRecordVedio();
  //�ر���Ƶ¼�ƽ���
  void closeRecordWindow();
  //����һ֡����ͼ��
  void updateOneFrame();
  //������ϸ��ͼΪ����ͼ
  void setLeftDetailView();
  //������ϸ��ͼΪ����ͼ
  void setRightDetailView();
private:
  //�������ڲ���
  void createLayout();

  //������ť
  void createWidget();

  void RecordFrame(CameraFrames& frames);

  void OnRecorderSaveDone();
protected:
  //����
  QTimer*		mTimer;		//��ʱ������
  Timer		mRecTimer;	//����¼�ƵĶ�ʱ��
  Timer		mProTimer;	//����ļ�ʱ��
  scoped_refptr<UsbCameraGroup>   mCameras;     //����ͷ

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
  QPushButton*	mPlay;			//Ԥ������,������ͣ��ť
  bool			mbPlay;			//true��ʾ����Ԥ��
  QPushButton*	mRecord;		//¼��,��ͣ��ť
  bool			mbRecord;		//true��ʾ����¼�ƣ�false��ʾ��ͣ¼��
  QPushButton*	mStop;			//ֹͣ¼�ư�ť
  bool			mbStop;			//true��ʾ�Ѿ�ֹͣ��¼��
  QPushButton*	mRecordFolder;	//����¼���ļ��еİ�ť
  int				mRecordCnt;		//��ǰ¼�Ƶ�֡����ÿ��¼�ƴ�1��ʼ
  int				mRecordStart;	//��ʼ¼��ʱ��ȫ��ʱ��

  //���ⲿ����
  QGraphicsPixmapItem*	mLeftCameraPixmap;	//����������Ļ��棨����ͼ���
  QGraphicsPixmapItem*	mRightCameraPixmap;	//����������Ļ��棨����ͼ���
  QStatusBar*				mStatusBar;			//״̬��
  QGraphicsScene*			mScene;				//����ָ��

  base::WeakPtrFactory<RecordWindow> weak_factory_;
};

#endif