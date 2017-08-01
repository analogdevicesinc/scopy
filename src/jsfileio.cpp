#include "jsfileio.h"

#include <QFile>
#include <QTextStream>

using namespace adiscope;

JsFileIo::JsFileIo(QObject *parent): QObject(parent)
{
}

void JsFileIo::writeToFile(const QString& data, const QString& path)
{
	QFile file(path);

	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream << data;
	}
}
