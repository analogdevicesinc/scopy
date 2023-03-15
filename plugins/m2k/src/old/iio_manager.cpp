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
#include "iio_manager.hpp"
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/short_to_float.h>

#include "pluginbase/preferences.h"
#include <libm2k/m2kexceptions.hpp>
#include <iio.h>
#include <QLoggingCategory>
#include <QDebug>

using namespace adiscope;
using namespace gr;

static const int KERNEL_BUFFERS_DEFAULT = 1;
std::map<const std::string, iio_manager::map_entry> iio_manager::dev_map;
unsigned iio_manager::_id = 0;

Q_LOGGING_CATEGORY(CAT_M2K_IIO_MANAGER, "M2kIIOManager");

iio_manager::iio_manager(unsigned int block_id,
		struct iio_context *ctx, const std::string &_dev,
		unsigned long _buffer_size) :
	QObject(nullptr),
	top_block("IIO Manager " + std::to_string(block_id)),
	id(block_id), _started(false), buffer_size(_buffer_size),
	m_mixed_source(nullptr)
{
	m_context = libm2k::context::m2kOpen(ctx, "");
	m_analogin = m_context->getAnalogIn();
	if (!ctx)
		throw std::runtime_error("IIO context not created");

	struct iio_device *dev = iio_context_find_device(ctx, _dev.c_str());
	if (!dev)
		throw std::runtime_error("Device not found");

	nb_channels = iio_device_get_channels_count(dev);


	Preferences *p = Preferences::GetInstance();
	double targetFps = p->get("general_plot_target_fps").toString().toDouble(); // get target fps from preferences

	iio_block = gr::m2k::analog_in_source::make_from(m_context, _buffer_size, {1, 1}, {0, 0}, 10000, 1,
							 KERNEL_BUFFERS_DEFAULT, false, false, {0, 0}, {0, 0}, 0, 0, {0, 0},
							 false, false, targetFps);

	/* Avoid unconnected channel errors by connecting a dummy sink */
	auto dummy_copy = blocks::copy::make(sizeof(short));
	auto dummy = blocks::null_sink::make(sizeof(short));


	//TODO - make dynamic
	freq_comp_filt[0][0] = adiscope::frequency_compensation_filter::make(false);
	freq_comp_filt[0][1] = adiscope::frequency_compensation_filter::make(false);
	freq_comp_filt[1][0] = adiscope::frequency_compensation_filter::make(false);
	freq_comp_filt[1][1] = adiscope::frequency_compensation_filter::make(false);

	for (unsigned i = 0; i < nb_channels; i++) {
		hier_block2::connect(iio_block,i,freq_comp_filt[i][0],0);
		hier_block2::connect(freq_comp_filt[i][0],0,freq_comp_filt[i][1],0);

		hier_block2::connect(freq_comp_filt[i][1], 0, dummy_copy, i);

		hier_block2::connect(dummy_copy, i, dummy, i);

	}

	dummy_copy->set_enabled(true);

	timeout_b = gnuradio::get_initial_sptr(new timeout_block("msg"));
	hier_block2::msg_connect(iio_block, "msg", timeout_b, "msg");

	QObject::connect(&*timeout_b, SIGNAL(timeout()), this,
			SLOT(got_timeout()));
}

iio_manager::~iio_manager()
{
}

std::shared_ptr<iio_manager> iio_manager::has_instance(const std::string &_dev)
{
	/* Search the dev_map if we already have a manager for the
	 * given device */
	if (!dev_map.empty()) {
		for (auto it = dev_map.begin(); it != dev_map.end(); ++it) {
			if (it->first.compare(_dev) != 0)
				continue;

			/* We found the entry. */
			if (!it->second.expired())
				return it->second.lock();
			else
				break;
		}
	}
	return nullptr;
}

std::shared_ptr<iio_manager> iio_manager::get_instance(
		struct iio_context *ctx, const std::string &_dev,
		unsigned long buffer_size)
{
	auto instance = has_instance(_dev);
	if (instance) {
		return instance;
	}

	/* No manager found - create a new one */
	auto manager = new iio_manager(_id++, ctx, _dev, buffer_size);
    std::shared_ptr<iio_manager> shared_manager(manager);

	/* Add it to the map */
	auto it = dev_map.insert(std::pair<const std::string, map_entry>(
				_dev, shared_manager));
	if (it.second == false)
		it.first->second = shared_manager;

	return shared_manager;
}

iio_manager::port_id iio_manager::connect(basic_block_sptr dst,
		int src_port, int dst_port, bool use_float,
		unsigned long _buffer_size)
{
	std::unique_lock<std::mutex> lock(copy_mutex);

	/* The copy block is used as a valve to turn on/off this
	 * specific channel. */
	auto copy = blocks::copy::make(sizeof(short));
	copy_blocks.push_back(std::make_pair(copy, _buffer_size));

	/* Disable the valve by default. */
	copy->set_enabled(false);

	/* Connect the IIO block to the valve, and the valve to the
	 * destination block */
	iio_manager::connect(freq_comp_filt[src_port][1], 0, copy, 0);

	/* TODO: Find a way to share one short_to_float block per channel,
	 * instead of having each client instanciate its own */
	if (use_float) {
		auto s2f = blocks::short_to_float::make();
		iio_manager::connect(copy, 0, s2f, 0);
		iio_manager::connect(s2f, 0, dst, dst_port);
	} else {
		iio_manager::connect(copy, 0, dst, dst_port);
	}

	/* Returns an ID that identifies the connection to the port,
	 * as there can be multiple blocks connected to one port */
	return copy;
}

void iio_manager::disconnect(iio_manager::port_id copy)
{
	std::unique_lock<std::mutex> lock(copy_mutex);

	copy->set_enabled(false);

	for (auto it = copy_blocks.begin(); it != copy_blocks.end(); ++it) {
		if (it->first == copy) {
			copy_blocks.erase(it);
			break;
		}
	}

	del_connection(copy, false);
	hier_block2::disconnect(copy);
}

void iio_manager::update_buffer_size_unlocked()
{
	unsigned long size = 0;

	for (auto it = copy_blocks.begin(); it != copy_blocks.end(); ++it) {
		if (it->first->enabled() && size < it->second)
			size = it->second;
	}

	if (size) {
		iio_block->set_buffer_size(size);
		if (m_mixed_source) {
			m_mixed_source->set_buffer_size(size);
		}
		this->buffer_size = size;
	}
}

void iio_manager::start(iio_manager::port_id copy)
{
	std::unique_lock<std::mutex> lock(copy_mutex);

	if (copy->enabled())
		return;

	qDebug(CAT_M2K_IIO_MANAGER) << "Enabling copy block" << copy->alias().c_str();
	copy->set_enabled(true);

	update_buffer_size_unlocked();

	if (!_started) {
		qDebug(CAT_M2K_IIO_MANAGER) << "Starting top block";
		top_block::start();
	}

	_started = true;
}

void iio_manager::set_filter_parameters(int channel, int index, bool enable, float TC, float gain, float sample_rate )
{
	freq_comp_filt[channel][index]->set_enable(enable);
	freq_comp_filt[channel][index]->set_TC(TC);
	freq_comp_filt[channel][index]->set_filter_gain(gain);
	freq_comp_filt[channel][index]->set_sample_rate(sample_rate);
}

void iio_manager::stop(iio_manager::port_id copy)
{
	std::unique_lock<std::mutex> lock(copy_mutex);
	bool inuse = false;

	if (!_started || !copy->enabled())
		return;

	qDebug(CAT_M2K_IIO_MANAGER) << "Disabling copy block" << copy->alias().c_str();
	copy->set_enabled(false);

	/* Verify whether all blocks are disabled */
	for (auto it = copy_blocks.cbegin();
			!inuse && it != copy_blocks.cend(); ++it)
		inuse = it->first->enabled();

	if (!inuse) {
		qDebug(CAT_M2K_IIO_MANAGER) << "Stopping top block";
		top_block::stop();
		top_block::wait();

		_started = false;
	} else {
		update_buffer_size_unlocked();
	}
}

void iio_manager::stop_all()
{
	for (auto it = copy_blocks.begin(); it != copy_blocks.end(); ++it)
		stop(it->first);
}

void iio_manager::connect(gr::basic_block_sptr src, int src_port,
		gr::basic_block_sptr dst, int dst_port)
{
	struct connection entry;
	entry.src = src;
	entry.dst = dst;
	entry.src_port = src_port;
	entry.dst_port = dst_port;

	connections.push_back(entry);
	hier_block2::connect(src, src_port, dst, dst_port);
}

void iio_manager::disconnect(basic_block_sptr src, int src_port,
		basic_block_sptr dst, int dst_port)
{
	for (auto it = connections.begin(); it != connections.end(); ++it) {
		if (it->src == src && it->dst == dst &&
				it->src_port == src_port &&
				it->dst_port == dst_port) {
			connections.erase(it);
			break;
		}
	}

	hier_block2::disconnect(src, src_port, dst, dst_port);
}

void iio_manager::del_connection(gr::basic_block_sptr block, bool reverse)
{
	bool found;

	do {
		found = false;

		for (auto it = connections.begin();
				it != connections.end(); ++it) {
			if (reverse) {
				if (block != it->dst || it->src == freq_comp_filt[0][1] || it->src == freq_comp_filt[1][1])
					continue;
			} else if (block != it->src) {
				continue;
			}

			qDebug(CAT_M2K_IIO_MANAGER) << "Removing" <<
				(reverse ? "backwards" : "forward")
				<< "connection between"
				<< it->src->alias().c_str()
				<< "port" << it->src_port << "and"
				<< it->dst->alias().c_str()
				<< "port" << it->dst_port;
			hier_block2::disconnect(it->src, it->src_port,
					it->dst, it->dst_port);

			auto src = it->src, dst = it->dst;
			connections.erase(it);
			if (reverse)
				del_connection(src, true);
			else
				del_connection(dst, true);
			found = true;
			break;
		}
	} while (found);

	if (reverse)
		del_connection(block, false);
}

void iio_manager::set_buffer_size(iio_manager::port_id copy, unsigned long size)
{
	std::unique_lock<std::mutex> lock(copy_mutex);

	for (auto it = copy_blocks.begin(); it != copy_blocks.end(); ++it) {
		if (it->first == copy) {
			it->second = size;
			break;
		}
	}

	update_buffer_size_unlocked();
}

void iio_manager::got_timeout()
{
	Q_EMIT timeout();
}

void iio_manager::set_device_timeout(unsigned int mseconds)
{
	iio_block->set_timeout_ms(mseconds);
	if (m_mixed_source) {
		m_mixed_source->set_timeout_ms(mseconds);
	}
}

void iio_manager::set_data_rate(double rate) {
	iio_block->set_data_rate(rate);
}

void iio_manager::set_kernel_buffer_count(int kb) {
	if(kb) {
		m_analogin->setKernelBuffersCount(kb);
	} else {
		m_analogin->setKernelBuffersCount(KERNEL_BUFFERS_DEFAULT);
	}
}

void iio_manager::enableMixedSignal(m2k::mixed_signal_source::sptr mixed_source)
{
	for (size_t i = 0; i < nb_channels; ++i) {
		hier_block2::disconnect(iio_block, i, freq_comp_filt[i][0], 0);
		hier_block2::connect(mixed_source, i, freq_comp_filt[i][0], 0);
	}

	hier_block2::msg_disconnect(iio_block, "msg", timeout_b, "msg");
	hier_block2::msg_connect(mixed_source, "msg", timeout_b, "msg");

	m_mixed_source = mixed_source;
}

void iio_manager::disableMixedSignal(m2k::mixed_signal_source::sptr mixed_source)
{
	for (size_t i = 0; i < nb_channels; ++i) {
		hier_block2::disconnect(mixed_source, i, freq_comp_filt[i][0], 0);
		hier_block2::connect(iio_block, i, freq_comp_filt[i][0], 0);
	}

	hier_block2::msg_disconnect(mixed_source, "msg", timeout_b, "msg");
	hier_block2::msg_connect(iio_block, "msg", timeout_b, "msg");

	m_mixed_source = nullptr;
	try {
		m_analogin->setKernelBuffersCount(KERNEL_BUFFERS_DEFAULT);
	} catch (libm2k::m2k_exception &e) {		
		qDebug() << e.what();
	}
}
