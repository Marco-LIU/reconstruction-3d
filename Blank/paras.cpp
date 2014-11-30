#include "paras.h"
#include<assert.h>

template<> Paras* Singleton<Paras>::ms_Singleton = 0;
Paras* Paras::getSingletonPtr(void)
{
    return ms_Singleton;
}
Paras& Paras::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}