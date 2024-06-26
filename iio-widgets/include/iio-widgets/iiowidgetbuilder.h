/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef SCOPY_IIOWIDGETBUILDER_H
#define SCOPY_IIOWIDGETBUILDER_H

#include <QWidget>
#include <QLoggingCategory>
#include <QMap>
#include <QLabel>
#include <QTimer>
#include <iio.h>
#include "iiowidget.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT IIOWidgetBuilder : public QObject
{
	Q_OBJECT
public:
	enum DS
	{ // Data Strategy
		NoDataStrategy,
		AttrData,
		TriggerData,
		DeviceAttrData,
		ContextAttrData,
	};

	enum UIS
	{ // UI Strategy
		NoUIStrategy,
		EditableUi,
		ComboUi,
		SwitchUi,
		RangeUi,
	};

	explicit IIOWidgetBuilder(QObject *parent = nullptr);
	~IIOWidgetBuilder();

	/**
	 * @brief Builds a single IIOWidget based on the given parameters.
	 * @return IIOWidget*
	 */
	IIOWidget *buildSingle();

	/**
	 * @brief Builds all possible IIOWidgets based on the given parameters.
	 * @return
	 */
	QList<IIOWidget *> buildAll();

	/**
	 * @brief Clears all given parameters.
	 */
	void clear();

	/**
	 * @brief If the connection is set, all IIOWidgets created will work through
	 * command queue, except if there is no data strategy that implements that.
	 * In that case, the non-commandQueue strategy will be selected.
	 * @param connection
	 */
	IIOWidgetBuilder &connection(scopy::Connection *connection);

	/**
	 * @brief Sets the UI of the widgets to compact mode, this way, the widget
	 * sizes will be more compact.
	 * @param isCompact If this is set to true, the widget will be in compact
	 * mode. If false, the widget will be in normal mode.
	 * @return
	 */
	IIOWidgetBuilder &compactMode(bool isCompact);

	/**
	 * @brief Sets the context that will be used, if no iio_device or iio_channel
	 * is set, the build functions will work with the context.
	 * @param context
	 */
	IIOWidgetBuilder &context(struct iio_context *context);

	/**
	 * @brief Sets the device that will be used, if no iio_channel is set, the
	 * build functions will work with the device. If a context is also set
	 * along side the device, the context will be ignored by the build functions.
	 * @param device
	 */
	IIOWidgetBuilder &device(struct iio_device *device);

	/**
	 * @brief Sets the channel that will be used. If a device or context is also
	 * set, they will be ingored by the build functions.
	 * @param channel
	 */
	IIOWidgetBuilder &channel(struct iio_channel *channel);

	/**
	 * @brief Sets the attribute that is required by the IIOWidget.
	 * @param attribute
	 */
	IIOWidgetBuilder &attribute(QString attribute);

	/**
	 * @brief Sets the attribute that needs to be read in order to populate the
	 * options from which the user can select to change the main attribute.
	 * @param optionsAttribute
	 */
	IIOWidgetBuilder &optionsAttribute(QString optionsAttribute);

	/**
	 * @brief Sets the values that represent the value read by the
	 * optionsAttriute. This is useful if you are trying to enforce a
	 * range or set artificial (not read from the context) values for
	 * a combo box.
	 * @param optionsValues
	 */
	IIOWidgetBuilder &optionsValues(QString optionsValues);

	/**
	 * @brief Sets the data strategy (DS) that will be used.
	 * @param dataStrategy
	 */
	IIOWidgetBuilder &dataStrategy(IIOWidgetBuilder::DS dataStrategy);

	/**
	 * @brief Sets the UI Strategy (UIS) that will be used. This is especially
	 * helpful when creating a more complex UI (e.g. a combo box) for a device
	 * with tinyiiod as the build functions do not need to make iio calls to
	 * determine the UIS themselves.
	 * @param uiStrategy
	 */
	IIOWidgetBuilder &uiStrategy(IIOWidgetBuilder::UIS uiStrategy);

	/**
	 * @brief Sets the parent of the IIOWidget that will be built.
	 * @param parent
	 */
	IIOWidgetBuilder &parent(QWidget *parent);

private:
	DataStrategyInterface *createDS();
	GuiStrategyInterface *createUIS();

	Connection *m_connection;
	bool m_isCompact;
	struct iio_context *m_context;
	struct iio_device *m_device;
	struct iio_channel *m_channel;
	QString m_attribute;
	QString m_optionsAttribute;
	QString m_optionsValues;
	IIOWidgetBuilder::DS m_dataStrategy;
	IIOWidgetBuilder::UIS m_uiStrategy;
	QWidget *m_widgetParent;
	IIOWidgetFactoryRecipe m_generatedRecipe;
};
} // namespace scopy

#endif // SCOPY_IIOWIDGETBUILDER_H
