#ifndef _MARKER_H_
#define _MARKER_H_


//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

#include <QtWidgets\qgraphicsitem.h>
#include "QtGui/qevent.h"
#include "QtCore/qstring.h"

class QGraphicsScene;

/*
  这个类用于提供一个带文本标签的十字指示框
  new Marker(QPoint(255,255),QString("01"),&mScene);
  */
class Marker : public QGraphicsPixmapItem
{
public:
  Marker(QPoint pos, QString name, QGraphicsScene* scene);
  ~Marker();

  //设置标注点位置（输入参数为标志点中心的位置）
  void setPosition(QPoint pos);
  QPoint getPositon();

  //选择标记物
  bool isMarkerSelected();
  void setMarkerSelected();

  //选择文本
  bool isTextSelected();
  void setTextSelected();

  //是否被选择
  bool getSelectedState();

  //返回当前的文本
  QString getText();
protected:
  //处理项变化的情况
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
  //处理上下左右键盘响应
  virtual void keyPressEvent(QKeyEvent* event);
private:
  //生成指示标记,alpha为透明度0为完全透明，bc为true表示绘制十字交叉线
  void createPixmap(int width = 21, int height = 21, int alpha = 192, bool bc = true);
  //设置不透明度
  void setAlpha(int alpha = 0, bool bc = true);
  //更新文本框的位置
  void updateTextPos();
private:
  int					mWidth;		//宽度
  int					mHeight;	//高度
  QPixmap				mPixmap;	//构造光标位图

  QGraphicsScene*		mScene;
  QGraphicsTextItem	mText;
};

#endif