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

#ifndef TXNODE_H
#define TXNODE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QColor>

#include <qcoro/qcorotask.h>

namespace scopy {
namespace component {
class Channel;
}
namespace dac {
class TxNode : public QObject
{
	Q_OBJECT
public:
	TxNode(QString uuid, component::Channel *chn = nullptr, QObject *parent = nullptr);
	virtual ~TxNode();

	TxNode *addChildNode(QString uuid, component::Channel *chn = nullptr);

	QMap<QString, TxNode *> getTones() const;
	QString getUuid() const;
	component::Channel *getChannel();
	unsigned int getFormatShift() const;
	unsigned int getFormatBits() const;
	bool getFormatSigned() const;

	QCoro::Task<bool> enableDds(bool enable);
	QCoro::Task<bool> readDds() const;

	const QColor &getColor() const;
	void setColor(const QColor &newColor);

private:
	QString m_txUuid;
	QMap<QString, TxNode *> m_childNodes = {};
	component::Channel *m_channel;
	unsigned int m_fmtShift;
	unsigned int m_fmtBits;
	bool m_fmtSigned;
	QColor m_color;
};
} // namespace dac
} // namespace scopy

#endif // TXNODE_H
