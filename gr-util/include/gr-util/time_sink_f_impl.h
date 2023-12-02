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

#include <QString>
namespace scopy {

class time_sink_f_impl : public time_sink_f
{
public:
	time_sink_f_impl(int size, float sampleRate, const std::string &name, int nconnections);
	~time_sink_f_impl();

	bool check_topology(int ninputs, int noutputs);
	std::string name() const;

	void updateData() override;

	bool rollingMode() override;
	void setRollingMode(bool b) override;

	bool singleShot() override;
	void setSingleShot(bool b) override;

	bool computeTags() override;
	void setComputeTags(bool newComputeTags) override;

	float freqOffset() override;
	void setFreqOffset(float) override;

	bool fftComplex() override;
	void setFftComplex(bool) override;

	bool finishedAcquisition() override;

	const std::vector<float> &time() const override;
	const std::vector<float> &freq() const override;
	const std::vector<std::vector<float>> &data() const override;
	const std::vector<std::vector<PlotTag_t>> &tags() const override;

	bool start() override;
	bool stop() override;
	int work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) override;

private:
	std::vector<std::deque<float>> m_buffers;
	std::vector<std::vector<gr::tag_t>> m_localtags;
	std::vector<std::deque<gr::tag_t>> m_tags;

	std::vector<float> m_time;
	std::vector<float> m_freq;

	std::vector<std::vector<float>> m_data;
	std::vector<std::vector<PlotTag_t>> m_dataTags;

	int m_size;
	float m_sampleRate;
	std::string m_name;
	int m_nconnections;
	bool m_rollingMode;
	bool m_singleShot;
	bool m_workFinished;
	bool m_dataUpdated;
	bool m_computeTags;
	float m_freqOffset;
	bool m_complexFft;

	void generate_time_axis();
};

} /* namespace scopy */

#endif /* M2K_time_sink_f_IMPL_H */
