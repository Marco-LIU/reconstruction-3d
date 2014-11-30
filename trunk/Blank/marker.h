#ifndef _MARKER_H_
#define _MARKER_H_


//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include <QtWidgets\qgraphicsitem.h>
#include "QtGui/qevent.h"
#include "QtCore/qstring.h"

class QGraphicsScene;

/*
  ����������ṩһ�����ı���ǩ��ʮ��ָʾ��
  new Marker(QPoint(255,255),QString("01"),&mScene);
  */
class Marker : public QGraphicsPixmapItem
{
public:
  Marker(QPoint pos, QString name, QGraphicsScene* scene);
  ~Marker();

  //���ñ�ע��λ�ã��������Ϊ��־�����ĵ�λ�ã�
  void setPosition(QPoint pos);
  QPoint getPositon();

  //ѡ������
  bool isMarkerSelected();
  void setMarkerSelected();

  //ѡ���ı�
  bool isTextSelected();
  void setTextSelected();

  //�Ƿ�ѡ��
  bool getSelectedState();

  //���ص�ǰ���ı�
  QString getText();
protected:
  //������仯�����
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
  //�����������Ҽ�����Ӧ
  virtual void keyPressEvent(QKeyEvent* event);
private:
  //����ָʾ���,alphaΪ͸����0Ϊ��ȫ͸����bcΪtrue��ʾ����ʮ�ֽ�����
  void createPixmap(int width = 21, int height = 21, int alpha = 192, bool bc = true);
  //���ò�͸����
  void setAlpha(int alpha = 0, bool bc = true);
  //�����ı����λ��
  void updateTextPos();
private:
  int					mWidth;		//���
  int					mHeight;	//�߶�
  QPixmap				mPixmap;	//������λͼ

  QGraphicsScene*		mScene;
  QGraphicsTextItem	mText;
};

#endif