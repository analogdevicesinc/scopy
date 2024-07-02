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

#ifndef SCOPY_MULTIDATASTRATEGY_H
#define SCOPY_MULTIDATASTRATEGY_H

#include <QWidget>
#include <QList>
#include <QSet>
#include <iio.h>
#include "datastrategy/datastrategyinterface.h"
#include "scopy-iio-widgets_export.h"

namespace scopy {
class SCOPY_IIO_WIDGETS_EXPORT MultiDataStrategy : public QWidget, public DataStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::DataStrategyInterface)
public:
	/**
	 * @brief MultiDataStrategy This is a class for incorporating multiple Data Strategies in the
	 * same IIOWidget. All results that are used but the IIOWidget class are aggregated, the ones
	 * that are related to the status of the operations are not.
	 * @param strategies A QList of pointers to DataStrategyInterfaces. More Data Strategies can be
	 * added after creation.
	 * @param parent QWidget*
	 */
	explicit MultiDataStrategy(QList<DataStrategyInterface *> strategies, QWidget *parent = nullptr);

	/**
	 * @brief addDataStrategy Add and connect a new Data Strategy.
	 * @param ds
	 */
	void addDataStrategy(DataStrategyInterface *ds);

	/**
	 * @brief removeDataStrategy Remove and disconnect a Data Strategy.
	 * @param ds
	 */
	void removeDataStrategy(DataStrategyInterface *ds);

	/**
	 * @brief removeDataStrategyByIndex Remove and disconnect a Data Strategy based on its index.
	 * The index can be found from the dataStrategies() function.
	 * @param index
	 */
	void removeDataStrategyByIndex(int index);

	/**
	 * @brief dataStrategies Return a list of all Data Strategies, useful when wanting to find a
	 * specific index.
	 * @return QList of DataStrategyInterface pointers.
	 */
	QList<DataStrategyInterface *> dataStrategies();

	QString data() override;
	QString optionalData() override;

public Q_SLOTS:
	int write(QString data) override;
	QPair<QString, QString> read() override;

	void writeAsync(QString data) override;
	void readAsync() override;

	void receiveSingleRead(QString data, QString optionalData);

Q_SIGNALS:
	void sendData(QString data, QString dataOptions) override;
	void aboutToWrite(QString oldData, QString newData) override;
	void emitStatus(QDateTime timestamp, QString oldData, QString newData, int returnCode, bool isReadOp) override;

private:
	QList<DataStrategyInterface *> m_dataStrategies;
	QList<QPair<QString, QString>> m_receivedData;

	QSet<QObject *> m_expectedSignals;
	QSet<QObject *> m_receivedSignals;

	QString m_data;
	QString m_optionalData;
	int m_returnCode;
};
} // namespace scopy

#endif // SCOPY_MULTIDATASTRATEGY_H
