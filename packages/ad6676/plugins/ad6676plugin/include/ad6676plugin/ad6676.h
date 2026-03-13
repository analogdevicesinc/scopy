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

#ifndef AD6676_H
#define AD6676_H

#include "scopy-ad6676plugin_export.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <iio.h>

#include <gui/tooltemplate.h>
#include <gui/widgets/animatedrefreshbtn.h>
#include <gui/widgets/menusectionwidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/iiowidget.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad6676 {

class SCOPY_AD6676PLUGIN_EXPORT Ad6676 : public QWidget
{
	Q_OBJECT
public:
	explicit Ad6676(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent = nullptr);
	~Ad6676();

Q_SIGNALS:
	void readRequested();

private Q_SLOTS:
	void updateBandwidthRange(QString adcFreqMhz);
	void onAdcFreqDisplayed(QString data, QString optData);

private:
	void setupUi();
	void connectSignals();

	// Widget helpers — inline, no separate factory class needed
	IIOWidget *createRangeWidget(iio_channel *ch, const QString &attr, const QString &range,
				     const QString &title);
	IIOWidget *createComboWidget(iio_channel *ch, const QString &attr, const QString &availableAttr,
				     const QString &title);
	IIOWidget *createReadOnlyWidget(iio_channel *ch, const QString &attr, const QString &title);

	// Section creation methods — one per glade frame
	QWidget *createAdcSettingsSection(QWidget *parent);
	QWidget *createBandwidthSettingsSection(QWidget *parent);
	QWidget *createReceiveSettingsSection(QWidget *parent);
	QWidget *createShufflerSettingsSection(QWidget *parent);
	QWidget *createTestSettingsSection(QWidget *parent);

	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	iio_device *m_dev = nullptr;
	iio_channel *m_chn = nullptr;

	QVBoxLayout *m_mainLayout = nullptr;
	ToolTemplate *m_tool = nullptr;
	AnimatedRefreshBtn *m_refreshButton = nullptr;
	QScrollArea *m_scrollArea = nullptr;
	QWidget *m_centralWidget = nullptr;

	// Kept as members for cross-widget wiring
	IIOWidget *m_adcFreqWidget = nullptr;
	IIOWidget *m_bwWidget = nullptr;
};

} // namespace scopy::ad6676

#endif // AD6676_H
