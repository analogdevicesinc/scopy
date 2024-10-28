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

#ifndef CSVFILESTRATEGY_H
#define CSVFILESTRATEGY_H

#include <QWidget>
#include "databufferstrategyinterface.h"
#include "scopy-dac_export.h"
#include "dac_logging_categories.h"
namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT CSVFileStrategy : public QObject, public DataBufferStrategyInterface
{
	Q_OBJECT
	Q_INTERFACES(scopy::dac::DataBufferStrategyInterface)
public:
	explicit CSVFileStrategy(QString filename, QWidget *parent = nullptr);
	~CSVFileStrategy(){};
	QVector<QVector<short>> data() override;

public Q_SLOTS:
	void recipeUpdated(DataBufferRecipe) override;
	void loadData() override;

Q_SIGNALS:
	void loadFinished() override;
	void loadFailed() override;
	void dataUpdated() override;

private:
	double m_max;
	QString m_filename;
	QString m_separator;
	QVector<QVector<double>> m_data;
	QVector<QVector<short>> m_dataConverted;
	DataBufferRecipe m_recipe;
	unsigned short convert(double value);
	void applyConversion();
};
} // namespace dac
} // namespace scopy
#endif // CSVFILESTRATEGY_H
