#include "marker.h"
#include "QtWidgets/qgraphicsscene.h"
#include <iostream>

Marker::Marker(QPoint pos, QString name, QGraphicsScene* scene) {
  mScene = scene;
  mWidth = 21;
  mHeight = 21;

  //����λͼ��������λ��
  createPixmap(mWidth, mHeight);
  setPixmap(mPixmap);

  setPosition(pos);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | ItemSendsGeometryChanges | ItemIsFocusable);

  //�����ı�
  mText.setPlainText(name);
  mText.setDefaultTextColor(QColor(255, 0, 0));	//Ĭ��Ϊ��ɫ
  mText.setTextInteractionFlags(Qt::TextEditorInteraction);
  updateTextPos();

  mScene->addItem(this);
  mScene->addItem(&mText);
}
Marker::~Marker() {
  mScene->removeItem(this);
  mScene->removeItem(&mText);
}

//���ñ�ע��λ�ã��������Ϊ��־�����ĵ�λ�ã�
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

//ѡ������
bool Marker::isMarkerSelected() {
  return this->isSelected();
}
void Marker::setMarkerSelected() {
  this->setSelected(true);
}

//ѡ���ı�
bool Marker::isTextSelected() {
  return mText.isSelected();
}
void Marker::setTextSelected() {
  mText.setSelected(true);
}

//�Ƿ�ѡ��
bool Marker::getSelectedState() {
  return (mText.isSelected() || this->isSelected());
}

//���ص�ǰ���ı�
QString Marker::getText() {
  return mText.toPlainText();
}

//������仯�����
QVariant Marker::itemChange(GraphicsItemChange change, const QVariant & value) {
  //������ƶ���
  if (change == ItemPositionChange) {
    updateTextPos();
  }

  //����ѡ����
  if (change == ItemSelectedChange) {
    bool s = value.toBool();
    if (s)
      setAlpha(16, false);
    else
      setAlpha(192);
  }

  return QGraphicsPixmapItem::itemChange(change, value);
}
//�����������Ҽ�����Ӧ
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
//����ָʾ���,alphaΪ͸����0Ϊ��ȫ͸����bcΪtrue��ʾ����ʮ�ֽ�����
void Marker::createPixmap(int width, int height, int alpha, bool bc) {
  //���ñ���
  QColor color(255, 0, 0, alpha);
  QColor midClr(0, 0, 255);

  //����һ��λͼ
  QImage temp(width, height, QImage::Format_ARGB32);
  temp.fill(color);
  //��һ�С��м�һ�к����һ��
  uchar* pfl = temp.scanLine(0);
  uchar* pml = temp.scanLine(height / 2);
  uchar* pll = temp.scanLine(height - 1);
  for (int i = 0; i < width; i++) {
    *(pfl + 3 + 4 * i) = 255;
    *(pll + 3 + 4 * i) = 255;

    if (bc)
      *(pml + 3 + 4 * i) = 255;
  }
  //��һ�У��м�һ�к����һ��
  for (int i = 0; i < height; i++) {
    uchar* pfc = temp.scanLine(i);
    *(pfc + 3) = 255;
    *(pfc - 1 + 4 * width) = 255;
    if (bc)
      *(pfc + 4 * (width / 2) + 3) = 255;
  }
  //���м�һ����������Ϊ��ɫ
  *(pml + 4 * (width / 2)) = 255;
  *(pml + 4 * (width / 2) + 2) = 0;
  if (bc)
    *(pml + 4 * (width / 2) + 3) = 255;
  else
    *(pml + 4 * (width / 2) + 3) = 96;

  //��λͼ����pixmap
  mPixmap.convertFromImage(temp);
}
//���ò�͸����
void Marker::setAlpha(int alpha, bool bc) {
  createPixmap(mWidth, mHeight, alpha, bc);
  setPixmap(mPixmap);
}
//�����ı����λ��
void Marker::updateTextPos() {
  QPoint pos = getPosition();
  mText.setPos(pos.x() - mPixmap.width() / 2, pos.y() - mPixmap.height() - 10);
}