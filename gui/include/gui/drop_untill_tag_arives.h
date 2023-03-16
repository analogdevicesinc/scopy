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
#ifndef DROP_UNTILL_TAG_ARIVES_H
#define DROP_UNTILL_TAG_ARIVES_H

#include <gnuradio/block.h>

class drop_untill_tag_arives : public gr::block
{
public:
        explicit drop_untill_tag_arives(size_t itemsize);
        ~drop_untill_tag_arives();

        void reset();
        void set_buffer_size(unsigned int buffer_size);

        int general_work(int noutput_items,
                         gr_vector_int &ninput_items_,
                         gr_vector_const_void_star &input_items,
                         gr_vector_void_star &output_items);

private:
        std::vector<unsigned int> d_buffer_size;
        std::vector<unsigned int> d_copied_items;
};

#endif // DROP_UNTILL_TAG_ARIVES_H
