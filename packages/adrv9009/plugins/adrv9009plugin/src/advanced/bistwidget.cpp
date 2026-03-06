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

#include "advanced/bistwidget.h"
#include <iio-widgets/iiowidgetgroup.h>
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_BIST, "BIST")

using namespace scopy;
using namespace scopy::adrv9009;

BistWidget::BistWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_BIST) << "No device provided to BIST widget";
		return;
	}

	setupUi();
}

BistWidget::~BistWidget() {}

void BistWidget::setupUi()
{
	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for all sections
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Add BIST section
	contentLayout->addWidget(createBistSection(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_BIST) << "BIST widget created with TX NCO section (3 custom widgets using iio-oscilloscope pattern) "
			    "and Framer PRBS section (2 standard IIOWidgets)";
}

QWidget *BistWidget::createBistSection(QWidget *parent)
{
	// BIST section with collapsible functionality
	MenuSectionCollapseWidget *bistSection = new MenuSectionCollapseWidget(
		"BIST", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// === TX NCO Section (Composite bist_tone attribute) ===
	bistSection->contentLayout()->addWidget(createTxNcoSection(bistSection));

	// === Framer PRBS Section (Standard IIO attributes) ===
	bistSection->contentLayout()->addWidget(createFramerPrbsSection(bistSection));

	// Connect refresh signal for custom TX NCO controls (since they're not IIOWidgets)
	connect(this, &BistWidget::readRequested, this, &BistWidget::readTxNcoFromDevice);

	return bistSection;
}

QWidget *BistWidget::createTxNcoSection(QWidget *parent)
{
	// TX NCO section with frame
	QGroupBox *widget = new QGroupBox("TX NCO", parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QHBoxLayout *layout = new QHBoxLayout(widget);

	// TX NCO Enable - Custom checkbox
	m_txNcoEnable = new MenuOnOffSwitch("TX NCO Enable", widget);
	layout->addWidget(m_txNcoEnable);

	// TX1 Frequency - Custom spinbox (display in MHz, store in kHz like iio-oscilloscope)
	// MHz range (converted to kHz internally)
	m_tx1FreqSpinBox = new gui::MenuSpinbox("TX1 Tone (MHz)", 0, "", -160, 160, true, false, false, widget);
	m_tx1FreqSpinBox->setScalingEnabled(false);
	layout->addWidget(m_tx1FreqSpinBox);

	// TX2 Frequency - Custom spinbox (display in MHz, store in kHz like iio-oscilloscope)
	// iio-oscilloscope glade range: -160 to 160 kHz -> displayed as MHz: -0.16 to 0.16
	m_tx2FreqSpinBox = new gui::MenuSpinbox("TX2 Tone (MHz)", 0, "", -0.16, 0.16, true, false, false, widget);
	m_tx2FreqSpinBox->setScalingEnabled(false);
	layout->addWidget(m_tx2FreqSpinBox);

	// Connect all controls to shared callback (iio-oscilloscope pattern)
	connect(m_txNcoEnable->onOffswitch(), &QCheckBox::toggled, this, &BistWidget::onTxNcoControlChanged);
	connect(m_tx1FreqSpinBox, &gui::MenuSpinbox::valueChanged, this, &BistWidget::onTxNcoControlChanged);
	connect(m_tx2FreqSpinBox, &gui::MenuSpinbox::valueChanged, this, &BistWidget::onTxNcoControlChanged);

	return widget;
}

QWidget *BistWidget::createFramerPrbsSection(QWidget *parent)
{
	// Framer PRBS section with frame
	QGroupBox *prbsGroup = new QGroupBox("Framer PRBS", parent);
	Style::setStyle(prbsGroup, style::properties::widget::border_interactive);

	QHBoxLayout *layout = new QHBoxLayout(prbsGroup);
	layout->setContentsMargins(10, 15, 10, 10);
	layout->setSpacing(15);

	// Create PRBS options map (from glade file analysis: 11 options, values 0-8, 14-15)
	QMap<QString, QString> prbOptions;
	prbOptions.insert("0", "ADC_DATA");
	prbOptions.insert("1", "CHECKERBOARD");
	prbOptions.insert("2", "TOGGLE0_1");
	prbOptions.insert("3", "PRBS31");
	prbOptions.insert("4", "PRBS23");
	prbOptions.insert("5", "PRBS15");
	prbOptions.insert("6", "PRBS9");
	prbOptions.insert("7", "PRBS7");
	prbOptions.insert("8", "RAMP");
	prbOptions.insert("14", "PATTERN_REPEAT");
	prbOptions.insert("15", "PATTERN_ONCE");

	// Framer A PRBS - Custom Combo Widget
	auto framerAWidget = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "bist_framer_a_prbs", prbOptions,
									    "Framer A PRBS", m_widgetGroup);
	if(framerAWidget) {
		layout->addWidget(framerAWidget);
		connect(this, &BistWidget::readRequested, framerAWidget, &IIOWidget::readAsync);
	}

	// Framer B PRBS - Custom Combo Widget
	auto framerBWidget = Adrv9009WidgetFactory::createCustomComboWidget(m_device, "bist_framer_b_prbs", prbOptions,
									    "Framer B PRBS", m_widgetGroup);
	if(framerBWidget) {
		layout->addWidget(framerBWidget);
		connect(this, &BistWidget::readRequested, framerBWidget, &IIOWidget::readAsync);
	}

	return prbsGroup;
}

void BistWidget::onTxNcoControlChanged()
{
	// Called when any TX NCO control changes - write immediately (iio-oscilloscope pattern)
	writeBistToneToDevice();
}

void BistWidget::readTxNcoFromDevice()
{
	// Called on refresh - read from device and update UI controls
	readBistToneFromDevice();
}

void BistWidget::writeBistToneToDevice()
{
	if(!m_device || !m_txNcoEnable || !m_tx1FreqSpinBox || !m_tx2FreqSpinBox) {
		return;
	}

	// Read current UI values (matching iio-oscilloscope exactly)
	unsigned enable = m_txNcoEnable->onOffswitch()->isChecked() ? 1 : 0;
	unsigned tx1_freq = (unsigned)(m_tx1FreqSpinBox->value() * 1000); // MHz to kHz
	unsigned tx2_freq = (unsigned)(m_tx2FreqSpinBox->value() * 1000); // MHz to kHz

	char composite[40];
	std::sprintf(composite, "%u %u %u", enable, tx1_freq, tx2_freq);

	// Use iio-oscilloscope pattern: reset then set (prevents issues)
	iio_device_debug_attr_write(m_device, "bist_tone", "0 0 0");
	iio_device_debug_attr_write(m_device, "bist_tone", composite);
}

void BistWidget::readBistToneFromDevice()
{
	if(!m_device || !m_txNcoEnable || !m_tx1FreqSpinBox || !m_tx2FreqSpinBox) {
		return;
	}

	char value[256];
	int ret = iio_device_debug_attr_read(m_device, "bist_tone", value, sizeof(value));
	if(ret < 0) {
		// Set default values on read error
		m_txNcoEnable->onOffswitch()->setChecked(false);
		m_tx1FreqSpinBox->setValue(0.0);
		m_tx2FreqSpinBox->setValue(0.0);
		qDebug(CAT_BIST) << "Failed to read bist_tone, using defaults";
		return;
	}

	// Parse "enable tx1_freq tx2_freq" exactly like iio-oscilloscope
	QStringList parts = QString::fromUtf8(value).trimmed().split(' ');
	if(parts.size() >= 3) {
		bool enableValue = (parts[0] == "1");
		double tx1FreqMHz = parts[1].toDouble() / 1000.0; // kHz to MHz
		double tx2FreqMHz = parts[2].toDouble() / 1000.0; // kHz to MHz

		// Temporarily disconnect signals to prevent triggering onTxNcoControlChanged
		m_txNcoEnable->blockSignals(true);
		m_tx1FreqSpinBox->blockSignals(true);
		m_tx2FreqSpinBox->blockSignals(true);

		m_txNcoEnable->onOffswitch()->setChecked(enableValue);
		m_tx1FreqSpinBox->setValue(tx1FreqMHz);
		m_tx2FreqSpinBox->setValue(tx2FreqMHz);

		// Re-enable signals
		m_txNcoEnable->blockSignals(false);
		m_tx1FreqSpinBox->blockSignals(false);
		m_tx2FreqSpinBox->blockSignals(false);

		qDebug(CAT_BIST) << "Read bist_tone from device:" << QString::fromUtf8(value).trimmed()
				 << "parsed as enable:" << enableValue << "tx1:" << tx1FreqMHz
				 << "MHz tx2:" << tx2FreqMHz << "MHz";
	} else {
		qWarning(CAT_BIST) << "Invalid bist_tone format from device:" << QString::fromUtf8(value).trimmed();
	}
}
