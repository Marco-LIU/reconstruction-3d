#include "paras.h"
#include<assert.h>
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/values.h"
#include "base/json/json_reader.h"

template<> Paras* Singleton<Paras>::ms_Singleton = 0;
Paras* Paras::getSingletonPtr(void)
{
    return ms_Singleton;
}
Paras& Paras::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}

void Paras::Load(const std::string& config_file) {
  base::FilePath data_path(L"./");
  data_path = data_path.AppendASCII(config_file);
  std::string data;
  if (base::ReadFileToString(data_path, &data)) {
    scoped_ptr<base::Value> v(base::JSONReader::Read(data));
    base::DictionaryValue* dict = NULL;

    if (v.get() && v->GetAsDictionary(&dict)) {
      if (dict->HasKey("ver")) {
        int ver = 0;
        bool g_reso = true;
        if (dict->GetInteger("ver", &ver) && ver == 1) {
          if (!dict->GetInteger("resolution", &resolution)) {
            g_reso = false;
            resolution = 0;
          }
          if (!dict->GetInteger("width", &width)) {
            width = 1280;
          }
          if (!dict->GetInteger("height", &height)) {
            height = 1024;
          }

          if (!dict->GetDouble("replay_speed", &replay_speed_)) {
            replay_speed_ = 1.0;
          }

          base::ListValue* cam_list = NULL;
          if (dict->GetList("cameras", &cam_list) && cam_list) {
            int count = cam_list->GetSize();
            for (int i = 0; i < count; ++i) {
              base::DictionaryValue* cam = NULL;
              if (cam_list->GetDictionary(i, &cam) && cam) {
                CameraConfig ccfg;
                if (CameraConfig::ParseFromDict(cam, ccfg)) {
                  if (g_reso) ccfg.reso = resolution;
                  camera_configs_[ccfg.sn] = ccfg;
                }
              }
            }
          }
        }
      }
    }
  }
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

void Paras::SetGain(int id, int gain, bool trigger_event) {
  if (id == 0) {
    if (trigger_event) {
      SetLeftGain(gain);
    } else {
      left_gain_ = gain;
    }
  } else if (id == 1) {
    if (trigger_event) {
      SetRightGain(gain);
    } else {
      right_gain_ = gain;
    }
  }
}

void Paras::SetExpo(int id, int expo, bool trigger_event /* = true */) {
  if (id == 0) {
    if (trigger_event) {
      SetLeftExpo(expo);
    } else {
      left_expo_ = expo;
    }
  } else if (id == 1) {
    if (trigger_event) {
      SetRightExpo(expo);
    } else {
      right_expo_ = expo;
    }
  }
}