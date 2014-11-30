#ifndef _MY_CAMERA_VIEW_H_
#define _MY_CAMERA_VIEW_H_

//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include <QtWidgets\qgraphicsview.h>
#include <QtWidgets\qgraphicsitem.h>
#include <QtWidgets\qscrollbar.h>
#include <QtGui\qevent.h>

//����ṹ�����ڴ洢���ų�����״̬
struct ZoomViewState
{
  QRect viewRect;		//�۲�ĳ���

  float mins;			//��С��������
  float maxs;			//�����������
  float scale;		//��ǰ��������

  int hvalue;			//��ǰˮƽ��������ֵ
  int vvalue;			//��ǰ��ֱ��������ֵ

  bool save;			//true��ʾ�洢��ֵ
};

/*
  ����������ʾ�����е�һ��������򣬿��Ե���ʹ�á�ͨ���������������ʾ�Ŵ�ľֲ���ͼ��������һЩ��꽻������
    1�����Ŵ��ڸ��±�������
    2������϶������´���λ�ã����¾��ο�λ��
    3�����������������´���λ�ã����¾��ο��λ��
    4������������һҳ�����¾��ο��λ��
    5������������һ�㣬���¾��ο��λ��
    6�������֣����ñ������ӣ��������ź��λ��

    7����ʾ�����ؾ��ο�

    8�����þ��ο��λ��
*/
class MyDetailView : public QGraphicsView
{
    Q_OBJECT
public:
  friend class  MyCameraView;
  //rect��Ӧ������Ҫ��ʾ�ľ�������
  MyDetailView(QGraphicsScene * parent,QRect rect);
  ~MyDetailView();
  //����Ϊ�鿴ģʽ
  void setZoomMode();
  //����Ϊ�϶�ģʽ
  void setClickMode();

  //������Щ����¼�����Ҫ����ƶ������Ź���
  //���ź�������С���ű���
  virtual void resizeEvent(QResizeEvent * event);
  //�������Ŵ���
  virtual void wheelEvent(QWheelEvent * event);
  //�϶��޸ľ��ε�λ��
  virtual void mouseMoveEvent(QMouseEvent * event);
  //��갴��
  virtual void mousePressEvent(QMouseEvent * event);
  //���̧��
  virtual void mouseReleaseEvent(QMouseEvent * event);
  //˫������һ�������ź�
  virtual void mouseDoubleClickEvent(QMouseEvent * event);
  //delete��������ɾ���ź�
  virtual void keyPressEvent(QKeyEvent * event);
  //�ػ���¶�Ӧ���ο��λ��(��Ӧ�������¼�)
  virtual void paintEvent(QPaintEvent * event);

  //������ʾ���ο�
  void showRect();
  void hideRect();
  //����ָʾ���ο��ڳ����е�λ��
  QPointF getRectPos();
  //���þ��ο��λ��,��������ϵ
  void setRectPos(int x,int y);
  
  //�����µĹ۲�����
  void setViewRect(QRect rect);
  //�ָ���ĳ���۲�״̬
  void restoreViewState(ZoomViewState zvs);
public slots:
  void OnSceneDestroyed();
signals:
  void createMarker(QPoint);
  void deleteMarker();
protected:
  //���ݵ�ǰ�������򣬸��³����еĶ�Ӧ��ָʾ���εĴ�С��λ��
  void updateRect();
private:
  QRect	mViewRect;		//�۲�ľ���	
  bool	mbDragging;		//true��ʾ�����϶�
  bool	mbZoomMode;		//true��ʾ�ڲ鿴ģʽ��false��ʾ���϶�ģʽ
  QPoint	mLastPoint;		//��һ�ΰ��µĵ�
  float	mScaleFactor;	//�������ӣ�Ĭ��Ϊ1
  float	mMaxScale;		//�����������
  float	mMinScale;		//��С��������

  QGraphicsRectItem*	mCtrlRect;	//�����ڶ�Ӧ�ľ��ο�
  QGraphicsScene*		mScene;		//���ڵĳ���
};

/*
  �������Ҫ��ʾһ����С��ȫ����ͼ
    1�����Ŵ��ڸ��±�������
    2�����Ը�������һ���Ŵ�ľֲ���ͼ��λ�úʹ�С�Ծ��ο���ʾ
    3������϶����Ըı���ο��λ��
*/
class MyCameraView : public QGraphicsView
{
    Q_OBJECT
public:
  //rect��Ӧ������Ҫ��ʾ�ľ�������
  MyCameraView(QGraphicsScene * parent,QRect rect);
  ~MyCameraView();
  //���ݴ��ڴ�С��������ϵ��
  virtual void resizeEvent(QResizeEvent * event);
  //����,���¼����ݸ����Ŵ���
  virtual void wheelEvent(QWheelEvent * event);
  //�϶��޸ľ��ε�λ��
  virtual void mouseMoveEvent(QMouseEvent * event);
  //��갴��
  virtual void mousePressEvent(QMouseEvent * event);
  //���̧��
  virtual void mouseReleaseEvent(QMouseEvent * event);
public:
  //�ֲ���ͼ
  void setZoomView(MyDetailView* zv);
  void unSetZoomView();
  MyDetailView* getZoomView();
  //���浱ǰ����
  void saveZoomViewState();
  ZoomViewState getSavedState();
signals:
  void getFocusSignal();
private:
  bool			mbDragging;			//true��ʾ�����϶�
  QPoint			mLastPoint;			//��갴��ʱ�ĵ�
  QPointF			mLastScenePoint;	//��갴��ʱ�������ڳ�������ϵ�еĵ�
  QRect			mViewRect;			//�۲�ľ���
  QGraphicsScene*	mScene;				//���ڵĳ���

  MyDetailView*	mZoomView;		//�ֲ�����Ŵ����Ƶ
  ZoomViewState	mLastZoomState;	//�洢�ľֲ�����Ŵ󴰿ڵ�����
};
#endif