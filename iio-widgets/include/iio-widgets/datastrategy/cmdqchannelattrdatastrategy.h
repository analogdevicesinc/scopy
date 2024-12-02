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

#ifndef CMDQCHANNELATTRDATASTRATEGY_H
#define CMDQCHANNELATTRDATASTRATEGY_H

#include <QWidget>
#include <QAtomicInteger>
#include <iioutil/commandqueue.h>
#include "iiowidgetdata.h"
#include "datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT CmdQChannelAttrDataStrategy : public QWidget, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	explicit CmdQChannelAttrDataStrategy(IIOWidgetFactoryRecipe recipe, CommandQueue *commandQueue,
					     QWidget *parent = nullptr);

public Q_SLOTS:
	void save(QString data) override;
	void requestData() override;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions) override;
	void emitStatus(int status) override;

private:
	CommandQueue *m_cmdQueue;

	// 2 commandq operations are needed, this variable will keep track of how many are made
	std::mutex m_mutex;
	QString m_dataRead;
	QString m_optionalDataRead;
};
} // namespace scopy

#endif // CMDQCHANNELATTRDATASTRATEGY_H
