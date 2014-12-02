#include "qt_utils.h"

#include "QtCore/qstring.h"
#include "QtCore/qfile.h"
#include "QtWidgets/qwidget.h"
#include "QtUiTools/quiloader.h"

#include "base/files/file_path.h"

QWidget* LoadUIFile(const QString& ui_file, QWidget* parent) {
  QUiLoader loader;

  QFile file(ui_file);
  file.open(QFile::ReadOnly);

  QWidget *formWidget = loader.load(&file, parent);
  file.close();

  return formWidget;
}


QWidget* LoadUIFile(const std::string& ui_file, QWidget* parent) {
  return LoadUIFile(QString(ui_file.c_str()), parent);
}
QWidget* LoadUIFile(const std::wstring& ui_file, QWidget* parent) {
  return LoadUIFile(QString::fromWCharArray(ui_file.c_str()), parent);
}
QWidget* LoadUIFile(const base::FilePath& ui_file, QWidget* parent) {
  return LoadUIFile(ui_file.value(), parent);
}