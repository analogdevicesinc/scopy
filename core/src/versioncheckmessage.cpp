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

#include "versioncheckmessage.h"

#include <pluginbase/preferences.h>
#include <QHBoxLayout>

using namespace scopy;
VersionCheckMessage::VersionCheckMessage(QWidget *parent)
	: QWidget(parent)
{
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	auto textLabel =
		new QLabel("<p>Should Scopy check for online versions?&nbsp;&nbsp;&nbsp;&nbsp;<a style='color: white; "
			   "font-weight: bold;' href='yes'>Yes</a>&nbsp;&nbsp;&nbsp;&nbsp;<a style='color: white; "
			   "font-weight: bold;' href='no'>No</a></p>",
			   this);
	connect(textLabel, &QLabel::linkActivated, this, [this](const QString &text) {
		if(text == "yes") {
			setCheckVersion(true);
		} else if(text == "no") {
			setCheckVersion(false);
		}

		delete this;
	});
	textLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	layout()->addWidget(textLabel);

	connect(this, &VersionCheckMessage::setCheckVersion, this, &VersionCheckMessage::saveCheckVersion);
}

VersionCheckMessage::~VersionCheckMessage() {}

void VersionCheckMessage::saveCheckVersion(bool allowed) { Preferences::set("general_check_online_version", allowed); }

#include "moc_versioncheckmessage.cpp"
