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

#ifndef INFOPAGE_H
#define INFOPAGE_H

#include "infopagekeyvaluewidget.h"
#include "scopy-gui_export.h"

#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {

class SCOPY_GUI_EXPORT InfoPage : public QWidget
{
	Q_OBJECT
public:
	InfoPage(QWidget *parent = nullptr);
	~InfoPage();

	void update(QString key, QString value);
	void clear();

	void setAdvancedMode(bool newAdvancedMode);

private:
	void getKeyMap();

	bool m_advancedMode;
	QVBoxLayout *lay;
	QMap<QString, QString> keyMap;
	QMap<QString, QString> map;
	QMap<QString, InfoPageKeyValueWidget *> wmap;
};

} // namespace scopy
#endif // INFOPAGE_H
