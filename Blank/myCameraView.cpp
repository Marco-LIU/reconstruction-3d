#include "myCameraView.h"

#include <iostream>

//rect��Ӧ������Ҫ��ʾ�ľ�������
MyDetailView::MyDetailView(QGraphicsScene * parent, QRect rect)
  : QGraphicsView(parent) {
  //���û�������
  mScene = parent;
  mbDragging = false;
  mbZoomMode = true;
  mMaxScale = 10;
  setBackgroundBrush(QColor(255, 255, 255));

  //���ù۲�����
  mViewRect = rect;
  setSceneRect(mViewRect);

  //����ָʾ����
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

//����Ϊ�鿴ģʽ
void MyDetailView::setZoomMode() {
  mbZoomMode = true;
}
//����Ϊ�϶�ģʽ
void MyDetailView::setClickMode() {
  mbZoomMode = false;
}

//������Щ����¼�����Ҫ����ƶ������Ź���
//���ź�������С���ű���
void MyDetailView::resizeEvent(QResizeEvent * event) {
  //������С��������
  QSize oldSize = event->oldSize();
  QSize curSize = event->size();

  //������С���ű���
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

  //�����µ�����ϵ��
  if (oldSize.width() > 0) {
    float xs = curSize.width() / (float)oldSize.width();
    float ys = curSize.height() / (float)oldSize.height();

    //������С�����ű���
    float ms;
    if (xs < ys)
      ms = xs;
    else
      ms = ys;

    //��������ϵ��
    mScaleFactor = mScaleFactor*ms;
  }

  if (mScaleFactor < mMinScale)
    mScaleFactor = mMinScale;
  if (mScaleFactor > mMaxScale)
    mScaleFactor = mMaxScale;

  QTransform t(mScaleFactor, 0, 0, mScaleFactor, 0, 0);
  setTransform(t);

  //���¶�Ӧ��ָʾ����
  updateRect();
}
//�������Ŵ���
void MyDetailView::wheelEvent(QWheelEvent * event) {
  //�����ǰ��������С
  if (event->angleDelta().y() > 0) {
    mScaleFactor = mScaleFactor / 1.05;
  }
  //�����󻬶����Ŵ�
  else {
    mScaleFactor = mScaleFactor*1.05;
  }

  //Լ���������С��Χ��
  if (mScaleFactor < mMinScale)
    mScaleFactor = mMinScale;
  if (mScaleFactor > mMaxScale)
    mScaleFactor = mMaxScale;

  //��������
  QTransform t(mScaleFactor, 0, 0, mScaleFactor, 0, 0);
  setTransform(t);

  //�������ź�Ĵ��ڶ�Ӧ�ľ��ο�
  updateRect();
}
//�϶��޸ľ��ε�λ��
void MyDetailView::mouseMoveEvent(QMouseEvent * event) {
  if (mbZoomMode && mbDragging) {
    //��ǰ��
    QPoint cp = event->pos();

    //��������ƶ������������������ƶ�
    QPoint dp = mLastPoint - cp;
    int cx = horizontalScrollBar()->value();
    int cy = verticalScrollBar()->value();
    int nx = cx + dp.x();
    int ny = cy + dp.y();

    //Լ���ƶ���Χ
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

    //���¹�������λ��
    horizontalScrollBar()->setValue(nx);
    verticalScrollBar()->setValue(ny);

    //��¼��һ�εĵ�
    mLastPoint = cp;

    //�������ź�Ĵ��ڶ�Ӧ�ľ��ο�
    updateRect();
  } else {
    QGraphicsView::mouseMoveEvent(event);
  }
}
//��갴��
void MyDetailView::mousePressEvent(QMouseEvent * event) {
  //ִ��ѡ���������
  QGraphicsView::mousePressEvent(event);

  if (!mScene->selectedItems().empty()) {
    //�����屻ѡ������Ϊ�϶�ģʽ
    setClickMode();
  } else {
    //��������Ϊ����ģʽ
    setZoomMode();
  }

  if (mbZoomMode) {
    //��갴�£��л�Ϊ����
    setDragMode(QGraphicsView::ScrollHandDrag);
    QPoint cp = event->pos();
    mLastPoint = cp;
    mbDragging = true;
  }
}
//���̧��
void MyDetailView::mouseReleaseEvent(QMouseEvent * event) {
  //ִ��������������
  QGraphicsView::mouseReleaseEvent(event);

  if (mbZoomMode) {
    //����ͷţ��ָ����������״
    setDragMode(QGraphicsView::NoDrag);

    mbDragging = false;
  }
}
//˫������һ�������ź�
void MyDetailView::mouseDoubleClickEvent(QMouseEvent * event) {
  //��¼�µ����㣬��ת������������ϵ
  QPoint pos = event->pos();
  QPointF sp = mapToScene(pos);

  //�������ź�
  emit createMarker(QPoint(sp.x(), sp.y()));
}
//delete��������ɾ���ź�
void MyDetailView::keyPressEvent(QKeyEvent * event) {
  //�����delete����ɾ����ýǵ�ı����
  if (event->key() == Qt::Key_Delete) {
    emit deleteMarker();
  }
  QGraphicsView::keyPressEvent(event);
}
//�ػ���¶�Ӧ���ο��λ��(��Ӧ�������¼�)
void MyDetailView::paintEvent(QPaintEvent * event) {
  QGraphicsView::paintEvent(event);
  updateRect();
}

//������ʾ���ο�
void MyDetailView::showRect() {
  mCtrlRect->setVisible(true);
}
void MyDetailView::hideRect() {
  mCtrlRect->setVisible(false);
}
//����ָʾ���ο��ڳ����е�λ��
QPointF MyDetailView::getRectPos() {
  return mCtrlRect->rect().topLeft();
}
//���þ��ο��λ��,��������ϵ
void MyDetailView::setRectPos(int x, int y) {
  //���þ��ο��λ��
  QRectF curRect = mCtrlRect->rect();
  //Լ����������Χ��
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

  //�������ڵ����ο��λ��
  float ypos = y - mViewRect.y();
  this->verticalScrollBar()->setValue(ypos*mScaleFactor);
  //��������⣺ˮƽ����������ʼλ��Ϊ��������*��������
  //float xpos = x-mViewRect.x();
  float xpos = x;
  this->horizontalScrollBar()->setValue(xpos*mScaleFactor);

  //���ݴ����������¸����¾���ָʾ���λ��
  update();
}

//�����µĹ۲�����
void MyDetailView::setViewRect(QRect rect) {
  //���ù۲�����
  mViewRect = rect;
  setSceneRect(mViewRect);

  //���ݹ۲����򣬸�����С����ϵ��
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

  //���ݴ������򣬸��¾���ָʾ��
  updateRect();
}
//�ָ���ĳ���۲�״̬
void MyDetailView::restoreViewState(ZoomViewState zvs) {
  //������������ԣ��Żָ�ȫ��״̬
  if (zvs.save) {
    //��������ϵ��
    mMinScale = zvs.mins;
    mMaxScale = zvs.maxs;
    mScaleFactor = zvs.scale;

    //���ù۲�����
    setViewRect(zvs.viewRect);

    //�������ڵ�ָ��λ��
    horizontalScrollBar()->setValue(zvs.hvalue);
    verticalScrollBar()->setValue(zvs.vvalue);

    //���´��ڶ�Ӧ��ָʾ���ο�
    updateRect();
  }
  //����ֻ���¹۲�����
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

//rect��Ӧ������Ҫ��ʾ�ľ�������
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

//���ݴ��ڴ�С��������ϵ��
void MyCameraView::resizeEvent(QResizeEvent * event) {
  //�����������
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
//����,���¼����ݸ����Ŵ���
void MyCameraView::wheelEvent(QWheelEvent * event) {
  if (mZoomView != NULL) {
    mZoomView->wheelEvent(event);
  }
}
//�϶��޸ľ��ε�λ��
void MyCameraView::mouseMoveEvent(QMouseEvent * event) {
  //�϶����ο�
  if (mbDragging &&mZoomView != NULL) {
    //��ǰ��
    QPoint cp = event->pos();

    //��ӳ�䵽��������ϵ
    QPointF lsp = mapToScene(mLastPoint);
    QPointF csp = mapToScene(cp);
    QPointF mv = csp - lsp;

    //���㵱ǰ���ο��λ��
    QPointF newPos = mLastScenePoint + mv;
    mZoomView->setRectPos(newPos.x(), newPos.y());
  }
}
//��갴��
void MyCameraView::mousePressEvent(QMouseEvent * event) {
  //����һ����ý�����ź�
  emit getFocusSignal();

  //��갴�£��л�Ϊ����
  setDragMode(QGraphicsView::ScrollHandDrag);

  //��¼�°��µ��Ǹ���
  QPoint cp = event->pos();
  mLastPoint = cp;

  if (mZoomView != NULL) {
    mLastScenePoint = mZoomView->getRectPos();
  }
  mbDragging = true;
}
//���̧��
void MyCameraView::mouseReleaseEvent(QMouseEvent * event) {
  //����ͷţ��ָ����������״
  setDragMode(QGraphicsView::NoDrag);

  mbDragging = false;
}
//�ֲ���ͼ
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
//���浱ǰ����
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