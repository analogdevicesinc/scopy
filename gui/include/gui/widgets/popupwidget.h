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


#ifndef SCOPY_POPUPWIDGET_H
#define SCOPY_POPUPWIDGET_H

#include <QWidget>
#include "scopy-gui_export.h"

namespace Ui { class PopupWidget; }

namespace scopy {
class SCOPY_GUI_EXPORT PopupWidget : public QWidget {
	Q_OBJECT
public:
	explicit PopupWidget(QWidget* parent = nullptr);
	~PopupWidget();

	void setFocusOnContinueButton();
	void setFocusOnExitButton();

	void enableTitleBar(bool enable = true);

	QString getDescription();
	void setDescription(const QString& description);

	QString getTitle();
	void setTitle(const QString& title);

	QString getContinueButtonText();
	void setContinueButtonText(const QString& text);

	QString getExitButtonText();
	void setExitButtonText(const QString& text);

Q_SIGNALS:
	void continueButtonClicked();
	void exitButtonClicked();

protected:
	Ui::PopupWidget *ui;
};
}

#endif //SCOPY_POPUPWIDGET_H
