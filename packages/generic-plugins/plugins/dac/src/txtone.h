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

#ifndef TXTONE_H
#define TXTONE_H

#include <iio-widgets/iiowidget.h>
#include <QWidget>

namespace scopy {
namespace dac {
class TxNode;
class TxTone : public QWidget
{
	Q_OBJECT
public:
	TxTone(TxNode *node, unsigned int idx, QWidget *parent = nullptr);
	virtual ~TxTone();

	void read();
	QString toneUuid() const;
	QString frequency();
	QString phase();
	QString scale();
	void setPairedTone(TxTone *tone);
	static QString scaleDStoUI(QString data);
	static QString scaleUItoDS(QString data);
Q_SIGNALS:
	void frequencyUpdated(unsigned int toneIdx, QString frequency);
	void scaleUpdated(unsigned int toneIdx, QString oldScale, QString scale);
	void phaseUpdated(unsigned int toneIdx, QString phase);

public Q_SLOTS:
	void updateFrequency(QString frequency);
	void updateScale(QString scale);
	void updatePhase(QString phase);

private Q_SLOTS:
	void forwardFreqChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);
	void forwardScaleChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);
	void forwardPhaseChange(QDateTime timestamp, QString oldData, QString newData, int retCode, bool readOp);

private:
	QString phaseDStoUI(QString data);
	QString phaseUItoDS(QString data);
	QString frequencyDStoUI(QString data);
	QString frequencyUItoDS(QString data);
	QString computePairedPhase(QString frequency, QString phase);

	TxNode *m_node;
	TxTone *m_pairedTone;
	unsigned int m_idx;
	IIOWidget *m_frequency;
	IIOWidget *m_scale;
	IIOWidget *m_phase;
};
} // namespace dac
} // namespace scopy
#endif // TXTONE_H
