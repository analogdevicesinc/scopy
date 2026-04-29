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
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <QGroupBox>
#include <QPushButton>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <style.h>
#include <gui/widgets/menucollapsesection.h>

Q_LOGGING_CATEGORY(CAT_AD9371_BIST, "AD9371_BIST")

using namespace scopy;
using namespace scopy::ad9371;

BistWidget::BistWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
	, m_ncoEnableCheckbox(nullptr)
	, m_tx1NcoFreqSpin(nullptr)
	, m_tx2NcoFreqSpin(nullptr)
{
	if(!m_device) {
		qWarning(CAT_AD9371_BIST) << "No device provided to BIST";
		return;
	}

	setupUi();
}

BistWidget::~BistWidget() {}

void BistWidget::setupUi()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);

	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"BIST", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	// BIST Controls section (5 attrs: 2 combos, 2 checkboxes, 1 button)
	section->contentLayout()->addWidget(createBistControlsSection(section));

	// Connect refresh signal for custom BIST Tone controls
	connect(this, &BistWidget::readRequested, this, &BistWidget::readBistToneFromDevice);

	contentLayout->addWidget(section);
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_BIST) << "BIST widget created with 5 standard attrs + 3 BIST Tone controls";
}

QWidget *BistWidget::createBistControlsSection(QWidget *parent)
{
	QWidget *container = new QWidget(parent);
	QVBoxLayout *containerLayout = new QVBoxLayout(container);
	containerLayout->setContentsMargins(0, 0, 0, 0);
	containerLayout->setSpacing(10);

	// TX NCO section
	QGroupBox *ncoGroup = new QGroupBox("TX NCO", container);
	Style::setStyle(ncoGroup, style::properties::widget::border_interactive);
	QVBoxLayout *ncoLayout = new QVBoxLayout(ncoGroup);
	ncoLayout->setContentsMargins(10, 15, 10, 10);
	ncoLayout->setSpacing(5);

	m_ncoEnableCheckbox = new QCheckBox("ENABLE", ncoGroup);
	ncoLayout->addWidget(m_ncoEnableCheckbox);

	m_tx1NcoFreqSpin = new gui::MenuSpinbox("TX1 Tone MHz", 0, "MHz", 0, 500.0, true, false, false, ncoGroup);
	m_tx1NcoFreqSpin->setScalingEnabled(false);
	ncoLayout->addWidget(m_tx1NcoFreqSpin);

	m_tx2NcoFreqSpin = new gui::MenuSpinbox("TX2 Tone MHz", 0, "MHz", 0, 500.0, true, false, false, ncoGroup);
	m_tx2NcoFreqSpin->setScalingEnabled(false);
	ncoLayout->addWidget(m_tx2NcoFreqSpin);

	connect(m_ncoEnableCheckbox, &QCheckBox::toggled, this, &BistWidget::onBistToneControlChanged);
	connect(m_tx1NcoFreqSpin, &gui::MenuSpinbox::valueChanged, this, &BistWidget::onBistToneControlChanged);
	connect(m_tx2NcoFreqSpin, &gui::MenuSpinbox::valueChanged, this, &BistWidget::onBistToneControlChanged);

	containerLayout->addWidget(ncoGroup);

	// TX -> RX Loopback section
	QGroupBox *txRxGroup = new QGroupBox("TX -> RX Loopback", container);
	Style::setStyle(txRxGroup, style::properties::widget::border_interactive);
	QVBoxLayout *txRxLayout = new QVBoxLayout(txRxGroup);
	txRxLayout->setContentsMargins(10, 15, 10, 10);
	txRxLayout->setSpacing(5);

	auto loopbackTxRxWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, "loopback_tx_rx", "ENABLE");
	if(loopbackTxRxWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(loopbackTxRxWidget);
		txRxLayout->addWidget(loopbackTxRxWidget);
		m_widgets.append(loopbackTxRxWidget);
		connect(this, &BistWidget::readRequested, loopbackTxRxWidget, &IIOWidget::readAsync);
	}
	containerLayout->addWidget(txRxGroup);

	// TX -> OBS Loopback section
	QGroupBox *txObsGroup = new QGroupBox("TX -> OBS Loopback", container);
	Style::setStyle(txObsGroup, style::properties::widget::border_interactive);
	QVBoxLayout *txObsLayout = new QVBoxLayout(txObsGroup);
	txObsLayout->setContentsMargins(10, 15, 10, 10);
	txObsLayout->setSpacing(5);

	auto loopbackTxObsWidget =
		Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, "loopback_tx_obs", "ENABLE");
	if(loopbackTxObsWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(loopbackTxObsWidget);
		txObsLayout->addWidget(loopbackTxObsWidget);
		m_widgets.append(loopbackTxObsWidget);
		connect(this, &BistWidget::readRequested, loopbackTxObsWidget, &IIOWidget::readAsync);
	}
	containerLayout->addWidget(txObsGroup);

	// Framer PRBS section
	QGroupBox *prbsGroup = new QGroupBox("Framer PRBS", container);
	Style::setStyle(prbsGroup, style::properties::widget::border_interactive);
	QVBoxLayout *prbsLayout = new QVBoxLayout(prbsGroup);
	prbsLayout->setContentsMargins(10, 15, 10, 10);
	prbsLayout->setSpacing(5);

	QMap<QString, QString> prbsOptions;
	prbsOptions.insert("0", "Off");
	prbsOptions.insert("1", "PRBS7");
	prbsOptions.insert("2", "PRBS15");
	prbsOptions.insert("3", "PRBS31");

	auto prbsRxWidget =
		Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, "bist_prbs_rx", prbsOptions, "RX");
	if(prbsRxWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(prbsRxWidget);
		prbsLayout->addWidget(prbsRxWidget);
		m_widgets.append(prbsRxWidget);
		connect(this, &BistWidget::readRequested, prbsRxWidget, &IIOWidget::readAsync);
	}

	auto prbsObsWidget =
		Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, "bist_prbs_obs", prbsOptions, "OBS");
	if(prbsObsWidget) {
		if(m_widgetGroup)
			m_widgetGroup->add(prbsObsWidget);
		prbsLayout->addWidget(prbsObsWidget);
		m_widgets.append(prbsObsWidget);
		connect(this, &BistWidget::readRequested, prbsObsWidget, &IIOWidget::readAsync);
	}
	containerLayout->addWidget(prbsGroup);

	// Initialize button
	QPushButton *initBtn = new QPushButton("Initialize", container);
	Style::setStyle(initBtn, style::properties::button::basicButton);
	connect(initBtn, &QPushButton::clicked, this, &BistWidget::onInitializeClicked);
	containerLayout->addWidget(initBtn);

	return container;
}

void BistWidget::onBistToneControlChanged() { writeBistToneToDevice(); }

void BistWidget::onInitializeClicked()
{
	if(!m_device) {
		return;
	}

	int ret = iio_device_debug_attr_write_longlong(m_device, "initialize", 1);
	if(ret < 0) {
		qWarning(CAT_AD9371_BIST) << "Failed to write initialize, error:" << ret;
	} else {
		qDebug(CAT_AD9371_BIST) << "Initialize triggered successfully";
	}

	// Trigger a settings reload after initialize
	Q_EMIT readRequested();
}

void BistWidget::writeBistToneToDevice()
{
	if(!m_device || !m_ncoEnableCheckbox || !m_tx1NcoFreqSpin || !m_tx2NcoFreqSpin) {
		return;
	}

	unsigned int enable = m_ncoEnableCheckbox->isChecked() ? 1 : 0;
	unsigned int tx1Freq = static_cast<unsigned int>(m_tx1NcoFreqSpin->value() * 1000000.0); // MHz to Hz
	unsigned int tx2Freq = static_cast<unsigned int>(m_tx2NcoFreqSpin->value() * 1000000.0); // MHz to Hz

	QString cmd = QString("%1 %2 %3").arg(enable).arg(tx1Freq).arg(tx2Freq);

	// Reset first, then write new (iio-oscilloscope pattern)
	iio_device_debug_attr_write(m_device, "bist_tone", "0 0 0");
	iio_device_debug_attr_write(m_device, "bist_tone", cmd.toStdString().c_str());

	qDebug(CAT_AD9371_BIST) << "Wrote bist_tone:" << cmd;
}

void BistWidget::readBistToneFromDevice()
{
	if(!m_device || !m_ncoEnableCheckbox || !m_tx1NcoFreqSpin || !m_tx2NcoFreqSpin) {
		return;
	}

	char value[256];
	int ret = iio_device_debug_attr_read(m_device, "bist_tone", value, sizeof(value));
	if(ret < 0) {
		m_ncoEnableCheckbox->setChecked(false);
		m_tx1NcoFreqSpin->setValue(0);
		m_tx2NcoFreqSpin->setValue(0);
		qDebug(CAT_AD9371_BIST) << "Failed to read bist_tone, using defaults";
		return;
	}

	// Parse "enable tx1_freq_hz tx2_freq_hz"
	QStringList parts = QString::fromUtf8(value).trimmed().split(' ');
	if(parts.size() >= 3) {
		bool enableValue = (parts[0] == "1");
		double tx1FreqMHz = parts[1].toDouble() / 1000000.0; // Hz to MHz
		double tx2FreqMHz = parts[2].toDouble() / 1000000.0; // Hz to MHz

		// Block signals to prevent triggering onBistToneControlChanged
		m_ncoEnableCheckbox->blockSignals(true);
		m_tx1NcoFreqSpin->blockSignals(true);
		m_tx2NcoFreqSpin->blockSignals(true);

		m_ncoEnableCheckbox->setChecked(enableValue);
		m_tx1NcoFreqSpin->setValue(tx1FreqMHz);
		m_tx2NcoFreqSpin->setValue(tx2FreqMHz);

		m_ncoEnableCheckbox->blockSignals(false);
		m_tx1NcoFreqSpin->blockSignals(false);
		m_tx2NcoFreqSpin->blockSignals(false);

		qDebug(CAT_AD9371_BIST) << "Read bist_tone:" << QString::fromUtf8(value).trimmed()
					<< "enable:" << enableValue << "tx1:" << tx1FreqMHz << "MHz tx2:" << tx2FreqMHz
					<< "MHz";
	} else {
		qWarning(CAT_AD9371_BIST) << "Invalid bist_tone format:" << QString::fromUtf8(value).trimmed();
	}
}
