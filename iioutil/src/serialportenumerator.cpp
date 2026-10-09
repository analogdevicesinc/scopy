/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "serialportenumerator.h"

#include <libserialport.h>

using namespace scopy;

QMap<QString, QString> SerialPortEnumerator::getSerialPortsName()
{
	QMap<QString, QString> serialPortsName;
	struct sp_port **serialPorts;
	int retCode = sp_list_ports(&serialPorts);
	if(retCode == SP_OK) {
		for(int i = 0; serialPorts[i]; i++) {
			QString fullDescription = "";
			QString fullName = "";

			char *p_name = sp_get_port_name(serialPorts[i]);
			if(p_name) {
				fullName += "[ ";
				fullName += QString(p_name);
				fullName += " ]";
			}

			char *p_description = sp_get_port_description(serialPorts[i]);
			if(p_description) {
				fullName += " ( ";
				fullName += QString(p_description);
				fullName += " )";

				fullDescription += QString(p_description);
				fullDescription += "\n";
			}

			int usb_bus, usb_addr;
			retCode = sp_get_port_usb_bus_address(serialPorts[i], &usb_bus, &usb_addr);
			if(retCode == SP_OK) {
				fullDescription += "USB bus: ";
				fullDescription += QString::number(usb_bus);
				fullDescription += " USB address: ";
				fullDescription += QString::number(usb_addr);
				fullDescription += "\n";
			}

			int usb_vid, usb_pid;
			retCode = sp_get_port_usb_vid_pid(serialPorts[i], &usb_vid, &usb_pid);
			if(retCode == SP_OK) {
				fullDescription += "USB VID: ";
				fullDescription += QString::number(usb_vid, 16);
				fullDescription += " USB PID: ";
				fullDescription += QString::number(usb_pid, 16);
				fullDescription += "\n";
			}

			char *p_manufacturer = sp_get_port_usb_manufacturer(serialPorts[i]);
			if(p_manufacturer) {
				fullDescription += "Manufacturer: ";
				fullDescription += QString(p_manufacturer);
				fullDescription += "\n";
			}

			char *p_usbProd = sp_get_port_usb_product(serialPorts[i]);
			if(p_usbProd) {
				fullDescription += "USB Product: ";
				fullDescription += QString(p_usbProd);
				fullDescription += "\n";
			}

			char *p_usbSerial = sp_get_port_usb_serial(serialPorts[i]);
			if(p_usbSerial) {
				fullDescription += "USB Serial: ";
				fullDescription += QString(p_usbSerial);
				fullDescription += "\n";
			}

			char *p_btAddr = sp_get_port_bluetooth_address(serialPorts[i]);
			if(p_btAddr) {
				fullDescription += "Bluetooth MAC address: ";
				fullDescription += QString(p_btAddr);
				fullDescription += "\n";
			}
			serialPortsName.insert(fullName, fullDescription);
		}
		sp_free_port_list(serialPorts);
	}
	return serialPortsName;
}
