/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2014 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <libsigrokdecode/libsigrokdecode.h>
#include <scopy/gui/logicanalyzer/row.hpp>

Row::Row()
	: m_decoder_(nullptr)
	, m_row_(nullptr)
{}

Row::Row(int index, const srd_decoder* decoder, const srd_decoder_annotation_row* row)
	: index_(index)
	, m_decoder_(decoder)
	, m_row_(row)
{}

const srd_decoder* Row::decoder() const { return m_decoder_; }

const srd_decoder_annotation_row* Row::row() const { return m_row_; }

const QString Row::title() const
{
	if (m_decoder_ && m_decoder_->name && m_row_ && m_row_->desc)
		return QString("%1: %2").arg(QString::fromUtf8(m_decoder_->name), QString::fromUtf8(m_row_->desc));
	if (m_decoder_ && m_decoder_->name)
		return QString::fromUtf8(m_decoder_->name);
	if (m_row_ && m_row_->desc)
		return QString::fromUtf8(m_row_->desc);
	return QString();
}

const QString Row::class_name() const
{
	if (m_row_ && m_row_->desc)
		return QString::fromUtf8(m_row_->desc);
	return QString();
}

int Row::index() const { return index_; }

bool Row::operator<(const Row& other) const
{
	return (m_decoder_ < other.m_decoder_) || (m_decoder_ == other.m_decoder_ && m_row_ < other.m_row_);
}
