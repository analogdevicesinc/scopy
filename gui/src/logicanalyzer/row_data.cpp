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

#include <QDebug>

#include <scopy/gui/logicanalyzer/row_data.hpp>

uint64_t RowData::getMaxSample() const
{
	if (m_annotations_.empty())
		return 0;
	return m_annotations_.back().endSample();
	//    return 0;
}

void RowData::getAnnotationSubset(vector<Annotation>& dest, uint64_t start_sample, uint64_t end_sample) const
{
	for (const auto& annotation : m_annotations_)
		if (annotation.endSample() > start_sample && annotation.startSample() <= end_sample)
			dest.push_back(annotation);
}

Annotation RowData::getAnnAt(uint64_t index) const { return m_annotations_[index]; }

void RowData::sortAnnotations()
{
	// Use stable_sort to keep the annotations having
	// the same start sample in the same order as
	// they came from libsigrokdecode
	std::stable_sort(m_annotations_.begin(), m_annotations_.end(),
			 [](const Annotation& a, const Annotation& b) { return a.startSample() < b.startSample(); });
}

std::pair<uint64_t, uint64_t> RowData::getAnnotationSubset(uint64_t start_sample, uint64_t end_sample) const
{

	//    qDebug() << start_sample << " " << end_sample;

	uint64_t first = 0, last = 0;

	bool found = false;
	for (int i = 0; i < m_annotations_.size(); ++i) {
		if (m_annotations_[i].endSample() > start_sample && m_annotations_[i].startSample() <= end_sample) {
			if (!found) {
				first = i;
				found = true;
			} else {
				last = i;
			}
		}
	}

	if (!last && first > 0 && first < m_annotations_.size() - 1) {
		last = first + 1;
	}

	// let s adjust the edges a bit
	if (first > 0)
		first--;
	if (last < m_annotations_.size() - 1)
		last++;

	return std::make_pair(first, last);
}

void RowData::emplaceAnnotation(srd_proto_data* pdata, const Row* row) { m_annotations_.emplace_back(pdata, row); }
