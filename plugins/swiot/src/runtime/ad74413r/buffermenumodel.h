#ifndef BUFFERMENUMODEL_H
#define BUFFERMENUMODEL_H

#include "qobject.h"
#include <QMap>

extern "C"{
struct iio_channel;
}

namespace scopy::swiot {
class BufferMenuModel: public QObject{
	Q_OBJECT
public:
	explicit BufferMenuModel(QMap<QString, iio_channel*> chnlsMap = {});
	~BufferMenuModel();

	QMap<QString, QMap<QString, QStringList>> getChnlAttrValues();
	QStringList readChnlAttr(struct iio_channel* iio_chnl, QString attrName);

	void init();
	void updateChnlAttributes(QMap<QString, QMap<QString,QStringList>> newValues, QString attrName, QString chnlType);
Q_SIGNALS:
	void attrWritten(QString value);
private:
	QMap<QString, iio_channel*> m_chnlsMap;
	QMap<QString, QMap<QString, QStringList>> m_chnlAttributes;

};
}

#endif // BUFFERMENUMODEL_H
