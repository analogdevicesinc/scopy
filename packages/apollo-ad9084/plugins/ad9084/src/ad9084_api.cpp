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

#include "ad9084_api.hpp"

#include "ad9084.h"
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/animatedrefreshbtn.h>
#include <gui/customSwitch.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>
#include <iio.h>
#include <QAbstractButton>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <limits>

using namespace scopy::ad9084;
using scopy::IIOWidget;

Q_LOGGING_CATEGORY(CAT_AD9084_API, "Ad9084_API")

namespace {

static double nanValue() { return std::numeric_limits<double>::quiet_NaN(); }

static QString channelKey(iio_channel *channel)
{
	if(!channel) {
		return QString();
	}
	const char *id = iio_channel_get_id(channel);
	if(!id || !*id) {
		id = iio_channel_get_name(channel);
	}
	return id ? QString::fromLatin1(id) : QString();
}

static IIOWidget *findChannelAttrWidget(QWidget *root, const QString &attr, const QString &channelId,
					bool requireOutput, bool outputFlag)
{
	if(!root) {
		return nullptr;
	}
	const auto widgets = root->findChildren<IIOWidget *>();
	for(IIOWidget *widget : widgets) {
		auto recipe = widget->getRecipe();
		if(recipe.data != attr) {
			continue;
		}
		if(requireOutput && recipe.channel &&
		   static_cast<bool>(iio_channel_is_output(recipe.channel)) != outputFlag) {
			continue;
		}
		if(channelId.isEmpty() || channelKey(recipe.channel) == channelId) {
			return widget;
		}
	}
	return nullptr;
}

static QList<IIOWidget *> findAttrWidgets(QWidget *root, const QString &attr)
{
	QList<IIOWidget *> result;
	if(!root) {
		return result;
	}
	const auto widgets = root->findChildren<IIOWidget *>();
	for(IIOWidget *widget : widgets) {
		auto recipe = widget->getRecipe();
		if(recipe.data == attr) {
			result.push_back(widget);
		}
	}
	return result;
}

static QList<IIOWidget *> findEnableWidgetsSorted(QWidget *root)
{
	auto widgets = findAttrWidgets(root, "en");
	std::sort(widgets.begin(), widgets.end(), [](IIOWidget *a, IIOWidget *b) {
		return channelKey(a ? a->getRecipe().channel : nullptr) <
			channelKey(b ? b->getRecipe().channel : nullptr);
	});
	return widgets;
}

static QWidget *widgetUi(IIOWidget *widget)
{
	if(!widget) {
		return nullptr;
	}
	auto *strategy = widget->getUiStrategy();
	QWidget *ui = strategy ? strategy->ui() : nullptr;
	return ui ? ui : widget;
}

static scopy::gui::MenuSpinbox *findSpinbox(IIOWidget *widget)
{
	QWidget *ui = widgetUi(widget);
	return ui ? ui->findChild<scopy::gui::MenuSpinbox *>() : nullptr;
}

static QLineEdit *findLineEdit(IIOWidget *widget)
{
	QWidget *ui = widgetUi(widget);
	return ui ? ui->findChild<QLineEdit *>() : nullptr;
}

static QAbstractButton *findToggleButton(IIOWidget *widget)
{
	QWidget *ui = widgetUi(widget);
	if(!ui) {
		return nullptr;
	}
	if(auto *onOff = ui->findChild<scopy::MenuOnOffSwitch *>()) {
		return onOff->onOffswitch();
	}
	if(auto *custom = ui->findChild<scopy::CustomSwitch *>()) {
		return custom;
	}
	const auto buttons = ui->findChildren<QAbstractButton *>();
	for(QAbstractButton *button : buttons) {
		if(button && button->isCheckable()) {
			return button;
		}
	}
	return nullptr;
}

static bool readWidgetNumeric(IIOWidget *widget, double &value)
{
	if(!widget) {
		return false;
	}
	if(auto *spinbox = findSpinbox(widget)) {
		value = spinbox->value();
		return true;
	}
	if(auto *lineEdit = findLineEdit(widget)) {
		bool ok = false;
		value = lineEdit->text().toDouble(&ok);
		return ok;
	}
	return false;
}

static bool readWidgetBool(IIOWidget *widget, bool &value)
{
	if(auto *button = findToggleButton(widget)) {
		value = button->isChecked();
		return true;
	}
	return false;
}

static bool setWidgetNumeric(IIOWidget *widget, double value)
{
	if(!widget) {
		return false;
	}
	if(auto *spinbox = findSpinbox(widget)) {
		if(qFuzzyCompare(spinbox->value() + 1.0, value + 1.0)) {
			return true;
		}
		spinbox->setValue(value);
		return true;
	}
	if(auto *lineEdit = findLineEdit(widget)) {
		const QString text = QString::number(value, 'g', 12);
		if(lineEdit->text() == text) {
			return true;
		}
		lineEdit->setText(text);
		QMetaObject::invokeMethod(lineEdit, "editingFinished", Qt::DirectConnection);
		return true;
	}
	return false;
}

static bool setWidgetBool(IIOWidget *widget, bool value)
{
	if(auto *button = findToggleButton(widget)) {
		if(button->isChecked() == value) {
			return true;
		}
		button->click();
		return true;
	}
	return false;
}

static scopy::AnimatedRefreshBtn *findRefreshButton(QWidget *root)
{
	return root ? root->findChild<scopy::AnimatedRefreshBtn *>() : nullptr;
}

} // namespace

Ad9084_API::Ad9084_API(struct iio_device *dev, QWidget *tool, QObject *parent)
	: ApiObject(parent)
	, m_dev(dev)
	, m_tool(tool)
{}

Ad9084_API::~Ad9084_API() = default;

void Ad9084_API::show()
{
	if(m_tool) {
		m_tool->show();
		m_tool->raise();
	}
}

void Ad9084_API::setRxTabEnabled(bool enable)
{
	auto *tool = qobject_cast<Ad9084 *>(m_tool);
	if(tool) {
		tool->setRxTabEnabled(enable);
	}
}

void Ad9084_API::setTxTabEnabled(bool enable)
{
	auto *tool = qobject_cast<Ad9084 *>(m_tool);
	if(tool) {
		tool->setTxTabEnabled(enable);
	}
}

bool Ad9084_API::getRxTabEnabled()
{
	auto *tool = qobject_cast<Ad9084 *>(m_tool);
	return tool ? tool->isRxTabEnabled() : false;
}

bool Ad9084_API::getTxTabEnabled()
{
	auto *tool = qobject_cast<Ad9084 *>(m_tool);
	return tool ? tool->isTxTabEnabled() : false;
}

bool Ad9084_API::setNco(double freqHz)
{
	const double valueMHz = freqHz / 1e6;
	bool ok = false;
	for(const auto &attr : {QString("main_nco_frequency"), QString("channel_nco_frequency")}) {
		const auto widgets = findAttrWidgets(m_tool, attr);
		for(IIOWidget *widget : widgets) {
			ok |= setWidgetNumeric(widget, valueMHz);
		}
	}
	if(!ok) {
		qWarning(CAT_AD9084_API) << "NCO attribute not available";
	}
	return ok;
}

double Ad9084_API::getNco()
{
	double value = 0.0;
	for(const auto &attr : {QString("main_nco_frequency"), QString("channel_nco_frequency")}) {
		IIOWidget *widget = findChannelAttrWidget(m_tool, attr, QString(), false, false);
		if(readWidgetNumeric(widget, value)) {
			return value * 1e6;
		}
	}
	qWarning(CAT_AD9084_API) << "NCO attribute not available";
	return nanValue();
}

bool Ad9084_API::setSampleRate(double rateHz)
{
	const double valueMHz = rateHz / 1e6;
	bool ok = false;
	for(const auto &attr : {QString("adc_frequency"), QString("dac_frequency")}) {
		const auto widgets = findAttrWidgets(m_tool, attr);
		for(IIOWidget *widget : widgets) {
			ok |= setWidgetNumeric(widget, valueMHz);
		}
	}
	if(!ok) {
		qWarning(CAT_AD9084_API) << "Sample rate attribute not available";
	}
	return ok;
}

double Ad9084_API::getSampleRate()
{
	double value = 0.0;
	for(const auto &attr : {QString("adc_frequency"), QString("dac_frequency")}) {
		IIOWidget *widget = findChannelAttrWidget(m_tool, attr, QString(), false, false);
		if(readWidgetNumeric(widget, value)) {
			return value * 1e6;
		}
	}
	qWarning(CAT_AD9084_API) << "Sample rate attribute not available";
	return nanValue();
}

bool Ad9084_API::setEnableChannel(int idx, bool enable)
{
	auto widgets = findEnableWidgetsSorted(m_tool);
	if(idx < 0 || idx >= widgets.size()) {
		qWarning(CAT_AD9084_API) << "Channel index out of range" << idx;
		return false;
	}
	return setWidgetBool(widgets.at(idx), enable);
}

bool Ad9084_API::getEnableChannel(int idx)
{
	auto widgets = findEnableWidgetsSorted(m_tool);
	if(idx < 0 || idx >= widgets.size()) {
		qWarning(CAT_AD9084_API) << "Channel index out of range" << idx;
		return false;
	}
	bool enabled = false;
	if(!readWidgetBool(widgets.at(idx), enabled)) {
		return false;
	}
	return enabled;
}

bool Ad9084_API::applyProfile(const QString &path)
{
	(void)path;
	qWarning(CAT_AD9084_API) << "applyProfile is not implemented for AD9084";
	return false;
}

void Ad9084_API::refresh()
{
	auto *button = findRefreshButton(m_tool);
	if(!button) {
		qWarning(CAT_AD9084_API) << "Refresh button not found";
		return;
	}
	button->click();
}
