/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include <cassert>
#include <scopy/gui/logicanalyzer/annotation.hpp>

extern "C"
{
#include <libsigrokdecode/libsigrokdecode.h>
}

#include <vector>

Annotation::Annotation(const srd_proto_data* const pdata, const Row* row)
	: m_startSample_(pdata->start_sample)
	, m_endSample_(pdata->end_sample)
	, m_row_(row)
{
	assert(pdata);
	const srd_proto_data_annotation* const pda = (const srd_proto_data_annotation*)pdata->data;
	assert(pda);

	m_annClass_ = (Class)(pda->ann_class);

	const char* const* annotations = (char**)pda->ann_text;
	while (*annotations) {
		m_annotations_.push_back(QString::fromUtf8(*annotations));
		annotations++;
	}
}

uint64_t Annotation::startSample() const { return m_startSample_; }

uint64_t Annotation::endSample() const { return m_endSample_; }

Annotation::Class Annotation::annClass() const { return m_annClass_; }

const vector<QString>& Annotation::annotations() const { return m_annotations_; }

const Row* Annotation::row() const { return m_row_; }

bool Annotation::operator<(const Annotation& other) const { return (m_startSample_ < other.m_startSample_); }
