#include "jsfileio.h"

#include <QFile>
#include <QTextStream>

using namespace adiscope;

JsFileIo::JsFileIo(QObject* parent)
	: QObject(parent)
{}

void JsFileIo::writeToFile(const QString& data, const QString& path)
{
	QFile file(path);

	if (file.open(QIODevice::WriteOnly)) {
		QTextStream stream(&file);
		stream << data;
	}
}

void JsFileIo::appendToFile(const QString& data, const QString& path)
{
	QFile file(path);

	if (file.open(QIODevice::Append)) {
		QTextStream stream(&file);
		stream << data;
	}
}

QString JsFileIo::readAll(const QString& path)
{
	QFile file(path);
	QString data;

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		data = stream.readAll();
	}
	return data;
}

QString JsFileIo::readLine(const QString& path, const int lineNumber)
{
	QFile file(path);
	int i = 0;
	QString data;

	if (file.open(QIODevice::ReadOnly)) {
		QTextStream stream(&file);
		while (i <= lineNumber) {
			data = stream.readLine();
			if (stream.atEnd())
				break;
			i++;
		}
	}
	return data;
}
