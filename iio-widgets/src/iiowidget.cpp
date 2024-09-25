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
#include "channelattrdatastrategy.h"
#include "contextattrdatastrategy.h"
#include "deviceattrdatastrategy.h"
#include "iiowidgetselector.h"
#include <iioutil/connectionprovider.h>
#include <QDateTime>
#include <QApplication>
#include <pluginbase/preferences.h>
#include <gui/utils.h>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_IIOWIDGET, "iioWidget")

IIOWidget::IIOWidget(GuiStrategyInterface *uiStrategy, DataStrategyInterface *dataStrategy, QWidget *parent)
	: QWidget(parent)
	, m_uiStrategy(uiStrategy)
	, m_dataStrategy(dataStrategy)
	, m_progressBar(new SmallProgressBar(this))
	, m_configBtn(new QPushButton(this))
	, m_lastOpTimestamp(nullptr)
	, m_lastOpState(nullptr)
	, m_lastReturnCode(0)
	, m_UItoDS(nullptr)
	, m_DStoUI(nullptr)
	, m_isConfigurable(false)
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

DataStrategyInterface *IIOWidget::swapDataStrategy(DataStrategyInterface *dataStrategy)
{
	QWidget *dataStrategyWidget = dynamic_cast<QWidget *>(m_dataStrategy);
	QWidget *uiStrategyWidget = dynamic_cast<QWidget *>(m_uiStrategy);
	QWidget *newDataStrategyWidget = dynamic_cast<QWidget *>(dataStrategy);

	// disconnect old data strategy
	disconnect(dataStrategyWidget, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		   SLOT(emitDataStatus(QDateTime, QString, QString, int, bool)));
	disconnect(uiStrategyWidget, SIGNAL(requestData()), dataStrategyWidget, SLOT(readAsync()));
	disconnect(dataStrategyWidget, SIGNAL(sendData(QString, QString)), uiStrategyWidget,
		   SLOT(receiveData(QString, QString)));
	disconnect(dataStrategyWidget, SIGNAL(sendData(QString, QString)), this,
		   SLOT(storeReadInfo(QString, QString))); // TODO: maybe do something with this slot..
	disconnect(dataStrategyWidget, SIGNAL(sendData(QString, QString)), this, SIGNAL(sendData(QString, QString)));
	disconnect(dataStrategyWidget, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		   SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)));
	disconnect(dataStrategyWidget, SIGNAL(aboutToWrite(QString, QString)), this,
		   SIGNAL(aboutToWrite(QString, QString)));
	disconnect(uiStrategyWidget, SIGNAL(emitData(QString)), this, SIGNAL(emitData(QString)));
	disconnect(uiStrategyWidget, SIGNAL(displayedNewData(QString, QString)), this,
		   SIGNAL(displayedNewData(QString, QString)));

	// connect new data strategy
	connect(newDataStrategyWidget, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SLOT(emitDataStatus(QDateTime, QString, QString, int, bool)));
	connect(uiStrategyWidget, SIGNAL(requestData()), newDataStrategyWidget, SLOT(readAsync()));
	connect(newDataStrategyWidget, SIGNAL(sendData(QString, QString)), uiStrategyWidget,
		SLOT(receiveData(QString, QString)));
	connect(newDataStrategyWidget, SIGNAL(sendData(QString, QString)), this, SLOT(storeReadInfo(QString, QString)));
	connect(dataStrategyWidget, SIGNAL(sendData(QString, QString)), this, SIGNAL(sendData(QString, QString)));
	connect(dataStrategyWidget, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)));
	connect(dataStrategyWidget, SIGNAL(aboutToWrite(QString, QString)), this,
		SIGNAL(aboutToWrite(QString, QString)));
	connect(uiStrategyWidget, SIGNAL(emitData(QString)), this, SIGNAL(emitData(QString)));
	connect(uiStrategyWidget, SIGNAL(displayedNewData(QString, QString)), this,
		SIGNAL(displayedNewData(QString, QString)));

	// save the new data strategy and return the old one
	DataStrategyInterface *oldDS = m_dataStrategy;
	m_dataStrategy = dataStrategy;
	return oldDS;
}

void IIOWidget::saveData(QString data)
{
	setLastOperationState(IIOWidget::Busy);
	setLastOperationTimestamp(QDateTime::currentDateTime());
	m_progressBar->setBarColor(StyleHelper::getColor("ProgressBarBusy"));
	setToolTip("Operation in progress.");

	qDebug(CAT_IIOWIDGET) << "Sending data" << data << "to data strategy.";
	m_dataStrategy->writeAsync(data);
}

void IIOWidget::changeTitle(QString title) { m_uiStrategy->changeName(title); }

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

IIOWidgetFactoryRecipe IIOWidget::getRecipe() { return m_recipe; }

void IIOWidget::setRecipe(IIOWidgetFactoryRecipe recipe) { m_recipe = recipe; }

QDateTime *IIOWidget::lastOperationTimestamp() { return m_lastOpTimestamp; }

IIOWidget::State *IIOWidget::lastOperationState() { return m_lastOpState; }

int IIOWidget::lastReturnCode() { return m_lastReturnCode; }

void IIOWidget::setUItoDataConversion(std::function<QString(QString)> func) { m_UItoDS = func; }

void IIOWidget::setDataToUIConversion(std::function<QString(QString)> func) { m_DStoUI = func; }

void IIOWidget::setConfigurable(bool isConfigurable)
{
	m_isConfigurable = isConfigurable;
	m_configBtn->setVisible(m_isConfigurable);
}

void IIOWidget::setUIEnabled(bool isEnabled) { m_uiStrategy->ui()->setEnabled(isEnabled); }

QString IIOWidget::optionalData() const { return m_dataStrategy->optionalData(); }

QString IIOWidget::data() const { return m_dataStrategy->data(); }

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

void IIOWidget::reconfigure()
{
	// display the popup and switch the DS
	if(m_recipe.context) {
		m_configPopup = new IIOConfigurationPopup(m_recipe.context, qApp->activeWindow());
	} else if(m_recipe.device) {
		m_configPopup = new IIOConfigurationPopup(m_recipe.device, qApp->activeWindow());
	} else if(m_recipe.channel) {
		m_configPopup = new IIOConfigurationPopup(m_recipe.channel, qApp->activeWindow());
	} else {
		qCritical(CAT_IIOWIDGET) << "No available context/device/channel";
	}

	connect(m_configPopup, &IIOConfigurationPopup::exitButtonClicked, this,
		[&]() { m_configPopup->deleteLater(); });
	connect(m_configPopup, &IIOConfigurationPopup::selectButtonClicked, this, [&](IIOItem *item) {
		DataStrategyInterface *dsCreated = nullptr;
		switch(item->type()) {
		case IIOItem::CHANNEL_ATTR:
			dsCreated = new ChannelAttrDataStrategy({.channel = item->chnl(), .data = item->name()}, this);
			break;
		case IIOItem::DEVICE_ATTR:
			dsCreated = new DeviceAttrDataStrategy({.device = item->dev(), .data = item->name()}, this);
			break;
		case IIOItem::CONTEXT_ATTR:
			dsCreated = new ContextAttrDataStrategy({.context = item->ctx(), .data = item->name()}, this);
		default:
			break;
		}

		if(!dsCreated) {
			qWarning(CAT_IIOWIDGET) << "Could not create a new data strategy.";
			return;
		}

		m_uiStrategy->changeName(item->name().toUpper());
		DataStrategyInterface *oldDS = swapDataStrategy(dsCreated);
		dsCreated->readAsync();
		delete oldDS;
		delete m_configPopup;
	});
	m_configPopup->enableTintedOverlay(true);
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
