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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <cassert>

#include "logic.hpp"
#include "logicsegment.hpp"

using std::deque;
using std::max;
using std::shared_ptr;
using std::vector;
using std::lock_guard;

namespace pv {
namespace data {

Logic::Logic(unsigned int num_channels) :
	SignalData(),
	num_channels_(num_channels)
{
	assert(num_channels_ > 0);
}

unsigned int Logic::num_channels() const
{
	return num_channels_;
}

void Logic::push_segment(
	shared_ptr<LogicSegment> &segment)
{
	lock_guard<boost::shared_mutex> lock(segments_mutex);
	segments_.push_front(segment);
}

const deque< shared_ptr<LogicSegment> >& Logic::logic_segments() const
{
	lock_guard<boost::shared_mutex> lock(segments_mutex);
	return segments_;
}

vector< shared_ptr<Segment> > Logic::segments() const
{
	lock_guard<boost::shared_mutex> lock(segments_mutex);
	return vector< shared_ptr<Segment> >(
		segments_.begin(), segments_.end());
}

void Logic::clear()
{
	lock_guard<boost::shared_mutex> lock(segments_mutex);
	segments_.clear();
}

void Logic::clear_old_data()
{
	lock_guard<boost::shared_mutex> lock(segments_mutex);
	if(segments_.size() > 1) {
		segments_.erase(segments_.begin()+1, segments_.end());
	}
}

uint64_t Logic::max_sample_count() const
{
	uint64_t l = 0;
	std::shared_ptr<LogicSegment> s = segments_[0];
	assert(s);
	l = max(l, s->get_sample_count());
//	for (std::shared_ptr<LogicSegment> s : segments_) {
//		assert(s);
//		l = max(l, s->get_sample_count());
//	}
	return l;
}

} // namespace data
} // namespace pv
