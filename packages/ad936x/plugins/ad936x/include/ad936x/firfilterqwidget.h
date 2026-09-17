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
#include "scopy-ad936x_export.h"
#include <QBoxLayout>
#include <menuonoffswitch.h>

namespace scopy {
namespace component {
class Device;
class Channel;
} // namespace component
namespace ad936x {
class SCOPY_AD936X_EXPORT FirFilterQWidget : public QWidget
{
	Q_OBJECT
public:
	// at least one device is required
	explicit FirFilterQWidget(component::Device *dev1, component::Device *dev2 = nullptr,
				  QWidget *parent = nullptr);

Q_SIGNALS:
	void autofilterToggled(bool toogled);
	void filterChanged();

private:
	QVBoxLayout *m_layout;
	QPushButton *m_chooseFileBtn;
	void chooseFile();
	component::Device *m_dev1;
	component::Device *m_dev2;
	bool m_isRxFilter;
	bool m_isTxFilter;
	void applyFirFilter(QString path);

	void applyChannelFilterToggled(bool isTx, bool toggled);
	void toggleDeviceFilter(component::Device *dev, bool toggled);
	void toggleChannelFilter(component::Channel *chn, QString attr, bool toggled);

	MenuOnOffSwitch *m_applyRxTxFilter;
};
} // namespace ad936x
} // namespace scopy
#endif // FIRFILTERQWIDGET_H
