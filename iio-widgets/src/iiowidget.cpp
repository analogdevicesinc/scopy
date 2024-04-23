/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "iiowidget.h"
#include <QDateTime>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_IIOWIDGET, "iioWidget")

IIOWidget::IIOWidget(GuiStrategyInterface *uiStrategy, DataStrategyInterface *dataStrategy, QWidget *parent)
	: QWidget(parent)
	, m_uiStrategy(uiStrategy)
	, m_dataStrategy(dataStrategy)
	, m_progressBar(new SmallProgressBar(this))
	, m_lastOpTimestamp(nullptr)
	, m_lastOpState(nullptr)
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setSpacing(0);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	QWidget *ui = uiStrategy->ui();
	if(ui) {
		layout()->addWidget(ui);
	}
	layout()->addWidget(m_progressBar);

	QWidget *uiStrategyWidget = dynamic_cast<QWidget *>(m_uiStrategy);
	QWidget *dataStrategyWidget = dynamic_cast<QWidget *>(m_dataStrategy);

	connect(m_progressBar, &SmallProgressBar::progressFinished, this, [this]() { this->saveData(m_lastData); });

	connect(uiStrategyWidget, SIGNAL(emitData(QString)), this, SLOT(startTimer(QString)));

	connect(dataStrategyWidget, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SLOT(emitDataStatus(QDateTime, QString, QString, int, bool)));

	// forward data request from ui strategy to data strategy
	connect(uiStrategyWidget, SIGNAL(requestData()), dataStrategyWidget, SLOT(requestData()));

	// forward data from data strategy to ui strategy
	connect(dataStrategyWidget, SIGNAL(sendData(QString, QString)), uiStrategyWidget,
		SLOT(receiveData(QString, QString)));

	// intercept the sendData from dataStrategy to collect information
	connect(dataStrategyWidget, SIGNAL(sendData(QString, QString)), this, SLOT(storeReadInfo(QString, QString)));

	m_dataStrategy->requestData();
}

void IIOWidget::saveData(QString data)
{
	setLastOperationState(IIOWidget::Busy);
	setLastOperationTimestamp(QDateTime::currentDateTime());
	m_progressBar->setBarColor(StyleHelper::getColor("ProgressBarBusy"));
	setToolTip("Operation in progress.");

	qDebug(CAT_IIOWIDGET) << "Sending data" << data << "to data strategy.";
	m_dataStrategy->save(data);
}

void IIOWidget::emitDataStatus(QDateTime timestamp, QString oldData, QString newData, int status, bool isReadOp)
{
	// The read operation will not be shown as a status here as it will overlap with the
	// write operation that is more likely to fail
	if(isReadOp) {
		qInfo(CAT_IIOWIDGET) << timestamp.toString("[hh:mm:ss]")
				     << "READ (return code: " << QString::number(status) << "):" << oldData << "->"
				     << newData;
		return;
	}
	setLastOperationTimestamp(timestamp);
	QString timestampFormat = timestamp.toString("hh:mm:ss");
	if(status < 0) {
		m_progressBar->setBarColor(StyleHelper::getColor("ProgressBarError"));
		QString statusString = "Tried to write \"" + m_lastData +
			"\", but failed.\nError: " + QString(strerror(-status)) + " (" + QString::number(status) + ").";
		setToolTip("[" + timestampFormat + "] " + statusString);
		setLastOperationState(IIOWidget::Error);
		qDebug(CAT_IIOWIDGET) << statusString;
	} else {
		m_progressBar->setBarColor(StyleHelper::getColor("ProgressBarSuccess"));
		QString statusString = "Operation finished successfully.";
		setToolTip("[" + timestampFormat + "] " + statusString);
		setLastOperationState(IIOWidget::Correct);
		qDebug(CAT_IIOWIDGET) << statusString << ". Wrote " + m_lastData + ".";
	}
	auto *timer = new QTimer();
	timer->setSingleShot(true);
	QObject::connect(timer, &QTimer::timeout, this, [this, timer]() {
		qDebug(CAT_IIOWIDGET) << "Timeout for displaying success finished.";
		m_progressBar->resetBarColor();
		timer->deleteLater();
	});
	timer->start(4000);
}

GuiStrategyInterface *IIOWidget::getUiStrategy() { return m_uiStrategy; }

DataStrategyInterface *IIOWidget::getDataStrategy() { return m_dataStrategy; }

IIOWidgetFactoryRecipe IIOWidget::getRecipe() { return m_recipe; }

void IIOWidget::setRecipe(IIOWidgetFactoryRecipe recipe) { m_recipe = recipe; }

QDateTime *IIOWidget::lastOperationTimestamp() { return m_lastOpTimestamp; }

IIOWidget::State *IIOWidget::lastOperationState() { return m_lastOpState; }

void IIOWidget::startTimer(QString data)
{
	m_lastData = data;
	m_progressBar->setBarColor(StyleHelper::getColor("ScopyBlue"));
	m_progressBar->startProgress();
}

void IIOWidget::storeReadInfo(QString data, QString optionalData)
{
	// the parameters are unused for the moment
	Q_UNUSED(data)
	Q_UNUSED(optionalData)
	setLastOperationTimestamp(QDateTime::currentDateTime());
}

void IIOWidget::setLastOperationTimestamp(QDateTime timestamp)
{
	if(m_lastOpTimestamp == nullptr) {
		m_lastOpTimestamp = new QDateTime();
	}
	*m_lastOpTimestamp = timestamp;
}

void IIOWidget::setLastOperationState(State state)
{
	if(m_lastOpState == nullptr) {
		m_lastOpState = new IIOWidget::State;
	}
	*m_lastOpState = state;
}

#include "moc_iiowidget.cpp"
