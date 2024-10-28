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

#ifndef LOGDATATOFILE_HPP
#define LOGDATATOFILE_HPP

#include <QObject>
#include <dataacquisitionmanager.hpp>

namespace scopy {
namespace datamonitor {
class LogDataToFile : public QObject
{
	Q_OBJECT
public:
	explicit LogDataToFile(DataAcquisitionManager *dataAcquisitionManager, QObject *parent = nullptr);

	void continuousLogData(QString path);
	void logData(QString path);
	void loadData(QString path);

Q_SIGNALS:
	void startLogData();
	void logDataError();
	void logDataCompleted();
	void startLoadData();
	void loadDataCompleted();

private:
	DataAcquisitionManager *m_dataAcquisitionManager;
	QString *currentFileHeader;
};
} // namespace datamonitor
} // namespace scopy
#endif // LOGDATATOFILE_HPP
