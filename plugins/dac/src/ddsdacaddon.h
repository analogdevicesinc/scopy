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

#ifndef DDSDACADDON_H_
#define DDSDACADDON_H_

#include <QWidget>
#include <gui/widgets/menucombo.h>
#include <gui/mapstackedwidget.h>
#include "dacaddon.h"

namespace scopy {
namespace dac {
class DacDataModel;
class TxNode;
class TxChannel;
class DdsDacAddon : public DacAddon
{
	Q_OBJECT
public:
	DdsDacAddon(DacDataModel *model, QWidget *parent = nullptr);
	virtual ~DdsDacAddon();
	virtual void enable(bool enable);
	virtual void setRunning(bool running);

private:
	DacDataModel *m_model;
	QMap<QWidget *, MapStackedWidget *> m_txWidgets;
	QWidget *setupDdsTx(TxNode *txNode);
	QWidget *setupTxMode(TxNode *txNode, unsigned int mode);
};

class TxMode : public QWidget
{
	Q_OBJECT
public:
	TxMode(TxNode *node, unsigned int mode, QWidget *parent = nullptr);
	virtual ~TxMode();

	typedef enum
	{
		DISABLED = 0,
		ONE_TONE = 1,
		TWO_TONES = 2,
		INDEPENDENT_IQ_CTRL
	} DdsMode;

	void read();
	void enable(bool enable);

private:
	TxNode *m_node;
	unsigned int m_mode;
	QList<TxChannel *> m_txChannels;
};
} // namespace dac
} // namespace scopy

#endif // DDSDACADDON_H_
