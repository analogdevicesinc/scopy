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

#include "filedataguistrategy.h"

#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <titlespinbox.h>
#include <menuonoffswitch.h>
#include <stylehelper.h>

#include "dac_logging_categories.h"

using namespace scopy;
using namespace scopy::dac;
FileDataGuiStrategy::FileDataGuiStrategy(QWidget *parent)
	: QObject(parent)
{
	m_ui = new QWidget(parent);

	m_ui->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto lay = new QHBoxLayout(m_ui);
	lay->setMargin(0);
	lay->setSpacing(0);
	m_ui->setLayout(lay);

	MenuSectionWidget *guiContainer = new MenuSectionWidget(m_ui);
	MenuCollapseSection *guiSection = new MenuCollapseSection("DATA CONFIGURATION", MenuCollapseSection::MHCW_ARROW,
								  MenuCollapseSection::MHW_BASEWIDGET, guiContainer);
	guiSection->contentLayout()->setSpacing(10);

	m_scaled = new MenuOnOffSwitch("Scaled", guiSection);
	m_scaleSpin = new TitleSpinBox("Scale (DBFS)", true, guiSection);
	m_scaleSpin->setMax(0.0);
	m_scaleSpin->setMin(-91.0);
	m_scaleSpin->setStep(1.0); // when value invalid display -Inf db
	StyleHelper::BackgroundWidget(m_scaleSpin);

	connect(m_scaled->onOffswitch(), &QAbstractButton::toggled, this, [this](bool b) {
		m_recipe.scaled = b;
		Q_EMIT recipeUpdated(m_recipe);
	});
	connect(m_scaleSpin->getLineEdit(), &QLineEdit::textChanged, this, [this](QString text) {
		bool ok = true;
		double val = text.toDouble(&ok);
		if(ok) {
			m_recipe.scale = val;
			Q_EMIT recipeUpdated(m_recipe);
		}
	});

	guiSection->contentLayout()->addWidget(m_scaleSpin);
	guiSection->contentLayout()->addWidget(m_scaled);
	guiContainer->contentLayout()->addWidget(guiSection);
	lay->addWidget(guiContainer);
}

QWidget *FileDataGuiStrategy::ui() { return m_ui; }

void FileDataGuiStrategy::init()
{
	m_scaleSpin->setValue(0.0);
	m_scaled->onOffswitch()->setChecked(true);
}
