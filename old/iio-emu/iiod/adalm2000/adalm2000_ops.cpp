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

#include "adalm2000_ops.hpp"
#include "utils/attr_ops_xml.hpp"
#include "utils/network_ops.hpp"
#include "devices/m2k_adc.hpp"
#include "devices/m2k_dac.hpp"
#include "devices/m2k_logic_rx.hpp"
#include "devices/m2k_logic_tx.hpp"
#include <QFile>

using namespace iio_emu;

static QFile *file;
static QDomDocument *doc;
static Socket *current_socket;

// devices
static M2kADC *adc;
static M2kDAC *dac_a;
static M2kDAC *dac_b;
static M2kLogicRX *logic_rx;
static M2kLogicTX *logix_tx;
static std::vector<AbstractDevice *> devices;

static char *ctx_xml;
static ssize_t xml_size;


Adalm2000Ops::Adalm2000Ops() : AbstractOps()
{
	file = new QFile(":/iiod/adalm2000/adalm2000.xml");
	file->open(QIODevice::ReadOnly | QIODevice::Text);
	doc = new QDomDocument();
	doc->setContent(file);
	file->close();

	// devices
	adc = new M2kADC("iio:device0", doc);
	dac_a = new M2kDAC("iio:device6", doc);
	dac_b = new M2kDAC("iio:device7", doc);
	logic_rx = new M2kLogicRX("iio:device10", doc);
	logix_tx = new M2kLogicTX("iio:device9", doc);
	devices = {adc, dac_a, dac_b, logic_rx, logix_tx};

	//connections
	adc->connectDevice(0, dac_a, 0);
	adc->connectDevice(1, dac_b, 0);
	logic_rx->connectDevice(0, logix_tx, 0);

	xml_size = getXml(doc, &ctx_xml);

	m_iiodops->get_xml = iio_get_xml;
	m_iiodops->write = write;
	//m_iiodops->read_line = read_line;
	m_iiodops->read = read;

	m_iiodops->read_attr = iio_read_attr;
	m_iiodops->write_attr = iio_write_attr;
	m_iiodops->ch_read_attr = iio_ch_read_attr;
	m_iiodops->ch_write_attr = iio_ch_write_attr;

//	m_iiodops->transfer_dev_to_mem = iio_transfer_dev_to_mem;
	m_iiodops->read_data = iio_read_dev;
	m_iiodops->transfer_mem_to_dev = iio_transfer_mem_to_dev;
	m_iiodops->write_data = iio_write_dev;
	m_iiodops->set_buffers_count = iio_set_buffers_count;

	m_iiodops->open = iio_open_dev;
	m_iiodops->close = iio_close_dev;
	m_iiodops->get_mask = iio_get_mask;
}

Adalm2000Ops::~Adalm2000Ops()
{
	delete m_iiodops;
	delete doc;
	delete file;
	delete ctx_xml;

	delete adc;
	delete dac_a;
	delete dac_b;
	delete logic_rx;
	delete logix_tx;
}

ssize_t Adalm2000Ops::iio_get_xml(char **outxml)
{
	if (!outxml) {
		return -1;
	}

	*outxml = ctx_xml;
	return xml_size;
}

ssize_t Adalm2000Ops::write(const char *buf, size_t len)
{
	current_socket->write(buf, len);
	return 0;
}

ssize_t Adalm2000Ops::read_line(char *buf, size_t len)
{
	return socket_read_line(current_socket, buf, len);
}

ssize_t Adalm2000Ops::read(char *buf, size_t len)
{
	return socket_read(current_socket, buf, len);
}

ssize_t Adalm2000Ops::iio_read_attr(const char *device_id, const char *attr,
				    char *buf, size_t len,
				    enum iio_attr_type type)
{
	return read_device_attr(doc, device_id, attr, buf, len, type);
}

ssize_t Adalm2000Ops::iio_write_attr(const char *device_id, const char *attr, const char *buf, size_t len,
				     enum iio_attr_type type)
{
	return write_dev_attr(doc, device_id, attr, buf, len, type);
}

ssize_t Adalm2000Ops::iio_ch_read_attr(const char *device_id, const char *channel, bool ch_out, const char *attr,
				       char *buf, size_t len)
{
	return read_channel_attr(doc, device_id, channel, ch_out, attr, buf, len);
}

ssize_t Adalm2000Ops::iio_ch_write_attr(const char *device_id, const char *channel, bool ch_out, const char *attr,
					const char *buf, size_t len)
{
	return write_channel_attr(doc, device_id, channel, ch_out, attr, buf, len);
}

int32_t Adalm2000Ops::iio_open_dev(const char *device, size_t sample_size, uint32_t mask, bool cyclic)
{
	AbstractDevice *abstractDevice = getDevice(device);
	if (abstractDevice) {
		return abstractDevice->open_dev(sample_size, mask, cyclic);
	}
	return -ENOENT;
}

ssize_t Adalm2000Ops::iio_read_dev(const char *device, char *pbuf, size_t offset, size_t bytes_count)
{
	AbstractDevice *abstractDevice = getDevice(device);
	if (abstractDevice) {
		auto *deviceIn = reinterpret_cast<AbstractDeviceIn *>(abstractDevice);
		return deviceIn->read_dev(pbuf, offset, bytes_count);
	}
	return -ENOENT;
}

int32_t Adalm2000Ops::iio_set_buffers_count(const char *device, uint32_t buffers_count)
{
	AbstractDevice *abstractDevice = getDevice(device);
	if (abstractDevice) {
		return abstractDevice->set_buffers_count(buffers_count);
	}
	return -ENOENT;
}

int32_t Adalm2000Ops::iio_get_mask(const char *device, uint32_t *mask)
{
	AbstractDevice *abstractDevice = getDevice(device);
	if (abstractDevice) {
		return abstractDevice->get_mask(mask);
	}
	return -ENOENT;
}

ssize_t Adalm2000Ops::iio_transfer_mem_to_dev(const char *device, size_t bytes_count)
{
	AbstractDevice *abstractDevice = getDevice(device);
	if (abstractDevice) {
		auto *deviceOut = reinterpret_cast<AbstractDeviceOut *>(abstractDevice);
		return deviceOut->transfer_mem_to_dev(bytes_count);
	}
	return -ENOENT;
}

ssize_t Adalm2000Ops::iio_write_dev(const char *device, const char *buf, size_t offset, size_t bytes_count)
{
	AbstractDevice *abstractDevice = getDevice(device);
	if (abstractDevice) {
		auto *deviceOut = reinterpret_cast<AbstractDeviceOut *>(abstractDevice);
		return deviceOut->write_dev(buf, offset, bytes_count);
	}
	return -ENOENT;
}

int32_t Adalm2000Ops::iio_close_dev(const char *device)
{
	AbstractDevice *abstractDevice = getDevice(device);
	if (abstractDevice) {
		return abstractDevice->close_dev();
	}
	return -ENOENT;
}

void Adalm2000Ops::setCurrentSocket(Socket *s)
{
	current_socket = s;
}

AbstractDevice *Adalm2000Ops::getDevice(const char *device_id)
{
	AbstractDevice *device = nullptr;

	for (auto d : devices) {
		if (!strcmp(d->getDeviceId(), device_id)) {
			device = d;
			break;
		}
	}
	return device;
}
