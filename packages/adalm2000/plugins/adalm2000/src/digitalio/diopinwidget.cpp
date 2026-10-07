/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "diopinwidget.h"

#include "m2kdigitaliocontroller.h"

#include "component/attribute.h"

#include <QFrame>
#include <QLabel>
#include <QLoggingCategory>
#include <QVBoxLayout>

#include <gui/style.h>
#include <gui/customSwitch.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/guistrategy/switchguistrategy.h>

Q_LOGGING_CATEGORY(CAT_DIOPINWIDGET, "DioPinWidget")

using namespace scopy;
using namespace scopy::adalm2000;

namespace {

QString ledStyle(bool high, bool shorted)
{
	const QColor border =
		shorted ? Style::getColor(json::global::led_error) : Style::getColor(json::theme::content_subtle);
	QString fill = QStringLiteral("transparent");
	if(shorted) {
		fill = Style::getColor(json::global::led_error).name();
	} else if(high) {
		fill = Style::getColor(json::global::led_success).name();
	}
	return QStringLiteral("QFrame { border: 1px solid %1; border-radius: 4px; background-color: %2; }")
		.arg(border.name(), fill);
}

} // namespace

DioPinWidget::DioPinWidget(M2kDigitalIoController *controller, IIOWidgetGroup *group, int pin, QWidget *parent)
	: QWidget(parent)
	, m_pin(pin)
	, m_group(group)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(Style::getDimension(json::global::unit_0_5));
	layout->setAlignment(Qt::AlignTop);

	m_label = new QLabel(QString::number(pin), this);
	m_label->setAlignment(Qt::AlignCenter);
	Style::setStyle(m_label, style::properties::label::menuSmall);
	layout->addWidget(m_label, 0, Qt::AlignHCenter);

	m_led = new QFrame(this);
	m_led->setFixedHeight(Style::getDimension(json::global::unit_0_5) + 2);
	m_led->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_led->setStyleSheet(ledStyle(false, false));
	layout->addWidget(m_led);

	if(!controller) {
		qWarning(CAT_DIOPINWIDGET) << "no controller for pin" << pin;
		return;
	}

	m_directionWidget = buildToggle(controller->pinDirection(pin), /*isDirection=*/true, this);
	if(m_directionWidget) {
		layout->addWidget(m_directionWidget);
	}

	m_valueWidget = buildToggle(controller->pinValue(pin), /*isDirection=*/false, this);
	if(m_valueWidget) {
		layout->addWidget(m_valueWidget);
	}
}

DioPinWidget::~DioPinWidget() {}

IIOWidget *DioPinWidget::buildToggle(component::Attribute *attr, bool isDirection, QWidget *parent)
{
	if(!attr) {
		return nullptr;
	}

	// SwitchUi labels the switch from the attribute's own option list in order, and
	// emits that same vocabulary, so there is no value translation to write.
	IIOWidget *w = IIOWidgetBuilder(parent)
			       .attribute(attr)
			       .uiStrategy(IIOWidgetBuilder::SwitchUi)
			       .infoMessage(isDirection ? QStringLiteral("Pin direction: out drives the pin")
							: QStringLiteral("Output level while the pin is an output"))
			       .compactMode(true)
			       .group(m_group)
			       .buildSingle();
	if(!w) {
		return nullptr;
	}

	w->showProgressBar(false);
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	// Self-check: a combo speaks the same vocabulary as a switch, so SwitchUi
	// silently resolving to ComboUi would pass every functional test.
	if(!w->findChild<CustomSwitch *>()) {
		qWarning(CAT_DIOPINWIDGET) << "no CustomSwitch inside the IIOWidget for pin" << m_pin
					   << "-- did UIS::SwitchUi stop resolving to SwitchAttrUi?";
	}

	stretchToggle(w);
	// CustomSwitch::update() re-pins setMaximumSize(sizeHint()) on every
	// receiveData, so the width has to be freed again after each redisplay.
	if(auto *uis = dynamic_cast<SwitchAttrUi *>(w->getUiStrategy())) {
		connect(uis, &SwitchAttrUi::displayedNewData, this, [this, w](QString, QString) { stretchToggle(w); });
	}
	return w;
}

void DioPinWidget::stretchToggle(IIOWidget *w)
{
	CustomSwitch *sw = w ? w->findChild<CustomSwitch *>() : nullptr;
	if(!sw) {
		return;
	}
	sw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	// Width only: the height stays capped by update() so the switch cannot grow
	// taller and distort the column.
	sw->setMaximumWidth(QWIDGETSIZE_MAX);
}

void DioPinWidget::setInputState(bool high, bool shorted)
{
	if(!m_led) {
		return;
	}
	m_led->setStyleSheet(ledStyle(high, shorted));
	m_led->setToolTip(shorted ? QStringLiteral("Pin %1 reads back %2 while driving %3")
					    .arg(m_pin)
					    .arg(high ? 1 : 0)
					    .arg(high ? 0 : 1)
				  : QStringLiteral("Pin %1 reads %2").arg(m_pin).arg(high ? 1 : 0));
}

#include "moc_diopinwidget.cpp"
