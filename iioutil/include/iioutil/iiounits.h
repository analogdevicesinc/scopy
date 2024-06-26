#ifndef IIOUNITS_H
#define IIOUNITS_H

#include <QString>
#include <QMap>
#include <QObject>

#include "scopy-iioutil_export.h"
#include <iio.h>

namespace scopy {

typedef struct IIOUnit_
{
	QString name;
	QString symbol;
	double scale = 1;
} IIOUnit;

class SCOPY_IIOUTIL_EXPORT IIOUnitsManager : public QObject
{
	Q_OBJECT
protected:
	IIOUnitsManager(QObject *parent = nullptr);
	~IIOUnitsManager();

public:
	// singleton
	IIOUnitsManager(IIOUnitsManager &other) = delete;
	void operator=(const IIOUnitsManager &) = delete;
	static IIOUnitsManager *GetInstance();

	static QMap<iio_chan_type, IIOUnit> iioChannelTypes();
	static QMap<hwmon_chan_type, IIOUnit> hwmonChannelTypes();

private:
	QMap<iio_chan_type, IIOUnit> _iioChannelTypes();
	QMap<hwmon_chan_type, IIOUnit> _hwmonChannelTypes();

private:
	static IIOUnitsManager *pinstance_;
	QMap<iio_chan_type, IIOUnit> m_iioChannelTypes;
	QMap<hwmon_chan_type, IIOUnit> m_hwmonChannelTypes;
};

}; // namespace scopy

#endif // IIOUNITS_H
