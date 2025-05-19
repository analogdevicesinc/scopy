/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef FIRFILTERQWIDGET_H
#define FIRFILTERQWIDGET_H

#include <QPushButton>
#include <QWidget>
#include <filebrowserwidget.h>
#include "scopy-pluto_export.h"
#include <iio.h>
#include <QBoxLayout>
#include <menuonoffswitch.h>

namespace scopy {
namespace pluto {
class SCOPY_PLUTO_EXPORT FirFilterQWidget : public QWidget
{
	Q_OBJECT
public:
	// at least one device is required
	explicit FirFilterQWidget(iio_device *dev1, iio_device *dev2 = nullptr, QWidget *parent = nullptr);

Q_SIGNALS:
	void autofilterToggled(bool toogled);
	void filterChanged();

private:
	QVBoxLayout *m_layout;
	FileBrowserWidget *fileBrowser;

	QPushButton *m_chooseFileBtn;
	void chooseFile();
	iio_device *m_dev1;
	iio_device *m_dev2;
	bool m_isRxFilter;
	bool m_isTxFilter;
	void applyFirFilter(QString path);

	void applyChannelFilterToggled(bool isTx, bool toggled);
	void toggleDeviceFilter(iio_device *dev, bool toggled);
	void toggleChannelFilter(iio_channel *chn, QString attr, bool toggled);

	MenuOnOffSwitch *m_applyRxTxFilter;
};
} // namespace pluto
} // namespace scopy
#endif // FIRFILTERQWIDGET_H
