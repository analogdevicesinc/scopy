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

#include "rowdata.h"

#include <QDebug>

uint64_t RowData::get_max_sample() const
{
    if (annotations_.empty())
        return 0;
    return annotations_.back().end_sample();
//    return 0;
}

void RowData::get_annotation_subset(
    vector<Annotation> &dest,
    uint64_t start_sample, uint64_t end_sample) const
{
    for (const auto& annotation : annotations_)
        if (annotation.end_sample() > start_sample &&
            annotation.start_sample() <= end_sample)
            dest.push_back(annotation);
}

Annotation RowData::getAnnAt(uint64_t index) const {

    return annotations_[index];
}

void RowData::sort_annotations() {
    // Use stable_sort to keep the annotations having
    // the same start sample in the same order as
    // they came from libsigrokdecode
    std::stable_sort(annotations_.begin(), annotations_.end(), [](const Annotation &a, const Annotation &b){
        return a.start_sample() < b.start_sample();
    });
}

std::pair<uint64_t, uint64_t> RowData::get_annotation_subset(uint64_t start_sample, uint64_t end_sample) const
{

//    qDebug() << start_sample << " " << end_sample;

    uint64_t first = 0, last = 0;

    bool found = false;
    for (int i = 0; i < annotations_.size(); ++i) {
        if (annotations_[i].end_sample() > start_sample &&
            annotations_[i].start_sample() <= end_sample) {
            if (!found) {
                first = i;
                found = true;
            } else {
                last = i;
            }
        }
    }

    if (!last && first > 0 && first < annotations_.size() - 1) {
        last = first + 1;
    }

    // let s adjust the edges a bit
    if (first > 0) first--;
    if (last < annotations_.size() - 1) last++;

    return std::make_pair(first, last);
}

void RowData::emplace_annotation(srd_proto_data *pdata, const Row *row)
{
    annotations_.emplace_back(pdata, row);
}


