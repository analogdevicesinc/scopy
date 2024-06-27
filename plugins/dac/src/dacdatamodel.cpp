#include "dacdatamodel.h"
#include "txnode.h"
#include <QDebug>

using namespace scopy;
DacDataModel::DacDataModel(struct iio_device *dev, QObject *parent)
	: QObject(parent)
{
	m_dev = dev;
	m_name = iio_device_get_name(m_dev);

	m_isBufferCapable = initBufferDac();
	m_isDds = initDdsDac();
}

DacDataModel::~DacDataModel()
{
	deinitBufferDac();
	deinitDdsDac();
}

QString DacDataModel::getName() const
{
	return m_name;
}

bool DacDataModel::isBufferCapable() const
{
	return m_isBufferCapable;
}

bool DacDataModel::isDds() const
{
	return m_isDds;
}

QMap<QString, TxNode*> DacDataModel::getDdsTxs() const
{
	return m_ddsTxs;
}

void DacDataModel::setCyclic(bool cyclic)
{
	m_cyclicBuffer = cyclic;
}

bool DacDataModel::initBufferDac()
{
	unsigned int txCount = 0;
	unsigned int channelCount = iio_device_get_channels_count(m_dev);
	for (unsigned int i = 0; i < channelCount; i++) {
		struct iio_channel *chn = iio_device_get_channel(m_dev, i);
		if (!iio_channel_is_output(chn))
			continue;
		if (iio_channel_is_scan_element(chn)) {
			txCount++;
			QString id = iio_channel_get_id(chn);
			m_bufferTxs.append(new TxNode(id, chn, this));
		}
	}
	return (txCount != 0);
}


/**
 * multiple scenarios here:
 * TX1_I_F1, TX1_I_F2, TX1_Q_F1, TX1_Q_F2 -> one channel, dual tone, complex
 * or
 * 1A, 1B -> one channel, dual tone, not complex
 *
 * https://github.com/analogdevicesinc/linux/blob/main/drivers/iio/frequency/cf_axi_dds.c#L1204-L1601
*/
bool DacDataModel::initDdsDac()
{
	unsigned int ddsTonesCount = 0;
	unsigned int channelCount = iio_device_get_channels_count(m_dev);
	for (unsigned int i = 0; i < channelCount; i++) {
		struct iio_channel * chn = iio_device_get_channel(m_dev, i);
		if (!iio_channel_is_output(chn))
			continue;
		iio_chan_type chnType = iio_channel_get_type(chn);
		if (chnType != IIO_ALTVOLTAGE)
			continue;
		ddsTonesCount++;

		// Name should contain TX*_I/Q_F or *A/*B
		QString name = iio_channel_get_name(chn);
		QString id = iio_channel_get_id(chn);
		if (name == "") {
			name = generateToneName(id);
		}
		QStringList txNodesNames = generateTxNodesForChannel(name);
		QString txNameCurrent = txNodesNames.at(0);
		TxNode *parentTxNode = m_ddsTxs.value(txNameCurrent, nullptr);
		if (!parentTxNode) {
			parentTxNode = new TxNode(txNameCurrent, nullptr, this);
			m_ddsTxs.insert(txNameCurrent, parentTxNode);
		}
		if (txNodesNames.size() == 2) {
			QString toneName = txNodesNames.at(1);
			parentTxNode->addChildNode(toneName, chn);
		} else if (txNodesNames.size() == 3) {
			QString complexChnName = txNodesNames.at(1);
			QString toneName = txNodesNames.at(2);
			TxNode *complexChnNode = parentTxNode->addChildNode(complexChnName, nullptr);
			complexChnNode->addChildNode(toneName, chn);
		}
	}
	return (ddsTonesCount != 0);
}

// returns a list of nodes - this is specifically ordered
// for channels of 1A/B type it returns: {"1", "1A/B"}
// for channels of TX*_I/Q_F* type it returns: {"TX*", "TX_I/Q", "TX*_I/Q_F*"}
QStringList DacDataModel::generateTxNodesForChannel(QString name)
{
	QStringList txNodes;
	QStringList id_list = name.split("_");
	QString tone_uuid = name;
	QString complex_chn_uuid = "";
	QString tx_uuid = "";
	if (id_list.size() == 3) { // TX*_Q/I_F* complex format
		complex_chn_uuid = name.mid(0, name.size() - id_list[2].size() - 1); //TX*_Q/I
		tx_uuid = id_list[0];
		txNodes << tx_uuid;
		txNodes << complex_chn_uuid;
	} else if (id_list.size() == 1) { // *A/*B non-complex format
		tx_uuid = name[0];
		txNodes << tx_uuid;
	}
	txNodes << tone_uuid;
	return txNodes;
}

QString DacDataModel::generateToneName(QString chnId)
{
	bool ok;
	QString name = "";
	int idx = chnId.indexOf(toneId);
	if (idx != -1) {
		int chnIndex = chnId.mid(idx + toneId.size()).toInt(&ok);
		if (!ok)
			return name;
		int txIndex = (chnIndex / MAX_NB_TONES) + 1; // TX indexing from 1
		int toneIndex = (chnIndex % 2) + 1; // assuming dual tones, indexing from 1
		QString typeIQ = (chnIndex & 0x02) ? Q_CHANNEL : I_CHANNEL; /* First two indexes are I, next two are Q and so on*/
		name = "TX"+QString::number(txIndex)+"_"+typeIQ+"_F"+QString::number(toneIndex);
	}
	return name;
}

void DacDataModel::deinitBufferDac()
{

}

void DacDataModel::deinitDdsDac()
{

}
