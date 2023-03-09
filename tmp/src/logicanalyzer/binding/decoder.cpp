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


#include <libsigrokdecode/libsigrokdecode.h>

#include <boost/none_t.hpp>

//#include <adiscope/data/decodesignal.hpp>
#include "../prop/double.hpp"
#include "../prop/enum.hpp"
#include "../prop/int.hpp"
#include "../prop/string.hpp"

#include "../logic_analyzer.h"
#include "decoder.hpp"
#include <glib.h>
#include <QVariant>

using boost::none;
using std::map;
using std::pair;
using std::shared_ptr;
using std::string;
using std::vector;

using adiscope::prop::Double;
using adiscope::prop::Enum;
using adiscope::prop::Int;
using adiscope::prop::Property;
using adiscope::prop::String;


namespace adiscope {
namespace bind {

Decoder::Decoder(
    adiscope::AnnotationDecoder *annDecoder,
    shared_ptr<adiscope::logic::Decoder> decoder) :
    m_annDecoder(annDecoder),
	decoder_(decoder)
{
	assert(decoder_);

	const srd_decoder *const dec = decoder_->decoder();
	assert(dec);

	for (GSList *l = dec->options; l; l = l->next) {
		const srd_decoder_option *const opt =
			(srd_decoder_option*)l->data;

		const QString name = QString::fromUtf8(opt->desc);

		const Property::Getter get = [&, opt]() {
			return getter(opt->id); };
		const Property::Setter set = [&, opt](QVariant value) {
			setter(opt->id, value); };

		shared_ptr<Property> prop;

		if (opt->values)
			prop = bind_enum(name, "", opt, get, set);
		else if (g_variant_is_of_type(opt->def, G_VARIANT_TYPE("d")))
			prop = shared_ptr<Property>(new Double(name, "", 2, "",
				none, none, get, set));
		else if (g_variant_is_of_type(opt->def, G_VARIANT_TYPE("x")))
			prop = shared_ptr<Property>(
				new Int(name, "", "", none, get, set, g_variant_classify(opt->def)));
		else if (g_variant_is_of_type(opt->def, G_VARIANT_TYPE("s")))
			prop = shared_ptr<Property>(
				new String(name, "", get, set));
		else
			continue;

		properties_.push_back(prop);
	}
}


shared_ptr<Property> Decoder::bind_enum(
	const QString &name, const QString &desc,
	const srd_decoder_option *option,
	Property::Getter getter, Property::Setter setter)
{
	vector< pair<QVariant, QString> > values;
	for (GSList *l = option->values; l; l = l->next) {
		QVariant var = gVariantToQVariant((GVariant*)l->data);
		values.emplace_back(var, print_variant(var));
	}

	return shared_ptr<Property>(new Enum(name, desc, values, getter, setter));
}

QVariant Decoder::gVariantToQVariant(GVariant *value)
{
	if (!value)
		return QVariant();

	auto classify = g_variant_classify(value);
	switch (classify) {
	case G_VARIANT_CLASS_BOOLEAN:
		return QVariant::fromValue(bool(g_variant_get_boolean(value)));
	case G_VARIANT_CLASS_BYTE:
		return QVariant::fromValue(char(g_variant_get_byte(value)));
	case G_VARIANT_CLASS_INT16:
		return QVariant::fromValue<int16_t>(g_variant_get_int16(value));
	case G_VARIANT_CLASS_UINT16:
		return QVariant::fromValue<uint16_t>(g_variant_get_uint16(value));
	case G_VARIANT_CLASS_INT32:
		return QVariant::fromValue<int32_t>(g_variant_get_int32(value));
	case G_VARIANT_CLASS_UINT32:
		return QVariant::fromValue<uint32_t>(g_variant_get_uint32(value));
	case G_VARIANT_CLASS_INT64:
		return QVariant::fromValue<int64_t>(g_variant_get_int64(value));
	case G_VARIANT_CLASS_UINT64:
		return QVariant::fromValue<uint64_t>(g_variant_get_uint64(value));
	case G_VARIANT_CLASS_DOUBLE:
		return QVariant::fromValue(g_variant_get_double(value));
	case G_VARIANT_CLASS_STRING: {
		QVariant variant = QVariant::fromValue(QString::fromUtf8(g_variant_get_string(value, 0)));
		return variant;
	}
	case G_VARIANT_CLASS_VARIANT: {
		GVariant *gvariant = g_variant_get_variant(value);
		QVariant qvariant = gVariantToQVariant(gvariant);
		g_variant_unref(gvariant);
		return qvariant;
	}
	case G_VARIANT_CLASS_TUPLE: {
		// we only handle the (dd) case in enum type property (just 2 children)
		if (g_variant_n_children(value) == 2) {
			std::vector<double> doubleVector;
			for (int i = 0; i < g_variant_n_children(value); i++) {
				GVariant *childVariant = g_variant_get_child_value(value, i);
				QVariant qVariant = gVariantToQVariant(childVariant);
				if (qVariant.isValid()) {
					g_variant_unref(childVariant);
					return QVariant();
				}
				doubleVector.push_back(qVariant.toDouble());
				g_variant_unref(childVariant);
			}
			if (doubleVector.size() != 2) {
				return QVariant();
			}
			return QVariant::fromValue<std::pair<double, double>>(
				std::pair<double, double>(doubleVector.at(0), doubleVector.at(1)));
		}
		return QVariant();
	}
	default: {
		const GVariantType *type = g_variant_get_type(value);
		if (g_variant_type_equal(type, G_VARIANT_TYPE_BYTESTRING)) {
			return QVariant::fromValue(QByteArray(g_variant_get_bytestring(value)));
		}
	}
	}

	return QVariant();
}


QVariant Decoder::getter(const char *id)
{
	GVariant *val = nullptr;
	QVariant qVal;

	assert(decoder_);

	// Get the value from the hash table if it is already present
	const map<string, GVariant*>& options = decoder_->options();
	const auto iter = options.find(id);

	if (iter != options.end()) {
		val = (*iter).second;
	} else {
		assert(decoder_->decoder());

		// Get the default value if not
		for (GSList *l = decoder_->decoder()->options; l; l = l->next) {
			const srd_decoder_option *const opt =
				(srd_decoder_option*)l->data;
			if (strcmp(opt->id, id) == 0) {
				val = opt->def;
				break;
			}
		}
	}

	qVal = gVariantToQVariant(val);

	return qVal;
}

void Decoder::setter(const char *id, QVariant value)
{
	assert(decoder_);
	GVariant *variant;

	QString prop_val = "";
	auto type = value.type();
	if (type == QMetaType::QString) {
		variant = g_variant_new_string(value.toString().toUtf8().constData());
	} else if (type == QMetaType::Bool) {
		variant = g_variant_new_boolean(value.toBool());
	} else if (type == QMetaType::Char) {
		variant = g_variant_new_byte(value.toChar().toLatin1());
	} else if (type == QMetaType::Short) {
		variant = g_variant_new_int16(value.value<int16_t>());
	} else if (type == QMetaType::UShort) {
		variant = g_variant_new_uint16(value.value<uint16_t>());
	} else if (type == QMetaType::Int) {
		variant = g_variant_new_int32(value.value<int32_t>());
	} else if (type == QMetaType::UInt) {
		variant = g_variant_new_uint32(value.value<uint32_t>());
	} else if (type == QMetaType::Long) {
		variant = g_variant_new_int64(value.value<int64_t>());
	} else if (type == QMetaType::LongLong) {
		variant = g_variant_new_int64(value.value<int64_t>());
	} else if (type == QMetaType::ULong) {
		variant = g_variant_new_uint64(value.value<uint64_t>());
	} else if (type == QMetaType::ULongLong) {
		variant = g_variant_new_uint64(value.value<uint64_t>());
	} else if (type == QMetaType::Double) {
		variant = g_variant_new_double(value.toDouble());
	} else {
		// Check if we got a tuple (dd) from the decoders
		bool pairDD = value.canConvert<std::pair<double, double>>();
		if (pairDD) {
			auto pairData = value.value<std::pair<double, double>>();
			GVariant *tupleVariants[] = {
				g_variant_new_double(pairData.first),
				g_variant_new_double(pairData.second)
			};
			variant = g_variant_new_tuple(tupleVariants, 2);
		} else {
			variant = g_variant_new_string(value.toString().toUtf8().constData());
		}
	}

	decoder_->set_option(id, variant);
	assert(m_annDecoder);
	m_annDecoder->startDecode();
}

}  // namespace binding
}  // namespace adiscope
