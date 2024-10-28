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

#include "infopage.h"

#include "QFile"
#include "qjsondocument.h"
#include "qjsonobject.h"

using namespace scopy;

InfoPage::InfoPage(QWidget *parent)
	: QWidget(parent)
	, m_advancedMode(false)
{
	getKeyMap();
	lay = new QVBoxLayout(this);
	lay->setSpacing(3);
}

InfoPage::~InfoPage()
{
	if(!map.empty()) {
		map.clear();
	}
	if(!wmap.empty()) {
		qDeleteAll(wmap);
		wmap.clear();
	}
	if(!keyMap.empty()) {
		keyMap.clear();
	}
}

void InfoPage::update(QString key, QString value)
{
	if(!m_advancedMode) {
		if(keyMap.contains(key)) {
			key = keyMap[key];
		} else {
			return;
		}
	}
	if(!map.contains(key)) {
		map.insert(key, value);
		InfoPageKeyValueWidget *w = new InfoPageKeyValueWidget(key, value, this);
		w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		wmap.insert(key, w);
		lay->addWidget(w);

	} else {
		map[key] = value;
		wmap[key]->updateValue(value);
	}
}

void InfoPage::clear()
{
	if(!map.empty()) {
		map.clear();
	}
	if(!wmap.empty()) {
		qDeleteAll(wmap);
		wmap.clear();
	}
}

void InfoPage::getKeyMap()
{
	QString val;
	QFile file;
	file.setFileName(":/gui/infoPageMap.json");
	bool fileIsOpen = file.open(QIODevice::ReadOnly | QIODevice::Text);
	val = file.readAll();
	file.close();

	if(fileIsOpen) {
		QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());
		QJsonObject json = d.object();
		foreach(const QString &key, json.keys()) {
			QJsonValue value = json.value(key);
			keyMap[key] = value["value"].toString();
		}
	} else {
		qWarning() << "infoPageMap.json couldn't be found!";
	}
}

void InfoPage::setAdvancedMode(bool newAdvancedMode) { m_advancedMode = newAdvancedMode; }

#include "moc_infopage.cpp"
