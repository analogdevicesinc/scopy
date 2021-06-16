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

#include <scopy/gui/logicanalyzer/annotation_decoder.hpp>
#include <scopy/gui/logicanalyzer/binding/binding.hpp>
#include <scopy/gui/logicanalyzer/prop/property.hpp>

using std::shared_ptr;

struct srd_decoder_option;

namespace scopy {
namespace logic {
class Decoder;
}
} // namespace scopy

namespace scopy {
namespace bind {

class Decoder : public Binding
{
public:
	Decoder(gui::AnnotationDecoder* annDec, shared_ptr<scopy::logic::Decoder> decoder);

private:
	static shared_ptr<scopy::prop::Property> bind_enum(const QString& name, const QString& desc,
							   const srd_decoder_option* option,
							   prop::Property::Getter getter,
							   prop::Property::Setter setter);

	Glib::VariantBase getter(const char* id);

	void setter(const char* id, Glib::VariantBase value);

private:
	gui::AnnotationDecoder* m_annDecoder;
	shared_ptr<logic::Decoder> m_decoder_;
};

} // namespace bind
} // namespace scopy

#endif // PULSEVIEW_PV_BINDING_DECODER_HPP
