#ifndef _MARKER_DEFINE_WINDOW_H_
#define _MARKER_DEFINE_WINDOW_H_

#include "QtWidgets/qwidget.h"
#include "QtCore/qstring.h"
#include "QtCore/qtimer.h"
#include "QtGui/qimage.h"

#include <QtWidgets\qmessagebox.h>
#include <QtWidgets\qlistwidget.h>

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
  void addToPredefineWindow()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu add it,you have the model");	//�ı�����
  }

  //Ԥ����������
  void markerUp()
  {
	  //���ص�ǰѡ�����
	  QListWidgetItem* temp = mCurList->selectedItems().first();
	  int row = mCurList->row(temp);
	  //ɾ��
	  mCurList->takeItem(row);
	  //���룬û�м����β
	  mCurList->insertItem(row-1,temp);
	  //ѡ��ǰ
	  mCurList->setCurrentRow(row-1);
	  //todoͬʱ�޸�ģ��
  }
  //Ԥ����������
  void markerDown()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu move model ");	//�ı�����
  }
  //Ԥ�����Ǳ༭
  void markerEdit()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu edit model,pop up a edit window");	//�ı�����
  }
  //Ԥ���������
  void markerAdd()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu add model,pop up a input window");	//�ı�����
  }
  //Ԥ������ɾ��
  void markerDelete()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu delete model");	//�ı�����
  }

  //Ԥ����������
  void lineUp()
  {
	  //���ص�ǰѡ�����
	  QListWidgetItem* temp = mLineList->selectedItems().first();
	  int row = mLineList->row(temp);

	  //todo ������β�ж�
	  //ɾ��
	  mLineList->takeItem(row);
	  //����
	  mLineList->insertItem(row-1,temp);
	  //ѡ��ǰ
	  mLineList->setCurrentRow(row-1);
	  //ͬʱ�޸�ģ��
  }
  //Ԥ����������
  void lineDown()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu move model ");	//�ı�����
  }
  //Ԥ�����߱༭
  void lineEdit()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu edit model,pop up a edit window");	//�ı�����
  }
  //Ԥ���������
  void lineAdd()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu add model,pop up a input window");	//�ı�����
  }
  //Ԥ������ɾ��
  void lineDelete()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu delete model");	//�ı�����
  }
  //���ݵ���β���ӣ��Զ�����Ԥ������
  void lineAutoGen()
  {
	QMessageBox::about(
		0,				//������
		"msg",		//������
		"liuliu generate the line model");	//�ı�����
  }
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
  QLabel*		mLPredefine;		//Ԥ������ǩ
  QLabel*		mLCurMarker;		//��ǰ������ǩ
  QLabel*		mLCurLine;			//��ǰ�����߱�ǩ
  QListWidget*	mDefList;			//Ԥ����ĸ��ٵ��б�
  QListWidget*	mCurList;			//��ǰʹ�ø��ٵ��б�
  QListWidget*	mLineList;			//��ǰʹ�õĸ��ٵ������б�

  QPushButton*	mAddToCur;			//��Ԥ�������ӵ���ǰ�ĸ��ٵ�
  QPushButton*	mSelectAll;			//ѡ��ȫ��
  QPushButton*	mUnSelectAll;		//ȫ����ѡ

  QPushButton*	mMarkerUp;			//���Ƶ�ǰ��ǵ�
  QPushButton*	mMarkerDown;		//���Ƶ�ǰ��ǵ�
  QPushButton*	mMarkerAdd;			//���Ƶ�ǰ��ǵ�
  QPushButton*	mMarkerDelete;		//���Ƶ�ǰ��ǵ�
  QPushButton*	mMarkerEdit;		//�༭��ǰ��ǵ�

  QPushButton*	mLineUp;			//���Ƶ�ǰ��ǵ�
  QPushButton*	mLineDown;			//���Ƶ�ǰ��ǵ�
  QPushButton*	mLineEdit;			//�༭��ǰ��ǵ�
  QPushButton*	mLineAdd;			//���Ƶ�ǰ��ǵ�
  QPushButton*	mLineDelete;		//���Ƶ�ǰ��ǵ�
  QPushButton*	mAutoGenerate;		//�Զ����ɱ�ǩ

  //���ڲ���
  QWidget*		mCenterWidget;		//���Ĵ���(��������,this)
  QHBoxLayout*	mMainLayout;		//������
  QVBoxLayout*	mLeftLayout;		//�󲼾�
  QVBoxLayout*	mRightLayout;		//�Ҳ���
  QHBoxLayout*	mLeftLCtrlLayout;	//Ԥ�����б�ť
  QHBoxLayout*	mMarkerLayout;		//��ǰ��Ԥ����㲼��
  QVBoxLayout*	mMarkerDefLayout;	//Ԥ������б���
  QVBoxLayout*  mMarkerCtrlLayout;	//Ԥ����㰴ť
  QHBoxLayout*	mLineLayout;		//��ǰ��Ԥ�����߲���
  QVBoxLayout*	mLineDefLayout;		//Ԥ������б���
  QVBoxLayout*  mLineCtrlLayout;	//Ԥ����㰴ť

  //���ⲿ����
  QStatusBar*	mStatusBar;			//״̬��
};

#endif