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

#ifndef ADRV9002_H
#define ADRV9002_H

#include "scopy-adrv9002plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <iio.h>
#include <profilemanager.h>
#include <profilegeneratorwidget.h>
#include <initialcalibrationswidget.h>
#include <gui/tooltemplate.h>
#include <gui/widgets/animatedrefreshbtn.h>
#include <iio-widgets/iiowidgetbuilder.h>

namespace scopy::adrv9002 {
class SCOPY_ADRV9002PLUGIN_EXPORT Adrv9002 : public QWidget
{
	Q_OBJECT
public:
	Adrv9002(iio_context *ctx, QWidget *parent = nullptr);
	~Adrv9002();

Q_SIGNALS:
	void readRequested();

private Q_SLOTS:
	void reloadSettings();

private:
	void setupUi();
	void connectSignals();
	void createControls(QWidget *centralWidget);

	// Tab creation methods
	void setupTabButtons();
	QWidget *createControlsWidget();
	QWidget *createBlockDiagramWidget();
	QWidget *createProfileGeneratorWidget();

	// UI Section Generation (ad936x pattern)
	QWidget *generateDeviceDriverAPIWidget(QWidget *parent);
	QString getDeviceDriverVersion();
	QWidget *generateGlobalSettingsWidget(QWidget *parent);
	QWidget *generateReceiveChainWidget(QWidget *parent);
	QWidget *generateTransmitChainWidget(QWidget *parent);

	// Channel Control Creation
	QWidget *createRxChannelControls(const QString &title, int channel);
	QWidget *createTxChannelControls(const QString &title, int channel);
	QWidget *createOrxControls();
	QWidget *createOrxChannelControls(const QString &title, int channel);

	// Widget Creation Helpers
	IIOWidget *createComboWidget(iio_channel *ch, const QString &attr, const QString &availableAttr,
				     const QString &title);
	IIOWidget *createRangeWidget(iio_channel *ch, const QString &attr, const QString &range, const QString &title);
	IIOWidget *createCheckboxWidget(iio_channel *ch, const QString &attr, const QString &label);
	QLabel *createReadOnlyLabel(iio_channel *ch, const QString &attr, double divisor = 1.0,
				    const QString &title = QString());

	// Standard Scopy tool components
	iio_context *m_ctx = nullptr;
	QVBoxLayout *m_mainLayout;
	ToolTemplate *m_tool;
	AnimatedRefreshBtn *m_refreshButton;

	// Tab system
	QStackedWidget *m_tabCentralWidget;
	QButtonGroup *m_tabButtons;

	// Tab widgets
	QWidget *m_controlsWidget;
	QWidget *m_blockDiagramWidget;
	QWidget *m_profileGeneratorWidget;

	// Tab buttons
	QPushButton *m_controlsBtn;
	QPushButton *m_blockDiagramBtn;
	QPushButton *m_profileGeneratorBtn;

	// Central content area (scrollable) - for Controls tab
	QScrollArea *m_scrollArea;
	QWidget *m_centralWidget;

	// Device handle
	iio_device *m_iio_dev = nullptr;

	// Profile manager
	ProfileManager *m_profileManager;

	// Initial calibrations widget
	InitialCalibrationsWidget *m_initialCalibrationsWidget;
};
} // namespace scopy::adrv9002
#endif // ADRV9002_H
