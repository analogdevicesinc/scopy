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
	explicit BufferMenuModel(struct iio_channel* iioChnl = nullptr);
	~BufferMenuModel();

	QMap<QString, QStringList> getChnlAttrValues();
	QStringList readChnlAttr(struct iio_channel* iio_chnl, QString attrName);

	void init();
	void updateChnlAttributes(QMap<QString,QStringList> newValues, QString attrName);
private:
	struct iio_channel* m_iioChnl;
	QMap<QString, QStringList> m_chnlAttributes;

};
}

#endif // BUFFERMENUMODEL_H
