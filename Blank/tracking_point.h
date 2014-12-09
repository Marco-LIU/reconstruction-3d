#pragma once

#include <set>
#include <string>
#include <list>
#include <map>

#include "base/memory/linked_ptr.h"
#include "base/files/file_path.h"
#include "base/containers/hash_tables.h"
#include "base/containers/linked_list.h"

#include "opencv2/core/core.hpp"

#include "QtGui/qcolor.h"

struct TrackingPoint {
  std::string name;

  CvPoint position;

  QColor color;
};

class TrackingPointList {
public:
  static const unsigned int kLastPosition = (unsigned int)-1;
  typedef std::list<linked_ptr<TrackingPoint> > PointList;
public:
  TrackingPointList(const std::string& cate);
public:
  const std::string& category() const { return category_; }
  unsigned int count() const { return point_list_.size(); }
  const PointList& points() const { return point_list_; }
  linked_ptr<TrackingPoint> get(unsigned int index) const;
  linked_ptr<TrackingPoint> get(const std::string& name) const;

  bool InList(linked_ptr<TrackingPoint> point) const {
    return (NULL != point.get()) && InList(point->name);
  }

  bool InList(const std::string& name) const {
    return NULL != get(name).get();
  }

  bool Insert(linked_ptr<TrackingPoint> point,
              unsigned int index = kLastPosition);

  bool Delete(unsigned int index);
  bool Delete(linked_ptr<TrackingPoint> point);
  bool Delete(const std::string& point);

  bool Move(unsigned int index_from, unsigned int index_to);
  bool Move(linked_ptr<TrackingPoint> point, unsigned int index_to);
  bool Move(const std::string& point, unsigned int index_to);

  bool Connect(unsigned int endpoint_index1,
               unsigned int endpoint_index2);
  bool Connect(linked_ptr<TrackingPoint> endpoint1,
               linked_ptr<TrackingPoint> endpoint2);
  bool Connect(const std::string& point1, const std::string& point2);

  bool IsConnected(const std::string& point1, const std::string& point2) const;
private:
  PointList::iterator IteratorFor(unsigned int index);
  PointList::const_iterator IteratorFor(unsigned int index) const;

  bool DoConnect(const std::string& point1, const std::string& point2);

  linked_ptr<TrackingPoint> DeleteInList(unsigned int index);

  void DeleteConnections(const std::string& point);
private:
  struct Connection {
    Connection(const std::string& p1, const std::string& p2)
        : point1(p1), point2(p2) {}
    std::string point1;
    std::string point2;
    friend bool operator<(const Connection& c1, const Connection& c2) {
      if (c1.point1 < c2.point1) return true;
      if (c1.point1 == c2.point1) return c1.point2 < c2.point2;
      return false;
    }
  };
  typedef std::set<Connection> Connections;

  std::string category_;
  PointList point_list_;
  Connections connections_;

  typedef base::hash_map<std::string, linked_ptr<TrackingPoint> > PointMap;
  PointMap point_map_;
};

class TrackingPointsManager {
public:
  TrackingPointsManager();
  ~TrackingPointsManager();

public:
  static TrackingPointsManager* GetInstance();

public:
  bool AddPoint(linked_ptr<TrackingPoint> point);
  linked_ptr<TrackingPoint> FindPoint(const std::string& name) const;

  linked_ptr<TrackingPointList> CreateList(const std::string& cate);
  linked_ptr<TrackingPointList> FindList(const std::string& cate) const;

  void LoadFromFile(const base::FilePath& path);
  bool SaveToFile(const base::FilePath& path) const;

private:
  typedef base::hash_map<std::string, linked_ptr<TrackingPoint> > PointsMap;
  PointsMap all_tracking_points_;

  typedef base::hash_map<std::string, linked_ptr<TrackingPointList> >PointLists;
  PointLists point_lists_;
};
