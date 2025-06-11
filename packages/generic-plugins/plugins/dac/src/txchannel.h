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

#ifndef TXCHANNEL_H
#define TXCHANNEL_H

#include <QWidget>
#include <QMap>

namespace scopy {
namespace dac {
class TxNode;
class TxTone;
class TxChannel : public QWidget
{
	Q_OBJECT
public:
	TxChannel(TxNode *node, unsigned int nbTonesMode, QWidget *parent = nullptr);
	virtual ~TxChannel();

	void read();
	void enable(bool enable);
	QString channelUuid() const;
	QString frequency(unsigned int toneIdx);
	QString phase(unsigned int toneIdx);
	TxTone *tone(unsigned int toneIdx);
	unsigned int toneCount();
	QMap<QString, QWidget *> getToneMenus() const;
Q_SIGNALS:
	void resetChannelScales();
public Q_SLOTS:
	void scaleUpdated(int toneIdx, QString oldScale, QString newScale);

private Q_SLOTS:
	void resetToneScales();

private:
	TxNode *m_node;
	QMap<int, TxTone *> m_tones;
	unsigned int m_nbTonesMode;

	TxTone *setupTxTone(TxNode *nodeTone, unsigned int index);
};
} // namespace dac
} // namespace scopy

#endif // TXCHANNEL_H
