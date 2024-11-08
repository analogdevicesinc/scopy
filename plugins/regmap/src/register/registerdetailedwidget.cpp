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
 *
 */

#include "registerdetailedwidget.hpp"

#include "../verticalscrollarea.hpp"
#include "bitfield/bitfielddetailedwidget.hpp"
#include "bitfield/bitfielddetailedwidgetfactory.hpp"
#include "qdebug.h"
#include "registermodel.hpp"
#include "utils.hpp"

#include <QtMath>
#include <qboxlayout.h>
#include <qlabel.h>
#include <regmapstylehelper.hpp>
#include "registermodel.hpp"
#include "qdebug.h"
#include "utils.hpp"
#include <QtMath>
#include <menucollapsesection.h>
#include <style.h>

using namespace scopy;
using namespace regmap;

RegisterDetailedWidget::RegisterDetailedWidget(RegisterModel *regModel, QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	Utils::removeLayoutMargins(layout);
	setLayout(layout);
	layout->setSpacing(5);

	regWidth = regModel->getWidth();

	QWidget *nameDescriptionWidget = new QWidget(this);
	Style::setStyle(nameDescriptionWidget, style::properties::widget::basicComponent, true, true);
	QHBoxLayout *nameDescriptionLayout = new QHBoxLayout(nameDescriptionWidget);
	nameDescriptionWidget->setLayout(nameDescriptionLayout);
	QLabel *nameLabel = new QLabel("Name: " + regModel->getName(), this);
	QLabel *descriptionLabel = new QLabel("Description: " + regModel->getDescription(), this);
	nameDescriptionLayout->addWidget(nameLabel);
	nameDescriptionLayout->addWidget(descriptionLabel);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

	QGridLayout *bitFieldsWidgetLayout = new QGridLayout();
	bitFieldsWidgetLayout->setMargin(0);
	bitFieldsWidgetLayout->setSpacing(4);

	bitFieldList = new QVector<BitFieldDetailedWidget *>();
	BitFieldDetailedWidgetFactory bitFieldDetailedWidgetFactory;
	QWidget *bitFieldsWidget = new QWidget();
	bitFieldsWidget->setLayout(bitFieldsWidgetLayout);
	VerticalScrollArea *scrollArea = new VerticalScrollArea();
	scrollArea->setWidget(bitFieldsWidget);
	layout->addWidget(scrollArea);

	bitFieldsWidgetLayout->addWidget(nameDescriptionWidget, 0, 0, 1, 8);
	int currentBitFieldCount = 0;
	int row = 1;
	int col = 0;
	for(int i = regModel->getBitFields()->size() - 1; i >= 0; --i) {
		BitFieldDetailedWidget *bitFieldDetailedWidget =
			bitFieldDetailedWidgetFactory.buildWidget(regModel->getBitFields()->at(i));
		bitFieldList->push_back(bitFieldDetailedWidget);
		bitFieldsWidgetLayout->addWidget(bitFieldDetailedWidget, row, col);
		bitFieldsWidgetLayout->setColumnStretch(col, 1);
		col++;
		currentBitFieldCount += bitFieldDetailedWidget->getWidth();

		if(col > (Utils::getBitsPerRowDetailed() - 1)) {
			row++;
			col = 0;
		}
		QObject::connect(bitFieldDetailedWidget, &BitFieldDetailedWidget::valueUpdated, this,
				 [=]() { Q_EMIT bitFieldValueChanged(getBitFieldsValue()); });
	}
	// add spacers to keep the shape of the detailed bitfileds when the number of bitfields didn't fill a row
	if(row == 1) {
		while(col <= (Utils::getBitsPerRowDetailed() - 1)) {
			BitFieldDetailedWidget *aux = new BitFieldDetailedWidget("", "", 0, "", 1, "", 0, nullptr);
			aux->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			aux->sizePolicy().setRetainSizeWhenHidden(true);
			aux->hide();
			bitFieldsWidgetLayout->addWidget(aux, row, col);
			bitFieldsWidgetLayout->setColumnStretch(col, 1);
			col++;
		}
		bitFieldsWidgetLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding),
					       row + 1, col);
	}
}

void RegisterDetailedWidget::updateBitFieldsValue(uint32_t value)
{
	int regOffset = 0;
	for(int i = bitFieldList->length() - 1; i >= 0; --i) {
		bitFieldList->at(i)->blockSignals(true);

		int width = bitFieldList->at(i)->getWidth();
		uint32_t bfVal = 0;
		if(width == regWidth) {
			bfVal = value;
		} else {
			bfVal = (((1 << (regOffset + width)) - 1) & value) >> regOffset;
		}
		QString bitFieldValue = Utils::convertToHexa(bfVal, width);
		bitFieldList->at(i)->registerValueUpdated(bitFieldValue);
		regOffset += width;

		bitFieldList->at(i)->blockSignals(false);
	}
}

QString RegisterDetailedWidget::getBitFieldsValue()
{
	uint32_t result = 0;
	for(int i = 0; i < bitFieldList->length(); ++i) {
		uint32_t aux = Utils::convertQStringToUint32(bitFieldList->at(i)->getValue());
		int regOffset = bitFieldList->at(i)->getRegOffset();
		result += qPow(2, regOffset) * aux;
		qDebug() << "aux = " << aux << " result = " << result;
	}
	return Utils::convertToHexa(result, regWidth);
	;
}
