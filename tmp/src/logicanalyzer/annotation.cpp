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


#include "annotation.h"
#include <cassert>

extern "C" {
#include <libsigrokdecode/libsigrokdecode.h>
}

#include <vector>

Annotation::Annotation(const srd_proto_data *const pdata, const Row *row) :
    start_sample_(pdata->start_sample),
    end_sample_(pdata->end_sample),
    row_(row)
{
    assert(pdata);
    const srd_proto_data_annotation *const pda =
        (const srd_proto_data_annotation*)pdata->data;
    assert(pda);

    ann_class_ = (Class)(pda->ann_class);

    const char *const *annotations = (char**)pda->ann_text;
    while (*annotations) {
        annotations_.push_back(QString::fromUtf8(*annotations));
        annotations++;
    }
}

uint64_t Annotation::start_sample() const
{
    return start_sample_;
}

uint64_t Annotation::end_sample() const
{
    return end_sample_;
}

Annotation::Class Annotation::ann_class() const
{
    return ann_class_;
}

const vector<QString>& Annotation::annotations() const
{
    return annotations_;
}

const Row* Annotation::row() const
{
    return row_;
}

bool Annotation::operator<(const Annotation &other) const
{
    return (start_sample_ < other.start_sample_);
}
