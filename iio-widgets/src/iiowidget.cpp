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
#include <style.h>
#include <pluginbase/preferences.h>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_IIOWIDGET, "iioWidget")

IIOWidget::IIOWidget(GuiStrategyInterface *uiStrategy, DataStrategyInterface *dataStrategy, QWidget *parent)
	: QWidget(parent)
	, m_uiStrategy(uiStrategy)
	, m_dataStrategy(dataStrategy)
	, m_progressBar(new SmallProgressBar(this))
	, m_lastOpTimestamp(nullptr)
	, m_lastOpState(nullptr)
	, m_lastReturnCode(0)
	, m_UItoDS(nullptr)
	, m_DStoUI(nullptr)
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setSpacing(0);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	QWidget *ui = m_uiStrategy->ui();
	if(ui) {
		layout()->addWidget(ui);
	}
	layout()->addWidget(m_progressBar);

	QObject *uiStrategyObject = dynamic_cast<QObject *>(m_uiStrategy);
	QObject *dataStrategyObject = dynamic_cast<QObject *>(m_dataStrategy);

	uiStrategyObject->setParent(this);
	dataStrategyObject->setParent(this);

	connect(m_progressBar, &SmallProgressBar::progressFinished, this,
		[this]() { this->convertUItoDS(m_lastData); });

	connect(uiStrategyObject, SIGNAL(emitData(QString)), this, SLOT(startTimer(QString)));

	connect(dataStrategyObject, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SLOT(emitDataStatus(QDateTime, QString, QString, int, bool)));

	// forward data request from ui strategy to data strategy
	connect(uiStrategyObject, SIGNAL(requestData()), dataStrategyObject, SLOT(readAsync()));

	// forward data from data strategy to ui strategy
	connect(dataStrategyObject, SIGNAL(sendData(QString, QString)), this, SLOT(convertDStoUI(QString, QString)));

	// intercept the sendData from dataStrategy to collect information
	connect(dataStrategyObject, SIGNAL(sendData(QString, QString)), this, SLOT(storeReadInfo(QString, QString)));

	// The data will be populated here
	bool useLazyLoading = Preferences::GetInstance()->get("iiowidgets_use_lazy_loading").toBool();
	if(!useLazyLoading) { // force skip lazy load
		LAZY_LOAD(initialize);
	}
}

QPair<QString, QString> IIOWidget::read() { return m_dataStrategy->read(); }

int IIOWidget::write(QString data) { return m_dataStrategy->write(data); }

void IIOWidget::readAsync() { m_dataStrategy->readAsync(); }

void IIOWidget::writeAsync(QString data) { m_dataStrategy->writeAsync(data); }

void IIOWidget::saveData(QString data)
{
	setLastOperationState(IIOWidget::Busy);
	setLastOperationTimestamp(QDateTime::currentDateTime());
	m_progressBar->setBarColor(Style::getAttribute(json::theme::content_busy));
	setToolTip("Operation in progress.");

	qDebug(CAT_IIOWIDGET) << "Sending data" << data << "to data strategy.";
	m_dataStrategy->writeAsync(data);
}

void IIOWidget::emitDataStatus(QDateTime timestamp, QString oldData, QString newData, int status, bool isReadOp)
{
	// The read operation will not be shown as a status here as it will overlap with the
	// write operation that is more likely to fail
	if(isReadOp) {
		qDebug(CAT_IIOWIDGET) << timestamp.toString("[hh:mm:ss]")
				      << "READ (return code: " << QString::number(status) << "):" << oldData << "->"
				      << newData;
		return;
	}
	setLastOperationTimestamp(timestamp);
	QString timestampFormat = timestamp.toString("hh:mm:ss");
	if(status < 0) {
		m_progressBar->setBarColor(Style::getAttribute(json::theme::content_error));
		QString statusString = "Tried to write \"" + m_lastData +
			"\", but failed.\nError: " + QString(strerror(-status)) + " (" + QString::number(status) + ").";
		setToolTip("[" + timestampFormat + "] " + statusString);
		setLastOperationState(IIOWidget::Error);
		qDebug(CAT_IIOWIDGET) << statusString;
	} else {
		m_progressBar->setBarColor(Style::getAttribute(json::theme::content_success));
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

int IIOWidget::lastReturnCode() { return m_lastReturnCode; }

void IIOWidget::setUItoDataConversion(std::function<QString(QString)> func) { m_UItoDS = func; }

void IIOWidget::setDataToUIConversion(std::function<QString(QString)> func) { m_DStoUI = func; }

void IIOWidget::startTimer(QString data)
{
	m_lastData = data;
	m_progressBar->setBarColor(Style::getAttribute(json::theme::interactive_subtle_idle));
	m_progressBar->startProgress();
}

void IIOWidget::storeReadInfo(QString data, QString optionalData)
{
	// the parameters are unused for the moment
	Q_UNUSED(data)
	Q_UNUSED(optionalData)
	setLastOperationTimestamp(QDateTime::currentDateTime());
}

void IIOWidget::convertUItoDS(QString data)
{
	if(m_UItoDS) {
		data = m_UItoDS(data);
	}
	this->saveData(data);
}

void IIOWidget::convertDStoUI(QString data, QString optionalData)
{
	if(m_DStoUI) { // only the data should be converted
		data = m_DStoUI(data);
	}
	m_uiStrategy->receiveData(data, optionalData);
}

void IIOWidget::initialize() { m_dataStrategy->readAsync(); }

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
