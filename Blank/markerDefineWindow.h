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
	这个窗口主要用于显示一个预定义跟踪点的窗口
*/
class MarkerDefineWindow : public QWidget
{
  Q_OBJECT
public:
  MarkerDefineWindow(QStatusBar* status);
signals:

public slots :
  //选择全部
  void selectAll();
  //全部不选
  void unSelectAll();
  //添加到预定义窗口
  void addToPredefineWindow()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu add it,you have the model");	//文本内容
  }

  //预定义标记上移
  void markerUp()
  {
	  //返回当前选择的项
	  QListWidgetItem* temp = mCurList->selectedItems().first();
	  int row = mCurList->row(temp);
	  //删除
	  mCurList->takeItem(row);
	  //插入，没有检查首尾
	  mCurList->insertItem(row-1,temp);
	  //选择当前
	  mCurList->setCurrentRow(row-1);
	  //todo同时修改模型
  }
  //预定义标记下移
  void markerDown()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu move model ");	//文本内容
  }
  //预定义标记编辑
  void markerEdit()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu edit model,pop up a edit window");	//文本内容
  }
  //预定义标记添加
  void markerAdd()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu add model,pop up a input window");	//文本内容
  }
  //预定义标记删除
  void markerDelete()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu delete model");	//文本内容
  }

  //预定义线上移
  void lineUp()
  {
	  //返回当前选择的项
	  QListWidgetItem* temp = mLineList->selectedItems().first();
	  int row = mLineList->row(temp);

	  //todo 增加首尾判断
	  //删除
	  mLineList->takeItem(row);
	  //插入
	  mLineList->insertItem(row-1,temp);
	  //选择当前
	  mLineList->setCurrentRow(row-1);
	  //同时修改模型
  }
  //预定义线下移
  void lineDown()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu move model ");	//文本内容
  }
  //预定义线编辑
  void lineEdit()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu edit model,pop up a edit window");	//文本内容
  }
  //预定义线添加
  void lineAdd()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu add model,pop up a input window");	//文本内容
  }
  //预定义线删除
  void lineDelete()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu delete model");	//文本内容
  }
  //根据点首尾连接，自动生成预定义线
  void lineAutoGen()
  {
	QMessageBox::about(
		0,				//父窗口
		"msg",		//标题栏
		"liuliu generate the line model");	//文本内容
  }
private:
  //创建窗口布局
  void createLayout();

  //创建按钮
  void createWidget();

  //创建列表
  void createPredefineList(QListWidget* lw);


protected:
  //属性参数
	
  //按钮
  QLabel*		mLPredefine;		//预定义点标签
  QLabel*		mLCurMarker;		//当前定义点标签
  QLabel*		mLCurLine;			//当前定义线标签
  QListWidget*	mDefList;			//预定义的跟踪点列表
  QListWidget*	mCurList;			//当前使用跟踪点列表
  QListWidget*	mLineList;			//当前使用的跟踪点连线列表

  QPushButton*	mAddToCur;			//把预定义的添加到当前的跟踪点
  QPushButton*	mSelectAll;			//选择全部
  QPushButton*	mUnSelectAll;		//全部不选

  QPushButton*	mMarkerUp;			//上移当前标记点
  QPushButton*	mMarkerDown;		//上移当前标记点
  QPushButton*	mMarkerAdd;			//上移当前标记点
  QPushButton*	mMarkerDelete;		//上移当前标记点
  QPushButton*	mMarkerEdit;		//编辑当前标记点

  QPushButton*	mLineUp;			//上移当前标记点
  QPushButton*	mLineDown;			//上移当前标记点
  QPushButton*	mLineEdit;			//编辑当前标记点
  QPushButton*	mLineAdd;			//上移当前标记点
  QPushButton*	mLineDelete;		//上移当前标记点
  QPushButton*	mAutoGenerate;		//自动生成标签

  //窗口布局
  QWidget*		mCenterWidget;		//中心窗口(即本窗口,this)
  QHBoxLayout*	mMainLayout;		//主布局
  QVBoxLayout*	mLeftLayout;		//左布局
  QVBoxLayout*	mRightLayout;		//右布局
  QHBoxLayout*	mLeftLCtrlLayout;	//预定义列表按钮
  QHBoxLayout*	mMarkerLayout;		//当前的预定义点布局
  QVBoxLayout*	mMarkerDefLayout;	//预定义点列表布局
  QVBoxLayout*  mMarkerCtrlLayout;	//预定义点按钮
  QHBoxLayout*	mLineLayout;		//当前的预定义线布局
  QVBoxLayout*	mLineDefLayout;		//预定义点列表布局
  QVBoxLayout*  mLineCtrlLayout;	//预定义点按钮

  //从外部传入
  QStatusBar*	mStatusBar;			//状态栏
};

#endif