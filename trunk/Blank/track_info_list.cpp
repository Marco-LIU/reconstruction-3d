#include "track_info_list.h"
#include "QtWidgets/qtablewidget.h"

TrackInfoList::TrackInfoList() {
  Init();
}

TrackInfoList::TrackInfoList(const TrackInfos& infos)
    : infos_(infos) {
  Init();
}

TrackInfoList::~TrackInfoList() {

}

void TrackInfoList::set_infos(const TrackInfos& infos) {
  infos_ = infos;
  ShowInfos();
}

void TrackInfoList::Init() {
  CreateLayout();
  CreateWidget();
  ShowInfos();
}

void TrackInfoList::CreateLayout() {

}

void TrackInfoList::CreateWidget() {
  infos_table_view_ = new QTableWidget(this);
}

void TrackInfoList::ShowInfos() {
  infos_table_view_->clearContents();
  infos_table_view_->setColumnCount(2);
  TrackInfos::iterator it = infos_.begin();
  while (it != infos_.end()) {
    
    
    ++it;
  }
}