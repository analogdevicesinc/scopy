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

#ifndef AD9371_HELPER_H
#define AD9371_HELPER_H

#include "scopy-ad9371_export.h"
#include <QWidget>
#include <iio-widgets/iiowidgetbuilder.h>

namespace scopy {
class IIOWidgetGroup;
namespace ad9371 {

class SCOPY_AD9371_EXPORT AD9371Helper : public QWidget
{
	Q_OBJECT
public:
	AD9371Helper(IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);

	QWidget *generateGlobalSettingsWidget(iio_device *dev, bool has_dpd, QString title, QWidget *parent);
	QWidget *generateRxChainWidget(iio_device *dev, iio_device *cap, bool is_2rx_2tx, QString title,
				       QWidget *parent);
	QWidget *generateObsRxChainWidget(iio_device *dev, QString title, QWidget *parent);
	QWidget *generateTxChainWidget(iio_device *dev, bool has_dpd, bool is_2rx_2tx, QString title, QWidget *parent);
	QWidget *generateFpgaWidget(iio_device *dds, iio_device *cap, bool is_2rx_2tx, QString title, QWidget *parent);

Q_SIGNALS:
	void readRequested();

private:
	IIOWidgetGroup *m_group = nullptr;
};
} // namespace ad9371
} // namespace scopy
#endif // AD9371_HELPER_H
