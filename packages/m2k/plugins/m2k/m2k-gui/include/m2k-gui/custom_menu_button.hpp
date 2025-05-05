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

#ifndef CUSTOMMENUBUTTON_H
#define CUSTOMMENUBUTTON_H

#include "scopy-m2k-gui_export.h"
#include "customPushButton.h"

#include <QCheckBox>
#include <QLabel>
#include <QWidget>

using namespace scopy;

namespace Ui {
class CustomMenuButton;
}

namespace scopy {
namespace m2kgui {

class SCOPY_M2K_GUI_EXPORT CustomMenuButton : public QWidget
{
	Q_OBJECT

public:
	explicit CustomMenuButton(QString labelText = nullptr, bool checkboxVisible = false,
				  bool checkBoxChecked = false, QWidget *parent = nullptr);
	explicit CustomMenuButton(QWidget *parent = nullptr);
	~CustomMenuButton();

	void setLabel(const QString &text);
	void setCheckboxVisible(bool visible);

	CustomPushButton *getBtn();
	QCheckBox *getCheckBox();
	bool getCheckBoxState();
	void setCheckBoxState(bool checked);
	void setMenuFloating(bool floating);

public Q_SLOTS:
	void checkBoxToggled(bool toggled);

private:
	Ui::CustomMenuButton *m_ui;

	bool m_floatingMenu;
};
} // namespace m2kgui
} // namespace scopy

#endif // CUSTOMMENUBUTTON_H
