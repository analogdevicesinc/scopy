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

#include <QDebug>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>
#include <gnuradio/buffer.h>
#include <gnuradio/prefs.h>
#include <string.h>

#include "time_sink_f_impl.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_TIME_SINK_F, "TimeSink_f");

using namespace gr;

namespace scopy {

time_sink_f::sptr
time_sink_f::make(int size, float sampleRate, const std::string &name,
		  int nconnections )
{
	return gnuradio::get_initial_sptr
		(new time_sink_f_impl(size, sampleRate, name, nconnections));
}

void time_sink_f_impl::generate_time_axis() {
	qInfo(CAT_TIME_SINK_F)<< "Generating x-Axis";
	double timeoffset = 0;
	m_time.clear();
	for (int i = 0;i < m_size;i++) {
		m_time.push_back(timeoffset + i/m_sampleRate);
	}
}

time_sink_f_impl::time_sink_f_impl(int size, float sampleRate, const std::string &name, int nconnections )
    : sync_block("time_sink_f",
	io_signature::make(nconnections, nconnections, sizeof(float)),
	io_signature::make(0, 0, 0)),
      m_size(size),
      m_sampleRate(sampleRate),
      m_name(name),
      m_nconnections(nconnections),
      m_rollingMode(false),
      m_workFinished(false),
      m_dataUpdated(false)
{
	qInfo(CAT_TIME_SINK_F)<<"ctor";
	// reserve memory for n buffers
	m_data.reserve(nconnections);
	m_dataTags.reserve(nconnections);
	m_tags.reserve(nconnections);

	for (int i = 0; i < m_nconnections;i++) {
		m_buffers.push_back(std::deque<float>());
		m_data.push_back(std::vector<float>());
		m_tags.push_back(std::vector<tag_t>());
		m_dataTags.push_back(std::vector<PlotTag_t>());
		// each data buffer reserves size
		m_data[i].reserve(size);
	}

	m_time.reserve(size);
	generate_time_axis();

	m_tags = std::vector< std::vector<gr::tag_t> >(m_nconnections);
}

time_sink_f_impl::~time_sink_f_impl() {
qInfo(CAT_TIME_SINK_F)<<"dtor";
}

bool time_sink_f_impl::check_topology(int ninputs, int noutputs) {
	return ninputs == m_nconnections;
}

std::string time_sink_f_impl::name() const {
	return m_name;
}

void time_sink_f_impl::updateData() {
	gr::thread::scoped_lock lock(d_setlock);

	for(int i = 0; i < m_nconnections; i++) {
		m_data[i].clear();
		for(int j = 0; j < m_buffers[i].size(); j++) {
			m_data[i].push_back(m_buffers[i][j]);
		}
		for(int j = 0 ;j < m_tags[i].size(); j++) {
			PlotTag_t tag;

			std::stringstream s;
			s << m_tags[i][j].key << ": " << m_tags[i][j].value;
			tag.str = QString::fromStdString(s.str());
			tag.offset = m_tags[i][j].offset;

			m_dataTags[i].push_back(tag);
		}
	}
	if(m_workFinished) {
		m_dataUpdated = true;
	}
}

bool time_sink_f_impl::rollingMode()
{
	return m_rollingMode;
}

void time_sink_f_impl::setRollingMode(bool b)
{
	m_rollingMode = b;
}

bool time_sink_f_impl::singleShot()
{
	return m_singleShot;
}

void time_sink_f_impl::setSingleShot(bool b)
{
	m_singleShot = b;
}

bool time_sink_f_impl::finishedAcquisition()
{
	return m_workFinished && m_dataUpdated;
}

const std::vector<float> &time_sink_f_impl::time() const {
	return m_time;
}

const std::vector<std::vector<float> > &time_sink_f_impl::data() const {
	return m_data;
}

const std::vector<std::vector<PlotTag_t> > &time_sink_f_impl::tags() const
{
	return m_dataTags;
}

bool time_sink_f_impl::start()
{
	m_workFinished = false;
	m_dataUpdated = false;
	return true;
}

bool time_sink_f_impl::stop()
{
	return true;
}


int time_sink_f_impl::work(int noutput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items) {

//	qInfo(CAT_TIME_SINK_F)<<"Work";
	gr::thread::scoped_lock lock(d_setlock);

	// Trigger on BUFFER_START (?)
	if(m_singleShot) {
		if(m_buffers[0].size() == m_size) {
			m_workFinished = true;
			return WORK_DONE;
		}
	}

	if(!m_rollingMode) {
	for(int i = 0; i < m_nconnections; i++) {
			if(m_buffers[i].size() >= m_size) {
				m_buffers[i].clear();
				m_tags[i].clear();
			}
		}
	}

	for(int i = 0; i < m_nconnections; i++) {
		for(int j = 0; j < noutput_items; j++) {
			if(m_buffers[i].size() >= m_size) {
				m_buffers[i].pop_back();
			}

			const float *in;
			in = (const float*)input_items[i];
			m_buffers[i].push_front(in[j]);
		}
		get_tags_in_window(m_tags[i],i,0,noutput_items);
	}


	return noutput_items;

}
} /* namespace gr */
