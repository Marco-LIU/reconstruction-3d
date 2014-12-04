#include "markerDefineWindow.h"

#include <QtWidgets\qlistwidget.h>
#include <QtWidgets\qpushbutton.h>
#include <QtWidgets\qlayout.h>
#include <QtWidgets\qstatusbar.h>
#include <QtWidgets\qlabel.h>

MarkerDefineWindow::MarkerDefineWindow(QStatusBar* status)
{
	mStatusBar = status;

	createLayout();
}

//创建窗口布局
void MarkerDefineWindow::createLayout()
{
	mCenterWidget = this;

	mMainLayout = new QHBoxLayout();
	mCenterWidget->setLayout(mMainLayout);

	mLeftLayout = new QVBoxLayout();
	mRightLayout = new QVBoxLayout();
	mMainLayout->addLayout(mLeftLayout,1);	//左右按1:2分割
	mMainLayout->addLayout(mRightLayout,2);

	mMarkerLayout = new QHBoxLayout();
	mLeftLCtrlLayout = new QHBoxLayout();
	mMarkerDefLayout = new QVBoxLayout();
	mMarkerCtrlLayout = new QVBoxLayout();
	mMarkerLayout->addLayout(mMarkerDefLayout);
	mMarkerLayout->addLayout(mMarkerCtrlLayout);
	mRightLayout->addLayout(mMarkerLayout);

	mLineLayout = new QHBoxLayout();
	mLineDefLayout = new QVBoxLayout();
	mLineCtrlLayout = new QVBoxLayout();
	mLineLayout->addLayout(mLineDefLayout);
	mLineLayout->addLayout(mLineCtrlLayout);
	mRightLayout->addLayout(mLineLayout);

	//添加一些功能按钮
	createWidget();
}

//创建按钮
void MarkerDefineWindow::createWidget()
{
	//1左边预定义列表
	mLPredefine = new QLabel();
	mLPredefine->setText(QString::fromWCharArray(L"预定义的跟踪点"));
	mAddToCur = new QPushButton();
	mAddToCur->setText(QString::fromWCharArray(L"添加 >>"));
	connect(mAddToCur, SIGNAL(pressed()), this, SLOT(addToPredefineWindow()));
	mSelectAll= new QPushButton();
	mSelectAll->setText(QString::fromWCharArray(L"全选"));
	connect(mSelectAll, SIGNAL(pressed()), this, SLOT(selectAll()));
	mUnSelectAll=new QPushButton();
	mUnSelectAll->setText(QString::fromWCharArray(L"全部不选"));
	connect(mUnSelectAll, SIGNAL(pressed()), this, SLOT(unSelectAll()));
	//预定义列表，todo 允许编辑
	mDefList = new QListWidget();
	mDefList->setSelectionMode(QAbstractItemView::MultiSelection);	//允许多选
//todo 刘浏根据模型修改下
createPredefineList(mDefList);
	mLeftLayout->addWidget(mLPredefine);
	mLeftLCtrlLayout->addWidget(mSelectAll);
	mLeftLCtrlLayout->addWidget(mUnSelectAll);
	mLeftLCtrlLayout->addWidget(mAddToCur);
	mLeftLayout->addLayout(mLeftLCtrlLayout);
	mLeftLayout->addWidget(mDefList);

	//2当前跟踪点
	mLCurMarker = new QLabel();
	mLCurMarker->setText(QString::fromWCharArray(L"当前使用的跟踪点"));
	mCurList= new QListWidget();
	mCurList->setDragDropMode(QAbstractItemView::InternalMove);
	mCurList->setDragEnabled(true);
//to delete just test
createPredefineList(mCurList);
	mMarkerDefLayout->addWidget(mLCurMarker);
	mMarkerDefLayout->addWidget(mCurList);
	mMarkerUp = new QPushButton();
	mMarkerUp->setText(QString::fromWCharArray(L"上移"));
	connect(mMarkerUp, SIGNAL(pressed()), this, SLOT(markerUp()));
	mMarkerDown = new QPushButton();
	mMarkerDown->setText(QString::fromWCharArray(L"下移"));
	connect(mMarkerDown, SIGNAL(pressed()), this, SLOT(markerDown()));
	mMarkerEdit = new QPushButton();
	mMarkerEdit->setText(QString::fromWCharArray(L"编辑"));
	connect(mMarkerEdit, SIGNAL(pressed()), this, SLOT(markerEdit()));
	mMarkerAdd = new QPushButton();
	mMarkerAdd->setText(QString::fromWCharArray(L"添加"));
	connect(mMarkerAdd, SIGNAL(pressed()), this, SLOT(markerAdd()));
	mMarkerDelete = new QPushButton();
	mMarkerDelete->setText(QString::fromWCharArray(L"删除"));
	connect(mMarkerDelete, SIGNAL(pressed()), this, SLOT(markerDelete()));
	//todo就这样了，以后调整按钮布局
	mMarkerCtrlLayout->addStretch(2);
	mMarkerCtrlLayout->addWidget(mMarkerUp,1);
	mMarkerCtrlLayout->addWidget(mMarkerDown,1);
	mMarkerCtrlLayout->addWidget(mMarkerEdit,1);
	mMarkerCtrlLayout->addWidget(mMarkerAdd,1);
	mMarkerCtrlLayout->addWidget(mMarkerDelete,1);
	mMarkerCtrlLayout->addStretch(6);

	//3当前跟踪线
	mLCurLine = new QLabel();
	mLCurLine->setText(QString::fromWCharArray(L"设置跟踪点连线"));
	mLineList= new QListWidget();
	mLineList->setDragDropMode(QAbstractItemView::InternalMove);
	mLineList->setDragEnabled(true);
//to delete just test
createPredefineList(mLineList);
	mLineDefLayout->addWidget(mLCurLine);
	mLineDefLayout->addWidget(mLineList);	

	mLineUp = new QPushButton();
	mLineUp->setText(QString::fromWCharArray(L"上移"));
	connect(mLineUp, SIGNAL(pressed()), this, SLOT(lineUp()));
	mLineDown = new QPushButton();
	mLineDown->setText(QString::fromWCharArray(L"下移"));
	connect(mLineDown, SIGNAL(pressed()), this, SLOT(lineDown()));
	mLineEdit = new QPushButton();
	mLineEdit->setText(QString::fromWCharArray(L"编辑"));
	connect(mLineEdit, SIGNAL(pressed()), this, SLOT(lineEdit()));
	mLineAdd = new QPushButton();
	mLineAdd->setText(QString::fromWCharArray(L"添加"));
	connect(mLineAdd, SIGNAL(pressed()), this, SLOT(lineAdd()));
	mLineDelete = new QPushButton();
	mLineDelete->setText(QString::fromWCharArray(L"删除"));
	connect(mLineDelete, SIGNAL(pressed()), this, SLOT(lineDelete()));
	mAutoGenerate = new QPushButton();
	mAutoGenerate->setText(QString::fromWCharArray(L"自动生成"));
	connect(mAutoGenerate, SIGNAL(pressed()), this, SLOT(lineAutoGen()));
	//todo就这样了，以后调整按钮布局
	mLineCtrlLayout->addStretch(2);
	mLineCtrlLayout->addWidget(mLineUp,1);
	mLineCtrlLayout->addWidget(mLineDown,1);
	mLineCtrlLayout->addWidget(mLineEdit,1);
	mLineCtrlLayout->addWidget(mLineAdd,1);
	mLineCtrlLayout->addWidget(mLineDelete,1);
	mLineCtrlLayout->addWidget(mAutoGenerate,1);
	mLineCtrlLayout->addStretch(5);
}

//创建列表
void MarkerDefineWindow::createPredefineList(QListWidget* lw)
{
	//todo 根据实际的数据填充
	for(int i=0;i<17;i++)
	{
		QPixmap tp(64,64);
		tp.fill(QColor((Qt::GlobalColor)(i+2)));
		QIcon ti(tp);
		QString t;
		t.sprintf("test_%d",i);
		QListWidgetItem* pl = new QListWidgetItem(ti,t);

		lw->addItem(pl);
	}
}

//选择全部
void MarkerDefineWindow::selectAll()
{
	mDefList->selectAll();
	/*
	for(int i=0;i<mDefList->count();i++)
	{
	  mDefList->item(i)->setSelected(true);
	}
	*/
}

//全部不选
void MarkerDefineWindow::unSelectAll()
{
	for(int i=0;i<mDefList->count();i++)
	{
	  mDefList->item(i)->setSelected(false);
	}
}