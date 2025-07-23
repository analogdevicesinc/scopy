#ifndef DATAWRITER_H
#define DATAWRITER_H

#include "common/scopyconfig.h"
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <qdir.h>

namespace scopy::qiqplugin {
class DataWriter : public QObject
{
	Q_OBJECT
public:
	DataWriter(QObject *parent = nullptr);
	~DataWriter();

	uchar *mappedData();
	bool openFile(const QString &path, int64_t dataSize);
	void unmap();
	QFileInfo getFileInfo();

private:
	QFile m_file;
	uchar *m_data = nullptr;
	int64_t m_dataSize = 0;
};
} // namespace scopy::qiqplugin

#endif // DATAWRITER_H
