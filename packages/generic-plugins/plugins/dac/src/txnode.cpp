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

#include "txnode.h"
#include "dac_logging_categories.h"

using namespace scopy;
using namespace scopy::dac;
TxNode::TxNode(QString uuid, iio_channel *chn, QObject *parent)
	: QObject(parent)
	, m_channel(chn)
	, m_txUuid(uuid)
	, m_fmtBits(0)
	, m_fmtShift(0)
{
	if(m_channel) {
		auto fmt = iio_channel_get_data_format(m_channel);
		m_fmtShift = fmt->shift;
		m_fmtBits = fmt->bits;
	}
}

TxNode::~TxNode()
{
	for(auto node : qAsConst(m_childNodes)) {
		delete node;
	}
	m_childNodes.clear();
	qDebug(CAT_DAC_DATA) << QString("Delete TX Node %1").arg(m_txUuid);
}

TxNode *TxNode::addChildNode(QString uuid, iio_channel *chn)
{
	TxNode *child = m_childNodes.value(uuid, nullptr);
	if(!child) {
		child = new TxNode(uuid, chn, this);
		m_childNodes.insert(uuid, child);
	}
	return child;
}

QMap<QString, TxNode *> TxNode::getTones() const { return m_childNodes; }

QString TxNode::getUuid() const { return m_txUuid; }

iio_channel *TxNode::getChannel() { return m_channel; }

unsigned int TxNode::getFormatShift() const { return m_fmtShift; }

unsigned int TxNode::getFormatBits() const { return m_fmtBits; }

bool TxNode::enableDds(bool enable)
{
	qDebug(CAT_DAC_DATA) << QString("Try enable:%1 DDS TXNode %2").arg(enable).arg(m_txUuid);
	if(m_channel) {
		if(iio_channel_get_type(m_channel) == IIO_ALTVOLTAGE) {
			int ret = iio_channel_attr_write_bool(m_channel, "raw", enable);
			if(ret < 0) {
				qDebug(CAT_DAC_DATA) << QString("Can't enable DDS channel, error: %1").arg(ret);
				return false;
			}
			qDebug(CAT_DAC_DATA) << QString("DDS channel %1 enabled: %2, ret code %3")
							.arg(m_txUuid)
							.arg(enable)
							.arg(ret);
		} else {
			qDebug(CAT_DAC_DATA) << QString("%1 not a DDS channel").arg(m_txUuid);
			return false;
		}
	} else if(m_childNodes.size() != 0) {
		for(auto node : qAsConst(m_childNodes)) {
			bool ret = node->enableDds(enable);
			if(!ret) {
				return ret;
			}
		}
	} else {
		qDebug(CAT_DAC_DATA) << "can't enable DDS channel, invalid selection";
		return false;
	}
	return true;
}

const QColor &TxNode::getColor() const { return m_color; }

void TxNode::setColor(const QColor &newColor) { m_color = newColor; }
