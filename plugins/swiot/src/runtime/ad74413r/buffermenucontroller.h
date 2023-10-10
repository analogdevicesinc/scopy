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

#ifndef BUFFERMENUCONTROLLER_H
#define BUFFERMENUCONTROLLER_H

#include "buffermenumodel.h"
#include "buffermenuview.h"
#include "qobject.h"

namespace scopy::swiot {
class BufferMenuController : public QObject
{
	Q_OBJECT
public:
	explicit BufferMenuController(BufferMenuView *genericMenu = nullptr, BufferMenuModel *model = nullptr,
				      int chnlIdx = -1);

	~BufferMenuController();

	void createConnections();

	int getChnlIdx();

public Q_SLOTS:
	void attributesChanged(QString attrName, QString chnlType);

	void addMenuAttrValues(QMap<QString, QMap<QString, QStringList>> values);
Q_SIGNALS:
	void broadcastThresholdReadForward(QString value);
	void broadcastThresholdReadBackward(QString value);
	void thresholdControlEnable(bool enable);
	void diagnosticFunctionUpdated();
	void setUnitPerDivision(int chnl, double unitPerDiv);
	void unitPerDivisionChanged(int chnl, double unitPerDiv);

private:
	int m_chnlIdx;
	BufferMenuView *m_genericMenu;
	BufferMenuModel *m_model;
};
} // namespace scopy::swiot

#endif // BUFFERMENUCONTROLLER_H
