#ifndef _MARKER_DEFINE_WINDOW_H_
#define _MARKER_DEFINE_WINDOW_H_

#include "QtWidgets/qwidget.h"
#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"
#include "QtGui/qimage.h"

class QPushButton;
class QLabel;
class QListWidget;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;

/*
  ���������Ҫ������ʾһ��Ԥ������ٵ�Ĵ���
  */
class MarkerDefineWindow : public QWidget
{
  Q_OBJECT
public:
  MarkerDefineWindow(QStatusBar* status);
signals:

public slots :
  //ѡ��ȫ��
  void selectAll();
  //ȫ����ѡ
  void unSelectAll();
  //��ӵ�Ԥ���崰��
  void addToPredefineWindow();

  //Ԥ����������
  void markerUp();
  //Ԥ����������
  void markerDown();
  //Ԥ�����Ǳ༭
  void markerEdit();
  //Ԥ���������
  void markerAdd();
  //Ԥ������ɾ��
  void markerDelete();

  //Ԥ����������
  void lineUp();
  //Ԥ����������
  void lineDown();
  //Ԥ�����߱༭
  void lineEdit();
  //Ԥ���������
  void lineAdd();
  //Ԥ������ɾ��
  void lineDelete();
  //���ݵ���β���ӣ��Զ�����Ԥ������
  void lineAutoGen();
private:
  //�������ڲ���
  void createLayout();

  //������ť
  void createWidget();

  //�����б�
  void createPredefineList(QListWidget* lw);
protected:
  //���Բ���

  //��ť
  QLabel*       mLPredefine;      //Ԥ������ǩ
  QLabel*       mLCurMarker;      //��ǰ������ǩ
  QLabel*       mLCurLine;        //��ǰ�����߱�ǩ
  QListWidget*  mDefList;         //Ԥ����ĸ��ٵ��б�
  QListWidget*  mCurList;         //��ǰʹ�ø��ٵ��б�
  QListWidget*  mLineList;        //��ǰʹ�õĸ��ٵ������б�

  QPushButton*  mAddToCur;        //��Ԥ�������ӵ���ǰ�ĸ��ٵ�
  QPushButton*  mSelectAll;       //ѡ��ȫ��
  QPushButton*  mUnSelectAll;     //ȫ����ѡ

  QPushButton*  mMarkerUp;        //���Ƶ�ǰ��ǵ�
  QPushButton*  mMarkerDown;      //���Ƶ�ǰ��ǵ�
  QPushButton*  mMarkerAdd;       //���Ƶ�ǰ��ǵ�
  QPushButton*  mMarkerDelete;    //���Ƶ�ǰ��ǵ�
  QPushButton*  mMarkerEdit;      //�༭��ǰ��ǵ�

  QPushButton*  mLineUp;          //���Ƶ�ǰ��ǵ�
  QPushButton*  mLineDown;        //���Ƶ�ǰ��ǵ�
  QPushButton*  mLineEdit;        //�༭��ǰ��ǵ�
  QPushButton*  mLineAdd;         //���Ƶ�ǰ��ǵ�
  QPushButton*  mLineDelete;      //���Ƶ�ǰ��ǵ�
  QPushButton*  mAutoGenerate;    //�Զ����ɱ�ǩ

  //���ڲ���
  QWidget*      mCenterWidget;    //���Ĵ���(��������,this)
  QHBoxLayout*  mMainLayout;      //������
  QVBoxLayout*  mLeftLayout;      //�󲼾�
  QVBoxLayout*  mRightLayout;     //�Ҳ���
  QHBoxLayout*  mLeftLCtrlLayout; //Ԥ�����б�ť
  QHBoxLayout*  mMarkerLayout;    //��ǰ��Ԥ����㲼��
  QVBoxLayout*  mMarkerDefLayout; //Ԥ������б���
  QVBoxLayout*  mMarkerCtrlLayout;//Ԥ����㰴ť
  QHBoxLayout*  mLineLayout;      //��ǰ��Ԥ�����߲���
  QVBoxLayout*  mLineDefLayout;   //Ԥ������б���
  QVBoxLayout*  mLineCtrlLayout;  //Ԥ����㰴ť

  //���ⲿ����
  QStatusBar*	mStatusBar;			//״̬��
};

#endif