/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "cn0357_api.h"
#include "cn0357.h"
#include "cn0357tool.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_CN0357_API, "Cn0357_API")

using namespace scopy::cn0357;

Cn0357_API::Cn0357_API(Cn0357Plugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

Cn0357_API::~Cn0357_API() {}

// --- Private helpers ---

Cn0357Tool *Cn0357_API::tool()
{
	if(m_plugin->m_toolList.isEmpty())
		return nullptr;
	return qobject_cast<Cn0357Tool *>(m_plugin->m_toolList[0]->tool());
}

QString Cn0357_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_CN0357_API) << "Widget group not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_CN0357_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void Cn0357_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_CN0357_API) << "Widget group not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_CN0357_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList Cn0357_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- Generic IIOWidget access ---

QStringList Cn0357_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup)
		return QStringList();
	return m_plugin->m_widgetGroup->keys();
}

QString Cn0357_API::readWidget(const QString &key) { return readFromWidget(key); }

void Cn0357_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

void Cn0357_API::refresh()
{
	if(!m_plugin->m_widgetGroup)
		return;

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

// --- ADC settings (IIOWidget-backed) ---

QString Cn0357_API::getSamplingFrequency() { return readFromWidget("ad7790/sampling_frequency"); }

void Cn0357_API::setSamplingFrequency(const QString &value) { writeToWidget("ad7790/sampling_frequency", value); }

QString Cn0357_API::getAdcMillivolts()
{
	QString raw = readFromWidget("ad7790/voltage0_in/raw");
	if(raw.isEmpty())
		return raw;
	bool ok;
	double val = raw.toDouble(&ok);
	if(!ok)
		return raw;
	return QString::number((val / 32768.0 - 1.0) * 1200.0, 'f', 3);
}

QString Cn0357_API::getSupplyVoltage()
{
	QString raw = readFromWidget("ad7790/supply_in/raw");
	if(raw.isEmpty())
		return raw;
	bool ok;
	double val = raw.toDouble(&ok);
	if(!ok)
		return raw;
	return QString::number((val / 32768.0 - 1.0) * 5.85, 'f', 3);
}

// --- Feedback settings (UI-only) ---

QString Cn0357_API::getFeedbackType()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_feedbackType)
		return QString();
	return t->m_feedbackType->currentText();
}

void Cn0357_API::setFeedbackType(const QString &value)
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_feedbackType)
		return;

	static const QStringList options = {"Rheostat", "Fixed Resistor"};
	if(!options.contains(value)) {
		qWarning(CAT_CN0357_API) << "Invalid feedback type:" << value << "Valid:" << options;
		return;
	}

	t->m_feedbackType->setCurrentText(value);
}

QString Cn0357_API::getRdacValue()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_rdacSpinBox)
		return QString();
	return QString::number(static_cast<int>(t->m_rdacSpinBox->value()));
}

void Cn0357_API::setRdacValue(const QString &value)
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_rdacSpinBox)
		return;
	t->m_rdacSpinBox->setValue(value.toDouble());
}

void Cn0357_API::programRheostat()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_programRdacBtn)
		return;
	t->m_programRdacBtn->click();
}

QString Cn0357_API::getFixedResistor()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_fixedRes)
		return QString();
	return QString::number(t->m_fixedRes->value(), 'f', 2);
}

void Cn0357_API::setFixedResistor(const QString &value)
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_fixedRes)
		return;
	t->m_fixedRes->setValue(value.toDouble());
}

// --- System settings (UI-only) ---

QString Cn0357_API::getSensorSensitivity()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_sensorSensitivity)
		return QString();
	return QString::number(t->m_sensorSensitivity->value(), 'f', 2);
}

void Cn0357_API::setSensorSensitivity(const QString &value)
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_sensorSensitivity)
		return;
	t->m_sensorSensitivity->setValue(value.toDouble());
}

// --- Computed values (read-only) ---

QString Cn0357_API::getConcentrationPpm()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_ppmDisplay)
		return QString();
	return t->m_ppmDisplay->text();
}

QString Cn0357_API::getFeedbackResistance()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_feedbackResDisplay)
		return QString();
	return t->m_feedbackResDisplay->text();
}

QString Cn0357_API::getPpmPerMv()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_ppmMvDisplay)
		return QString();
	return t->m_ppmMvDisplay->text();
}

QString Cn0357_API::getMvPerPpm()
{
	Cn0357Tool *t = tool();
	if(!t || !t->m_mvPpmDisplay)
		return QString();
	return t->m_mvPpmDisplay->text();
}

#include "moc_cn0357_api.cpp"
