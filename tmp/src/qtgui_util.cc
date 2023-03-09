/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/* -*- c++ -*- */
/*
 * Copyright 2008,2009 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "utils.h"
#include <sstream>
#include <string>
#include <algorithm>
#include <QDebug>
#include <QSizePolicy>

QwtPickerDblClickPointMachine::QwtPickerDblClickPointMachine()
#if QWT_VERSION < 0x060000
    : QwtPickerMachine ()
#else
    : QwtPickerMachine (PointSelection)
#endif
{
}

QwtPickerDblClickPointMachine::~QwtPickerDblClickPointMachine()
{
}

#if QWT_VERSION < 0x060000
#define CMDLIST_TYPE QwtPickerMachine::CommandList
#else
#define CMDLIST_TYPE QList<QwtPickerMachine::Command>
#endif
CMDLIST_TYPE
QwtPickerDblClickPointMachine::transition(const QwtEventPattern &eventPattern,
					  const QEvent *e)
{
  CMDLIST_TYPE cmdList;
  switch(e->type()) {
    case QEvent::MouseButtonDblClick:
      if ( eventPattern.mouseMatch(QwtEventPattern::MouseSelect1,
				   (const QMouseEvent *)e) ) {
	cmdList += QwtPickerMachine::Begin;
	cmdList += QwtPickerMachine::Append;
	cmdList += QwtPickerMachine::End;
      }
      break;
  default:
    break;
  }
  return cmdList;
}

#if QWT_VERSION < 0x060100
QwtDblClickPlotPicker::QwtDblClickPlotPicker(QwtPlotCanvas* canvas)
#else /* QWT_VERSION < 0x060100 */
QwtDblClickPlotPicker::QwtDblClickPlotPicker(QWidget* canvas)
#endif /* QWT_VERSION < 0x060100 */
  : QwtPlotPicker(canvas)
{
#if QWT_VERSION < 0x060000
  setSelectionFlags(QwtPicker::PointSelection);
#endif
}

QwtDblClickPlotPicker::~QwtDblClickPlotPicker()
{
}

QwtPickerMachine*
QwtDblClickPlotPicker::stateMachine(int n) const
{
  return new QwtPickerDblClickPointMachine;
}

void Util::retainWidgetSizeWhenHidden(QWidget *w, bool retain)
{
	QSizePolicy sp_retain = w->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(retain);
	w->setSizePolicy(sp_retain);
}

void Util::setWidgetNrOfChars(QWidget *w,
		int minNrOfChars, int maxNrOfChars)
{
	QFontMetrics labelm(w->font());

	auto label_min_width = labelm.width(QString(minNrOfChars,'X'));
	w->setMinimumWidth(label_min_width);

	if (maxNrOfChars!=0) {
		auto label_max_width = labelm.width(QString(maxNrOfChars,'X'));
		w->setMaximumWidth(label_max_width);
	}
}

QString Util::loadStylesheetFromFile(const QString &path)
{
	QFile file(path);
	file.open(QFile::ReadOnly);
	QString stylesheet = QString::fromLatin1(file.readAll());
	return stylesheet;
}

bool Util::compareNatural(const std::string& a, const std::string& b) {
	if (a == b) {
		return (a < b);
	} else if (a.empty()) {
		return true;
	} else if (b.empty()) {
		return false;
	} else if (std::isdigit(a[0]) && !std::isdigit(b[0])) {
		return true;
	} else if (!std::isdigit(a[0]) && std::isdigit(b[0])) {
		return false;
	} else if (!std::isdigit(a[0]) && !std::isdigit(b[0])) {
		if (a[0] == b[0]) {
			return compareNatural(a.substr(1), b.substr(1));
		}
		return (a < b);
	}

	std::istringstream string_stream_a(a);
	std::istringstream string_stream_b(b);

	int int_a, int_b;
	std::string a_new, b_new;

	string_stream_a >> int_a;
	string_stream_b >> int_b;
	if (int_a != int_b) {
		return (int_a < int_b);
	}

	std::getline(string_stream_a, a_new);
	std::getline(string_stream_b, b_new);
	return (compareNatural(a_new, b_new));
}

QDockWidget *DockerUtils::createDockWidget(QMainWindow *mainWindow, QWidget *widget, const QString &title)
{
	QDockWidget* dockWidget = new QDockWidget(title, mainWindow);
	dockWidget->setFeatures(dockWidget->features() & ~QDockWidget::DockWidgetClosable);
	dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
	dockWidget->setWidget(widget);

#ifdef __ANDROID__
	dockWidget->setFeatures(dockWidget->features() & ~QDockWidget::DockWidgetClosable
				& ~QDockWidget::DockWidgetFloatable);
#endif

	return dockWidget;
}

void DockerUtils::configureTopBar(QDockWidget *docker)
{
	connect(docker, &QDockWidget::topLevelChanged, [=](bool topLevel){
		QString icon_path = "";

		if (QIcon::themeName() == "scopy-default") {
			icon_path +=":/icons/scopy-default/icons";
		} else {
			icon_path +=":/icons/scopy-light/icons";
		}

		if(topLevel) {
			docker->setWindowFlags(Qt::CustomizeWindowHint |
							Qt::Window |
							Qt::WindowMinimizeButtonHint |
							Qt::WindowMaximizeButtonHint);
			docker->show();

			docker->setStyleSheet("QDockWidget {"
						"titlebar-normal-icon: url(" + icon_path + "/sba_cmb_box_arrow.svg);"
						"}");
			docker->setContentsMargins(10, 0, 10, 10);
		} else {
			docker->setStyleSheet("QDockWidget {"
						  "titlebar-normal-icon: url(" + icon_path + "/sba_cmb_box_arrow_right.svg);"
						  "}");
			docker->setContentsMargins(0, 0, 0, 0);
		}
	});
}
