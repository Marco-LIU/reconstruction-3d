#pragma once
#include <string>

#include "QtGui/qimage.h"

#include "base/memory/ref_counted_memory.h"

class QWidget;

namespace base
{
  class FilePath;
}

QWidget* LoadUIFile(const std::string& ui_file, QWidget* parent = NULL);
QWidget* LoadUIFile(const std::wstring& ui_file, QWidget* parent = NULL);
QWidget* LoadUIFile(const base::FilePath& ui_file, QWidget* parent = NULL);

QImage FromRawGray(scoped_refptr<base::RefCountedBytes> buffer,
                   unsigned int w, unsigned int h);

QImage FromRawGray(char* buffer, unsigned int w, unsigned int h);