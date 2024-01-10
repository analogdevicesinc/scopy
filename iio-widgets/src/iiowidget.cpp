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

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_IIOWIDGET, "iioWidget")

IIOWidget::IIOWidget(AttrUiStrategyInterface *uiStrategy, SaveStrategyInterface *saveStrategy,
		     DataStrategyInterface *dataStrategy, QWidget *parent)
	: QWidget(parent)
	, m_uiStrategy(uiStrategy)
	, m_saveStrategy(saveStrategy)
	, m_dataStrategy(dataStrategy)
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	QWidget *ui = uiStrategy->ui();
	if(ui) {
		layout()->addWidget(ui);
	}

	QWidget *saveUi = saveStrategy->ui();
	if(saveUi) {
		layout()->addWidget(saveUi);
	}

	connect(dynamic_cast<QWidget *>(m_uiStrategy), SIGNAL(emitData(QString)),
		dynamic_cast<QWidget *>(m_saveStrategy), SLOT(receiveData(QString)));
	connect(dynamic_cast<QWidget *>(m_saveStrategy), SIGNAL(saveData(QString)),
		dynamic_cast<QWidget *>(m_dataStrategy), SLOT(save(QString)));

	connect(dynamic_cast<QWidget *>(m_saveStrategy), SIGNAL(saveData(QString)), this, SLOT(saveData(QString)));

	connect(dynamic_cast<QWidget *>(m_dataStrategy), SIGNAL(emitStatus(int)), this, SLOT(emitDataStatus(int)));

	connect(dynamic_cast<QWidget *>(m_uiStrategy), SIGNAL(requestData()), dynamic_cast<QWidget *>(m_dataStrategy),
		SLOT(requestData()));
	connect(dynamic_cast<QWidget *>(m_dataStrategy), SIGNAL(sendData(QString, QString)),
		dynamic_cast<QWidget *>(m_uiStrategy), SLOT(receiveData(QString, QString)));

	m_dataStrategy->requestData();
}

void IIOWidget::saveData(QString data)
{
	Q_EMIT currentStateChanged(State::Busy, "Operation in progress.");
	m_dataStrategy->save(data);
}

void IIOWidget::emitDataStatus(int status)
{
	if(status < 0) {
		Q_EMIT currentStateChanged(
			State::Error, "Error: " + QString(strerror(-status)) + " (" + QString::number(status) + ")");
	} else {
		Q_EMIT currentStateChanged(State::Correct, "Operation finished successfully.");
	}
}

SaveStrategyInterface *IIOWidget::getSaveStrategy() { return m_saveStrategy; }

AttrUiStrategyInterface *IIOWidget::getUiStrategy() { return m_uiStrategy; }

DataStrategyInterface *IIOWidget::getDataStrategy() { return m_dataStrategy; }

IIOWidgetFactoryRecipe IIOWidget::getRecipe() { return m_recipe; }

void IIOWidget::setRecipe(IIOWidgetFactoryRecipe recipe) { m_recipe = recipe; }

#include "moc_iiowidget.cpp"
