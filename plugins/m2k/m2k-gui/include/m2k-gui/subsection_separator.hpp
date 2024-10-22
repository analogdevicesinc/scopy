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

#ifndef SUBSECTIONSEPARATOR_H
#define SUBSECTIONSEPARATOR_H

#include "scopy-m2k-gui_export.h"
#include <QLabel>
#include <QPushButton>
#include <QWidget>

namespace Ui {
class SubsectionSeparator;
}

namespace scopy {
namespace m2kgui {
class SCOPY_M2K_GUI_EXPORT SubsectionSeparator : public QWidget
{
	Q_OBJECT

public:
	explicit SubsectionSeparator(QWidget *parent = nullptr);
	explicit SubsectionSeparator(const QString &text, const bool buttonVisible = false, QWidget *parent = nullptr);
	~SubsectionSeparator();

private:
	Ui::SubsectionSeparator *m_ui;

public:
	QPushButton *getButton();
	void setButtonVisible(bool buttonVisible);

	bool getButtonChecked();
	void setButtonChecked(bool checked);

	QLabel *getLabel();
	void setLabel(const QString &text);

	void setLabelVisible(bool visible);
	void setLineVisible(bool visible);

	void setContent(QWidget *content);
	QWidget *getContentWidget();
};
} // namespace m2kgui
} // namespace scopy

#endif // SUBSECTIONSEPARATOR_H
