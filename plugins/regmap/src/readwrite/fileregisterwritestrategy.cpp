#include "fileregisterwritestrategy.hpp"

#include <QFile>
#include <QTextStream>
#include "../logging_categories.h"

FileRegisterWriteStrategy::FileRegisterWriteStrategy(QString path)
	:path(path)
{

}

void FileRegisterWriteStrategy::write(uint32_t address, uint32_t val)
{
	//TODO HOW SHOULD WE TREAT WRITE ?
	QFile file(path);
	QTextStream exportStream(&file);
	if (!file.isOpen()) {
		file.open(QIODevice::Append);
		exportStream << QString::number(address, 16) << "," << QString::number(val, 16) << "\n";
	} else {
		qDebug(CAT_IIO_OPERATION) << "File already opened! ";
		Q_EMIT writeError("device write err");
	}

	if (file.isOpen()) file.close();
}
