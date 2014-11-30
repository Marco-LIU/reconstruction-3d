#ifndef _MY_CAMERA_VIEW_H_
#define _MY_CAMERA_VIEW_H_

//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

#include <QtWidgets\qgraphicsview.h>
#include <QtWidgets\qgraphicsitem.h>
#include <QtWidgets\qscrollbar.h>
#include <QtGui\qevent.h>

//这个结构，用于存储缩放场景的状态
struct ZoomViewState
{
  QRect viewRect;		//观察的场景

  float mins;			//最小缩放因子
  float maxs;			//最大缩放因子
  float scale;		//当前缩放因子

  int hvalue;			//当前水平滚动条的值
  int vvalue;			//当前垂直滚动条的值

  bool save;			//true表示存储过值
};

/*
  这个类可以显示场景中的一块矩形区域，可以单独使用。通常情况下它用于显示放大的局部视图，并进行一些鼠标交互功能
    1、缩放窗口更新比例因子
    2、鼠标拖动，更新窗口位置，更新矩形框位置
    3、滑动条滚动，更新窗口位置，更新矩形框的位置
    4、滑动条，上一页，更新矩形框的位置
    5、滑动条，上一点，更新矩形框的位置
    6、鼠标滚轮，设置比例因子，调整缩放后的位置

    7、显示，隐藏矩形框

    8、设置矩形框的位置
*/
class MyDetailView : public QGraphicsView
{
    Q_OBJECT
public:
  friend class  MyCameraView;
  //rect对应场景中要显示的矩形区域
  MyDetailView(QGraphicsScene * parent,QRect rect);
  ~MyDetailView();
  //设置为查看模式
  void setZoomMode();
  //设置为拖动模式
  void setClickMode();

  //下面这些鼠标事件，主要完成移动，缩放功能
  //缩放后，设置最小缩放比例
  virtual void resizeEvent(QResizeEvent * event);
  //滚动缩放窗口
  virtual void wheelEvent(QWheelEvent * event);
  //拖动修改矩形的位置
  virtual void mouseMoveEvent(QMouseEvent * event);
  //鼠标按下
  virtual void mousePressEvent(QMouseEvent * event);
  //鼠标抬起
  virtual void mouseReleaseEvent(QMouseEvent * event);
  //双击发出一个创建信号
  virtual void mouseDoubleClickEvent(QMouseEvent * event);
  //delete键，发出删除信号
  virtual void keyPressEvent(QKeyEvent * event);
  //重绘更新对应矩形框的位置(响应滚动条事件)
  virtual void paintEvent(QPaintEvent * event);

  //隐藏显示矩形框
  void showRect();
  void hideRect();
  //返回指示矩形框在场景中的位置
  QPointF getRectPos();
  //设置矩形框的位置,场景坐标系
  void setRectPos(int x,int y);
  
  //设置新的观察区域
  void setViewRect(QRect rect);
  //恢复到某个观察状态
  void restoreViewState(ZoomViewState zvs);
public slots:
  void OnSceneDestroyed();
signals:
  void createMarker(QPoint);
  void deleteMarker();
protected:
  //根据当前窗口区域，更新场景中的对应的指示矩形的大小和位置
  void updateRect();
private:
  QRect	mViewRect;		//观察的矩形	
  bool	mbDragging;		//true表示正在拖动
  bool	mbZoomMode;		//true表示在查看模式，false表示在拖动模式
  QPoint	mLastPoint;		//上一次按下的点
  float	mScaleFactor;	//缩放因子，默认为1
  float	mMaxScale;		//最大缩放因子
  float	mMinScale;		//最小缩放因子

  QGraphicsRectItem*	mCtrlRect;	//本窗口对应的矩形框
  QGraphicsScene*		mScene;		//所在的场景
};

/*
  这个类主要显示一个缩小的全局视图
    1、缩放窗口更新比例因子
    2、可以给它设置一个放大的局部视图，位置和大小以矩形框显示
    3、鼠标拖动可以改变矩形框的位置
*/
class MyCameraView : public QGraphicsView
{
    Q_OBJECT
public:
  //rect对应场景中要显示的矩形区域
  MyCameraView(QGraphicsScene * parent,QRect rect);
  ~MyCameraView();
  //根据窗口大小调整缩放系数
  virtual void resizeEvent(QResizeEvent * event);
  //滚动,把事件传递给缩放窗口
  virtual void wheelEvent(QWheelEvent * event);
  //拖动修改矩形的位置
  virtual void mouseMoveEvent(QMouseEvent * event);
  //鼠标按下
  virtual void mousePressEvent(QMouseEvent * event);
  //鼠标抬起
  virtual void mouseReleaseEvent(QMouseEvent * event);
public:
  //局部视图
  void setZoomView(MyDetailView* zv);
  void unSetZoomView();
  MyDetailView* getZoomView();
  //保存当前属性
  void saveZoomViewState();
  ZoomViewState getSavedState();
signals:
  void getFocusSignal();
private:
  bool			mbDragging;			//true表示正在拖动
  QPoint			mLastPoint;			//鼠标按下时的点
  QPointF			mLastScenePoint;	//鼠标按下时，矩形在场景坐标系中的点
  QRect			mViewRect;			//观察的矩形
  QGraphicsScene*	mScene;				//所在的场景

  MyDetailView*	mZoomView;		//局部区域放大的视频
  ZoomViewState	mLastZoomState;	//存储的局部区域放大窗口的属性
};
#endif