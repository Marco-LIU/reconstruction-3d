//Qt5在vs2010下，需要这个指令
#pragma execution_character_set("utf-8")
//由于使用了utf8编码，调试的时候使用english，否则gbk会出现乱码

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
  //创建程序和窗口
  QApplication a(argc, argv);

  //初始化程序参数
  new Paras();
  Paras::getSingleton().width = 1280;
  Paras::getSingleton().height = 1024;
  Paras::getSingleton().LeftImagesFoler = "./save/left/";
  Paras::getSingleton().RightImagesFoler ="./save/right/";
  Paras::getSingleton().ImagesFoler = "./save/";
  //必须创建QApplication后，才能创建Qt的对象
  Paras::getSingleton().LeftBlankImg.load("left.jpg");
  Paras::getSingleton().RightBlankImg.load("right.jpg");
  
  MainWindow w;
  //显示窗口
  w.show();

  //处理窗口消息事件
    return a.exec();	
}