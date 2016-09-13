/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "iio_manager.hpp"

#include <QDebug>

#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/short_to_float.h>

#include <iio.h>

using namespace adiscope;
using namespace gr;

std::map<const std::string, iio_manager::map_entry> iio_manager::dev_map;
unsigned iio_manager::_id = 0;

iio_manager::iio_manager(unsigned int block_id,
		struct iio_context *ctx, const std::string &_dev,
		unsigned long _buffer_size) :
	top_block("IIO Manager " + std::to_string(block_id)),
	id(block_id), _started(false), buffer_size(_buffer_size)
{
	if (!ctx)
		throw std::runtime_error("IIO context not created");

	struct iio_device *dev = iio_context_find_device(ctx, _dev.c_str());
	if (!dev)
		throw std::runtime_error("Device not found");

	unsigned int nb_channels = iio_device_get_channels_count(dev);

	iio_block = iio::device_source::make_from(ctx, _dev,
			std::vector<std::string>(), _dev,
			std::vector<std::string>(),
			_buffer_size);

	/* Avoid unconnected channel errors by connecting a dummy sink */
	auto dummy_copy = blocks::copy::make(sizeof(short));
	auto dummy = blocks::null_sink::make(sizeof(short));
	for (unsigned i = 0; i < nb_channels; i++) {
		hier_block2::connect(iio_block, i, dummy_copy, i);
		hier_block2::connect(dummy_copy, i, dummy, i);
	}

	dummy_copy->set_enabled(true);
}

iio_manager::~iio_manager()
{
}

boost::shared_ptr<iio_manager> iio_manager::get_instance(
		struct iio_context *ctx, const std::string &_dev,
		unsigned long buffer_size)
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

	/* No manager found - create a new one */
	auto manager = new iio_manager(_id++, ctx, _dev, buffer_size);
	boost::shared_ptr<iio_manager> shared_manager(manager);

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
	copy_mutex.lock();

	/* The copy block is used as a valve to turn on/off this
	 * specific channel. */
	auto copy = blocks::copy::make(sizeof(short));
	copy_blocks.push_back(copy);

	/* Disable the valve by default. */
	copy->set_enabled(false);

	/* Connect the IIO block to the valve, and the valve to the
	 * destination block */
	connect(iio_block, src_port, copy, 0);

	/* TODO: Find a way to share one short_to_float block per channel,
	 * instead of having each client instanciate its own */
	if (use_float) {
		auto s2f = blocks::short_to_float::make();
		connect(copy, 0, s2f, 0);
		connect(s2f, 0, dst, dst_port);
	} else {
		connect(copy, 0, dst, dst_port);
	}

	buffer_mutex.lock();
	if (_buffer_size > this->buffer_size)
		this->set_buffer_size_unlocked(_buffer_size);

	buffer_sizes.push_back(_buffer_size);

	buffer_mutex.unlock();
	copy_mutex.unlock();

	/* Returns an ID that identifies the connection to the port,
	 * as there can be multiple blocks connected to one port */
	return copy;
}

void iio_manager::disconnect(iio_manager::port_id copy)
{
	copy_mutex.lock();

	copy->set_enabled(false);

	auto pos = std::find(copy_blocks.begin(), copy_blocks.end(), copy);
	copy_blocks.erase(pos);

	del_connection(copy);
	hier_block2::disconnect(copy);

	buffer_mutex.lock();
	buffer_sizes.erase(buffer_sizes.begin() + (pos - copy_blocks.begin()));

	unsigned long size = 0;
	for (auto it = buffer_sizes.begin(); it != buffer_sizes.end(); ++it)
		if (*it > size)
			size = *it;
	if (!size)
		size = IIO_BUFFER_SIZE;
	if (size != this->buffer_size)
		this->set_buffer_size_unlocked(size);

	buffer_mutex.unlock();
	copy_mutex.unlock();
}

void iio_manager::start(iio_manager::port_id copy)
{
	copy_mutex.lock();

	if (copy->enabled())
		goto unlock;

	if (!_started) {
		qDebug() << "Starting top block";
		top_block::start();
	}

	qDebug() << "Enabling copy block" << copy->alias().c_str();
	copy->set_enabled(true);
	_started = true;

unlock:
	copy_mutex.unlock();
}

void iio_manager::stop(iio_manager::port_id copy)
{
	bool inuse = false;

	copy_mutex.lock();

	if (!_started)
		goto unlock;

	if (!copy->enabled())
		goto unlock;

	qDebug() << "Disabling copy block" << copy->alias().c_str();
	copy->set_enabled(false);

	/* Verify whether all blocks are disabled */
	for (auto it = copy_blocks.cbegin();
			!inuse && it != copy_blocks.cend(); ++it)
		inuse = (*it)->enabled();

	if (!inuse) {
		qDebug() << "Stopping top block";
		top_block::stop();
		top_block::wait();
		_started = false;
	}

unlock:
	copy_mutex.unlock();
}

void iio_manager::stop_all()
{
	for (auto it = copy_blocks.begin(); it != copy_blocks.end(); ++it)
		stop(*it);
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
				it->src_port == src_port && it->dst_port == dst_port) {
			connections.erase(it);
			break;
		}
	}

	hier_block2::disconnect(src, src_port, dst, dst_port);
}

void iio_manager::del_connection(gr::basic_block_sptr block)
{
	bool found;

	do {
		found = false;

		for (auto it = connections.begin();
				it != connections.end(); ++it) {
			if (block != it->src)
				continue;

			qDebug() << "Removing forward connection between"
				<< it->src->alias().c_str()
				<< "port" << it->src_port << "and"
				<< it->dst->alias().c_str()
				<< "port" << it->dst_port;
			hier_block2::disconnect(it->src, it->src_port,
					it->dst, it->dst_port);

			auto src = it->src, dst = it->dst;
			connections.erase(it);
			del_connection(dst);
			found = true;
			break;
		}
	} while (found);
}

void iio_manager::set_buffer_size_unlocked(unsigned long size)
{
	iio_block->set_buffer_size((unsigned int) size);
	this->buffer_size = size;
}

void iio_manager::set_buffer_size(iio_manager::port_id copy, unsigned long size)
{
	buffer_mutex.lock();
	if (size > this->buffer_size)
		this->set_buffer_size_unlocked(size);

	auto pos = std::find(copy_blocks.begin(), copy_blocks.end(), copy);
	buffer_sizes[pos - copy_blocks.begin()] = size;

	buffer_mutex.unlock();
}
