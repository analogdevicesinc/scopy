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


#ifndef ROWDATA_H
#define ROWDATA_H

#include <libsigrokdecode/libsigrokdecode.h>

#include "annotation.h"

#include <vector>
#include <set>

class Row;

class RowData
{
public:
    RowData() = default;

public:
    uint64_t get_max_sample() const;

    uint64_t size() const {
        return annotations_.size();
    }

    /**
     * Extracts annotations between the given sample range into a vector.
     * Note: The annotations are unsorted and only annotations that fully
     * fit into the sample range are considered.
     */
    void get_annotation_subset(
        vector<Annotation> &dest,
        uint64_t start_sample, uint64_t end_sample) const;

    void emplace_annotation(srd_proto_data *pdata, const Row *row);

    std::pair<uint64_t, uint64_t> get_annotation_subset(uint64_t start_sample,
                                                        uint64_t end_sample) const;

    Annotation getAnnAt(uint64_t index) const;

    void sort_annotations();

private:
    struct annotation_compare {
        bool operator() (const Annotation &a, const Annotation &b) const {
            return a.start_sample() < b.start_sample();
        }
    };

    std::vector<Annotation/*, annotation_compare*/> annotations_;
};

#endif // ROWDATA_H
