/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (c) 2020 Analog Devices Inc.
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

#ifndef DECODER_H
#define DECODER_H

#include <QString>

#include <glib.h>
#include <map>
#include <memory>
#include <scopy/gui/logicanalyzer/annotation.hpp>
#include <scopy/gui/logicanalyzer/row.hpp>
#include <scopy/gui/logicanalyzer/row_data.hpp>
#include <set>
#include <vector>

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

struct srd_decoder;
struct srd_decoder_inst;
struct srd_channel;
struct srd_session;

namespace scopy {
namespace logic {
class Decoder;
}
} // namespace scopy

struct DecodeChannel
{
	uint16_t id;	///< Global numerical ID for the decode channels in the stack
	uint16_t bitId; ///< Tells which bit within a sample represents this channel
	const bool isOptional;
	bool assignedSignal;
	const QString name, desc;
	int initialPinState;
	const shared_ptr<scopy::logic::Decoder> decoder_;
	const srd_channel* pdch_;
};

struct DecodeSegment
{
	map<const Row, RowData> annotationRows;
	//    pv::util::Timestamp start_time;
	double samplerate;
	int64_t samplesDecodedIncl, samplesDecodedExcl;
};

namespace scopy {
namespace logic {

class Decoder
{
public:
	Decoder(const srd_decoder* const dec);

	virtual ~Decoder();

	const srd_decoder* decoder() const;

	bool shown() const;
	void show(bool show = true);

	const vector<DecodeChannel*>& channels() const;
	void setChannels(vector<DecodeChannel*> channels);

	const map<string, GVariant*>& options() const;

	void setOption(const char* id, GVariant* value);

	void applyAllOptions();

	bool haveRequiredChannels() const;

	srd_decoder_inst* createDecoderInst(srd_session* session);
	void invalidateDecoderInst();

private:
	const srd_decoder* const m_decoder_;

	bool m_shown_;

	vector<DecodeChannel*> m_channels_;
	map<string, GVariant*> m_options_;
	srd_decoder_inst* m_decoderInst_;
};

} // namespace logic
} // namespace scopy
#endif // DECODER_H
