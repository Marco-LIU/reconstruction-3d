#include "tracking_point.h"
#include "base/logging.h"
#include "base/memory/singleton.h"

//------------------------------------------------------------------------------
TrackingPointList::TrackingPointList(const std::string& cate)
    : category_(cate) {

}
//------------------------------------------------------------------------------
linked_ptr<TrackingPoint> TrackingPointList::get(unsigned int index) const {
  if (index >= count()) return linked_ptr<TrackingPoint>();
  PointList::const_iterator cit = IteratorFor(index);

  return *cit;
}
//------------------------------------------------------------------------------
linked_ptr<TrackingPoint> TrackingPointList::get(
    const std::string& name) const {
  PointMap::const_iterator cit = point_map_.find(name);
  if (cit != point_map_.end())
    return cit->second;
  return linked_ptr<TrackingPoint>();
}
//------------------------------------------------------------------------------
bool TrackingPointList::Insert(linked_ptr<TrackingPoint> point,
                               unsigned int index /* = kLastPosition */) {
  DCHECK(point.get());
  if (!point.get() || get(point->name).get()) return false;

  if (index == kLastPosition) {
    point_list_.push_back(point);
    point_map_[point->name] = point;
    return true;
  }

  if (index >= count()) return false;

  PointList::iterator cit = IteratorFor(index);

  point_list_.insert(cit, point);
  point_map_[point->name] = point;
  return true;
}
//------------------------------------------------------------------------------
bool TrackingPointList::Delete(unsigned int index) {
  linked_ptr<TrackingPoint> pt = DeleteInList(index);
  if (pt.get()) {
    PointMap::iterator it = point_map_.find(pt->name);
    if (it != point_map_.end())
      point_map_.erase(it);
    DeleteConnections(pt->name);
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool TrackingPointList::Delete(linked_ptr<TrackingPoint> point) {
  if (!point.get()) return false;
  return Delete(point->name);
}
//------------------------------------------------------------------------------
bool TrackingPointList::Delete(const std::string& point) {
  PointList::const_iterator cit = point_list_.begin();
  while (cit != point_list_.end()) {
    if ((*cit)->name == point) {
      point_list_.erase(cit);
      PointMap::iterator it = point_map_.find((*cit)->name);
      if (it != point_map_.end())
        point_map_.erase(it);
      DeleteConnections(point);
      return true;
    }
    ++cit;
  }
  return false;
}
//------------------------------------------------------------------------------
linked_ptr<TrackingPoint> TrackingPointList::DeleteInList(unsigned int index) {
  if (index >= count()) return linked_ptr<TrackingPoint>();
  PointList::iterator cit = IteratorFor(index);
  linked_ptr<TrackingPoint> result = *cit;
  point_list_.erase(cit);
  return result;
}
//------------------------------------------------------------------------------
void TrackingPointList::DeleteConnections(const std::string& point) {
  Connections::iterator it = connections_.begin();
  while (it != connections_.end()) {
    if (it->point1 == point || it->point2 == point) {
      it = connections_.erase(it);
    } else {
      ++it;
    }
  }
}
//------------------------------------------------------------------------------
bool TrackingPointList::Move(unsigned int index_from, unsigned int index_to) {
  if (index_from >= count() || index_to >= count()) return false;

  if (index_from == index_to) return true;
  linked_ptr<TrackingPoint> pt = get(index_from);
  if (index_from < index_to)
    --index_to;

  if (Delete(index_from)) {
    if (Insert(pt, index_to)) {
      return true;
    } else {
      Insert(pt, index_from);
      return false;
    }
  }
  return false;
}
//------------------------------------------------------------------------------
bool TrackingPointList::Move(linked_ptr<TrackingPoint> point,
                             unsigned int index_to) {
  if (!InList(point)) return false;
  return Move(point->name, index_to);
}
//------------------------------------------------------------------------------
bool TrackingPointList::Move(const std::string& point, unsigned int index_to) {
  PointList::const_iterator cit = point_list_.begin();
  int index = 0;
  bool found = false;
  while (cit != point_list_.end()) {
    if ((*cit)->name == point) {
      found = true;
      break;
    }
    ++index;
    ++cit;
  }

  if (!found) return false;

  return Move(index, index_to);
}
//------------------------------------------------------------------------------
bool TrackingPointList::Connect(unsigned int endpoint_index1,
                                      unsigned int endpoint_index2) {
  linked_ptr<TrackingPoint> pt1 = get(endpoint_index1);
  linked_ptr<TrackingPoint> pt2 = get(endpoint_index2);
  
  return Connect(pt1, pt2);
}
//------------------------------------------------------------------------------
bool TrackingPointList::Connect(linked_ptr<TrackingPoint> endpoint1,
                                linked_ptr<TrackingPoint> endpoint2) {
  if (endpoint1.get() && endpoint2.get()) {
    return Connect(endpoint1->name, endpoint2->name);
  }
  return false;
}
//------------------------------------------------------------------------------
bool TrackingPointList::Connect(const std::string& point1,
                                const std::string& point2) {
  if (get(point1).get() && get(point2).get()) {
    return DoConnect(point1, point2);
  }
  return false;
}
//------------------------------------------------------------------------------
bool TrackingPointList::DoConnect(const std::string& point1,
                                  const std::string& point2) {
  Connection c1(point1, point2);
  Connection c2(point2, point1);
  if (connections_.find(c1) == connections_.end() &&
      connections_.find(c2) == connections_.end()) {
    connections_.insert(c1);
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool TrackingPointList::IsConnected(const std::string& point1,
                                    const std::string& point2) const {
  Connection c1(point1, point2);
  Connection c2(point2, point1);
  if (connections_.find(c1) == connections_.end() &&
      connections_.find(c2) == connections_.end()) {
    return false;
  }
  return true;
}
//------------------------------------------------------------------------------
TrackingPointList::PointList::iterator TrackingPointList::IteratorFor(
    unsigned int index) {
  PointList::iterator cit = point_list_.begin();
  while (index--) {
    ++cit;
  }
  return cit;
}
//------------------------------------------------------------------------------
TrackingPointList::PointList::const_iterator TrackingPointList::IteratorFor(
    unsigned int index) const {
  PointList::const_iterator cit = point_list_.begin();
  while (index--) {
    ++cit;
  }
  return cit;
}
//------------------------------------------------------------------------------
// static
TrackingPointsManager* TrackingPointsManager::GetInstance() {
  return Singleton<TrackingPointsManager>::get();
}
//------------------------------------------------------------------------------
TrackingPointsManager::TrackingPointsManager() {
}
//------------------------------------------------------------------------------
TrackingPointsManager::~TrackingPointsManager() {
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
