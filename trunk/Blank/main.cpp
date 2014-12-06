//Qt5��vs2010�£���Ҫ���ָ��
#pragma execution_character_set("utf-8")
//����ʹ����utf8���룬���Ե�ʱ��ʹ��english������gbk���������

#include <iostream>

#include "highgui.h"
#include "cv.h"
#include "UsbCameras.h"
#include "paras.h"

#include <QtWidgets\qapplication.h>
#include <QtWidgets\qmainwindow.h>
#include "mainwindow.h"

#include "base/command_line.h"
#include "base/at_exit.h"
#include "base/message_loop/message_loop.h"

#include "runtime_context.h"
#include "working_thread.h"

#ifdef _DEBUG
int main(int argc,char** argv)
#else
int CALLBACK WinMain(
  _In_  HINSTANCE hInstance,
  _In_  HINSTANCE hPrevInstance,
  _In_  LPSTR lpCmdLine,
  _In_  int nCmdShow
  )
#endif
{
  base::AtExitManager at_exit;
  CommandLine::Init(0, NULL);
  LLX_INFO() << "Start";
  LLX::AssureWorkingThreadStartup();

#ifndef _DEBUG
  int argc = 0;
  char** argv = NULL;
#endif

  //��������ʹ���
  QApplication a(argc, argv);

  //��ʼ���������
  new Paras();
  Paras::getSingleton().width = 1280;
  Paras::getSingleton().height = 1024;
  Paras::getSingleton().LeftImagesFoler = "./save/left/";
  Paras::getSingleton().RightImagesFoler ="./save/right/";
  Paras::getSingleton().ImagesFoler = "./save/";
  //���봴��QApplication�󣬲��ܴ���Qt�Ķ���
  Paras::getSingleton().LeftBlankImg.load("left.jpg");
  Paras::getSingleton().RightBlankImg.load("right.jpg");

  MainWindow w;
  //��ʾ����
  w.show();

  // ��ʼ��QT����Ϣ���ӣ�����ʹ�������Լ�����Ϣѭ��ʱ�޷�����QT��Ϣ
  QApplication::processEvents();

  // �����Լ�����Ϣѭ��
  base::MessageLoopForUI ui_ml;
  ui_ml.Run();

  LLX::NotifyWorkingThreadShutdown();

  LLX_INFO() << "End";

  return 0;

  //��������Ϣ�¼�
  //return a.exec();
}