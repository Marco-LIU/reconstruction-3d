#ifndef _PARAS_H_
#define _PARAS_H_

/*
�﷨
1���������ʵ��
#include "stdafx.h"
#include<iostream>
#include "windows.h"
#include<assert.h>
using namespace std;

template<typename T> 
class Singleton
{
protected:
    static T* ms_Singleton;	//ָ��ĳ�����һ����̬����
public:
	//���캯��������˽�б���ms_Singletonָ��ĳ�����һ����̬����
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
    
	//���ص������б���Ķ��������
	static T& getSingleton( void )
	{	
		assert( ms_Singleton );  
		return ( *ms_Singleton ); 
	}
    //���ص������б���Ķ����ָ��
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
����Ķ�����õ��Ǳ������еĶ���

2�����ҵ������԰������߼�˳���Ϊ������������֣�
A������һ��ͨ�õ�ģ�嵥����
template<typename T> 
class Singleton
{
protected:
    static T* ms_Singleton;	//ָ��ĳ�����һ����̬����
public:
	//���캯��������˽�б���ms_Singletonָ��ĳ�����һ����̬����
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
    
	//���ص������б���Ķ��������
	static T& getSingleton( void )
	{	
		assert( ms_Singleton );  
		return ( *ms_Singleton ); 
	}
    //���ص������б���Ķ����ָ��
	static T* getSingletonPtr( void )
	{ 
		return ms_Singleton; 
	}
};

B������һ��ʹ�õ���ģʽ����
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

B1��������������͵ĵ�����
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

B2������һ��������
class Test:public Singleton<test>
{
};

C��ʹ�������
Test::getSingletonPtr()->show();
*/

#include <QtCore\qstring.h>
#include <QtGui\qpixmap.h>
#include <set>

//����һ��������
template<typename T> 
class Singleton
{
protected:
	//��̬�������붨�壬����ֻ�ǽ���������
    static T* ms_Singleton;	//ָ��ĳ�����һ����̬����
public:
	//���캯��������˽�б���ms_Singletonָ��ĳ�����һ����̬����
    Singleton( void )
    {
        assert( !ms_Singleton );

		ms_Singleton = static_cast< T* >( this );
    }
    ~Singleton( void )
        {  assert( ms_Singleton );  ms_Singleton = 0;  }
    
	//���ص������б���Ķ��������
	static T& getSingleton( void )
	{	assert( ms_Singleton );  return ( *ms_Singleton ); }
    //���ص������б���Ķ����ָ��
	static T* getSingletonPtr( void )
	{ return ms_Singleton; }
};

class ParaObserver;

//����ౣ���˳���������Ҫʹ�õĲ���
class Paras : public Singleton<Paras>
{
public:
	int			width;				//ͼ����
	int			height;				//ͼ��߶�
	QString		LeftImagesFoler;	//��ͼ�񱣴��λ��
	QString		RightImagesFoler;	//��ͼ�񱣴��λ��
	QString		ImagesFoler;		//���ļ���
	QPixmap		LeftBlankImg;		//��հ�ͼ��
	QPixmap		RightBlankImg;		//�ҿհ�ͼ��

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