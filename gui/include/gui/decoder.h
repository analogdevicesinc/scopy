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

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <glib.h>
#include <QString>
#include <memory>

#include "annotation.h"
#include "row.h"
#include "rowdata.h"
#include "scopygui_export.h"

using std::map;
using std::string;
using std::vector;
using std::shared_ptr;

struct srd_decoder;
struct srd_decoder_inst;
struct srd_channel;
struct srd_session;

namespace adiscope {
namespace logic {
class Decoder;
}
}

struct SCOPYGUI_EXPORT DecodeChannel
{
    uint16_t id;     ///< Global numerical ID for the decode channels in the stack
    uint16_t bit_id; ///< Tells which bit within a sample represents this channel
    const bool is_optional;
    bool assigned_signal;
    const QString name, desc;
    int initial_pin_state;
	const shared_ptr<adiscope::logic::Decoder> decoder_;
    const srd_channel *pdch_;
};

struct SCOPYGUI_EXPORT DecodeSegment
{
    map<const Row, RowData> annotation_rows;
//    pv::util::Timestamp start_time;
    double samplerate;
    int64_t samples_decoded_incl, samples_decoded_excl;
};

namespace adiscope {
namespace logic {

class SCOPYGUI_EXPORT Decoder
{
public:
    Decoder(const srd_decoder *const dec);

    virtual ~Decoder();

    const srd_decoder* decoder() const;

    bool shown() const;
    void show(bool show = true);

    const vector<DecodeChannel*>& channels() const;
    void set_channels(vector<DecodeChannel*> channels);

    const map<string, GVariant*>& options() const;

    void set_option(const char *id, GVariant *value);

    void apply_all_options();

    bool have_required_channels() const;

    srd_decoder_inst* create_decoder_inst(srd_session *session);
    void invalidate_decoder_inst();

private:
    const srd_decoder *const decoder_;

    bool shown_;

    vector<DecodeChannel*> channels_;
    map<string, GVariant*> options_;
    srd_decoder_inst *decoder_inst_;
};

}
}
#endif // DECODER_H
