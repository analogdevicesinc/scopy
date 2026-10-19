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

#include "adalm2000voltmeter_api.h"

#include "adalm2000voltmetertool.h"
#include "menucombo.h"

#include <QAbstractButton>
#include <QComboBox>
#include <QLineEdit>
#include <QLoggingCategory>

#include <gui/widgets/filebrowserwidget.h>
#include <gui/widgets/menuonoffswitch.h>
#include <pluginbase/scopyjs.h>

Q_LOGGING_CATEGORY(CAT_ADALM2000VOLTMETER_API, "Adalm2000Voltmeter_API")

using namespace scopy::adalm2000;

Adalm2000Voltmeter_API::Adalm2000Voltmeter_API(Adalm2000VoltmeterTool *tool)
	: ApiObject()
	, m_tool(tool)
{}

Adalm2000Voltmeter_API::~Adalm2000Voltmeter_API() { ScopyJS::GetInstance()->unregisterApi(this); }

bool Adalm2000Voltmeter_API::running() const { return m_tool->m_runBtn->isChecked(); }

void Adalm2000Voltmeter_API::setRunning(bool on) { m_tool->m_runBtn->setChecked(on); }

bool Adalm2000Voltmeter_API::modeAcCh1() const { return m_tool->m_acMode[0]; }

void Adalm2000Voltmeter_API::setModeAcCh1(bool ac) { m_tool->m_modeCombo[0]->combo()->setCurrentIndex(ac ? 1 : 0); }

bool Adalm2000Voltmeter_API::modeAcCh2() const { return m_tool->m_acMode[1]; }

void Adalm2000Voltmeter_API::setModeAcCh2(bool ac) { m_tool->m_modeCombo[1]->combo()->setCurrentIndex(ac ? 1 : 0); }

QList<int> Adalm2000Voltmeter_API::gainModes() const
{
	return {m_tool->m_gainCombo[0]->combo()->currentIndex(), m_tool->m_gainCombo[1]->combo()->currentIndex()};
}

void Adalm2000Voltmeter_API::setGainModes(const QList<int> &modes)
{
	if(modes.size() != 2) {
		qWarning(CAT_ADALM2000VOLTMETER_API) << "gainModes requires a list of exactly 2 integers";
		return;
	}
	for(int c = 0; c < 2; ++c) {
		const int idx = modes[c];
		if(idx < 0 || idx >= m_tool->m_gainCombo[c]->combo()->count()) {
			qWarning(CAT_ADALM2000VOLTMETER_API) << "gainModes[" << c << "] out of range:" << idx;
			continue;
		}
		m_tool->m_gainCombo[c]->combo()->setCurrentIndex(idx);
	}
}

double Adalm2000Voltmeter_API::valueCh1() const { return m_tool->m_lastReading[0]; }

double Adalm2000Voltmeter_API::valueCh2() const { return m_tool->m_lastReading[1]; }

bool Adalm2000Voltmeter_API::plotVisibleCh1() const
{
	return m_tool->m_plotVisibleSwitch[0]->onOffswitch()->isChecked();
}

void Adalm2000Voltmeter_API::setPlotVisibleCh1(bool on)
{
	m_tool->m_plotVisibleSwitch[0]->onOffswitch()->setChecked(on);
}

bool Adalm2000Voltmeter_API::plotVisibleCh2() const
{
	return m_tool->m_plotVisibleSwitch[1]->onOffswitch()->isChecked();
}

void Adalm2000Voltmeter_API::setPlotVisibleCh2(bool on)
{
	m_tool->m_plotVisibleSwitch[1]->onOffswitch()->setChecked(on);
}

bool Adalm2000Voltmeter_API::peakHoldEn() const { return m_tool->m_peakHoldSwitch->onOffswitch()->isChecked(); }

void Adalm2000Voltmeter_API::setPeakHoldEn(bool en) { m_tool->m_peakHoldSwitch->onOffswitch()->setChecked(en); }

double Adalm2000Voltmeter_API::peakMinCh1() const { return m_tool->m_peakMin[0]; }

double Adalm2000Voltmeter_API::peakMinCh2() const { return m_tool->m_peakMin[1]; }

double Adalm2000Voltmeter_API::peakMaxCh1() const { return m_tool->m_peakMax[0]; }

double Adalm2000Voltmeter_API::peakMaxCh2() const { return m_tool->m_peakMax[1]; }

void Adalm2000Voltmeter_API::resetPeakHold() { m_tool->resetPeakHold(); }

bool Adalm2000Voltmeter_API::dataLoggingEn() const { return m_tool->m_logSwitch->onOffswitch()->isChecked(); }

void Adalm2000Voltmeter_API::setDataLoggingEn(bool en) { m_tool->m_logSwitch->onOffswitch()->setChecked(en); }

QString Adalm2000Voltmeter_API::dataLoggingPath() const { return m_tool->m_logFilePath->lineEdit()->text(); }

void Adalm2000Voltmeter_API::setDataLoggingPath(const QString &path)
{
	m_tool->m_logFilePath->lineEdit()->setText(path);
}

#include "moc_adalm2000voltmeter_api.cpp"
