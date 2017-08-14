/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012-14 Joel Holdsworth <joel@airwebreathe.org.uk>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef PULSEVIEW_PV_SESSION_HPP
#define PULSEVIEW_PV_SESSION_HPP

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
// Windows: Avoid boost/thread namespace pollution (which includes windows.h).
#define NOGDI
#define NORESOURCE
#endif
#include <boost/thread/shared_mutex.hpp>

#include <QObject>
#include <QString>

#include "util.hpp"

struct srd_decoder;
struct srd_channel;

namespace sigrok {
class Analog;
class Channel;
class Device;
class Logic;
class Meta;
class Packet;
class Session;
}

namespace pv {

class DeviceManager;

namespace data {
class Analog;
class AnalogSegment;
class Logic;
class LogicSegment;
class SignalData;
}

namespace devices {
class Device;
}

namespace view {
class DecodeTrace;
class LogicSignal;
class Signal;
}

class Session : public QObject
{
	Q_OBJECT

public:
	enum capture_state {
		Stopped,
		AwaitingTrigger,
		Running
	};

public:
	Session(DeviceManager &device_manager);

	~Session();

	DeviceManager& device_manager();

	const DeviceManager& device_manager() const;

	std::shared_ptr<sigrok::Session> session() const;

	std::shared_ptr<devices::Device> device() const;

	/**
	 * Sets device instance that will be used in the next capture session.
	 */
    void set_device(std::shared_ptr<devices::Device> device);

	void set_default_device();

	capture_state get_capture_state() const;

	void start_capture(std::function<void (const QString)> error_handler);

	void stop_capture();

	std::set< std::shared_ptr<data::SignalData> > get_data() const;

	double get_samplerate() const;

	const std::unordered_set< std::shared_ptr<view::Signal> >
		signals() const;

 
	bool add_decoder(srd_decoder *const dec);

	std::vector< std::shared_ptr<view::DecodeTrace> >
		get_decode_signals() const;

	void remove_decode_signal(view::DecodeTrace *signal);

	std::shared_ptr<pv::view::DecodeTrace> add_decoder();

	void remove_decode_clones();

	void remove_signal_clones();

	void add_signal(std::shared_ptr<view::LogicSignal> sig);

	std::shared_ptr<view::LogicSignal> create_signal_from_id(int index, int height);

	void set_buffersize(size_t value);

	void set_entire_buffersize(size_t value);

	void set_screen_mode(bool value);

	bool is_screen_mode();

	void set_samplerate(double value);

	void set_timeSpan(double value);

	void set_timespanLimit(double value);

	bool is_data();

	int get_logic_sample_count();

	int get_logic_active_sample();

private:
	void set_capture_state(capture_state state);

	void update_signals();

	std::shared_ptr<view::Signal> signal_from_channel(
		std::shared_ptr<sigrok::Channel> channel) const;

private:
	void sample_thread_proc(std::shared_ptr<devices::Device> device,
		std::function<void (const QString)> error_handler);

	void feed_in_header();

	void feed_in_meta(std::shared_ptr<sigrok::Meta> meta);

	void feed_in_trigger();

	void feed_in_frame_begin();

	void feed_in_logic(std::shared_ptr<sigrok::Logic> logic);

	void feed_in_analog(std::shared_ptr<sigrok::Analog> analog);

	void data_feed_in(std::shared_ptr<sigrok::Device> device,
		std::shared_ptr<sigrok::Packet> packet);

private:
	DeviceManager &device_manager_;
	std::shared_ptr<devices::Device> device_;

	std::vector< std::shared_ptr<view::DecodeTrace> > decode_traces_;

	mutable std::mutex sampling_mutex_; //!< Protects access to capture_state_.
	capture_state capture_state_;

	mutable boost::shared_mutex signals_mutex_;
	std::unordered_set< std::shared_ptr<view::Signal> > signals_;

	mutable std::recursive_mutex data_mutex_;
	std::shared_ptr<data::Logic> logic_data_;
	uint64_t cur_samplerate_;
	std::shared_ptr<data::LogicSegment> cur_logic_segment_;
	std::map< std::shared_ptr<sigrok::Channel>, std::shared_ptr<data::AnalogSegment> >
		cur_analog_segments_;

	std::thread sampling_thread_;

	bool out_of_memory_;

	size_t buffersize_;

	size_t entire_buffersize_;

	bool screen_mode_;

	double timeSpan;

	double timespanLimitStream;

Q_SIGNALS:
	void capture_state_changed(int state);
	void device_selected();

	void signals_changed();

	void trigger_event(util::Timestamp location);

	void frame_began();

	void data_received();

	void frame_ended();
};

} // namespace pv
#endif // PULSEVIEW_PV_SESSION_HPP
