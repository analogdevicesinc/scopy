#ifndef LIBSERIALPORTSUPPORT_H
#define LIBSERIALPORTSUPPORT_H

#include "scopy-iioutil_export.h"
#include <QObject>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT LibSerialPortSupport : public QObject
{
public:
	explicit LibSerialPortSupport(QObject *parent = nullptr);
	~LibSerialPortSupport();

	bool hasLibSerialSupport();
	QVector<QString> getSerialPortsName();
};

}

#endif // LIBSERIALPORTSUPPORT_H
