#ifndef XMLFILEMANAGER_HPP
#define XMLFILEMANAGER_HPP

#include "registermaptemplate.hpp"
#include "scopy-regmap_export.h"

#include <iio.h>

#include <QObject>

class QDomElement;
class QString;

namespace scopy::regmap {
class RegisterModel;
class BitFieldModel;

class SCOPY_REGMAP_EXPORT XmlFileManager : public QObject
{
	Q_OBJECT

public:
	XmlFileManager(struct iio_device *dev, QString filePath);

	QList<QString> *getAllAddresses();
	QMap<uint32_t, RegisterModel *> *getAllRegisters(RegisterMapTemplate *parent);
	RegisterModel *getRegAtAddress(QString addr);
	RegisterModel *getRegister(QDomElement reg, RegisterMapTemplate *parent);
	void getRegMapData(QString addr);
	QVector<BitFieldModel *> *getBitFields(QString regAddr);
	QVector<BitFieldModel *> *getBitFieldsOfRegister(QDomElement reg, bool reverseBitOrder);
	BitFieldModel *getBitField(QDomElement bitField);

private:
	QString filePath;
	struct iio_device *dev;
};
} // namespace scopy::regmap
#endif // XMLFILEMANAGER_HPP