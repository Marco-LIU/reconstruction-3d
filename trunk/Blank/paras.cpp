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

void Paras::AddOb(ParaObserver* ob) {
  observers_.insert(ob);
}

void Paras::RemoveOb(ParaObserver* ob) {
  observers_.erase(ob);
}

void Paras::SetLeftGain(int gain) {
  if (left_gain_ != gain) {
    std::set<ParaObserver*>::iterator it = observers_.begin();
    while (it != observers_.end()) {
      (*it)->OnGainChanged(0, gain);
      ++it;
    }
    left_gain_ = gain;
  }
}

void Paras::SetLeftExpo(int expo) {
  if (left_expo_ != expo) {
    std::set<ParaObserver*>::iterator it = observers_.begin();
    while (it != observers_.end()) {
      (*it)->OnExposureChanged(0, expo);
      ++it;
    }
    left_expo_ = expo;
  }
}

void Paras::SetRightGain(int gain) {
  if (right_gain_ != gain) {
    std::set<ParaObserver*>::iterator it = observers_.begin();
    while (it != observers_.end()) {
      (*it)->OnGainChanged(1, gain);
      ++it;
    }
    right_gain_ = gain;
  }
}

void Paras::SetRightExpo(int expo) {
  if (right_expo_ != expo) {
    std::set<ParaObserver*>::iterator it = observers_.begin();
    while (it != observers_.end()) {
      (*it)->OnExposureChanged(1, expo);
      ++it;
    }
    right_expo_ = expo;
  }
}