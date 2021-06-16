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

#include <QDebug>

#include <cassert>
#include <libsigrokdecode/libsigrokdecode.h>
#include <scopy/gui/logicanalyzer/logic_decoder.hpp>

using std::map;
using std::string;
using namespace scopy::logic;

Decoder::Decoder(const srd_decoder* const dec)
	: m_decoder_(dec)
	, m_shown_(true)
	, m_decoderInst_(nullptr)
{}

Decoder::~Decoder()
{
	for (auto& option : m_options_)
		g_variant_unref(option.second);
}

const srd_decoder* Decoder::decoder() const { return m_decoder_; }

bool Decoder::shown() const { return m_shown_; }

void Decoder::show(bool show) { m_shown_ = show; }

const vector<DecodeChannel*>& Decoder::channels() const { return m_channels_; }

void Decoder::setChannels(vector<DecodeChannel*> channels) { m_channels_ = channels; }

const map<string, GVariant*>& Decoder::options() const { return m_options_; }

void Decoder::setOption(const char* id, GVariant* value)
{
	assert(value);
	g_variant_ref(value);
	m_options_[id] = value;

	// If we have a decoder instance, apply option value immediately
	applyAllOptions();
}

void Decoder::applyAllOptions()
{
	if (m_decoderInst_) {
		GHashTable* const opt_hash =
			g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

		for (const auto& option : m_options_) {
			GVariant* const value = option.second;
			g_variant_ref(value);
			g_hash_table_replace(opt_hash, (void*)g_strdup(option.first.c_str()), value);
		}

		srd_inst_option_set(m_decoderInst_, opt_hash);
		g_hash_table_destroy(opt_hash);
	}
}

bool Decoder::haveRequiredChannels() const
{
	for (DecodeChannel* ch : m_channels_)
		if (!ch->assignedSignal && !ch->isOptional)
			return false;

	return true;
}

srd_decoder_inst* Decoder::createDecoderInst(srd_session* session)
{
	GHashTable* const opt_hash =
		g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

	for (const auto& option : m_options_) {
		GVariant* const value = option.second;
		g_variant_ref(value);
		g_hash_table_replace(opt_hash, (void*)g_strdup(option.first.c_str()), value);
	}

	if (m_decoderInst_)
		qDebug() << "WARNING: previous decoder instance" << m_decoderInst_ << "exists";

	m_decoderInst_ = srd_inst_new(session, m_decoder_->id, opt_hash);
	g_hash_table_destroy(opt_hash);

	if (!m_decoderInst_)
		return nullptr;

	// Setup the channels
	GArray* const init_pin_states = g_array_sized_new(false, true, sizeof(uint8_t), m_channels_.size());

	g_array_set_size(init_pin_states, m_channels_.size());

	GHashTable* const channels =
		g_hash_table_new_full(g_str_hash, g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

	for (DecodeChannel* ch : m_channels_) {
		if (!ch->assignedSignal)
			continue;

		init_pin_states->data[ch->id] = ch->initialPinState;

		GVariant* const gvar = g_variant_new_int32(ch->bitId); // bit_id = bit position
		g_variant_ref_sink(gvar);
		// key is channel name (pdch->id), value is bit position in each sample (gvar)
		g_hash_table_insert(channels, ch->pdch_->id, gvar);
	}

	srd_inst_channel_set_all(m_decoderInst_, channels);

	//    g_hash_table_remove_all(channels);
	//    g_hash_table_unref(channels);

	srd_inst_initial_pins_set_all(m_decoderInst_, init_pin_states);
	g_array_free(init_pin_states, true);

	return m_decoderInst_;
}

void Decoder::invalidateDecoderInst() { m_decoderInst_ = nullptr; }
