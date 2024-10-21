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

#include "detachedtoolwindow.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLoggingCategory>

#include <pluginbase/preferences.h>

Q_LOGGING_CATEGORY(CAT_DETACHEDTOOL, "DetachedToolWindow")

using namespace scopy;
DetachedToolWindow::DetachedToolWindow(QWidget *parent, ToolMenuEntry *tme)
	: QWidget(parent)
{

	this->tme = tme;

	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setSpacing(0);
	lay->setMargin(0);
	w = tme->tool();
	setWindowIcon(QApplication::windowIcon());
	setWindowTitle("Scopy - " + tme->pluginName() + " - " + tme->name() + " - " + tme->param());

	lay->addWidget(w);
	loadToolGeometry(tme, this);
	tme->tool()->show();
	show();
}

DetachedToolWindow::~DetachedToolWindow()
{
	saveToolGeometry(tme, this);
	w->setParent(nullptr);
}

void DetachedToolWindow::closeEvent(QCloseEvent *event)
{
	saveToolGeometry(tme, w);
	tme->setAttached(true);
}

void DetachedToolWindow::saveToolGeometry(ToolMenuEntry *tme, QWidget *w)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_save_attached").toBool())
		return;
	QString prefId;
	if(w) {
		prefId = tme->id() + "_geometry";
		QRect geometry = w->geometry();
		p->set(prefId, geometry);
		qDebug(CAT_DETACHEDTOOL) << "Saving " << prefId << " " << geometry;
	}
}

void DetachedToolWindow::loadToolGeometry(ToolMenuEntry *tme, QWidget *w)
{
	Preferences *p = Preferences::GetInstance();
	if(!p->get("general_save_attached").toBool())
		return;
	QString prefId;

	if(w && !tme->attached()) {
		prefId = tme->id() + "_geometry";
		QRect geometry = p->get(prefId).toRect();
		if(!geometry.isNull())
			w->setGeometry(geometry);

		qDebug(CAT_DETACHEDTOOL) << "Loading " << prefId << tme->tool()->geometry();
	}
}

#include "moc_detachedtoolwindow.cpp"
