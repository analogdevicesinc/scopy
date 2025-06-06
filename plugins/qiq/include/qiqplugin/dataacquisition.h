#ifndef DATAACQUISITION_H
#define DATAACQUISITION_H

#include <QObject>
#include <qfile.h>
#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/analog/m2kanalogout.hpp>
#include "scopy-qiqplugin_config.h"
#include <QDir>

using namespace std;
using namespace libm2k;
using namespace libm2k::analog;
using namespace libm2k::context;

#define SAMPLES_PER_CHANNEL 1024 * 10

namespace scopy::qiqplugin {
class DataAcquisition : public QObject
{
	Q_OBJECT
public:
	DataAcquisition(QObject *parent = nullptr);
	~DataAcquisition();

	void readDeviceData();

Q_SIGNALS:
	void dataAvailable(const double *data, const int &dataSize, const QString &path);

private:
	M2k *m_ctx;
	M2kAnalogIn *m_ain;
	M2kAnalogOut *m_aout;

	QFile m_file;
	uchar *m_mappedData;
	const QString FILE_PATH = QString(QIQPLUGIN_RES_PATH) + QDir::separator() + "device_data.bin";

	void setupAnalogIn();
	void setupAnalogOut();
	void createOutputBuffers();
	void writeToMappedFile(const double *data, int dataSize, const QString &filePath);
};
} // namespace scopy::qiqplugin

#endif // DATAACQUISITION_H
