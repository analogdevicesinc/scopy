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


#ifndef PULSEVIEW_PV_BINDING_DECODER_HPP
#define PULSEVIEW_PV_BINDING_DECODER_HPP

#include "binding.hpp"

#include "../prop/property.hpp"

#include "../annotationdecoder.h"

using std::shared_ptr;

struct srd_decoder_option;

namespace adiscope {
namespace logic {
class Decoder;
}
}

namespace adiscope {

namespace bind {

class Decoder : public Binding
{
public:
	Decoder(adiscope::AnnotationDecoder *annDec,
	shared_ptr<adiscope::logic::Decoder> decoder);

private:
	static shared_ptr<adiscope::prop::Property> bind_enum(const QString &name,
		const QString &desc, const srd_decoder_option *option,
		prop::Property::Getter getter, prop::Property::Setter setter);

	Glib::VariantBase getter(const char *id);

	void setter(const char *id, Glib::VariantBase value);

private:
	adiscope::AnnotationDecoder *m_annDecoder;
    shared_ptr<logic::Decoder> decoder_;
};

}  // namespace binding
}  // namespace pv

#endif // PULSEVIEW_PV_BINDING_DECODER_HPP
