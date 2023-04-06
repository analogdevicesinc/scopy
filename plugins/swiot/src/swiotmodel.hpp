#ifndef SWIOTMODEL_HPP
#define SWIOTMODEL_HPP

#include "qobject.h"
#include <QMap>

extern "C"{
	struct iio_channel;
}

namespace adiscope{
class SwiotAdModel: public QObject{
	Q_OBJECT
public:
	explicit SwiotAdModel(struct iio_channel* iioChnl = nullptr);
	~SwiotAdModel();

	QMap<QString, QStringList> getChnlAttrValues();
	QStringList readChnlAttr(struct iio_channel* iio_chnl, QString attrName);

	void init();
	void updateChnlAttributes(QMap<QString,QStringList> newValues, QString attrName);
private:
	struct iio_channel* m_iioChnl;
	QMap<QString, QStringList> m_chnlAttributes;

};
}

#endif // SWIOTMODEL_HPP
