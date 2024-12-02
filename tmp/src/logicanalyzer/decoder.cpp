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

#include "decoder.h"

#include <libsigrokdecode/libsigrokdecode.h>

#include <QDebug>
#include <cassert>

using std::map;
using std::string;

using namespace adiscope;
using namespace adiscope::logic;

Decoder::Decoder(const srd_decoder *const dec) :
    decoder_(dec),
    shown_(true),
    decoder_inst_(nullptr)
{

}

Decoder::~Decoder()
{
    for (auto& option : options_)
        g_variant_unref(option.second);
}

const srd_decoder* Decoder::decoder() const
{
    return decoder_;
}

bool Decoder::shown() const
{
    return shown_;
}

void Decoder::show(bool show)
{
    shown_ = show;
}

const vector<DecodeChannel*>& Decoder::channels() const
{
    return channels_;
}

void Decoder::set_channels(vector<DecodeChannel*> channels)
{
    channels_ = channels;
}

const map<string, GVariant*>& Decoder::options() const
{
    return options_;
}

void Decoder::set_option(const char *id, GVariant *value)
{
    assert(value);
    g_variant_ref(value);
    options_[id] = value;

    // If we have a decoder instance, apply option value immediately
    apply_all_options();
}

void Decoder::apply_all_options()
{
    if (decoder_inst_) {
        GHashTable *const opt_hash = g_hash_table_new_full(g_str_hash,
            g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

        for (const auto& option : options_) {
            GVariant *const value = option.second;
            g_variant_ref(value);
            g_hash_table_replace(opt_hash, (void*)g_strdup(
                option.first.c_str()), value);
        }

        srd_inst_option_set(decoder_inst_, opt_hash);
        g_hash_table_destroy(opt_hash);
    }
}

bool Decoder::have_required_channels() const
{
    for (DecodeChannel *ch : channels_)
        if (!ch->assigned_signal && !ch->is_optional)
            return false;

    return true;
}

srd_decoder_inst* Decoder::create_decoder_inst(srd_session *session)
{
    GHashTable *const opt_hash = g_hash_table_new_full(g_str_hash,
        g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

    for (const auto& option : options_) {
        GVariant *const value = option.second;
        g_variant_ref(value);
        g_hash_table_replace(opt_hash, (void*)g_strdup(
            option.first.c_str()), value);
    }

    if (decoder_inst_)
        qDebug() << "WARNING: previous decoder instance" << decoder_inst_ << "exists";

    decoder_inst_ = srd_inst_new(session, decoder_->id, opt_hash);
    g_hash_table_destroy(opt_hash);

    if (!decoder_inst_)
        return nullptr;

    // Setup the channels
    GArray *const init_pin_states = g_array_sized_new(false, true,
        sizeof(uint8_t), channels_.size());

    g_array_set_size(init_pin_states, channels_.size());

    GHashTable *const channels = g_hash_table_new_full(g_str_hash,
        g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

    for (DecodeChannel *ch : channels_) {
        if (!ch->assigned_signal)
            continue;

        init_pin_states->data[ch->id] = ch->initial_pin_state;

        GVariant *const gvar = g_variant_new_int32(ch->bit_id);  // bit_id = bit position
        g_variant_ref_sink(gvar);
        // key is channel name (pdch->id), value is bit position in each sample (gvar)
        g_hash_table_insert(channels, ch->pdch_->id, gvar);
    }

    srd_inst_channel_set_all(decoder_inst_, channels);

//    g_hash_table_remove_all(channels);
//    g_hash_table_unref(channels);

    srd_inst_initial_pins_set_all(decoder_inst_, init_pin_states);
    g_array_free(init_pin_states, true);

    return decoder_inst_;
}

void Decoder::invalidate_decoder_inst()
{
    decoder_inst_ = nullptr;
}
