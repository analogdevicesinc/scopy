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

#include "adrv9002_api.hpp"

#include "adrv9002.h"
#include "initialcalibrationswidget.h"
#include "profilemanager.h"

#include <gui/widgets/animatedrefreshbtn.h>
#include <gui/customSwitch.h>
#include <gui/widgets/filebrowserwidget.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuspinbox.h>
#include <iio-widgets/iiowidget.h>
#include <iio.h>
#include <QAbstractButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QTextEdit>
#include <QWidget>
#include <cmath>
#include <limits>

using namespace scopy::adrv9002;
using scopy::IIOWidget;

Q_LOGGING_CATEGORY(CAT_ADRV9002_API, "Adrv9002_API")

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
	return channelId == channelKey(channel);
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

static QString findDriverVersionLabel(QWidget *root)
{
	if(!root) {
		return QString();
	}
	QLabel *title = nullptr;
	const auto labels = root->findChildren<QLabel *>();
	for(QLabel *label : labels) {
		if(label->text().trimmed() == "Device Driver API") {
			title = label;
			break;
		}
	}
	if(!title) {
		return QString();
	}
	QWidget *parent = title->parentWidget();
	if(!parent) {
		return QString();
	}
	const auto siblings = parent->findChildren<QLabel *>(QString(), Qt::FindDirectChildrenOnly);
	for(QLabel *label : siblings) {
		if(label != title) {
			return label->text().trimmed();
		}
	}
	return QString();
}

static ProfileManager *findProfileManager(QWidget *root)
{
	return root ? root->findChild<ProfileManager *>() : nullptr;
}

static QTextEdit *findProfileConfigText(ProfileManager *manager)
{
	return manager ? manager->findChild<QTextEdit *>() : nullptr;
}

static scopy::FileBrowserWidget *findProfileBrowser(ProfileManager *manager, bool stream)
{
	if(!manager) {
		return nullptr;
	}
	const auto browsers = manager->findChildren<scopy::FileBrowserWidget *>();
	if(browsers.size() < 2) {
		return nullptr;
	}
	return stream ? browsers.at(1) : browsers.at(0);
}

static scopy::AnimatedRefreshBtn *findRefreshButton(QWidget *root)
{
	return root ? root->findChild<scopy::AnimatedRefreshBtn *>() : nullptr;
}

} // namespace

Adrv9002_API::Adrv9002_API(struct iio_context *ctx, QWidget *tool, QObject *parent)
	: ApiObject(parent)
	, m_ctx(ctx)
	, m_tool(tool)
{}

Adrv9002_API::~Adrv9002_API() = default;

void Adrv9002_API::show()
{
	if(m_tool) {
		m_tool->show();
		m_tool->raise();
	}
}

QString Adrv9002_API::getDriverVersion()
{
	const QString version = findDriverVersionLabel(m_tool);
	if(version.isEmpty()) {
		qWarning(CAT_ADRV9002_API) << "Driver version label not found";
	}
	return version;
}

double Adrv9002_API::getTemperatureC()
{
	IIOWidget *widget = findChannelAttrWidget(m_tool, "input", "temp0", false, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "Temperature attribute not available";
		return nanValue();
	}
	return value;
}

bool Adrv9002_API::isInitialCalibrationsSupported()
{
	if(!m_tool) {
		return false;
	}
	auto *calWidget = m_tool->findChild<InitialCalibrationsWidget *>();
	return calWidget ? calWidget->isEnabled() : false;
}

QString Adrv9002_API::getProfileConfig()
{
	auto *manager = findProfileManager(m_tool);
	auto *text = findProfileConfigText(manager);
	return text ? text->toPlainText().trimmed() : QString();
}

QString Adrv9002_API::getStreamConfig()
{
	auto *manager = findProfileManager(m_tool);
	auto *browser = findProfileBrowser(manager, true);
	if(!browser || !browser->lineEdit()) {
		return QString();
	}
	const QString text = browser->lineEdit()->text().trimmed();
	if(!text.isEmpty()) {
		return text;
	}
	return browser->lineEdit()->placeholderText().trimmed();
}

bool Adrv9002_API::setRxLo(double freqHz)
{
	IIOWidget *widget = findChannelAttrWidget(m_tool, "RX1_LO_frequency", "altvoltage0", true, true);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "RX LO widget not found";
		return false;
	}
	return setWidgetNumeric(widget, freqHz / 1e6);
}

double Adrv9002_API::getRxLo()
{
	IIOWidget *widget = findChannelAttrWidget(m_tool, "RX1_LO_frequency", "altvoltage0", true, true);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "RX LO frequency not available";
		return nanValue();
	}
	return value * 1e6;
}

bool Adrv9002_API::setTxLo(double freqHz)
{
	IIOWidget *widget = findChannelAttrWidget(m_tool, "TX1_LO_frequency", "altvoltage2", true, true);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "TX LO widget not found";
		return false;
	}
	return setWidgetNumeric(widget, freqHz / 1e6);
}

double Adrv9002_API::getTxLo()
{
	IIOWidget *widget = findChannelAttrWidget(m_tool, "TX1_LO_frequency", "altvoltage2", true, true);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "TX LO frequency not available";
		return nanValue();
	}
	return value * 1e6;
}

bool Adrv9002_API::setRxGain(int chan, double value)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "hardwaregain", chName, true, false);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "RX gain widget not found" << chName;
		return false;
	}
	return setWidgetNumeric(widget, value);
}

double Adrv9002_API::getRxGain(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "hardwaregain", chName, true, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "RX gain attribute not available" << chName;
		return nanValue();
	}
	return value;
}

bool Adrv9002_API::setRxGainMode(int chan, const QString &mode)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "gain_control_mode", chName, true, false);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "RX gain mode widget not found" << chName;
		return false;
	}
	return setWidgetText(widget, mode);
}

QString Adrv9002_API::getRxGainMode(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "gain_control_mode", chName, true, false);
	return readWidgetText(widget);
}

bool Adrv9002_API::setRxPowerdown(int chan, bool powerdown)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "en", chName, true, false);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "RX enable widget not found" << chName;
		return false;
	}
	return setWidgetBool(widget, !powerdown);
}

bool Adrv9002_API::getRxPowerdown(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "en", chName, true, false);
	bool enabled = false;
	if(!readWidgetBool(widget, enabled)) {
		return false;
	}
	return !enabled;
}

bool Adrv9002_API::setTxAtten(int chan, double value)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "hardwaregain", chName, true, true);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "TX attenuation widget not found" << chName;
		return false;
	}
	return setWidgetNumeric(widget, value);
}

double Adrv9002_API::getTxAtten(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "hardwaregain", chName, true, true);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "TX attenuation attribute not available" << chName;
		return nanValue();
	}
	return value;
}

bool Adrv9002_API::setTxAttenMode(int chan, const QString &mode)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "atten_control_mode", chName, true, true);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "TX attenuation mode widget not found" << chName;
		return false;
	}
	return setWidgetText(widget, mode);
}

QString Adrv9002_API::getTxAttenMode(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "atten_control_mode", chName, true, true);
	return readWidgetText(widget);
}

bool Adrv9002_API::setTxPowerdown(int chan, bool powerdown)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "en", chName, true, true);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "TX enable widget not found" << chName;
		return false;
	}
	return setWidgetBool(widget, !powerdown);
}

bool Adrv9002_API::getTxPowerdown(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "en", chName, true, true);
	bool enabled = false;
	if(!readWidgetBool(widget, enabled)) {
		return false;
	}
	return !enabled;
}

bool Adrv9002_API::setRxEnsmMode(int chan, const QString &mode)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "ensm_mode", chName, true, false);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "RX ENSM widget not found" << chName;
		return false;
	}
	return setWidgetText(widget, mode);
}

QString Adrv9002_API::getRxEnsmMode(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "ensm_mode", chName, true, false);
	return readWidgetText(widget);
}

bool Adrv9002_API::setTxEnsmMode(int chan, const QString &mode)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "ensm_mode", chName, true, true);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "TX ENSM widget not found" << chName;
		return false;
	}
	return setWidgetText(widget, mode);
}

QString Adrv9002_API::getTxEnsmMode(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "ensm_mode", chName, true, true);
	return readWidgetText(widget);
}

double Adrv9002_API::getRxDecimatedPower(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "decimated_power", chName, true, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "RX decimated power not available" << chName;
		return nanValue();
	}
	return value;
}

double Adrv9002_API::getRxBandwidth(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "rf_bandwidth", chName, true, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "RX bandwidth not available" << chName;
		return nanValue();
	}
	return value * 1e6;
}

double Adrv9002_API::getTxBandwidth(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "rf_bandwidth", chName, true, true);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "TX bandwidth not available" << chName;
		return nanValue();
	}
	return value * 1e6;
}

double Adrv9002_API::getRxSampleRate(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "sampling_frequency", chName, true, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "RX sampling rate not available" << chName;
		return nanValue();
	}
	return value * 1e6;
}

double Adrv9002_API::getTxSampleRate(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "sampling_frequency", chName, true, true);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "TX sampling rate not available" << chName;
		return nanValue();
	}
	return value * 1e6;
}

bool Adrv9002_API::setSampleRate(double rateHz)
{
	const QString chName = "voltage0";
	IIOWidget *rx = findChannelAttrWidget(m_tool, "sampling_frequency", chName, true, false);
	IIOWidget *tx = findChannelAttrWidget(m_tool, "sampling_frequency", chName, true, true);
	bool ok = false;
	if(rx && rx->isEnabled()) {
		ok |= setWidgetNumeric(rx, rateHz / 1e6);
	}
	if(tx && tx->isEnabled()) {
		ok |= setWidgetNumeric(tx, rateHz / 1e6);
	}
	if(!ok) {
		qWarning(CAT_ADRV9002_API) << "Sample rate control not available";
	}
	return ok;
}

double Adrv9002_API::getSampleRate()
{
	const QString chName = "voltage0";
	IIOWidget *rx = findChannelAttrWidget(m_tool, "sampling_frequency", chName, true, false);
	IIOWidget *tx = findChannelAttrWidget(m_tool, "sampling_frequency", chName, true, true);
	double value = 0.0;
	if(readWidgetNumeric(rx, value) || readWidgetNumeric(tx, value)) {
		return value * 1e6;
	}
	qWarning(CAT_ADRV9002_API) << "Sample rate attribute not available";
	return nanValue();
}

bool Adrv9002_API::setRfBandwidth(double bwHz)
{
	const QString chName = "voltage0";
	IIOWidget *rx = findChannelAttrWidget(m_tool, "rf_bandwidth", chName, true, false);
	IIOWidget *tx = findChannelAttrWidget(m_tool, "rf_bandwidth", chName, true, true);
	bool ok = false;
	if(rx && rx->isEnabled()) {
		ok |= setWidgetNumeric(rx, bwHz / 1e6);
	}
	if(tx && tx->isEnabled()) {
		ok |= setWidgetNumeric(tx, bwHz / 1e6);
	}
	if(!ok) {
		qWarning(CAT_ADRV9002_API) << "RF bandwidth control not available";
	}
	return ok;
}

double Adrv9002_API::getRfBandwidth()
{
	const QString chName = "voltage0";
	IIOWidget *rx = findChannelAttrWidget(m_tool, "rf_bandwidth", chName, true, false);
	IIOWidget *tx = findChannelAttrWidget(m_tool, "rf_bandwidth", chName, true, true);
	double value = 0.0;
	if(readWidgetNumeric(rx, value) || readWidgetNumeric(tx, value)) {
		return value * 1e6;
	}
	qWarning(CAT_ADRV9002_API) << "RF bandwidth attribute not available";
	return nanValue();
}

bool Adrv9002_API::setOrxGain(int chan, double value)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "orx_hardwaregain", chName, true, false);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "ORX gain widget not found" << chName;
		return false;
	}
	return setWidgetNumeric(widget, value);
}

double Adrv9002_API::getOrxGain(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "orx_hardwaregain", chName, true, false);
	double value = 0.0;
	if(!readWidgetNumeric(widget, value)) {
		qWarning(CAT_ADRV9002_API) << "ORX gain not available" << chName;
		return nanValue();
	}
	return value;
}

bool Adrv9002_API::setOrxPowerdown(int chan, bool powerdown)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "orx_en", chName, true, false);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "ORX enable widget not found" << chName;
		return false;
	}
	return setWidgetBool(widget, !powerdown);
}

bool Adrv9002_API::getOrxPowerdown(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "orx_en", chName, true, false);
	bool enabled = false;
	if(!readWidgetBool(widget, enabled)) {
		return false;
	}
	return !enabled;
}

bool Adrv9002_API::setBbdcRejection(int chan, bool enable)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "orx_bbdc_rejection_en", chName, true, false);
	if(!widget) {
		qWarning(CAT_ADRV9002_API) << "BBDC rejection widget not found" << chName;
		return false;
	}
	return setWidgetBool(widget, enable);
}

bool Adrv9002_API::getBbdcRejection(int chan)
{
	const QString chName = QString("voltage%1").arg(chan);
	IIOWidget *widget = findChannelAttrWidget(m_tool, "orx_bbdc_rejection_en", chName, true, false);
	bool enabled = false;
	if(!readWidgetBool(widget, enabled)) {
		return false;
	}
	return enabled;
}

bool Adrv9002_API::applyProfile(const QString &path)
{
	if(path.isEmpty()) {
		qWarning(CAT_ADRV9002_API) << "Profile path is empty";
		return false;
	}
	auto *manager = findProfileManager(m_tool);
	auto *browser = findProfileBrowser(manager, false);
	if(!browser || !browser->lineEdit()) {
		qWarning(CAT_ADRV9002_API) << "Profile browser not available";
		return false;
	}
	browser->lineEdit()->setText(path);
	return true;
}

void Adrv9002_API::refresh()
{
	auto *button = findRefreshButton(m_tool);
	if(!button) {
		qWarning(CAT_ADRV9002_API) << "Refresh button not found";
		return;
	}
	button->click();
}
