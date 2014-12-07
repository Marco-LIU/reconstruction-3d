#ifndef _PARAS_H_
#define _PARAS_H_

/*
语法
1、单件类的实现
#include "stdafx.h"
#include<iostream>
#include "windows.h"
#include<assert.h>
using namespace std;

template<typename T> 
class Singleton
{
protected:
    static T* ms_Singleton;	//指向某种类的一个静态对象
public:
	//构造函数，设置私有变量ms_Singleton指向某种类的一个静态对象
    Singleton( void )
    {
        assert( !ms_Singleton );
		ms_Singleton = static_cast< T* >( this );
    }
    ~Singleton( void )
    {  
		assert( ms_Singleton );  
		ms_Singleton = 0;  
	}
    
	//返回单件类中保存的对象的引用
	static T& getSingleton( void )
	{	
		assert( ms_Singleton );  
		return ( *ms_Singleton ); 
	}
    //返回单件类中保存的对象的指针
	static T* getSingletonPtr( void )
	{ 
		return ms_Singleton; 
	}
};
class test:public Singleton<test>
{
public:
	void show()
	{
		cout<<"show"<<endl;
	}

	static test& getSingleton(void);

	static test* getSingletonPtr(void);
};

template<> test* Singleton<test>::ms_Singleton = 0;
test* test::getSingletonPtr(void)
{
    return ms_Singleton;
}
test& test::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}
int main(int argc, char ** argv)
{	
	test::getSingletonPtr()->show();
	int orz;
	cin>>orz;
    return 0;
}
上面的定义采用的是本工程中的定义

2、按我的理解可以把它按逻辑顺序分为下面的三个部分：
A、定义一个通用的模板单件类
template<typename T> 
class Singleton
{
protected:
    static T* ms_Singleton;	//指向某种类的一个静态对象
public:
	//构造函数，设置私有变量ms_Singleton指向某种类的一个静态对象
    Singleton( void )
    {
        assert( !ms_Singleton );
		ms_Singleton = static_cast< T* >( this );
    }
    ~Singleton( void )
    {  
		assert( ms_Singleton );  
		ms_Singleton = 0;  
	}
    
	//返回单件类中保存的对象的引用
	static T& getSingleton( void )
	{	
		assert( ms_Singleton );  
		return ( *ms_Singleton ); 
	}
    //返回单件类中保存的对象的指针
	static T* getSingletonPtr( void )
	{ 
		return ms_Singleton; 
	}
};

B、定义一个使用单件模式的类
class test
{
public:
	void show()
	{
		cout<<"show"<<endl;
	}
};

template<> test* Singleton<test>::ms_Singleton = 0;

template<>
test* Singleton<test>::getSingletonPtr(void)
{
    return ms_Singleton;
}

template<>
test& Singleton<test>::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}

class Test:public Singleton<test>
{
};

B1、特例化这个类型的单件类
class test
{
public:
	void show()
	{
		cout<<"show"<<endl;
	}
};

template<> test* Singleton<test>::ms_Singleton = 0;

template<>
test* Singleton<test>::getSingletonPtr(void)
{
    return ms_Singleton;
}

template<>
test& Singleton<test>::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}

B2、派生一个单件类
class Test:public Singleton<test>
{
};

C、使用这个类
Test::getSingletonPtr()->show();
*/

#include <QtCore\qstring.h>
#include <QtGui\qpixmap.h>
#include <set>

//定义一个单件类
template<typename T> 
class Singleton
{
protected:
	//静态变量必须定义，这里只是进行了声明
    static T* ms_Singleton;	//指向某种类的一个静态对象
public:
	//构造函数，设置私有变量ms_Singleton指向某种类的一个静态对象
    Singleton( void )
    {
        assert( !ms_Singleton );

		ms_Singleton = static_cast< T* >( this );
    }
    ~Singleton( void )
        {  assert( ms_Singleton );  ms_Singleton = 0;  }
    
	//返回单件类中保存的对象的引用
	static T& getSingleton( void )
	{	assert( ms_Singleton );  return ( *ms_Singleton ); }
    //返回单件类中保存的对象的指针
	static T* getSingletonPtr( void )
	{ return ms_Singleton; }
};

class ParaObserver;

//这个类保存了程序中所有要使用的参数
class Paras : public Singleton<Paras>
{
public:
	int			width;				//图像宽度
	int			height;				//图像高度
	QString		LeftImagesFoler;	//左图像保存的位置
	QString		RightImagesFoler;	//右图像保存的位置
	QString		ImagesFoler;		//总文件夹
	QPixmap		LeftBlankImg;		//左空白图像
	QPixmap		RightBlankImg;		//右空白图像

  int left_gain_;
  int right_gain_;
  int left_expo_;
  int right_expo_;

public:
  void SetLeftGain(int gain);
  void SetLeftExpo(int expo);
  void SetRightGain(int gain);
  void SetRightExpo(int expo);

  void SetGain(int id, int gain, bool trigger_event = true);
  void SetExpo(int id, int expo, bool trigger_event = true);

public:
	static Paras& getSingleton(void);
	static Paras* getSingletonPtr(void);

public:
  void AddOb(ParaObserver* ob);
  void RemoveOb(ParaObserver* ob);

private:
  std::set<ParaObserver*> observers_;
};

class ParaObserver {
public:
  virtual void OnGainChanged(int id, int gain) {}
  virtual void OnExposureChanged(int id, int expo) {}
};

#endif