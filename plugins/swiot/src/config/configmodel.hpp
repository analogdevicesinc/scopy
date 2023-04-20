#ifndef SWIOTCONFIGMODEL_HPP
#define SWIOTCONFIGMODEL_HPP

#include "qobject.h"

extern "C"{
struct iio_channel;
struct iio_device;
}

namespace scopy::swiot {
class ConfigModel : public QObject {
	Q_OBJECT
public:
	explicit ConfigModel(struct iio_channel *adChannel, struct iio_channel *maxChannel);

	~ConfigModel();

	QVector<QMap<QString, QStringList>> getChnlsAttrValues();

	QStringList getActiveFunctions();

	QStringList readChnlAttr(struct iio_channel *iio_chnl, QString attrName);

	void initChnlAttrValues(struct iio_channel *iioChnl);

	void updateChnlAttributes(QVector<QMap<QString, QStringList>> newValues, QString attrName, int deviceIdx);

private:
	struct iio_device *m_iioDev;
	QVector<struct iio_channel *> m_channels;
	QVector<QMap<QString, QStringList>> m_attrValues;
};
}
#endif // SWIOTCONFIGMODEL_HPP
