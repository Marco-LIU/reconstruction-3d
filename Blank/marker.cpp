#include "marker.h"
#include "QtWidgets/qgraphicsscene.h"
#include <iostream>

Marker::Marker(QPoint pos, QString name, QGraphicsScene* scene) {
  mScene = scene;
  mWidth = 21;
  mHeight = 21;

  //创建位图，并设置位置
  createPixmap(mWidth, mHeight);
  setPixmap(mPixmap);

  setPosition(pos);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | ItemSendsGeometryChanges | ItemIsFocusable);

  //创建文本
  mText.setPlainText(name);
  mText.setDefaultTextColor(QColor(255, 0, 0));	//默认为红色
  mText.setTextInteractionFlags(Qt::TextEditorInteraction);
  updateTextPos();

  mScene->addItem(this);
  mScene->addItem(&mText);
}
Marker::~Marker() {
  mScene->removeItem(this);
  mScene->removeItem(&mText);
}

//设置标注点位置（输入参数为标志点中心的位置）
void Marker::setPosition(QPoint pos) {
  QPoint temp = pos;
  temp.setX(temp.x() - mPixmap.width() / 2);
  temp.setY(temp.y() - mPixmap.height() / 2);
  this->setPos(temp);
}
QPoint Marker::getPosition() {
  QPoint temp = this->pos().toPoint();
  temp.setX(temp.x() + mPixmap.width() / 2);
  temp.setY(temp.y() + mPixmap.height() / 2);
  return temp;
}

//选择标记物
bool Marker::isMarkerSelected() {
  return this->isSelected();
}
void Marker::setMarkerSelected() {
  this->setSelected(true);
}

//选择文本
bool Marker::isTextSelected() {
  return mText.isSelected();
}
void Marker::setTextSelected() {
  mText.setSelected(true);
}

//是否被选择
bool Marker::getSelectedState() {
  return (mText.isSelected() || this->isSelected());
}

//返回当前的文本
QString Marker::getText() {
  return mText.toPlainText();
}

//处理项变化的情况
QVariant Marker::itemChange(GraphicsItemChange change, const QVariant & value) {
  //如果项移动了
  if (change == ItemPositionChange) {
    updateTextPos();
  }

  //如果项被选择了
  if (change == ItemSelectedChange) {
    bool s = value.toBool();
    if (s)
      setAlpha(16, false);
    else
      setAlpha(192);
  }

  return QGraphicsPixmapItem::itemChange(change, value);
}
//处理上下左右键盘响应
void Marker::keyPressEvent(QKeyEvent * event) {
  QPoint cp = getPosition();
  if (event->key() == Qt::Key_Up) {
    cp.setY(cp.y() - 1);
  }

  if (event->key() == Qt::Key_Down) {
    cp.setY(cp.y() + 1);
  }

  if (event->key() == Qt::Key_Left) {
    cp.setX(cp.x() - 1);
  }

  if (event->key() == Qt::Key_Right) {
    cp.setX(cp.x() + 1);
  }
  setPosition(cp);
}
//生成指示标记,alpha为透明度0为完全透明，bc为true表示绘制十字交叉线
void Marker::createPixmap(int width, int height, int alpha, bool bc) {
  //设置变量
  QColor color(255, 0, 0, alpha);
  QColor midClr(0, 0, 255);

  //构造一张位图
  QImage temp(width, height, QImage::Format_ARGB32);
  temp.fill(color);
  //第一行、中间一行和最后一行
  uchar* pfl = temp.scanLine(0);
  uchar* pml = temp.scanLine(height / 2);
  uchar* pll = temp.scanLine(height - 1);
  for (int i = 0; i < width; i++) {
    *(pfl + 3 + 4 * i) = 255;
    *(pll + 3 + 4 * i) = 255;

    if (bc)
      *(pml + 3 + 4 * i) = 255;
  }
  //第一列，中间一列和最后一列
  for (int i = 0; i < height; i++) {
    uchar* pfc = temp.scanLine(i);
    *(pfc + 3) = 255;
    *(pfc - 1 + 4 * width) = 255;
    if (bc)
      *(pfc + 4 * (width / 2) + 3) = 255;
  }
  //把中间一个像素设置为蓝色
  *(pml + 4 * (width / 2)) = 255;
  *(pml + 4 * (width / 2) + 2) = 0;
  if (bc)
    *(pml + 4 * (width / 2) + 3) = 255;
  else
    *(pml + 4 * (width / 2) + 3) = 96;

  //从位图构造pixmap
  mPixmap.convertFromImage(temp);
}
//设置不透明度
void Marker::setAlpha(int alpha, bool bc) {
  createPixmap(mWidth, mHeight, alpha, bc);
  setPixmap(mPixmap);
}
//更新文本框的位置
void Marker::updateTextPos() {
  QPoint pos = getPosition();
  mText.setPos(pos.x() - mPixmap.width() / 2, pos.y() - mPixmap.height() - 10);
}