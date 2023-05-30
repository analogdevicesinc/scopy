#include "libserialportsupport.h"
#include "scopy-iioutil_config.h"
#include <iio.h>
#include <libserialport.h>
using namespace scopy;

LibSerialPortSupport::LibSerialPortSupport(QObject *parent)
{}

LibSerialPortSupport::~LibSerialPortSupport()
{}

bool LibSerialPortSupport::hasLibSerialSupport()
{
#ifdef WITH_LIBSERIALPORT
	return true;
#endif
	return false;
}

QVector<QString> LibSerialPortSupport::getSerialPortsName()
{
	QVector<QString> serialPortsName;
	struct sp_port** serialPorts;
	int retCode = sp_list_ports(&serialPorts);
	if (retCode == SP_OK) {
		for (int i = 0; serialPorts[i]; i++) {
		      serialPortsName.push_back(QString(sp_get_port_name(serialPorts[i])));
		}
		sp_free_port_list(serialPorts);
	}
	return serialPortsName;
}
