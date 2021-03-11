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


#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <cstdint>
#include <vector>

#include <QString>

struct srd_proto_data;
class Row;

using std::vector;

class Annotation
{
public:
    typedef uint32_t Class;

public:
    Annotation() = default;
    Annotation(const Annotation &other) = default;
    Annotation(const srd_proto_data *const pdata, const Row *row);

    uint64_t start_sample() const;
    uint64_t end_sample() const;
    Class ann_class() const;
    const vector<QString>& annotations() const;
    const Row* row() const;

    bool operator<(const Annotation &other) const;

private:
    uint64_t start_sample_;
    uint64_t end_sample_;
    Class ann_class_;
    vector<QString> annotations_;
    const Row *row_;

};

#endif // ANNOTATION_H
