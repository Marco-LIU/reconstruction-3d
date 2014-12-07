#include "myCameraView.h"

#include <iostream>

//rect对应场景中要显示的矩形区域
MyDetailView::MyDetailView(QGraphicsScene * parent, QRect rect)
  : QGraphicsView(parent) {
  //设置基本属性
  mScene = parent;
  mbDragging = false;
  mbZoomMode = true;
  mMaxScale = 10;
  setBackgroundBrush(QColor(255, 255, 255));

  //设置观察区域
  mViewRect = rect;
  setSceneRect(mViewRect);

  //设置指示矩形
  QPen pen(QColor(0, 0, 255));
  pen.setWidth(5);
  mCtrlRect = mScene->addRect(QRectF(0, 0, 1, 1));
  mCtrlRect->setPen(pen);

  connect(mScene, SIGNAL(destroyed()), this, SLOT(OnSceneDestroyed()));
}
MyDetailView::~MyDetailView() {
  if (mScene) {
    disconnect(mScene, SIGNAL(destroyed()), this, SLOT(OnSceneDestroyed()));
  }

  if (mCtrlRect && mScene)
    mScene->removeItem(mCtrlRect);
}

void MyDetailView::OnSceneDestroyed() {
  mScene = NULL;
  mCtrlRect = NULL;
}

//设置为查看模式
void MyDetailView::setZoomMode() {
  mbZoomMode = true;
}
//设置为拖动模式
void MyDetailView::setClickMode() {
  mbZoomMode = false;
}

//下面这些鼠标事件，主要完成移动，缩放功能
//缩放后，设置最小缩放比例
void MyDetailView::resizeEvent(QResizeEvent * event) {
  //设置最小缩放因子
  QSize oldSize = event->oldSize();
  QSize curSize = event->size();

  //更新最小缩放比例
  if (curSize.width() > 0) {
    float scaleHeight = (float)curSize.height() / mViewRect.height();
    float scaleWidth = (float)curSize.width() / mViewRect.width();
    float ms = 1;
    if (scaleHeight > scaleWidth)
      ms = scaleHeight;
    else
      ms = scaleWidth;
    mMinScale = ms;
  }

  //计算新的缩放系数
  if (oldSize.width() > 0) {
    float xs = curSize.width() / (float)oldSize.width();
    float ys = curSize.height() / (float)oldSize.height();

    //计算最小的缩放倍数
    float ms;
    if (xs < ys)
      ms = xs;
    else
      ms = ys;

    //更新缩放系数
    mScaleFactor = mScaleFactor*ms;
  }

  if (mScaleFactor < mMinScale)
    mScaleFactor = mMinScale;
  if (mScaleFactor > mMaxScale)
    mScaleFactor = mMaxScale;

  QTransform t(mScaleFactor, 0, 0, mScaleFactor, 0, 0);
  setTransform(t);

  //更新对应的指示矩形
  updateRect();
}
//滚动缩放窗口
void MyDetailView::wheelEvent(QWheelEvent * event) {
  //鼠标向前滑动，缩小
  if (event->angleDelta().y() > 0) {
    mScaleFactor = mScaleFactor / 1.05;
  }
  //鼠标向后滑动，放大
  else {
    mScaleFactor = mScaleFactor*1.05;
  }

  //约束到最大最小范围内
  if (mScaleFactor < mMinScale)
    mScaleFactor = mMinScale;
  if (mScaleFactor > mMaxScale)
    mScaleFactor = mMaxScale;

  //更新缩放
  QTransform t(mScaleFactor, 0, 0, mScaleFactor, 0, 0);
  setTransform(t);

  //更新缩放后的窗口对应的矩形框
  updateRect();
}
//拖动修改矩形的位置
void MyDetailView::mouseMoveEvent(QMouseEvent * event) {
  if (mbZoomMode && mbDragging) {
    //当前点
    QPoint cp = event->pos();

    //根据鼠标移动，计算滚动条的相对移动
    QPoint dp = mLastPoint - cp;
    int cx = horizontalScrollBar()->value();
    int cy = verticalScrollBar()->value();
    int nx = cx + dp.x();
    int ny = cy + dp.y();

    //约束移动范围
    if (nx < 0)
      nx = 0;
    if (ny < 0)
      ny = 0;
    int mx = horizontalScrollBar()->maximum();
    int my = verticalScrollBar()->maximum();
    if (nx > mx)
      nx = mx;
    if (ny > my)
      ny = my;

    //更新滚动条的位置
    horizontalScrollBar()->setValue(nx);
    verticalScrollBar()->setValue(ny);

    //记录上一次的点
    mLastPoint = cp;

    //更新缩放后的窗口对应的矩形框
    updateRect();
  } else {
    QGraphicsView::mouseMoveEvent(event);
  }
}
//鼠标按下
void MyDetailView::mousePressEvent(QMouseEvent * event) {
  //执行选择物体操作
  QGraphicsView::mousePressEvent(event);

  if (!mScene->selectedItems().empty()) {
    //有物体被选择设置为拖动模式
    setClickMode();
  } else {
    //否则设置为缩放模式
    setZoomMode();
  }

  if (mbZoomMode) {
    //鼠标按下，切换为手型
    setDragMode(QGraphicsView::ScrollHandDrag);
    QPoint cp = event->pos();
    mLastPoint = cp;
    mbDragging = true;
  }
}
//鼠标抬起
void MyDetailView::mouseReleaseEvent(QMouseEvent * event) {
  //执行正常的鼠标操作
  QGraphicsView::mouseReleaseEvent(event);

  if (mbZoomMode) {
    //鼠标释放，恢复正常鼠标形状
    setDragMode(QGraphicsView::NoDrag);

    mbDragging = false;
  }
}
//双击发出一个创建信号
void MyDetailView::mouseDoubleClickEvent(QMouseEvent * event) {
  //记录下单击点，并转换到场景坐标系
  QPoint pos = event->pos();
  QPointF sp = mapToScene(pos);

  //并触发信号
  emit createMarker(QPoint(sp.x(), sp.y()));
}
//delete键，发出删除信号
void MyDetailView::keyPressEvent(QKeyEvent * event) {
  //如果是delete键，删除获得角点的标记物
  if (event->key() == Qt::Key_Delete) {
    emit deleteMarker();
  }
  QGraphicsView::keyPressEvent(event);
}
//重绘更新对应矩形框的位置(响应滚动条事件)
void MyDetailView::paintEvent(QPaintEvent * event) {
  QGraphicsView::paintEvent(event);
  updateRect();
}

//隐藏显示矩形框
void MyDetailView::showRect() {
  mCtrlRect->setVisible(true);
}
void MyDetailView::hideRect() {
  mCtrlRect->setVisible(false);
}
//返回指示矩形框在场景中的位置
QPointF MyDetailView::getRectPos() {
  return mCtrlRect->rect().topLeft();
}
//设置矩形框的位置,场景坐标系
void MyDetailView::setRectPos(int x, int y) {
  //设置矩形框的位置
  QRectF curRect = mCtrlRect->rect();
  //约束到场景范围内
  if (x<mViewRect.x())
    x = mViewRect.x();
  if (x + curRect.width() > mViewRect.right())
    x = mViewRect.right() - curRect.width();
  if (y<mViewRect.y())
    y = mViewRect.y();
  if (y + curRect.height() > mViewRect.bottom())
    y = mViewRect.bottom() - curRect.height();
  QRectF newRect(x, y, curRect.width(), curRect.height());
  mCtrlRect->setRect(newRect);

  //滚动窗口到矩形框的位置
  float ypos = y - mViewRect.y();
  this->verticalScrollBar()->setValue(ypos*mScaleFactor);
  //这里很特殊：水平滑动条的起始位置为场景坐标*缩放因子
  //float xpos = x-mViewRect.x();
  float xpos = x;
  this->horizontalScrollBar()->setValue(xpos*mScaleFactor);

  //根据窗口区域，重新更新下矩形指示框的位置
  update();
}

//设置新的观察区域
void MyDetailView::setViewRect(QRect rect) {
  //设置观察区域
  mViewRect = rect;
  setSceneRect(mViewRect);

  //根据观察区域，更新最小缩放系数
  QRect size = this->rect();
  float scaleHeight = (float)size.height() / mViewRect.height();
  float scaleWidth = (float)size.width() / mViewRect.width();
  float ms = 1;
  if (scaleHeight > scaleWidth)
    ms = scaleHeight;
  else
    ms = scaleWidth;
  mMinScale = ms;
  if (mScaleFactor < mMinScale)
    mScaleFactor = mMinScale;
  QTransform t(mScaleFactor, 0, 0, mScaleFactor, 0, 0);
  setTransform(t);

  //根据窗口区域，更新矩形指示框
  updateRect();
}
//恢复到某个观察状态
void MyDetailView::restoreViewState(ZoomViewState zvs) {
  //如果保存了属性，才恢复全部状态
  if (zvs.save) {
    //更新缩放系数
    mMinScale = zvs.mins;
    mMaxScale = zvs.maxs;
    mScaleFactor = zvs.scale;

    //设置观察区域
    setViewRect(zvs.viewRect);

    //滑动窗口到指定位置
    horizontalScrollBar()->setValue(zvs.hvalue);
    verticalScrollBar()->setValue(zvs.vvalue);

    //更新窗口对应的指示矩形框
    updateRect();
  }
  //否则只更新观察区域
  else {
    setViewRect(zvs.viewRect);
  }
}

void MyDetailView::updateRect() {
  QRect size = this->viewport()->rect();
  QPointF tl = this->mapToScene(size.x(), size.y());
  QPointF rb = this->mapToScene(size.width(), size.height());
  mCtrlRect->setRect(QRectF(tl.x() - 5, tl.y() - 5, rb.x() - tl.x() + 10, rb.y() - tl.y() + 10));
}

//rect对应场景中要显示的矩形区域
MyCameraView::MyCameraView(QGraphicsScene * parent, QRect rect) :QGraphicsView(parent) {
  mZoomView = NULL;
  mScene = parent;
  mViewRect = rect;
  setBackgroundBrush(QColor(255, 255, 0));
  setSceneRect(mViewRect);
  mLastZoomState.save = false;
  mbDragging = false;
}
MyCameraView::~MyCameraView() {

}

//根据窗口大小调整缩放系数
void MyCameraView::resizeEvent(QResizeEvent * event) {
  //缩放左摄像机
  QRectF size = rect();
  float sx = size.width() / (float)mViewRect.width();
  float sy = size.height() / (float)mViewRect.height();
  sx = sx*0.95;
  sy = sy*0.95;
  if (sx < sy) {
    QTransform t(sx, 0, 0, sx, 0, 0);
    setTransform(t);
  } else {
    QTransform t(sy, 0, 0, sy, 0, 0);
    setTransform(t);
  }

  QGraphicsView::resizeEvent(event);
}
//滚动,把事件传递给缩放窗口
void MyCameraView::wheelEvent(QWheelEvent * event) {
  if (mZoomView != NULL) {
    mZoomView->wheelEvent(event);
  }
}
//拖动修改矩形的位置
void MyCameraView::mouseMoveEvent(QMouseEvent * event) {
  //拖动矩形框
  if (mbDragging &&mZoomView != NULL) {
    //当前点
    QPoint cp = event->pos();

    //都映射到场景坐标系
    QPointF lsp = mapToScene(mLastPoint);
    QPointF csp = mapToScene(cp);
    QPointF mv = csp - lsp;

    //计算当前矩形框的位置
    QPointF newPos = mLastScenePoint + mv;
    mZoomView->setRectPos(newPos.x(), newPos.y());
  }
}
//鼠标按下
void MyCameraView::mousePressEvent(QMouseEvent * event) {
  //触发一个获得焦点的信号
  emit getFocusSignal();

  //鼠标按下，切换为手型
  setDragMode(QGraphicsView::ScrollHandDrag);

  //记录下按下的那个点
  QPoint cp = event->pos();
  mLastPoint = cp;

  if (mZoomView != NULL) {
    mLastScenePoint = mZoomView->getRectPos();
  }
  mbDragging = true;
}
//鼠标抬起
void MyCameraView::mouseReleaseEvent(QMouseEvent * event) {
  //鼠标释放，恢复正常鼠标形状
  setDragMode(QGraphicsView::NoDrag);

  mbDragging = false;
}
//局部视图
void MyCameraView::setZoomView(MyDetailView* zv) {
  mZoomView = zv;
  mLastZoomState.viewRect = mViewRect;
}
void MyCameraView::unSetZoomView() {
  mZoomView = NULL;
}
MyDetailView* MyCameraView::getZoomView() {
  return mZoomView;
}
//保存当前属性
void MyCameraView::saveZoomViewState() {
  if (mZoomView != NULL) {
    mLastZoomState.viewRect = mViewRect;

    mLastZoomState.mins = mZoomView->mMinScale;
    mLastZoomState.maxs = mZoomView->mMaxScale;
    mLastZoomState.scale = mZoomView->mScaleFactor;

    mLastZoomState.hvalue = mZoomView->horizontalScrollBar()->value();
    mLastZoomState.vvalue = mZoomView->verticalScrollBar()->value();

    mLastZoomState.save = true;
  }
}
ZoomViewState MyCameraView::getSavedState() {
  return mLastZoomState;
}