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

#include <vector>
#include "xml_utils.hpp"
#include <QRegExp>

using namespace iio_emu;

static char xml_doctype[] = "<!DOCTYPE context ["
			    "<!ELEMENT context (device | context-attribute)*>"
			    "<!ELEMENT context-attribute EMPTY>"
			    "<!ELEMENT device (channel | attribute | debug-attribute | buffer-attribute)*>"
			    "<!ELEMENT channel (scan-element? , attribute*)>"
			    "<!ELEMENT attribute EMPTY>"
			    "<!ELEMENT scan-element EMPTY>"
			    "<!ELEMENT debug-attribute EMPTY>"
			    "<!ELEMENT buffer-attribute EMPTY>"
			    "<!ATTLIST context name CDATA #REQUIRED>"
			    "<!ATTLIST context description CDATA #IMPLIED>"
			    "<!ATTLIST context-attribute name CDATA #REQUIRED>"
			    "<!ATTLIST context-attribute value CDATA #REQUIRED>"
			    "<!ATTLIST device id CDATA #REQUIRED>"
			    "<!ATTLIST device name CDATA #IMPLIED>"
			    "<!ATTLIST channel id CDATA #REQUIRED>"
			    "<!ATTLIST channel type (input | output) #REQUIRED>"
			    "<!ATTLIST channel name CDATA #IMPLIED>"
			    "<!ATTLIST scan-element index CDATA #REQUIRED>"
			    "<!ATTLIST scan-element format CDATA #REQUIRED>"
			    "<!ATTLIST scan-element scale CDATA #IMPLIED>"
			    "<!ATTLIST attribute name CDATA #REQUIRED >"
			    "<!ATTLIST attribute filename CDATA #IMPLIED>"
			    "<!ATTLIST attribute value CDATA #IMPLIED>"
			    "<!ATTLIST debug-attribute name CDATA #REQUIRED>"
			    "<!ATTLIST debug-attribute value CDATA #IMPLIED>"
			    "<!ATTLIST buffer-attribute name CDATA #REQUIRED>"
			    "<!ATTLIST buffer-attribute value CDATA #IMPLIED>"
			    "]>";

QDomNode iio_emu::getNode(QDomNode node, const char *tag, const char *attr_name, const char *attr_value)
{

	while (!node.isNull()) {
		QDomElement element = node.toElement();
		if (!element.isNull()) {
			if (element.tagName() == tag && element.attribute(attr_name) == attr_value) {
				return node;
			}
		}
		node = node.nextSibling();
	}
	return QDomNode();
}

static QDomNode getNodeNAttrs(QDomNode node, const char *tag, std::vector<const char *> attr_names,
			      std::vector<const char *> attr_values)
{
	if (attr_names.size() != attr_values.size()) {
		return QDomNode();
	}

	bool found = true;
	while (!node.isNull()) {
		QDomElement element = node.toElement();
		if (!element.isNull()) {
			if (element.tagName() == tag) {
				for (int i = 0; i < attr_names.size(); i++) {
					if (element.attribute(attr_names[i]) != attr_values[i]) {
						found = false;
						break;
					}
					found = true;
				}
				if (found) {
					return node;
				}
			}

		}
		node = node.nextSibling();
	}
	return QDomNode();
}

QDomNode iio_emu::getDeviceAttr(QDomDocument *doc, const char *device, enum iio_attr_type type, const char *attr)
{
	QDomNode root = doc->documentElement().firstChild();
	QDomNode node_device = getNode(root, "device", "id", device);
	if (node_device.isNull()) {
		return node_device;
	}
	QDomNode node_attr;
	switch (type) {
		case IIO_ATTR_TYPE_DEVICE:
			node_attr = getNode(node_device.firstChild(),
					    "attribute", "name",
					    attr);
			break;
		case IIO_ATTR_TYPE_DEBUG:
			node_attr = getNode(node_device.firstChild(),
					    "debug-attribute",
					    "name", attr);
			break;
		case IIO_ATTR_TYPE_BUFFER:
			node_attr = getNode(node_device.firstChild(),
					    "buffer-attribute",
					    "name", attr);
			break;
	}
	return node_attr;
}

QDomNode iio_emu::getChannelAttr(QDomDocument *doc, const char *chn, const char *dev,
			const char *attr, bool ch_out)
{

	if (doc->isNull()) {
		return QDomNode();
	}

	const char *output = ch_out ? "output" : "input";
	std::vector<const char *> attrs_names = {"id", "type"};
	std::vector<const char *> attrs_values = {chn, output};

	QDomNode root = doc->documentElement().firstChild();
	QDomNode node_device = getNode(root, "device", "id", dev);

	if (node_device.isNull()) {
		return QDomNode();
	}

	QDomNode node_channel = getNodeNAttrs(node_device.firstChild(),
					      "channel", attrs_names,
					      attrs_values);

	if (node_channel.isNull()) {
		return QDomNode();
	}

	QDomNode node_attr = getNode(node_channel.firstChild(), "attribute",
				     "name", attr);

	return node_attr;
}

size_t iio_emu::getXml(QDomDocument *doc, char **buf)
{
	QString xml = doc->toString();
	xml.replace(QRegExp("<!DOCTYPE context>"), xml_doctype);
	xml.remove(QRegExp("\n"));

	*buf = new char[xml.size()]();
	memcpy(*buf, xml.toLocal8Bit().data(), xml.size());

	return xml.size();
}
