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

namespace
{
  void QImageCleanup(void* p) {
    base::RefCountedBytes* b = (base::RefCountedBytes*)p;
    if (b) b->Release();
  }
}

QImage FromRawGray(scoped_refptr<base::RefCountedBytes> buffer,
                   unsigned int w,
                   unsigned int h) {
  unsigned char* pd = const_cast<unsigned char *>(buffer->front());
  buffer->AddRef();
  QImage image(pd, w, h, QImage::Format_Indexed8, &QImageCleanup, buffer.get());
  QVector<QRgb> colorTable;
  for (int i = 0; i < 256; ++i)
    colorTable << qRgb(i, i, i);
  image.setColorTable(colorTable);
  return image;
}

QImage FromRawGray(char* buffer, unsigned int w, unsigned int h) {
  unsigned char* pd = (unsigned char*)buffer;
  QImage image(pd, w, h, QImage::Format_Indexed8);
  QVector<QRgb> colorTable;
  for (int i = 0; i < 256; ++i)
    colorTable << qRgb(i, i, i);
  image.setColorTable(colorTable);
  return image;
}