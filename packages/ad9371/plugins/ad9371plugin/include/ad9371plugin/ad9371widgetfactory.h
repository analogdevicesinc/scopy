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
 */

#ifndef AD9371WIDGETFACTORY_H
#define AD9371WIDGETFACTORY_H

#include "scopy-ad9371plugin_export.h"
#include <iio-widgets/iiowidget.h>
#include <QString>
#include <QWidget>

namespace scopy {
namespace component {
class Device;
class Channel;
} // namespace component

namespace ad9371 {

class SCOPY_AD9371PLUGIN_EXPORT Ad9371WidgetFactory
{
public:
	// Device attribute widgets
	static IIOWidget *createSpinboxWidget(component::Device *device, QString attr, QString title,
					      QWidget *parent = nullptr);
	static IIOWidget *createCheckboxWidget(component::Device *device, QString attr, QString title,
					       QWidget *parent = nullptr);
	static IIOWidget *createComboWidget(component::Device *device, QString attr, QString availableAttr,
					    QString title, QWidget *parent = nullptr);
	static IIOWidget *createCustomComboWidget(component::Device *device, QString attr,
						  const QMap<QString, QString> &optionsMap, QString title,
						  QWidget *parent = nullptr);
	static IIOWidget *createRangeWidget(component::Device *device, QString attr, QString range, QString title,
					    QWidget *parent = nullptr);
	static IIOWidget *createReadOnlyWidget(component::Device *device, QString attr, QString title,
					       bool compactMode = true, QWidget *parent = nullptr);

	// Channel attribute widgets
	static IIOWidget *createSpinboxWidget(component::Channel *channel, QString attr, QString title,
					      QWidget *parent = nullptr);
	static IIOWidget *createCheckboxWidget(component::Channel *channel, QString attr, QString title,
					       QWidget *parent = nullptr);
	static IIOWidget *createComboWidget(component::Channel *channel, QString attr, QString availableAttr,
					    QString title, QWidget *parent = nullptr);
	static IIOWidget *createRangeWidget(component::Channel *channel, QString attr, QString range, QString title,
					    QWidget *parent = nullptr);
	static IIOWidget *createReadOnlyWidget(component::Channel *channel, QString attr, QString title,
					       bool compactMode = true, QWidget *parent = nullptr);

	// Debug attribute widgets (for advanced plugin features)
	static IIOWidget *createDebugRangeWidget(component::Device *device, QString attr, QString range, QString title,
						 QWidget *parent = nullptr);
	static IIOWidget *createDebugCustomComboWidget(component::Device *device, QString attr,
						       const QMap<QString, QString> &optionsMap, QString title,
						       QWidget *parent = nullptr);
	static IIOWidget *createDebugCheckboxWidget(component::Device *device, QString attr, QString title,
						    QWidget *parent = nullptr);

private:
	Ad9371WidgetFactory() = delete; // Static class, no instances
};

} // namespace ad9371
} // namespace scopy

#endif // AD9371WIDGETFACTORY_H
