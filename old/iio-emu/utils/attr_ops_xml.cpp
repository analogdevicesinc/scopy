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

#include "attr_ops_xml.hpp"


ssize_t iio_emu::read_device_attr(QDomDocument *doc, const char *device_id, const char *attr, char *buf, size_t len,
				  enum iio_attr_type type)
{
	if (doc->isNull()) {
		return -ENOENT;
	}

	QDomNode node_attr = getDeviceAttr(doc, device_id, type, attr);

	if (node_attr.isNull()) {
		return -ENOENT;
	}

	const char *value = node_attr.toElement().attribute("value").toLocal8Bit().data();
	memcpy(buf, value, strnlen(value, len) + 1);

	return (strnlen(buf, len) + 1);
}

ssize_t iio_emu::write_dev_attr(QDomDocument *doc, const char *device_id, const char *attr, const char *buf, size_t len,
				enum iio_attr_type type)
{
	if (doc->isNull()) {
		return -ENOENT;
	}

	QDomNode node_attr = getDeviceAttr(doc, device_id, type, attr);

	if (node_attr.isNull()) {
		return -ENOENT;
	}

	node_attr.toElement().setAttribute("value", buf);

	return (strnlen(buf, len) + 1);
}

ssize_t iio_emu::read_channel_attr(QDomDocument *doc, const char *device_id, const char *channel, bool ch_out,
				   const char *attr, char *buf, size_t len)
{
	if (doc->isNull()) {
		return -ENOENT;
	}

	QDomNode node_attr = getChannelAttr(doc, channel, device_id, attr, ch_out);

	if (node_attr.isNull()) {
		return -ENOENT;
	}

	const char *value = node_attr.toElement().attribute("value").toLocal8Bit().data();
	memcpy(buf, value, strnlen(value, len) + 1);

	return (strnlen(buf, len) + 1);
}

ssize_t iio_emu::write_channel_attr(QDomDocument *doc, const char *device_id, const char *channel, bool ch_out,
				    const char *attr, const char *buf, size_t len)
{
	if (doc->isNull()) {
		return -ENOENT;
	}

	QDomNode node_attr = getChannelAttr(doc, channel, device_id, attr, ch_out);

	if (node_attr.isNull()) {
		return -ENOENT;
	}

	node_attr.toElement().setAttribute("value", buf);

	return (strnlen(buf, len) + 1);
}
