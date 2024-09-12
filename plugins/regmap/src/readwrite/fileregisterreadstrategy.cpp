#include "fileregisterreadstrategy.hpp"

#include "../logging_categories.h"
#include "iregisterreadstrategy.hpp"
#include "utils.hpp"

#include <QFile>
#include <QString>
#include <QStringList>

using namespace scopy::regmap;

FileRegisterReadStrategy::FileRegisterReadStrategy(QString path)
	: path(path)
{}

void FileRegisterReadStrategy::read(uint32_t address)
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly)) {
		qDebug(CAT_IIO_OPERATION) << "device read error " << file.errorString();
		Q_EMIT readError("device read error");
	} else {
		while(!file.atEnd()) {
			QString line(file.readLine());
			QString addr(line.split(',').first());
			if(addr.contains(QString::number(address, 16))) {
				Q_EMIT readDone(address, Utils::convertQStringToUint32(line.split(',').at(1)));
				qDebug(CAT_IIO_OPERATION) << "device read success for " << address << " with value "
							  << Utils::convertQStringToUint32(line.split(',').at(1));
				break;
			}
		}
	}
}

void FileRegisterReadStrategy::readAll()
{
	QFile file(path);
	if(!file.open(QIODevice::ReadOnly)) {
		qDebug(CAT_IIO_OPERATION) << "device read error " << file.errorString();
		Q_EMIT readError("device read error");
	} else {
		while(!file.atEnd()) {
			QString line(file.readLine());
			uint32_t address = Utils::convertQStringToUint32(line.split(',').first());
			uint32_t value = Utils::convertQStringToUint32(line.split(',').at(1));

			Q_EMIT readDone(address, value);
			qDebug(CAT_IIO_OPERATION) << "device read success for " << address << " with value " << value;
		}
	}
}
