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

#include "ad936x_api.hpp"
#include "ad936xplugin.h"

#include <iio.h>
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/animatedrefreshbtn.h>
#include <gui/widgets/menuspinbox.h>
#include <QComboBox>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QWidget>
#include <QString>
#include <cmath>
#include <limits>

using namespace scopy::ad936x;
using scopy::IIOWidget;

Q_LOGGING_CATEGORY(CAT_AD936X_API, "Ad936x_API")

namespace {

static double nanValue() { return std::numeric_limits<double>::quiet_NaN(); }

static IIOWidget *findDeviceAttrWidget(QWidget *root, const QString &attr)
{
	if(!root) {
		return nullptr;
	}
	const auto widgets = root->findChildren<IIOWidget *>();
	for(IIOWidget *widget : widgets) {
		auto recipe = widget->getRecipe();
		if(recipe.channel == nullptr && recipe.device != nullptr && recipe.data == attr) {
			return widget;
		}
	}
	return nullptr;
}

static bool channelMatches(iio_channel *channel, const QString &channelId, bool requireOutput, bool outputFlag)
{
	if(!channel) {
		return false;
	}
	if(requireOutput && static_cast<bool>(iio_channel_is_output(channel)) != outputFlag) {
		return false;
	}
	if(channelId.isEmpty()) {
		return true;
	}
	const char *id = iio_channel_get_id(channel);
	if(!id || !*id) {
		id = iio_channel_get_name(channel);
	}
	if(!id) {
		return false;
	}
	return channelId == QString::fromLatin1(id);
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
		if(channelMatches(recipe.channel, channelId, requireOutput, outputFlag)) {
			return widget;
		}
	}
	return nullptr;
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

static scopy::AnimatedRefreshBtn *findRefreshButton(QWidget *root)
{
	return root ? root->findChild<scopy::AnimatedRefreshBtn *>() : nullptr;
}

static QComboBox *findCombo(IIOWidget *widget)
{
	QWidget *ui = widgetUi(widget);
	return ui ? ui->findChild<QComboBox *>() : nullptr;
}

static QLineEdit *findLineEdit(IIOWidget *widget)
{
	QWidget *ui = widgetUi(widget);
	return ui ? ui->findChild<QLineEdit *>() : nullptr;
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

static QString readWidgetText(IIOWidget *widget)
{
	if(!widget) {
		return QString();
	}
	if(auto *combo = findCombo(widget)) {
		return combo->currentText().trimmed();
	}
	if(auto *lineEdit = findLineEdit(widget)) {
		return lineEdit->text().trimmed();
	}
	return QString();
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

static bool setWidgetText(IIOWidget *widget, const QString &value)
{
	if(!widget) {
		return false;
	}
	if(auto *combo = findCombo(widget)) {
		int idx = combo->findText(value);
		if(idx < 0) {
			return false;
		}
		if(combo->currentIndex() == idx) {
			return true;
		}
		combo->setCurrentIndex(idx);
		return true;
	}
	if(auto *lineEdit = findLineEdit(widget)) {
		if(lineEdit->text() == value) {
			return true;
		}
		lineEdit->setText(value);
		QMetaObject::invokeMethod(lineEdit, "editingFinished", Qt::DirectConnection);
		return true;
	}
	return false;
}

} // namespace

Ad936x_API::Ad936x_API(Ad936xPlugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

Ad936x_API::~Ad936x_API() = default;

void Ad936x_API::setToolRole(ToolRole role) { m_role = role; }

struct iio_context *Ad936x_API::context() const { return m_plugin ? m_plugin->context() : nullptr; }

QWidget *Ad936x_API::toolWidget() const
{
	if(!m_plugin) {
		return nullptr;
	}
	return m_role == ToolRole::Advanced ? m_plugin->advancedTool() : m_plugin->mainTool();
}

bool Ad936x_API::setRxLo(double freqHz)
{
	IIOWidget *widget = findChannelAttrWidget(toolWidget(), "frequency", "altvoltage0", false, false);
	if(!widget) {
		qWarning(CAT_AD936X_API) << "RX LO channel not found";
		return false;
	}
	return setWidgetNumeric(widget, freqHz / 1e6);
}

double Ad936x_API::getRxLo()
{
	IIOWidget *widget = findChannelAttrWidget(toolWidget(), "frequency", "altvoltage0", false, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_AD936X_API) << "RX LO frequency not available";
		return nanValue();
	}
	return value * 1e6;
}

bool Ad936x_API::setTxLo(double freqHz)
{
	IIOWidget *widget = findChannelAttrWidget(toolWidget(), "frequency", "altvoltage1", false, false);
	if(!widget) {
		qWarning(CAT_AD936X_API) << "TX LO channel not found";
		return false;
	}
	return setWidgetNumeric(widget, freqHz / 1e6);
}

double Ad936x_API::getTxLo()
{
	IIOWidget *widget = findChannelAttrWidget(toolWidget(), "frequency", "altvoltage1", false, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_AD936X_API) << "TX LO frequency not available";
		return nanValue();
	}
	return value * 1e6;
}

bool Ad936x_API::setSampleRate(double rateHz)
{
	bool ok = false;
	IIOWidget *rx = findChannelAttrWidget(toolWidget(), "sampling_frequency", "voltage0", true, false);
	IIOWidget *tx = findChannelAttrWidget(toolWidget(), "sampling_frequency", "voltage0", true, true);
	ok |= setWidgetNumeric(rx, rateHz / 1e6);
	ok |= setWidgetNumeric(tx, rateHz / 1e6);
	if(!ok) {
		qWarning(CAT_AD936X_API) << "Sample rate attribute not available";
	}
	return ok;
}

double Ad936x_API::getSampleRate()
{
	IIOWidget *rx = findChannelAttrWidget(toolWidget(), "sampling_frequency", "voltage0", true, false);
	IIOWidget *tx = findChannelAttrWidget(toolWidget(), "sampling_frequency", "voltage0", true, true);
	double value = 0.0;
	if(readWidgetNumeric(rx, value) || readWidgetNumeric(tx, value)) {
		return value * 1e6;
	}
	qWarning(CAT_AD936X_API) << "Sample rate attribute not available";
	return nanValue();
}

bool Ad936x_API::setRfBandwidth(double bwHz)
{
	bool ok = false;
	IIOWidget *rx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, false);
	IIOWidget *tx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, true);
	ok |= setWidgetNumeric(rx, bwHz / 1e6);
	ok |= setWidgetNumeric(tx, bwHz / 1e6);
	if(!ok) {
		qWarning(CAT_AD936X_API) << "RF bandwidth attribute not available";
	}
	return ok;
}

double Ad936x_API::getRfBandwidth()
{
	IIOWidget *rx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, false);
	IIOWidget *tx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, true);
	double value = 0.0;
	if(readWidgetNumeric(rx, value) || readWidgetNumeric(tx, value)) {
		return value * 1e6;
	}
	qWarning(CAT_AD936X_API) << "RF bandwidth attribute not available";
	return nanValue();
}

bool Ad936x_API::setRxBandwidth(double bwHz)
{
	IIOWidget *rx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, false);
	if(!rx) {
		qWarning(CAT_AD936X_API) << "RX channel not found for bandwidth";
		return false;
	}
	return setWidgetNumeric(rx, bwHz / 1e6);
}

double Ad936x_API::getRxBandwidth()
{
	IIOWidget *rx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, false);
	double value = 0.0;
	if(readWidgetNumeric(rx, value)) {
		return value * 1e6;
	}
	qWarning(CAT_AD936X_API) << "RX bandwidth attribute not available";
	return nanValue();
}

bool Ad936x_API::setTxBandwidth(double bwHz)
{
	IIOWidget *tx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, true);
	if(!tx) {
		qWarning(CAT_AD936X_API) << "TX channel not found for bandwidth";
		return false;
	}
	return setWidgetNumeric(tx, bwHz / 1e6);
}

double Ad936x_API::getTxBandwidth()
{
	IIOWidget *tx = findChannelAttrWidget(toolWidget(), "rf_bandwidth", "voltage0", true, true);
	double value = 0.0;
	if(readWidgetNumeric(tx, value)) {
		return value * 1e6;
	}
	qWarning(CAT_AD936X_API) << "TX bandwidth attribute not available";
	return nanValue();
}

bool Ad936x_API::setGain(int chan, double value)
{
	QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(toolWidget(), "hardwaregain", chName, true, false);
	if(!widget) {
		widget = findChannelAttrWidget(toolWidget(), "hardwaregain", chName, true, true);
	}
	if(!widget) {
		qWarning(CAT_AD936X_API) << "Gain channel not found" << chName;
		return false;
	}
	return setWidgetNumeric(widget, value);
}

double Ad936x_API::getGain(int chan)
{
	QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(toolWidget(), "hardwaregain", chName, true, false);
	if(!widget) {
		widget = findChannelAttrWidget(toolWidget(), "hardwaregain", chName, true, true);
	}
	double value = 0.0;
	if(readWidgetNumeric(widget, value)) {
		return value;
	}
	qWarning(CAT_AD936X_API) << "Gain attribute not available" << chName;
	return nanValue();
}

bool Ad936x_API::setEnsmMode(const QString &mode)
{
	IIOWidget *widget = findDeviceAttrWidget(toolWidget(), "ensm_mode");
	if(!widget) {
		qWarning(CAT_AD936X_API) << "ENSM mode widget not found";
		return false;
	}
	return setWidgetText(widget, mode);
}

QString Ad936x_API::getEnsmMode()
{
	IIOWidget *widget = findDeviceAttrWidget(toolWidget(), "ensm_mode");
	QString value = readWidgetText(widget);
	if(value.isEmpty()) {
		qWarning(CAT_AD936X_API) << "ENSM mode not available";
	}
	return value;
}

bool Ad936x_API::setCalibrationMode(const QString &mode)
{
	IIOWidget *widget = findDeviceAttrWidget(toolWidget(), "calib_mode");
	if(!widget) {
		qWarning(CAT_AD936X_API) << "Calibration mode widget not found";
		return false;
	}
	return setWidgetText(widget, mode);
}

QString Ad936x_API::getCalibrationMode()
{
	IIOWidget *widget = findDeviceAttrWidget(toolWidget(), "calib_mode");
	QString value = readWidgetText(widget);
	if(value.isEmpty()) {
		qWarning(CAT_AD936X_API) << "Calibration mode not available";
	}
	return value;
}

bool Ad936x_API::setRateGovernor(const QString &mode)
{
	IIOWidget *widget = findDeviceAttrWidget(toolWidget(), "trx_rate_governor");
	if(!widget) {
		qWarning(CAT_AD936X_API) << "Rate governor widget not found";
		return false;
	}
	return setWidgetText(widget, mode);
}

QString Ad936x_API::getRateGovernor()
{
	IIOWidget *widget = findDeviceAttrWidget(toolWidget(), "trx_rate_governor");
	QString value = readWidgetText(widget);
	if(value.isEmpty()) {
		qWarning(CAT_AD936X_API) << "Rate governor not available";
	}
	return value;
}

void Ad936x_API::refresh()
{
	auto *button = findRefreshButton(toolWidget());
	if(!button) {
		qWarning(CAT_AD936X_API) << "Refresh button not found";
		return;
	}
	button->click();
}
