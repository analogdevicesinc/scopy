#include "dataloggerreaderthread.hpp"
#include <libm2k/analog/dmm.hpp>

DataLoggerReaderThread::DataLoggerReaderThread()
{

}

void DataLoggerReaderThread::setDataLoggerStatus(bool status)
{
	dataLoggerStatus = status;
}

void DataLoggerReaderThread::channelToggled(int id, bool toggled)
{
	m_activeChannels[id].first = toggled;
}

void DataLoggerReaderThread::addChannel(int id, std::string dmmId, libm2k::analog::DMM *dmm)
{
	m_activeChannels[id].first = true;
	m_activeChannels[id].second.dmmId = dmmId;
	m_activeChannels[id].second.dmm = dmm;
}

void DataLoggerReaderThread::run()
{
	if (!m_activeChannels.empty()) {
		for (int ch : m_activeChannels.keys()) {
			//check if channel was closed before reading
			if (m_activeChannels[ch].first) {
				auto updatedRead = m_activeChannels[ch].second.dmm->readChannel(m_activeChannels[ch].second.dmmId);
				Q_EMIT updateChannelData(ch, updatedRead.value,QString::fromStdString(updatedRead.unit_name),
						  QString::fromStdString(updatedRead.unit_symbol));
				if (dataLoggerStatus) {
					Q_EMIT updateDataLoggerValue(QString::fromStdString(m_activeChannels[ch].second.dmm->getName() + ":" + m_activeChannels[ch].second.dmmId),
								     QString::number(updatedRead.value));
				}
			}
		}
	}
}
