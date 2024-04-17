/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#include "max14906/diodigitalchannel.h"
#include "swiot_logging_categories.h"
#include <QColor>
#include <QLineEdit>
#include <gui/plotaxis.h>
#include <gui/stylehelper.h>

using namespace scopy::swiotrefactor;

DioDigitalChannel::DioDigitalChannel(const QString &deviceName, const QString &deviceType, QWidget *parent)
	: QWidget(parent)
	, m_deviceName(deviceName)
	, m_deviceType(deviceType)
{
	setMinimumWidth(300);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	initData();
	initPlot();
	QWidget *topContainer = createTopContainer(this);
	QWidget *bottomContainer = createBottomContainer(this);

	m_channelName->setText(deviceName);
	m_channelType->setText(deviceType);

	layout->addWidget(topContainer);
	layout->addWidget(m_plot);
	layout->addWidget(bottomContainer);

	if(m_deviceType == "INPUT") {
		m_valueSwitch->setVisible(false);
		m_currentLimitsCombo->hide();
	}
	connectSignalsAndSlots();
}

DioDigitalChannel::~DioDigitalChannel() {}

void DioDigitalChannel::connectSignalsAndSlots()
{
	connect(m_valueSwitch, &CustomSwitch::toggled, this, [this]() {
		bool isChecked = m_valueSwitch->isChecked();
		Q_EMIT outputValueChanged(isChecked);
	});
}

void DioDigitalChannel::initData()
{
	for(int i = 0; i <= m_timeSpan; i++) {
		m_xTime.push_back(i);
	}
}

void DioDigitalChannel::initPlot()
{
	m_plot = new PlotWidget(this);
	m_plot->xAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->xAxis()->setInterval(0, m_timeSpan);
	m_plot->yAxis()->scaleEngine()->setAttribute(QwtScaleEngine::Floating, true);
	m_plot->yAxis()->scaleEngine()->setMajorTicksCount(2);
	m_plot->yAxis()->scaleDraw()->setFloatPrecision(0);
	m_plot->yAxis()->setInterval(0, 1);
	m_plot->setShowXAxisLabels(true);
	m_plot->setShowYAxisLabels(true);
	m_plot->replot();

	QPen chPen = QPen(QColor(StyleHelper::getColor("CH0")), 1);
	m_plotCh = new PlotChannel("CH0", chPen, m_plot->xAxis(), m_plot->yAxis(), this);
	m_plot->addPlotChannel(m_plotCh);
	m_plotCh->setEnabled(true);

	m_plot->replot();
}

void DioDigitalChannel::resetPlot()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_yValues.clear();
	m_xTime.clear();
	for(int i = 0; i <= m_timeSpan; i++) {
		m_xTime.push_back(i);
	}
	m_plotCh->curve()->setRawSamples(m_xTime.data(), m_yValues.data(), m_yValues.size());
	m_plot->replot();
}

void DioDigitalChannel::updateTimeScale(double newMax)
{
	m_timeSpan = newMax;
	m_plot->xAxis()->setMax(newMax);
	resetPlot();
}

void DioDigitalChannel::addDataSample(double value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_yValues.push_back(value);
	if(m_yValues.size() > m_timeSpan + 1) {
		m_yValues.pop_front();
	}
	if(m_yValues.size() <= m_timeSpan + 1) {
		m_plotCh->curve()->setRawSamples(m_xTime.data() + (m_xTime.size() - m_yValues.size()), m_yValues.data(),
						 m_yValues.size());
	}
	m_plot->replot();
	m_lcdNumber->display(value);
}

const QStringList DioDigitalChannel::getConfigModes() const { return m_configModes; }

void DioDigitalChannel::setConfigModes(QStringList &configModes)
{
	m_configModes = configModes;

	m_configModesCombo->combo()->clear();
	for(const QString &item : qAsConst(m_configModes)) {
		m_configModesCombo->combo()->addItem(item);
	}
}

const QString &DioDigitalChannel::getSelectedConfigMode() const { return m_selectedConfigMode; }

void DioDigitalChannel::setSelectedConfigMode(const QString &selectedConfigMode)
{
	m_selectedConfigMode = selectedConfigMode;

	int index = m_configModesCombo->combo()->findText(selectedConfigMode);
	m_configModesCombo->combo()->setCurrentIndex(index);
	qDebug(CAT_SWIOT_MAX14906) << "The channel " << m_deviceName << " read selected config mode "
				   << selectedConfigMode;
}

QWidget *DioDigitalChannel::createBottomContainer(QWidget *parent)
{
	QWidget *bottomWidget = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(bottomWidget);

	m_configModesCombo = new MenuCombo("Mode", bottomWidget);
	m_configModesCombo->setMinimumWidth(140);
	layout->addWidget(m_configModesCombo);

	m_currentLimitsCombo = new MenuCombo("Current limit", bottomWidget);
	layout->addWidget(m_currentLimitsCombo);

	layout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));

	QWidget *readBackWidget = new QWidget(bottomWidget);
	readBackWidget->setLayout(new QVBoxLayout());
	readBackWidget->layout()->setContentsMargins(0, 0, 0, 0);

	QLabel *readValueLabel = new QLabel(readBackWidget);
	readValueLabel->setText("Readback");
	StyleHelper::MenuSmallLabel(readValueLabel);
	readBackWidget->layout()->addWidget(readValueLabel);

	m_lcdNumber = new LcdNumber(readBackWidget);
	m_lcdNumber->setDigitCount(1);
	m_lcdNumber->setPrecision(0);
	m_lcdNumber->setSegmentStyle(QLCDNumber::Flat);
	m_lcdNumber->setFrameShape(QFrame::NoFrame);
	StyleHelper::OrangeWidget(m_lcdNumber);
	readBackWidget->layout()->addWidget(m_lcdNumber);

	layout->addWidget(readBackWidget);

	return bottomWidget;
}

QWidget *DioDigitalChannel::createTopContainer(QWidget *parent)
{
	QWidget *topWidget = new QWidget(parent);
	QHBoxLayout *layout = new QHBoxLayout(topWidget);

	m_channelName = new QLabel(topWidget);
	StyleHelper::MenuSmallLabel(m_channelName);
	layout->addWidget(m_channelName, 0, Qt::AlignLeft);

	QLineEdit *channelTitleLineEdit = new QLineEdit(topWidget);
	channelTitleLineEdit->setPlaceholderText("Enter title here...");
	layout->addWidget(channelTitleLineEdit, 0, Qt::AlignLeft);

	layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

	m_channelType = new QLabel(topWidget);
	StyleHelper::MenuSmallLabel(m_channelType);
	layout->addWidget(m_channelType, 0, Qt::AlignRight);

	m_valueSwitch = new SmallOnOffSwitch(topWidget);
	layout->addWidget(m_valueSwitch, 0, Qt::AlignRight);

	return topWidget;
}
