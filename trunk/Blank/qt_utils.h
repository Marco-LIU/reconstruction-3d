#pragma once
#include <string>

class QWidget;

namespace base
{
  class FilePath;
}

QWidget* LoadUIFile(const std::string& ui_file, QWidget* parent = NULL);
QWidget* LoadUIFile(const std::wstring& ui_file, QWidget* parent = NULL);
QWidget* LoadUIFile(const base::FilePath& ui_file, QWidget* parent = NULL);