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

IIOWidget::IIOWidget(AttrUiStrategyInterface *uiStrategy, DataStrategyInterface *dataStrategy, QWidget *parent)
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
	connect(m_progressBar, &SmallProgressBar::progressFinished, this, [this]() { this->saveData(m_lastData); });

	connect(dynamic_cast<QWidget *>(m_uiStrategy), SIGNAL(emitData(QString)), this, SLOT(startTimer(QString)));
	connect(dynamic_cast<QWidget *>(m_dataStrategy), SIGNAL(emitStatus(int)), this, SLOT(emitDataStatus(int)));

	// forward data request from ui strategy to data strategy
	connect(dynamic_cast<QWidget *>(m_uiStrategy), SIGNAL(requestData()), dynamic_cast<QWidget *>(m_dataStrategy),
		SLOT(requestData()));

	// forward data from data strategy to ui strategy
	connect(dynamic_cast<QWidget *>(m_dataStrategy), SIGNAL(sendData(QString, QString)),
		dynamic_cast<QWidget *>(m_uiStrategy), SLOT(receiveData(QString, QString)));

	m_dataStrategy->requestData();
}

void IIOWidget::saveData(QString data)
{
	setLastOperationState(IIOWidget::Busy);
	m_progressBar->setBarColor(StyleHelper::getColor("ProgressBarBusy"));
	setToolTip("Operation in progress.");

	qDebug(CAT_IIOWIDGET) << "Sending data" << data << "to data strategy.";
	m_dataStrategy->save(data);
}

void IIOWidget::emitDataStatus(int status)
{
	setLastOperationTimestamp(QDateTime::currentDateTime());
	QString timestamp = m_lastOpTimestamp->toString("hh:mm:ss");
	if(status < 0) {
		m_progressBar->setBarColor(StyleHelper::getColor("ProgressBarError"));
		QString statusString = "Tried to write \"" + m_lastData +
			"\", but failed.\nError: " + QString(strerror(-status)) + " (" + QString::number(status) + ").";
		setToolTip("[" + timestamp + "] " + statusString);
		setLastOperationState(IIOWidget::Error);
		qDebug(CAT_IIOWIDGET) << statusString;
	} else {
		m_progressBar->setBarColor(StyleHelper::getColor("ProgressBarSuccess"));
		QString statusString = "Operation finished successfully.";
		setToolTip("[" + timestamp + "] " + statusString);
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

AttrUiStrategyInterface *IIOWidget::getUiStrategy() { return m_uiStrategy; }

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
