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

int main(int argc,char** argv)
{
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

  //��������Ϣ�¼�
    return a.exec();	
}