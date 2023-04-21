#include "fileregisterreadstrategy.hpp"

#include "iregisterreadstrategy.hpp"
#include "../logging_categories.h"
#include <QString>
#include <QFile>
#include <QStringList>

FileRegisterReadStrategy::FileRegisterReadStrategy(QString path)
	:path(path)
{
}

void FileRegisterReadStrategy::read(uint32_t address)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug(CAT_IIO_OPERATION) << "device read error " << file.errorString();
		Q_EMIT readError("device read error");
	} else {
		while (!file.atEnd()) {
			QString line(file.readLine());
			QString addr(line.split(',').first());
			if (addr.contains(QString::number(address,16))) {
				bool ok;
				Q_EMIT readDone(address,line.split(',').at(1).toInt(&ok,16));
				qDebug(CAT_IIO_OPERATION) << "device read success for " << address << " with value " <<line.split(',').at(1).toInt(&ok,16);
				break;
			}
		}
	}
}
