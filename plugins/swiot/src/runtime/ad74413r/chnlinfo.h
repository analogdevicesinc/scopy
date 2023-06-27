#ifndef CHNLINFO_H
#define CHNLINFO_H

#include "plot_utils.hpp"
#include <qobject.h>
#include <iio.h>
#include <QMap>

#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
namespace scopy::swiot {
class ChnlInfo : public QObject
{
	Q_OBJECT
public:
	explicit ChnlInfo(QString plotUm = "", QString hwUm = "", iio_channel *iioChnl = nullptr);
	~ChnlInfo();

	virtual double convertData(unsigned int data) = 0;
	iio_channel *iioChnl() const;

	bool isOutput() const;

	bool isScanElement() const;

	QString chnlId() const;

	std::pair<int, int> rangeValues() const;

	std::pair<double, double> offsetScalePair() const;

	bool isEnabled() const;
	void setIsEnabled(bool newIsEnabled);

	QString unitOfMeasure() const;

protected:
	bool m_isOutput;
	bool m_isEnabled;
	bool m_isScanElement;
	QString m_chnlId;
	QString m_plotUm;
	QString m_hwUm;
	std::pair<int, int> m_rangeValues = {-5, 5};
	std::pair<double, double> m_offsetScalePair;
	QMap<QString, double> m_unitOfMeasureFactor;

private:
	void initUnitOfMeasureFactor();
	struct iio_channel *m_iioChnl;
};
}

#endif // CHNLINFO_H
