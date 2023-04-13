/* -*- c++ -*- */
/*
 * Copyright 2012,2014 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
/*
 * Copyright (c) 2019 Analog Devices Inc.
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

#ifndef M2K_STREAM_TO_VECTOR_OVERLAP_IMPL_H
#define M2K_STREAM_TO_VECTOR_OVERLAP_IMPL_H

#include "stream_to_vector_overlap.h"

namespace scopy {

class stream_to_vector_overlap_impl : public stream_to_vector_overlap
{
public:
	explicit stream_to_vector_overlap_impl(size_t itemsize,
					       size_t nitems_per_block,
					       double overlap_factor);

	int work(int noutput_items,
		 gr_vector_const_void_star& input_items,
		 gr_vector_void_star& output_items);
	void set_overlap_factor(double);

private:
	double m_overlap_factor;
	size_t m_nb_overlapped_items;
	size_t m_itemsize;
	size_t m_nitems_per_block;
};
} /* namespace scopy */

#endif /* M2K_STREAM_TO_VECTOR_OVERLAP_IMPL_H */
