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
 */

#ifndef DOCKWRAPPERCLASSIC_H
#define DOCKWRAPPERCLASSIC_H

#include "scopy-gui_export.h"
#include <QWidget>
#include "docking/dockwrapperinterface.h"

namespace scopy::classic {
class SCOPY_GUI_EXPORT DockWrapper : public QWidget, public DockWrapperInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DockWrapperInterface)
public:
	explicit DockWrapper(QString name, QWidget *parent = nullptr);

	void setInnerWidget(QWidget *innerWidget) override;
	QWidget *innerWidget() const override;

	void setActivated(bool isActivated) override;

private:
	void init();
	QWidget *m_innerWidget;
};
} // namespace scopy::classic

#endif // DOCKWRAPPERCLASSIC_H
