#include "frame_record.h"

#include <fstream>
#include <sstream>

#include "base/files/file_util.h"
#include "base/strings/string_number_conversions.h"

QImage FrameRecord::ToQImage() const {
  if (!cached_image.isNull()) return cached_image;
  cached_image.load(QString::fromWCharArray(image_path.value().c_str()));
  return cached_image;
}

void FrameRecord::Purge() {
  cached_image.swap(QImage());
}

bool LoadRecordedFrames(const base::FilePath& dir, RecordedFrames& frames) {
  const int left_id = 0, right_id = 1;
  frames.clear();
  frames[left_id] = FrameRecords();
  frames[right_id] = FrameRecords();
  base::FilePath left_list = dir.Append(L"left").Append(L"0_img_list.txt");
  base::FilePath right_list = dir.Append(L"right").Append(L"0_img_list.txt");

  if (!base::PathExists(left_list) || !base::PathExists(right_list))
    return false;

  char buf[256];

  std::ifstream left_ifs, right_ifs;
  left_ifs.open(left_list.value().c_str());

  while (!left_ifs.eof()) {
    left_ifs.getline(buf, 256);
    std::istringstream ss(buf);
    FrameRecord r;
    int64 ts1, ts2;
    ss >> r.frame_seq >> ts1 >> ts2;
    r.time_stamp = base::Time::FromInternalValue(ts1 * 1000);
    r.sync_stamp = base::Time::FromInternalValue(ts2 * 1000);

    std::string name = base::IntToString(r.frame_seq) + "_" +
        base::Int64ToString(ts1) + ".jpg";

    r.image_path = dir.Append(L"left").AppendASCII(name);

    if (!base::PathExists(r.image_path))
      return false;

    frames[left_id].push_back(r);
  }

  right_ifs.open(right_list.value().c_str());

  while (!right_ifs.eof()) {
    right_ifs.getline(buf, 256);
    std::istringstream ss(buf);
    FrameRecord r;
    int64 ts1, ts2;
    ss >> r.frame_seq >> ts1 >> ts2;
    r.time_stamp = base::Time::FromInternalValue(ts1 * 1000);
    r.sync_stamp = base::Time::FromInternalValue(ts2 * 1000);

    std::string name = base::IntToString(r.frame_seq) + "_" +
      base::Int64ToString(ts1) + ".jpg";

    r.image_path = dir.Append(L"right").AppendASCII(name);

    if (!base::PathExists(r.image_path))
      return false;

    frames[right_id].push_back(r);
  }
  return true;
}