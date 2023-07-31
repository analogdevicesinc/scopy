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

#ifndef TIME_SINK_F_IMPL_H
#define TIME_SINK_F_IMPL_H

#include "time_sink_f.h"

namespace scopy {

class time_sink_f_impl : public time_sink_f
{
public:
	time_sink_f_impl(int size, float sampleRate, const std::string &name, int nconnections);
	~time_sink_f_impl();

	bool check_topology(int ninputs, int noutputs);
	std::string name() const;

	void updateData();

	bool rollingMode();
	void setRollingMode(bool b);

	const std::vector<float> &time() const;
	const std::vector<std::vector<float>> &data() const;
	int work(int noutput_items,
		 gr_vector_const_void_star &input_items,
			 gr_vector_void_star &output_items);
private:
	std::vector <std::deque <float>> m_buffers;
	std::vector< std::vector<gr::tag_t> > m_tags;
	std::vector<float> m_time;
	std::vector<std::vector<float>> m_data;

	int m_size;
	float m_sampleRate;
	std::string m_name;
	int m_nconnections;
	bool m_rollingMode;

	void generate_time_axis();
};

} /* namespace scopy */

#endif /* M2K_time_sink_f_IMPL_H */
