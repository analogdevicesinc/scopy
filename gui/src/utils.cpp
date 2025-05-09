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

#include "utils.h"

#include "QtSvg/qsvgrenderer.h"
#include "qpainter.h"

#include <QDebug>
#include <QFile>
#include <QSizePolicy>

#include <algorithm>
#include <sstream>
#include <string>

void Util::retainWidgetSizeWhenHidden(QWidget *w, bool retain)
{
	QSizePolicy sp_retain = w->sizePolicy();
	sp_retain.setRetainSizeWhenHidden(retain);
	w->setSizePolicy(sp_retain);
}

void Util::setWidgetNrOfChars(QWidget *w, int minNrOfChars, int maxNrOfChars)
{
	QFontMetrics labelm(w->font());

	auto label_min_width = labelm.horizontalAdvance(QString(minNrOfChars, 'X'));
	w->setMinimumWidth(label_min_width);

	if(maxNrOfChars != 0) {
		auto label_max_width = labelm.horizontalAdvance(QString(maxNrOfChars, 'X'));
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

bool Util::compareNatural(const std::string &a, const std::string &b)
{
	if(a == b) {
		return (a < b);
	} else if(a.empty()) {
		return true;
	} else if(b.empty()) {
		return false;
	} else if(std::isdigit(a[0]) && !std::isdigit(b[0])) {
		return true;
	} else if(!std::isdigit(a[0]) && std::isdigit(b[0])) {
		return false;
	} else if(!std::isdigit(a[0]) && !std::isdigit(b[0])) {
		if(a[0] == b[0]) {
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
	if(int_a != int_b) {
		return (int_a < int_b);
	}

	std::getline(string_stream_a, a_new);
	std::getline(string_stream_b, b_new);
	return (compareNatural(a_new, b_new));
}

QString Util::doubleToQString(double value)
{
	QString ret = QString::number(value, 'f', 7); // magic number
	int i = ret.size() - 1;
	int toChop = 0;
	while(ret[i] == '0') {
		++toChop;
		--i;
	}

	if(ret[i] == '.') {
		++toChop;
	}

	ret.chop(toChop);
	return ret;
}

QWidget *Util::findContainingWindow(QWidget *w)
{
	while(dynamic_cast<QWidget *>(w->parent()) != nullptr)
		w = dynamic_cast<QWidget *>(w->parent());
	return w;
}

QDockWidget *DockerUtils::createDockWidget(QMainWindow *mainWindow, QWidget *widget, const QString &title)
{
	QDockWidget *dockWidget = new QDockWidget(title, mainWindow);
	dockWidget->setFeatures(dockWidget->features() & ~QDockWidget::DockWidgetClosable);
	dockWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
	dockWidget->setWidget(widget);

#ifdef __ANDROID__
	dockWidget->setFeatures(dockWidget->features() & ~QDockWidget::DockWidgetClosable &
				~QDockWidget::DockWidgetFloatable);
#endif

	return dockWidget;
}

void DockerUtils::configureTopBar(QDockWidget *docker)
{
	connect(docker, &QDockWidget::topLevelChanged, [=](bool topLevel) {
		QString icon_path = "";

		if(QIcon::themeName() == "scopy-default") {
			icon_path += ":/gui/icons/scopy-default/icons";
		} else {
			icon_path += ":/gui/icons/scopy-light/icons";
		}

		if(topLevel) {
			docker->setWindowFlags(Qt::CustomizeWindowHint | Qt::Window | Qt::WindowMinimizeButtonHint |
					       Qt::WindowMaximizeButtonHint);
			docker->show();

			docker->setStyleSheet("QDockWidget {"
					      "titlebar-normal-icon: url(" +
					      icon_path +
					      "/sba_cmb_box_arrow.svg);"
					      "}");
			docker->setContentsMargins(10, 0, 10, 10);
		} else {
			docker->setStyleSheet("QDockWidget {"
					      "titlebar-normal-icon: url(" +
					      icon_path +
					      "/sba_cmb_box_arrow_right.svg);"
					      "}");
			docker->setContentsMargins(0, 0, 0, 0);
		}
	});
}

void Util::SetAttrRecur(QDomElement &elem, SVGSpec s)
{
	// if it has the tagname then overwritte desired attribute
	if(elem.tagName().compare(s.tag) == 0) {
		if(s.id.isEmpty()) {
			elem.setAttribute(s.attrName, s.attrVal);
		} else if(elem.attribute("id").compare(s.id) == 0) {
			elem.setAttribute(s.attrName, s.attrVal);
		}
	}

	// loop all children
	for(int i = 0; i < elem.childNodes().count(); i++) {
		if(!elem.childNodes().at(i).isElement()) {
			continue;
		}
		QDomElement docElem = elem.childNodes().at(i).toElement(); //<-- make const "variable"
		SetAttrRecur(docElem, s);
	}
}

/*QIcon*/ QPixmap Util::ChangeSVGColor(QString iconPath, QString color, float opacity)
{
	SVGSpec pathFill{"path", "", "fill", color};
	SVGSpec pathOpacity{"path", "", "opacity", QString::number(opacity, 'g', 2)};
	SVGSpec polygonFill{"polygon", "", "fill", color};
	QPixmap pix = ChangeSVGAttr(iconPath, {pathFill, pathOpacity, polygonFill});
	return pix;
}

QPixmap Util::ChangeSVGAttr(QString iconPath, QList<SVGSpec> list)
{
	QFile file(iconPath);
	if(!file.open(QIODevice::ReadOnly))
		return {};
	QByteArray baData = file.readAll();
	// load svg contents to xml document and edit contents
	QDomDocument doc;
	doc.setContent(baData);
	// recurivelly change color
	QDomElement docElem = doc.documentElement(); //<-- make const "variable"
	for(const SVGSpec &s : list) {
		SetAttrRecur(docElem, s);
	}
	QSvgRenderer svgRenderer(doc.toByteArray());
	// create pixmap target (could be a QImage)
	QPixmap pix(svgRenderer.defaultSize());
	pix.fill(Qt::transparent);
	// create painter to act over pixmap
	QPainter pixPainter(&pix);
	// use renderer to render over painter which paints on pixmap
	svgRenderer.render(&pixPainter);
	return pix;
}
