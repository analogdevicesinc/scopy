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

#include <boost/none_t.hpp>
#include <libsigrokdecode/libsigrokdecode.h>

//#include <scopy/data/decodesignal.hpp>
#include <scopy/gui/logicanalyzer/prop/double.hpp>
#include <scopy/gui/logicanalyzer/prop/enum.hpp>
#include <scopy/gui/logicanalyzer/prop/int.hpp>
#include <scopy/gui/logicanalyzer/prop/string.hpp>

//#include "../logic_analyzer.h"
#include <scopy/gui/logicanalyzer/binding/binding_decoder.hpp>

using boost::none;
using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::vector;

using scopy::prop::Double;
using scopy::prop::Enum;
using scopy::prop::Int;
using scopy::prop::Property;
using scopy::prop::String;

namespace scopy {
namespace bind {

Decoder::Decoder(gui::AnnotationDecoder* annDecoder, shared_ptr<scopy::logic::Decoder> decoder)
	: m_annDecoder(annDecoder)
	, m_decoder_(decoder)
{
	assert(m_decoder_);

	const srd_decoder* const dec = m_decoder_->decoder();
	assert(dec);

	for (GSList* l = dec->options; l; l = l->next) {
		const srd_decoder_option* const opt = (srd_decoder_option*)l->data;

		const QString name = QString::fromUtf8(opt->desc);

		const Property::Getter get = [&, opt]() { return getter(opt->id); };
		const Property::Setter set = [&, opt](Glib::VariantBase value) { setter(opt->id, value); };

		shared_ptr<Property> prop;

		if (opt->values)
			prop = bind_enum(name, "", opt, get, set);
		else if (g_variant_is_of_type(opt->def, G_VARIANT_TYPE("d")))
			prop = shared_ptr<Property>(new Double(name, "", 2, "", none, none, get, set));
		else if (g_variant_is_of_type(opt->def, G_VARIANT_TYPE("x")))
			prop = shared_ptr<Property>(new Int(name, "", "", none, get, set));
		else if (g_variant_is_of_type(opt->def, G_VARIANT_TYPE("s")))
			prop = shared_ptr<Property>(new String(name, "", get, set));
		else
			continue;

		properties_.push_back(prop);
	}
}

shared_ptr<Property> Decoder::bind_enum(const QString& name, const QString& desc, const srd_decoder_option* option,
					Property::Getter getter, Property::Setter setter)
{
	vector<pair<Glib::VariantBase, QString>> values;
	for (GSList* l = option->values; l; l = l->next) {
		Glib::VariantBase var = Glib::VariantBase((GVariant*)l->data, true);
		values.emplace_back(var, print_gvariant(var));
	}

	return shared_ptr<Property>(new Enum(name, desc, values, getter, setter));
}

Glib::VariantBase Decoder::getter(const char* id)
{
	GVariant* val = nullptr;

	assert(m_decoder_);

	// Get the value from the hash table if it is already present
	const map<string, GVariant*>& options = m_decoder_->options();
	const auto iter = options.find(id);

	if (iter != options.end())
		val = (*iter).second;
	else {
		assert(m_decoder_->decoder());

		// Get the default value if not
		for (GSList* l = m_decoder_->decoder()->options; l; l = l->next) {
			const srd_decoder_option* const opt = (srd_decoder_option*)l->data;
			if (strcmp(opt->id, id) == 0) {
				val = opt->def;
				break;
			}
		}
	}

	return (val) ? Glib::VariantBase(val, true) : Glib::VariantBase();
}

void Decoder::setter(const char* id, Glib::VariantBase value)
{
	assert(m_decoder_);
	m_decoder_->setOption(id, value.gobj());

	assert(m_annDecoder);
	m_annDecoder->startDecode();
}

} // namespace bind
} // namespace scopy
