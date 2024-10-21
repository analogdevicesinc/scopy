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

#include <QVBoxLayout>
#include <style.h>

#include <stylehelper.h>
#include <widgets/cursorsettings.h>

using namespace scopy;

CursorSettings::CursorSettings(QWidget *parent)
	: QWidget(parent)
{
	initUI();
	connectSignals();
	initSession();
}

CursorSettings::~CursorSettings() {}

void CursorSettings::initUI()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	StyleHelper::TransparentWidget(this, "cursorSettings");
	setFixedWidth(200);
	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);

	// x controls
	xControls = new MenuSectionWidget(this);
	Style::setStyle(xControls, style::properties::widget::border);
	layout->addWidget(xControls);
	xControls->contentLayout()->setSpacing(10);

	xEn = new MenuOnOffSwitch(tr("Enable X"), xControls, false);
	xEn->onOffswitch()->setChecked(true);
	xControls->contentLayout()->addWidget(xEn);
	xLock = new MenuOnOffSwitch(tr("Lock"), xControls, false);
	xControls->contentLayout()->addWidget(xLock);
	xTrack = new MenuOnOffSwitch(tr("Track"), xControls, false);
	xControls->contentLayout()->addWidget(xTrack);

	// y controls
	yControls = new MenuSectionWidget(this);
	Style::setStyle(yControls, style::properties::widget::border);
	layout->addWidget(yControls);
	yControls->contentLayout()->setSpacing(10);

	yEn = new MenuOnOffSwitch(tr("Enable Y"), yControls, false);
	yEn->onOffswitch()->setChecked(true);
	yControls->contentLayout()->addWidget(yEn);
	yLock = new MenuOnOffSwitch(tr("Lock"), yControls, false);
	yControls->contentLayout()->addWidget(yLock);

	// readouts controls
	readoutsControls = new MenuSectionWidget(this);
	Style::setStyle(readoutsControls, style::properties::widget::border);
	layout->addWidget(readoutsControls);

	readoutsDrag = new MenuOnOffSwitch(tr("Move readouts"), readoutsControls, false);
	readoutsControls->contentLayout()->addWidget(readoutsDrag);
}

void CursorSettings::connectSignals()
{
	connect(xEn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool toggled) {
		xTrack->setEnabled(toggled);
		xLock->setEnabled(toggled);
	});
	connect(yEn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool toggled) {
		yLock->setEnabled(toggled);
		if(!toggled)
			xTrack->onOffswitch()->setChecked(false);
	});
	connect(xTrack->onOffswitch(), &QAbstractButton::toggled, this, [=](bool toggled) {
		if(toggled && !yEn->onOffswitch()->isChecked())
			yEn->onOffswitch()->setChecked(true);

		yLock->setEnabled(yEn->onOffswitch()->isChecked() && !toggled);
		yLock->onOffswitch()->setChecked(false);
	});
}

QAbstractButton *CursorSettings::getXEn() { return xEn->onOffswitch(); }

QAbstractButton *CursorSettings::getXLock() { return xLock->onOffswitch(); }

QAbstractButton *CursorSettings::getXTrack() { return xTrack->onOffswitch(); }

QAbstractButton *CursorSettings::getYEn() { return yEn->onOffswitch(); }

QAbstractButton *CursorSettings::getYLock() { return yLock->onOffswitch(); }

QAbstractButton *CursorSettings::getReadoutsDrag() { return readoutsDrag->onOffswitch(); }

void CursorSettings::initSession()
{
	getXEn()->setChecked(true);
	getXLock()->setChecked(false);
	getXTrack()->setChecked(false);
	getYEn()->setChecked(false);
	getYLock()->setChecked(false);
	getReadoutsDrag()->setChecked(false);
}

void CursorSettings::updateSession()
{
	Q_EMIT getXEn()->toggled(getXEn()->isChecked());
	Q_EMIT getXLock()->toggled(getXLock()->isChecked());
	Q_EMIT getXTrack()->toggled(getXTrack()->isChecked());
	Q_EMIT getYEn()->toggled(getYEn()->isChecked());
	Q_EMIT getYLock()->toggled(getYLock()->isChecked());
	Q_EMIT getReadoutsDrag()->toggled(getReadoutsDrag()->isChecked());
	Q_EMIT sessionUpdated();
}

#include "moc_cursorsettings.cpp"
