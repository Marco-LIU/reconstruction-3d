#pragma once
#include "QtWidgets/qwidget.h"

#include <vector>
#include <string>

class QTableWidget;

class TrackInfo {
  std::string info_desc;
  std::string info_value;
};

typedef std::vector<TrackInfo> TrackInfos;


class TrackInfoList : public QWidget {
public:
  TrackInfoList();
  TrackInfoList(const TrackInfos& infos);
  ~TrackInfoList();

  const TrackInfos& infos() const { return infos_; }
  TrackInfos infos() { return infos_; }
  void set_infos(const TrackInfos& infos);

protected:
  //创建窗口布局
  void CreateLayout();
  //创建按钮
  void CreateWidget();

  void Init();

  void ShowInfos();

protected:
  QTableWidget*      infos_table_view_;
  
  TrackInfos infos_;
};