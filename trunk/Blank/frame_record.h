#pragma once
#include <vector>

#include "base/time/time.h"
#include "base/files/file_path.h"
#include "base/containers/hash_tables.h"

#include "QtGui/qimage.h"

struct FrameRecord {
  base::FilePath image_path;
  unsigned int frame_seq;
  base::Time time_stamp;
  base::Time sync_stamp;

  mutable QImage cached_image;

  QImage ToQImage() const;

  void Purge();
};

typedef std::vector<FrameRecord> FrameRecords;

typedef base::hash_map<int, FrameRecords> RecordedFrames;

bool LoadRecordedFrames(const base::FilePath& dir, RecordedFrames& frames);