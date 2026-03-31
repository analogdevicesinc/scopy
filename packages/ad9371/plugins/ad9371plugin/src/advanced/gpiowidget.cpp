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

#include "advanced/gpiowidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <QGroupBox>
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_GPIO, "AD9371_GPIO")

using namespace scopy;
using namespace scopy::ad9371;

GpioWidget::GpioWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_GPIO) << "No device provided to GPIO";
		return;
	}

	setupUi();
	readGpio3v3OeMask();
	readGpioOeMask();

	Style::setStyle(this, style::properties::widget::border_interactive);
}

GpioWidget::~GpioWidget() {}

void GpioWidget::setupUi()
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

	// GPIO 3.3V section
	MenuSectionCollapseWidget *gpio3v3Section = new MenuSectionCollapseWidget(
		"GPIO 3.3V", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	gpio3v3Section->contentLayout()->addWidget(createGpio3v3Section(gpio3v3Section));
	connect(this, &GpioWidget::readRequested, this, &GpioWidget::readGpio3v3OeMask);
	contentLayout->addWidget(gpio3v3Section);

	// GPIO Low Voltage section
	MenuSectionCollapseWidget *gpioSection =
		new MenuSectionCollapseWidget("GPIO Low Voltage", MenuCollapseSection::MHCW_ARROW,
					      MenuCollapseSection::MHW_BASEWIDGET, contentWidget);
	gpioSection->contentLayout()->addWidget(createGpioSection(gpioSection));
	connect(this, &GpioWidget::readRequested, this, &GpioWidget::readGpioOeMask);
	contentLayout->addWidget(gpioSection);

	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_GPIO) << "GPIO widget created with 39 attributes (15 GPIO 3V3 + 24 GPIO)";
}

QWidget *GpioWidget::createGpio3v3Section(QWidget *parent)
{
	QGroupBox *widget = new QGroupBox("GPIO 3.3V", parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 15, 10, 10);
	layout->setSpacing(5);

	QMap<QString, QString> srcCtrl3v3Map;
	srcCtrl3v3Map.insert("1", "LEVELTRANSLATE_MODE");
	srcCtrl3v3Map.insert("2", "INVLEVELTRANSLATE_MODE");
	srcCtrl3v3Map.insert("3", "BITBANG_MODE");
	srcCtrl3v3Map.insert("4", "EXTATTEN_LUT_MODE");

	// Pin group rows: [0..3], [4..7], [8..11]
	struct
	{
		int startPin;
		int endPin;
		const char *label;
		const char *attr;
	} groups[] = {
		{0, 3, "[0 ... 3]", "adi,gpio-3v3-src-ctrl3_0"},
		{4, 7, "[4 ... 7]", "adi,gpio-3v3-src-ctrl7_4"},
		{8, 11, "[8 ... 11]", "adi,gpio-3v3-src-ctrl11_8"},
	};

	for(const auto &g : groups) {
		QWidget *row = new QWidget(widget);
		QHBoxLayout *rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(0, 2, 0, 2);
		rowLayout->setSpacing(5);

		rowLayout->addWidget(new QLabel(g.label, row));

		auto *srcCtrl =
			Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, g.attr, srcCtrl3v3Map, "", row);
		if(srcCtrl) {
			if(m_widgetGroup) m_widgetGroup->add(srcCtrl);
			rowLayout->addWidget(srcCtrl);
			m_widgets.append(srcCtrl);
			connect(this, &GpioWidget::readRequested, srcCtrl, &IIOWidget::readAsync);
		}

		for(int pin = g.startPin; pin <= g.endPin; pin++) {
			auto *sw = new MenuOnOffSwitch("ENABLE", row);
			sw->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
			rowLayout->addWidget(sw);
			m_gpio3v3OeBits.append(sw);
			connect(sw->onOffswitch(), &QAbstractButton::toggled, this,
				&GpioWidget::onGpio3v3OeMaskChanged);
		}

		layout->addWidget(row);
	}

	layout->addStretch();
	return widget;
}

QWidget *GpioWidget::createGpioSection(QWidget *parent)
{
	QGroupBox *widget = new QGroupBox("GPIO Low Voltage", parent);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 15, 10, 10);
	layout->setSpacing(5);

	QMap<QString, QString> srcCtrlMap;
	srcCtrlMap.insert("0", "MONITOR_MODE");
	srcCtrlMap.insert("3", "BITBANG_MODE");
	srcCtrlMap.insert("9", "ARM_OUT_MODE");
	srcCtrlMap.insert("10", "SLICER_OUT_MODE");

	// Pin group rows: [0..3], [4..7], [8..11], [12..15], [16..18]
	struct
	{
		int startPin;
		int endPin;
		const char *label;
		const char *attr;
	} groups[] = {
		{0, 3, "[0 ... 3]", "adi,gpio-src-ctrl3_0"},	   {4, 7, "[4 ... 7]", "adi,gpio-src-ctrl7_4"},
		{8, 11, "[8 ... 11]", "adi,gpio-src-ctrl11_8"},	   {12, 15, "[12 ... 15]", "adi,gpio-src-ctrl15_12"},
		{16, 18, "[16 ... 18]", "adi,gpio-src-ctrl18_16"},
	};

	for(const auto &g : groups) {
		QWidget *row = new QWidget(widget);
		QHBoxLayout *rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(0, 2, 0, 2);
		rowLayout->setSpacing(5);

		rowLayout->addWidget(new QLabel(g.label, row));

		auto *srcCtrl =
			Ad9371WidgetFactory::createDebugCustomComboWidget(m_device, g.attr, srcCtrlMap, "", row);
		if(srcCtrl) {
			if(m_widgetGroup) m_widgetGroup->add(srcCtrl);
			rowLayout->addWidget(srcCtrl);
			m_widgets.append(srcCtrl);
			connect(this, &GpioWidget::readRequested, srcCtrl, &IIOWidget::readAsync);
		}

		for(int pin = g.startPin; pin <= g.endPin; pin++) {
			auto *sw = new MenuOnOffSwitch("ENABLE", row);
			sw->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
			rowLayout->addWidget(sw);
			m_gpioOeBits.append(sw);
			connect(sw->onOffswitch(), &QAbstractButton::toggled, this, &GpioWidget::onGpioOeMaskChanged);
		}

		layout->addWidget(row);
	}

	layout->addStretch();
	return widget;
}

void GpioWidget::onGpio3v3OeMaskChanged() { writeGpio3v3OeMask(); }

void GpioWidget::onGpioOeMaskChanged() { writeGpioOeMask(); }

void GpioWidget::readGpio3v3OeMask()
{
	if(!m_device || m_gpio3v3OeBits.size() != 12) {
		return;
	}

	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,gpio-3v3-oe-mask", &mask);
	if(ret < 0) {
		qDebug(CAT_AD9371_GPIO) << "Failed to read gpio-3v3-oe-mask, using defaults";
		mask = 0;
	}

	for(int i = 0; i < 12; i++) {
		m_gpio3v3OeBits[i]->blockSignals(true);
		m_gpio3v3OeBits[i]->onOffswitch()->setChecked((mask & (1LL << i)) != 0);
		m_gpio3v3OeBits[i]->blockSignals(false);
	}
}

void GpioWidget::writeGpio3v3OeMask()
{
	if(!m_device || m_gpio3v3OeBits.size() != 12) {
		return;
	}

	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,gpio-3v3-oe-mask", &mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_GPIO) << "Failed to read gpio-3v3-oe-mask before write, error:" << ret;
		return;
	}

	// Clear managed bits 0-11
	mask &= ~((1LL << 12) - 1);

	for(int i = 0; i < 12; i++) {
		if(m_gpio3v3OeBits[i]->onOffswitch()->isChecked()) {
			mask |= (1LL << i);
		}
	}

	ret = iio_device_debug_attr_write_longlong(m_device, "adi,gpio-3v3-oe-mask", mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_GPIO) << "Failed to write gpio-3v3-oe-mask, error:" << ret;
	}
}

void GpioWidget::readGpioOeMask()
{
	if(!m_device || m_gpioOeBits.size() != 19) {
		return;
	}

	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,gpio-oe-mask", &mask);
	if(ret < 0) {
		qDebug(CAT_AD9371_GPIO) << "Failed to read gpio-oe-mask, using defaults";
		mask = 0;
	}

	for(int i = 0; i < 19; i++) {
		m_gpioOeBits[i]->blockSignals(true);
		m_gpioOeBits[i]->onOffswitch()->setChecked((mask & (1LL << i)) != 0);
		m_gpioOeBits[i]->blockSignals(false);
	}
}

void GpioWidget::writeGpioOeMask()
{
	if(!m_device || m_gpioOeBits.size() != 19) {
		return;
	}

	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,gpio-oe-mask", &mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_GPIO) << "Failed to read gpio-oe-mask before write, error:" << ret;
		return;
	}

	// Clear managed bits 0-18
	mask &= ~((1LL << 19) - 1);

	for(int i = 0; i < 19; i++) {
		if(m_gpioOeBits[i]->onOffswitch()->isChecked()) {
			mask |= (1LL << i);
		}
	}

	ret = iio_device_debug_attr_write_longlong(m_device, "adi,gpio-oe-mask", mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_GPIO) << "Failed to write gpio-oe-mask, error:" << ret;
	}
}
