#ifndef FIRFILTERQIDGET_H
#define FIRFILTERQIDGET_H

#include <QPushButton>
#include <QWidget>
#include "scopy-plutoplugin_export.h"
#include <iio.h>
#include <QBoxLayout>
#include <menuonoffswitch.h>

namespace scopy {
namespace pluto {
class SCOPY_PLUTOPLUGIN_EXPORT FirFilterQidget : public QWidget
{
	Q_OBJECT
public:
	// at least one device is required
	explicit FirFilterQidget(iio_device *dev1, iio_device *dev2 = nullptr, QWidget *parent = nullptr);

Q_SIGNALS:
	void autofilterToggled(bool toogled);
	void filterChangeWasMade();

private:
	QVBoxLayout *m_layout;
	QPushButton *m_choseFileBtn;
	void chooseFile();
	iio_device *m_dev1;
	iio_device *m_dev2;
	void applyFirFilter(QString path);

	void refreshVisibleFilters();
	void hideAllFilters();
	void toggleDeviceFilter(iio_device *dev, bool toggled);
	void toggleChannelFilter(iio_device *dev, bool isTx, bool toggled);

	MenuOnOffSwitch *m_applyRxTxFilter;
	MenuOnOffSwitch *m_applyRxFilter;
	MenuOnOffSwitch *m_applyTxFilter;
	MenuOnOffSwitch *m_disableAllFilters;
};
} // namespace pluto
} // namespace scopy
#endif // FIRFILTERQIDGET_H
