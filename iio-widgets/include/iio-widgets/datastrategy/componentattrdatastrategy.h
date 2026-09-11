/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef SCOPY_COMPONENTATTRDATASTRATEGY_H
#define SCOPY_COMPONENTATTRDATASTRATEGY_H

#include <QWidget>
#include "datastrategy/datastrategyinterface.h"
#include "iiowidgetdata.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
namespace component {
class Attribute;
}

/**
 * @brief Data strategy bound to a generic device-controller component::Attribute
 * (IIO, m2k, or any future backend) instead of a raw iio_channel/iio_device.
 * I/O is delegated to the Attribute's read/write capabilities: the async paths
 * are non-blocking (eager QCoro tasks driven by the event loop), while the
 * synchronous read()/write() honour their inline-return contract via QCoro::waitFor.
 * sendData is driven solely from Attribute::valueChanged, so two widgets bound to
 * the same Attribute stay in sync automatically.
 */
class SCOPY_IIO_WIDGETS_EXPORT ComponentAttrDataStrategy : public QObject, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	explicit ComponentAttrDataStrategy(IIOWidgetFactoryRecipe recipe, QObject *parent = nullptr);
	~ComponentAttrDataStrategy();

	QString data() override;
	QString optionalData() override;

public Q_SLOTS:
	int write(QString data) override;
	QPair<QString, QString> read() override;

	void writeAsync(QString data) override;
	void readAsync() override;

Q_SIGNALS:
	void sendData(QString data, QString dataOptions) override;
	void aboutToWrite(QString oldData, QString newData) override;
	void emitStatus(QDateTime timestamp, QString oldData, QString newData, int returnCode, bool isReadOp) override;

private:
	scopy::component::Attribute *m_attr;
	QString m_data;
	QString m_optionalData;
};
} // namespace scopy

#endif // SCOPY_COMPONENTATTRDATASTRATEGY_H
