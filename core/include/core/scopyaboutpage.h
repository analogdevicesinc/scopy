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

#ifndef SCOPYABOUTPAGE_H
#define SCOPYABOUTPAGE_H

#include "scopy-core_export.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>

namespace scopy {
class SCOPY_CORE_EXPORT ScopyAboutPage : public QWidget
{
	Q_OBJECT
public:
	ScopyAboutPage(QWidget *parent = nullptr);
	~ScopyAboutPage();
	void addHorizontalTab(QWidget *w, QString text);
	QWidget *buildPage(QString src);

private:
	void initUI();
	void initNavigationWidget(QTextBrowser *browser);
	QWidget *buildStylePage();
	QWidget *buildPageColors();
	QTabWidget *tabWidget;
	QVBoxLayout *layout;
};
} // namespace scopy

#endif // SCOPYABOUTPAGE_H
