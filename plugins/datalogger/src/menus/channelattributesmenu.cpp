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

#include "menus/channelattributesmenu.hpp"

#include <iiowidgetbuilder.h>
#include <menuheader.h>
#include <menuspinbox.h>
#include <menuspinbox.h>
#include <float.h>

using namespace scopy;
using namespace datamonitor;
using namespace gui;

ChannelAttributesMenu::ChannelAttributesMenu(DataMonitorModel *model, QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	MenuHeaderWidget *header = new MenuHeaderWidget(model->getName(), model->getColor(), this);
	mainLayout->addWidget(header);

	QWidget *settingsBody = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	///////////////////// channel attributes ///////////////////////////////////////////

	MenuSectionWidget *attrcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr = new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE,
							    MenuCollapseSection::MHW_BASEWIDGET, attrcontainer);

	QVBoxLayout *attrLayout = new QVBoxLayout();
	attrLayout->setSpacing(10);
	attrLayout->setMargin(0);
	attrLayout->setContentsMargins(0, 0, 0, 10); // bottom margin

	if(qobject_cast<DmmDataMonitorModel *>(model)) {
		QList<IIOWidget *> attrWidgets =
			IIOWidgetBuilder(attr)
				.channel(dynamic_cast<DmmDataMonitorModel *>(model)->iioChannel())
				.buildAll();

		for(auto w : attrWidgets) {
			attrLayout->addWidget(w);
		}

	} else {
		attrLayout->addWidget(new QLabel("NO ATTRIBUTE FOUND !"));
	}

	////////////////////// scalling settings ////////////////////////////////////////////////

	MenuSectionWidget *scalingcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *scaling = new MenuCollapseSection("Scaling", MenuCollapseSection::MHCW_NONE,
							       MenuCollapseSection::MHW_BASEWIDGET, scalingcontainer);

	QVBoxLayout *scalingLayout = new QVBoxLayout();
	scalingLayout->setSpacing(10);
	scalingLayout->setMargin(0);
	scalingLayout->setContentsMargins(0, 0, 0, 10); // bottom margin

	MenuCombo *scaleModeCBb = new MenuCombo("Scale Mode", scaling);
	auto scaleCb = scaleModeCBb->combo();

	scaleCb->addItem("Raw", SCALE_RAW);

	if(model->hasScale()) {
		scaleCb->addItem("Sacle", SCALE_SCALED);
	}

	scaleCb->addItem("Scale override", SCALE_OVERRIDE);

	MenuSpinbox *scaleOverride =
		new MenuSpinbox("Override scale ", 1, "counts", -DBL_MAX, DBL_MAX, true, false, scaling);
	scaleOverride->setIncrementMode(MenuSpinbox::IS_FIXED);
	scaleOverride->setScalingEnabled(false);
	scaleOverride->setPrecision(12);
	scaleOverride->setVisible(false);

	connect(scaleCb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		if(scaleCb->itemData(idx) == SCALE_RAW) {

			scaleOverride->setVisible(false);
			// non scaled value is 1
			model->setScale(1);

		} else if(scaleCb->itemData(idx) == SCALE_SCALED) {

			scaleOverride->setVisible(true);
			scaleOverride->setEnabled(false);
			model->setScale(model->defaultScale());
			scaleOverride->blockSignals(true);
			scaleOverride->setValue(model->defaultScale());
			scaleOverride->blockSignals(false);

		} else if(scaleCb->itemData(idx) == SCALE_OVERRIDE) {

			scaleOverride->setVisible(true);
			scaleOverride->setEnabled(true);
			model->setScale(model->defaultScale());
			scaleOverride->blockSignals(true);
			scaleOverride->setValue(model->defaultScale());
			scaleOverride->blockSignals(false);
		}
	});

	if(model->hasScale()) {
		scaleCb->setCurrentIndex(1);
	}

	scalingLayout->addWidget(scaleModeCBb);
	scalingLayout->addWidget(scaleOverride);

	connect(scaleOverride, &MenuSpinbox::valueChanged, this, [=](double value) {
		if(qobject_cast<DmmDataMonitorModel *>(model)) {
			DmmDataMonitorModel *dmmModel = dynamic_cast<DmmDataMonitorModel *>(model);
			if(dmmModel) {
				dmmModel->setScale(value);
			}
		}
	});

	////////////////////// offset settings ////////////////////////////////////////////////

	MenuCombo *offsetModeCBb = new MenuCombo("Offset Mode", scaling);
	auto offsetCb = offsetModeCBb->combo();

	offsetCb->addItem("Raw", OFFSET_RAW);

	if(model->hasOffset()) {
		offsetCb->addItem("Offset", OFFSET_OFFSETED);
	}

	offsetCb->addItem("Offset override", OFFSET_OVERRIDE);

	MenuSpinbox *offsetOverride =
		new MenuSpinbox("Override offset", 1, "counts", -DBL_MAX, DBL_MAX, true, false, scaling);
	offsetOverride->setIncrementMode(MenuSpinbox::IS_FIXED);
	offsetOverride->setScalingEnabled(false);
	offsetOverride->setVisible(false);

	connect(offsetCb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		if(offsetCb->itemData(idx) == OFFSET_RAW) {

			offsetOverride->setVisible(false);
			// non offset value is 0
			model->setOffset(0);

		} else if(offsetCb->itemData(idx) == OFFSET_OFFSETED) {

			offsetOverride->setVisible(true);
			offsetOverride->setEnabled(false);
			model->setOffset(model->defaultOffset());
			offsetOverride->blockSignals(true);
			offsetOverride->setValue(model->defaultOffset());
			offsetOverride->blockSignals(false);

		} else if(offsetCb->itemData(idx) == OFFSET_OVERRIDE) {

			offsetOverride->setVisible(true);
			offsetOverride->setEnabled(true);
			model->setOffset(model->defaultOffset());
			offsetOverride->blockSignals(true);
			offsetOverride->setValue(model->defaultOffset());
			offsetOverride->blockSignals(false);
		}
	});

	if(model->hasOffset()) {
		offsetCb->setCurrentIndex(1);
	}

	scalingLayout->addWidget(offsetModeCBb);
	scalingLayout->addWidget(offsetOverride);

	connect(offsetOverride, &MenuSpinbox::valueChanged, this, [=](double value) {
		if(qobject_cast<DmmDataMonitorModel *>(model)) {
			DmmDataMonitorModel *dmmModel = dynamic_cast<DmmDataMonitorModel *>(model);
			if(dmmModel) {
				dmmModel->setOffset(value);
			}
		}
	});
	///////////////// add attributes to settings ///////////////////
	attr->contentLayout()->addLayout(attrLayout);
	attrcontainer->contentLayout()->addWidget(attr);

	layout->addWidget(attrcontainer);

	//////////////// add scallign attributes to settings ///////////////////////////////
	scaling->contentLayout()->addLayout(scalingLayout);
	scalingcontainer->contentLayout()->addWidget(scaling);
	layout->addWidget(scalingcontainer);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);
}

#include "moc_channelattributesmenu.cpp"
