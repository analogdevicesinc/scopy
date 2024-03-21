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

#include "faults/faultsgroup.h"
#include "swiot_logging_categories.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWidget>
#include <gui/stylehelper.h>

#include <utility>

using namespace scopy::swiotrefactor;

#define MAX_COLS_IN_GRID 100

FaultsGroup::FaultsGroup(QString name, const QString &path, QWidget *parent)
	: QWidget(parent)
	, m_name(std::move(name))
{
	setMinimumSize(700, 90);
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	m_activeStoredWidget = new QWidget(this);
	m_activeStoredWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QVBoxLayout *activeStoredLay = new QVBoxLayout(m_activeStoredWidget);
	activeStoredLay->setContentsMargins(0, 0, 0, 0);
	activeStoredLay->setSpacing(0);
	activeStoredLay->setAlignment(Qt::AlignCenter);
	m_activeStoredWidget->setLayout(activeStoredLay);

	m_customColGrid = new FlexGridLayout(MAX_COLS_IN_GRID, this);
	layout->addWidget(m_activeStoredWidget);
	layout->addWidget(m_customColGrid);

	connect(m_customColGrid, &FlexGridLayout::reqestLayoutUpdate, this, &FaultsGroup::layoutUpdate);

	createFaultWidgets(path);

	m_customColGrid->toggleAll(true);
	m_customColGrid->itemSizeChanged();
}

FaultsGroup::~FaultsGroup() {}

const QVector<FaultWidget *> &FaultsGroup::getFaults() const { return m_faults; }

const QString &FaultsGroup::getName() const { return m_name; }

void FaultsGroup::setName(const QString &name_) { FaultsGroup::m_name = name_; }

void FaultsGroup::clearSelection()
{
	for(unsigned int i : m_currentlySelected) {
		m_faults[(int)(i)]->setPressed(false);
	}

	m_currentlySelected.clear();
	Q_EMIT selectionUpdated();
}

void FaultsGroup::update(uint32_t faultsNumeric)
{
	for(int i = 0; i < m_faults.size(); i++) {
		bool bit = (bool)((faultsNumeric >> i) & 0b1);
		if(m_faults.at(i)->isActive() && bit) { // if we get 2 active signals, we set the stored to 1
			m_faults.at(i)->setStored(true);
		}
		m_faults.at(i)->setActive(bit);

		if(bit) {
			m_actives.insert(i);
		} else {
			m_actives.erase(i);
		}
	}
}

QStringList FaultsGroup::getExplanations()
{
	QStringList res;
	for(auto fault : qAsConst(m_faults)) {
		res += getExplanation(fault->getId());
	}

	return res;
}

QString FaultsGroup::getExplanation(unsigned int id)
{
	QString res = "";
	auto fault = m_faults.at(id);
	if(fault) {
		res += QString("Bit%1 (%2): %3")
			       .arg(QString::number(fault->getId()), fault->getName(), fault->getFaultExplanation());
	}

	return res;
}

QWidget *FaultsGroup::buildActiveStoredWidget()
{
	auto widget = new QWidget(this);
	auto storedLabel = new QLabel(this);
	storedLabel->setText("Stored");
	storedLabel->setContentsMargins(0, 0, 0, 0);
	storedLabel->setAlignment(Qt::AlignLeft);
	StyleHelper::MenuMediumLabel(storedLabel);
	auto activeLabel = new QLabel(this);
	activeLabel->setText("Active");
	activeLabel->setContentsMargins(0, 0, 0, 0);
	activeLabel->setAlignment(Qt::AlignLeft);
	StyleHelper::MenuMediumLabel(activeLabel);
	auto spacer = new QSpacerItem(0, 30, QSizePolicy::Fixed, QSizePolicy::Fixed);

	widget->setLayout(new QVBoxLayout(widget));
	widget->layout()->addWidget(storedLabel);
	widget->layout()->addWidget(activeLabel);
	widget->layout()->addItem(spacer);
	widget->layout()->setContentsMargins(0, 9, 0, 9);

	return widget;
}

QJsonArray *FaultsGroup::getJsonArray(const QString &path)
{
	QString contents;
	QFile file;
	file.setFileName(path);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	if(!file.isOpen()) {
		qCritical(CAT_SWIOT_FAULTS) << "File could not be opened (read): " << path;
	} else {
		qDebug(CAT_SWIOT_FAULTS) << "File opened (read): " << path;
	}
	contents = file.readAll();
	file.close();

	QJsonParseError parseError{};
	QJsonDocument document = QJsonDocument::fromJson(contents.toUtf8(), &parseError);
	if(document.isNull()) {
		qCritical(CAT_SWIOT_FAULTS) << "Invalid json: " << parseError.errorString();
	}

	QJsonObject documentObject = document.object();
	QJsonValue deviceValue = documentObject.value(m_name);
	if(deviceValue == QJsonValue::Undefined) {
		qCritical(CAT_SWIOT_FAULTS) << "Invalid json: Could not extract value " << m_name;
	}

	QJsonObject deviceObject = deviceValue.toObject();
	QJsonValue maxFaultsValue = deviceObject["max_faults"]; // max number of faults
	m_maxFaults = maxFaultsValue.toInt(-1);

	QJsonValue faultsJson = deviceObject["faults"];

	auto *array = new QJsonArray();
	*array = faultsJson.toArray();

	return array;
}

std::set<unsigned int> FaultsGroup::getSelectedIndexes() { return m_currentlySelected; }

std::set<unsigned int> FaultsGroup::getActiveIndexes() { return m_actives; }

void FaultsGroup::layoutUpdate()
{
	if(m_activeStoredWidget->layout()->count() != m_customColGrid->rows()) {
		while(m_activeStoredWidget->layout()->count() < m_customColGrid->rows()) {
			m_activeStoredWidget->layout()->addWidget(buildActiveStoredWidget());
		}
		while(m_activeStoredWidget->layout()->count() > m_customColGrid->rows()) {
			QWidget *widgetToDelete = m_activeStoredWidget->layout()
							  ->itemAt(m_activeStoredWidget->layout()->count() - 1)
							  ->widget();
			m_activeStoredWidget->layout()->removeWidget(widgetToDelete);
			delete widgetToDelete;
		}
	}
	m_activeStoredWidget->setMaximumHeight(m_customColGrid->sizeHint().height());
	m_activeStoredWidget->updateGeometry();
	setMaximumHeight(sizeHint().height() + 1);

	Q_EMIT minimumSizeChanged();
}

void FaultsGroup::onFaultSelected(unsigned int id)
{
	bool added = m_currentlySelected.insert(id).second;
	if(!added) {
		m_currentlySelected.erase(id);
		m_faults.at((int)(id))->setPressed(false);
	}
	Q_EMIT selectionUpdated();
}

void FaultsGroup::createFaultWidgets(const QString &path)
{
	QJsonArray *faultsObj = getJsonArray(path);
	m_maxFaults = faultsObj->size();

	for(int i = 0; i < m_maxFaults; ++i) {
		QJsonObject faultObject = faultsObj->at(i).toObject();
		QString faultName = faultObject.value("name").toString();
		QString faultDescription = faultObject.value("description").toString();
		auto faultWidget = new FaultWidget(i, faultName, faultDescription, this);

		if(faultObject.contains("condition")) {
			QJsonObject condition = faultObject["condition"].toObject();
			faultWidget->setFaultExplanationOptions(condition);
			connect(this, &FaultsGroup::specialFaultsUpdated, faultWidget,
				&FaultWidget::specialFaultUpdated);
			connect(faultWidget, &FaultWidget::specialFaultExplanationChanged, this,
				&FaultsGroup::specialFaultExplanationChanged);
		}
		connect(faultWidget, &FaultWidget::faultSelected, this, &FaultsGroup::onFaultSelected);

		m_faults.push_back(faultWidget);
		m_customColGrid->addQWidgetToList(faultWidget);
	}
	delete faultsObj;
}
