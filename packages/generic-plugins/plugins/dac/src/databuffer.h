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

#ifndef DATABUFFER_H
#define DATABUFFER_H
#include "scopy-dac_export.h"
#include "databufferstrategyinterface.h"
#include "dataguistrategyinterface.h"

#include <QWidget>
namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT DataBuffer : public QObject
{
	Q_OBJECT
public:
	explicit DataBuffer(DataGuiStrategyInterface *guids, DataBufferStrategyInterface *ds,
			    QWidget *parent = nullptr);
	~DataBuffer();
	DataBufferStrategyInterface *getDataBufferStrategy();
	DataGuiStrategyInterface *getDataGuiStrategyInterface();

	QWidget *getParent();

public Q_SLOTS:
	void loadData();

Q_SIGNALS:
	void loadFinished();
	void loadFailed();
	void dataUpdated();

private:
	QWidget *m_parent;
	DataBufferStrategyInterface *m_dataStrategy;
	DataGuiStrategyInterface *m_guiStrategy;
};
} // namespace dac
} // namespace scopy
#endif // DATABUFFER_H
