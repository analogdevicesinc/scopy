/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "auxdacwidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

AuxDacWidget::AuxDacWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: m_device(device)
	, m_group(group)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	Style::setStyle(widget, style::properties::widget::border_interactive);
	m_layout->addWidget(widget);

	QList<IIOWidget *> allWidgets;

	auto addRange = [&](const char *attr, const QString &title) {
		IIOWidget *w = makeDebugRange(widget, m_device, attr, title, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};
	auto addCheck = [&](const char *attr, const QString &title) {
		IIOWidget *w = makeDebugCheck(widget, m_device, attr, title, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};

	QLabel *title = new QLabel("AUX DAC", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	for(int i = 0; i < 10; i++) {
		QLabel *dacLabel = new QLabel(QString("DAC %1").arg(i), widget);
		Style::setStyle(dacLabel, style::properties::label::menuBig);
		layout->addWidget(dacLabel);

		QString enableAttr = QString("adi,aux-dac-enable%1").arg(i);
		addCheck(enableAttr.toUtf8().constData(), QString("DAC %1 Enable").arg(i));

		QString valueAttr = QString("adi,aux-dac-value%1").arg(i);
		addRange(valueAttr.toUtf8().constData(), QString("DAC %1 Value").arg(i));

		QString slopeAttr = QString("adi,aux-dac-slope%1").arg(i);
		addRange(slopeAttr.toUtf8().constData(), QString("DAC %1 Slope").arg(i));

		QString vrefAttr = QString("adi,aux-dac-vref%1").arg(i);
		addRange(vrefAttr.toUtf8().constData(), QString("DAC %1 Vref").arg(i));
	}

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &AuxDacWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

AuxDacWidget::~AuxDacWidget() {}
