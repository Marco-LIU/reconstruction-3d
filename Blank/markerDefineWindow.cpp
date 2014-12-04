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

//�������ڲ���
void MarkerDefineWindow::createLayout()
{
	mCenterWidget = this;

	mMainLayout = new QHBoxLayout();
	mCenterWidget->setLayout(mMainLayout);

	mLeftLayout = new QVBoxLayout();
	mRightLayout = new QVBoxLayout();
	mMainLayout->addLayout(mLeftLayout,1);	//���Ұ�1:2�ָ�
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

	//���һЩ���ܰ�ť
	createWidget();
}

//������ť
void MarkerDefineWindow::createWidget()
{
	//1���Ԥ�����б�
	mLPredefine = new QLabel();
	mLPredefine->setText(QString::fromWCharArray(L"Ԥ����ĸ��ٵ�"));
	mAddToCur = new QPushButton();
	mAddToCur->setText(QString::fromWCharArray(L"��� >>"));
	connect(mAddToCur, SIGNAL(pressed()), this, SLOT(addToPredefineWindow()));
	mSelectAll= new QPushButton();
	mSelectAll->setText(QString::fromWCharArray(L"ȫѡ"));
	connect(mSelectAll, SIGNAL(pressed()), this, SLOT(selectAll()));
	mUnSelectAll=new QPushButton();
	mUnSelectAll->setText(QString::fromWCharArray(L"ȫ����ѡ"));
	connect(mUnSelectAll, SIGNAL(pressed()), this, SLOT(unSelectAll()));
	//Ԥ�����б�todo ����༭
	mDefList = new QListWidget();
	mDefList->setSelectionMode(QAbstractItemView::MultiSelection);	//�����ѡ
//todo ��䯸���ģ���޸���
createPredefineList(mDefList);
	mLeftLayout->addWidget(mLPredefine);
	mLeftLCtrlLayout->addWidget(mSelectAll);
	mLeftLCtrlLayout->addWidget(mUnSelectAll);
	mLeftLCtrlLayout->addWidget(mAddToCur);
	mLeftLayout->addLayout(mLeftLCtrlLayout);
	mLeftLayout->addWidget(mDefList);

	//2��ǰ���ٵ�
	mLCurMarker = new QLabel();
	mLCurMarker->setText(QString::fromWCharArray(L"��ǰʹ�õĸ��ٵ�"));
	mCurList= new QListWidget();
	mCurList->setDragDropMode(QAbstractItemView::InternalMove);
	mCurList->setDragEnabled(true);
//to delete just test
createPredefineList(mCurList);
	mMarkerDefLayout->addWidget(mLCurMarker);
	mMarkerDefLayout->addWidget(mCurList);
	mMarkerUp = new QPushButton();
	mMarkerUp->setText(QString::fromWCharArray(L"����"));
	connect(mMarkerUp, SIGNAL(pressed()), this, SLOT(markerUp()));
	mMarkerDown = new QPushButton();
	mMarkerDown->setText(QString::fromWCharArray(L"����"));
	connect(mMarkerDown, SIGNAL(pressed()), this, SLOT(markerDown()));
	mMarkerEdit = new QPushButton();
	mMarkerEdit->setText(QString::fromWCharArray(L"�༭"));
	connect(mMarkerEdit, SIGNAL(pressed()), this, SLOT(markerEdit()));
	mMarkerAdd = new QPushButton();
	mMarkerAdd->setText(QString::fromWCharArray(L"���"));
	connect(mMarkerAdd, SIGNAL(pressed()), this, SLOT(markerAdd()));
	mMarkerDelete = new QPushButton();
	mMarkerDelete->setText(QString::fromWCharArray(L"ɾ��"));
	connect(mMarkerDelete, SIGNAL(pressed()), this, SLOT(markerDelete()));
	//todo�������ˣ��Ժ������ť����
	mMarkerCtrlLayout->addStretch(2);
	mMarkerCtrlLayout->addWidget(mMarkerUp,1);
	mMarkerCtrlLayout->addWidget(mMarkerDown,1);
	mMarkerCtrlLayout->addWidget(mMarkerEdit,1);
	mMarkerCtrlLayout->addWidget(mMarkerAdd,1);
	mMarkerCtrlLayout->addWidget(mMarkerDelete,1);
	mMarkerCtrlLayout->addStretch(6);

	//3��ǰ������
	mLCurLine = new QLabel();
	mLCurLine->setText(QString::fromWCharArray(L"���ø��ٵ�����"));
	mLineList= new QListWidget();
	mLineList->setDragDropMode(QAbstractItemView::InternalMove);
	mLineList->setDragEnabled(true);
//to delete just test
createPredefineList(mLineList);
	mLineDefLayout->addWidget(mLCurLine);
	mLineDefLayout->addWidget(mLineList);	

	mLineUp = new QPushButton();
	mLineUp->setText(QString::fromWCharArray(L"����"));
	connect(mLineUp, SIGNAL(pressed()), this, SLOT(lineUp()));
	mLineDown = new QPushButton();
	mLineDown->setText(QString::fromWCharArray(L"����"));
	connect(mLineDown, SIGNAL(pressed()), this, SLOT(lineDown()));
	mLineEdit = new QPushButton();
	mLineEdit->setText(QString::fromWCharArray(L"�༭"));
	connect(mLineEdit, SIGNAL(pressed()), this, SLOT(lineEdit()));
	mLineAdd = new QPushButton();
	mLineAdd->setText(QString::fromWCharArray(L"���"));
	connect(mLineAdd, SIGNAL(pressed()), this, SLOT(lineAdd()));
	mLineDelete = new QPushButton();
	mLineDelete->setText(QString::fromWCharArray(L"ɾ��"));
	connect(mLineDelete, SIGNAL(pressed()), this, SLOT(lineDelete()));
	mAutoGenerate = new QPushButton();
	mAutoGenerate->setText(QString::fromWCharArray(L"�Զ�����"));
	connect(mAutoGenerate, SIGNAL(pressed()), this, SLOT(lineAutoGen()));
	//todo�������ˣ��Ժ������ť����
	mLineCtrlLayout->addStretch(2);
	mLineCtrlLayout->addWidget(mLineUp,1);
	mLineCtrlLayout->addWidget(mLineDown,1);
	mLineCtrlLayout->addWidget(mLineEdit,1);
	mLineCtrlLayout->addWidget(mLineAdd,1);
	mLineCtrlLayout->addWidget(mLineDelete,1);
	mLineCtrlLayout->addWidget(mAutoGenerate,1);
	mLineCtrlLayout->addStretch(5);
}

//�����б�
void MarkerDefineWindow::createPredefineList(QListWidget* lw)
{
	//todo ����ʵ�ʵ��������
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

//ѡ��ȫ��
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

//ȫ����ѡ
void MarkerDefineWindow::unSelectAll()
{
	for(int i=0;i<mDefList->count();i++)
	{
	  mDefList->item(i)->setSelected(false);
	}
}