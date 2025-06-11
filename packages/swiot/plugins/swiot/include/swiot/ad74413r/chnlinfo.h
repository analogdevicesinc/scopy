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

#ifndef CHNLINFO_H
#define CHNLINFO_H

#include <iio.h>

#include <QMap>
#include <qobject.h>

#include <iioutil/command.h>
#include <iioutil/commandqueue.h>

#define SWAP_UINT32(x) (((x) >> 24) | (((x)&0x00FF0000) >> 8) | (((x)&0x0000FF00) << 8) | ((x) << 24))
namespace scopy::swiot {
class ChnlInfo : public QObject
{
	Q_OBJECT
public:
	explicit ChnlInfo(QString plotUm, QString hwUm, iio_channel *iioChnl, CommandQueue *cmdQueue);
	~ChnlInfo();

	virtual double convertData(unsigned int data) = 0;
	iio_channel *iioChnl() const;

	bool isOutput() const;

	bool isScanElement() const;

	QString chnlId() const;

	std::pair<int, int> rangeValues() const;

	std::pair<double, double> offsetScalePair() const;

	bool isEnabled() const;
	void setIsEnabled(bool newIsEnabled);

	QString unitOfMeasure() const;

	void addReadScaleCommand();
	void addReadOffsetCommand();
private Q_SLOTS:
	void readScaleCommandFinished(scopy::Command *cmd);
	void readOffsetCommandFinished(scopy::Command *cmd);

protected:
	bool m_isOutput;
	bool m_isEnabled;
	bool m_isScanElement;
	QString m_chnlId;
	QString m_plotUm;
	QString m_hwUm;
	std::pair<int, int> m_rangeValues = {-5, 5};
	std::pair<double, double> m_offsetScalePair;
	QMap<QString, double> m_unitOfMeasureFactor;
	CommandQueue *m_commandQueue;

private:
	void initUnitOfMeasureFactor();
	struct iio_channel *m_iioChnl;
};
} // namespace scopy::swiot

#endif // CHNLINFO_H
