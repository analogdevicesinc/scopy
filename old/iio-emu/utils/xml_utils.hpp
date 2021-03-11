/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of iio-emu
 * (see http://www.github.com/analogdevicesinc/iio-emu).
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

#ifndef IIO_EMU_XML_UTILS_HPP
#define IIO_EMU_XML_UTILS_HPP

#include <QDomNode>
#include <tinyiiod/tinyiiod.h>

namespace iio_emu {

QDomNode getNode(QDomNode node, const char *tag, const char *attr_name, const char *attr_value);

QDomNode getDeviceAttr(QDomDocument *doc, const char *device, enum iio_attr_type type, const char *attr);

QDomNode getChannelAttr(QDomDocument *doc, const char *chn, const char *dev, const char *attr, bool ch_out);

size_t getXml(QDomDocument *doc, char **buf);

}
#endif //IIO_EMU_XML_UTILS_HPP
