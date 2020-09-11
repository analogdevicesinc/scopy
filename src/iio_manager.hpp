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

#ifndef IIO_MANAGER_HPP
#define IIO_MANAGER_HPP

#include <QObject>

#include <gnuradio/top_block.h>
#include <iio/device_source.h>
#include <gnuradio/blocks/copy.h>
#include <gnuradio/blocks/float_to_complex.h>
#include <frequency_compensation_filter.h>

#include <m2k/analog_in_source.h>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/digital/m2kdigital.hpp>

#include <m2k/mixed_signal_source.h>

#include <mutex>

#include "timeout_block.hpp"

/* 1k samples by default */
#define IIO_BUFFER_SIZE 0x400

namespace adiscope {
	class iio_manager : public QObject, public gr::top_block
	{
		Q_OBJECT

	public:
		typedef boost::weak_ptr<iio_manager> map_entry;
		typedef gr::blocks::copy::sptr port_id;

		const unsigned id;

		/* Get a shared pointer to the instance of iio_manager that
		 * manages the requested device */
		static boost::shared_ptr<iio_manager> get_instance(
				struct iio_context *ctx,
				const std::string &dev,
				unsigned long buffer_size = IIO_BUFFER_SIZE);

		~iio_manager();

		/* Connect a block to one of the channels of the IIO source.
		 * This function returns the ID, that can later be used with
		 * start() and stop().
		 * Warning: the flowgraph needs to be locked first! */
		port_id connect(gr::basic_block_sptr dst, int src_port,
				int dst_port, bool use_float = false,
				unsigned long buffer_size = IIO_BUFFER_SIZE);

		/* Connect two regular blocks between themselves. */
		void connect(gr::basic_block_sptr src, int src_port,
				gr::basic_block_sptr dst, int dst_port);

		/* Disconnect the whole tree of blocks connected to this port ID */
		void disconnect(port_id id);

		/* Disconnect two regular blocks. */
		void disconnect(gr::basic_block_sptr src, int src_port,
				gr::basic_block_sptr dst, int dst_port);

		/* Start feeding data to the client connected at [port, id] */
		void start(port_id id);

		/* Stop feeding client at [port, id] */
		void stop(port_id id);

		/* Stop feeding all clients */
		void stop_all();

		/* Returns true if the GNU Radio flowgraph is running */
		bool started() { return _started; }

		/* Change the buffer size at runtime.
		 * Warning: the flowgraph needs to be locked first! */
		void set_buffer_size(port_id id, unsigned long size);
		void set_filter_parameters(int channel, int index, bool enable, float TC, float gain, float sample_rate );

		/* VERY ugly hack. The reconfiguration that happens after
		 * locking/unlocking the flowgraph is sort of broken; the tags
		 * are not properly routed to the blocks connected during the
		 * reconfiguration. So until GNU Radio gets fixed, we just force
		 * the whole flowgraph to stop when connecting new blocks. */
		void lock() {
			gr::top_block::stop();
			// cancel acquisition, work might be blocked waiting for a trigger
			// and the timeout. Don't wait for it just cancel the acquisition
//			m_analogin->cancelAcquisition();
//			m_analogin->stopAcquisition();
//			m_context->stopMixedSignalAcquisition();
//			m_analogin->cancelAcquisition();
//			m_context->getDigital()->cancelAcquisition();

//			m_context->stopMixedSignalAcquisition();

			gr::top_block::wait();
		}
		void unlock() {
//			m_context->startMixedSignalAcquisition(buffer_size);
			gr::top_block::start(); }

		/* Set the timeout for the source device */
		void set_device_timeout(unsigned int mseconds);

		/* Replace the iio_block source with the mixed source */
		void enableMixedSignal(gr::m2k::mixed_signal_source::sptr mixed_source);

		/* Bring back the data from the iio_block source */
		void disableMixedSignal(gr::m2k::mixed_signal_source::sptr mixed_source);

		adiscope::frequency_compensation_filter::sptr freq_comp_filt[2][2];

	private:
		libm2k::analog::M2kAnalogIn *m_analogin;
		libm2k::context::M2k *m_context;

		static std::map<const std::string, map_entry> dev_map;
		static unsigned _id;
		std::mutex copy_mutex;
		bool _started;

		unsigned long buffer_size;
		std::vector<unsigned long> buffer_sizes;

		std::vector<std::pair<port_id, unsigned long> > copy_blocks;

		gr::m2k::analog_in_source::sptr iio_block;
		unsigned int nb_channels;

		boost::shared_ptr<timeout_block> timeout_b;
		gr::m2k::mixed_signal_source::sptr m_mixed_source;

		struct connection {
			gr::basic_block_sptr src;
			gr::basic_block_sptr dst;
			int src_port, dst_port;
		};

		std::vector<connection> connections;

		iio_manager(unsigned int id, struct iio_context *ctx,
				const std::string &dev,
				unsigned long buffer_size);

		void del_connection(gr::basic_block_sptr block, bool reverse);

		void update_buffer_size_unlocked();

	private Q_SLOTS:
		void got_timeout();

	Q_SIGNALS:
		void timeout();
	};
}

#endif /* IIO_MANAGER_HPP */
