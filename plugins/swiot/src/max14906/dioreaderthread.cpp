#include "dioreaderthread.h"
#include "core/logging_categories.h"

using namespace adiscope::swiot;

DioReaderThread::DioReaderThread() {
	qDebug(CAT_MAX14906) << "Creating reader thread for MAX14906";
}

DioReaderThread::~DioReaderThread()
{

}

void DioReaderThread::run() {
	qDebug(CAT_MAX14906) << "DioReaderThread started";
	try {
		if (!this->m_channels.empty()) {
			for (int index: this->m_channels.keys()) {
				double raw = -1;
				iio_channel_attr_read_double(this->m_channels[index], "raw", &raw);
				qDebug(CAT_MAX14906) << "Channel with index " << index << " read raw value: " << raw;
				Q_EMIT channelDataChanged(index, raw);
			}
		}
	} catch (...) {
		qCritical(CAT_MAX14906) << "Failed to acquire data on DioReaderThread";
	}
}

void DioReaderThread::addChannel(int index, struct iio_channel *channel) {
	this->m_channels.insert(index, channel);
}

//void DioReaderThread::toggleChannel(int index, bool toggled) {
//	if (!this->m_channels.contains(index)) {
//		qCritical(CAT_MAX14906) << "No such channel index (index = " << index << ")";
//		return;
//	}
//	this->m_channels[index].first = toggled;
//}
//
//bool DioReaderThread::isChannelToggled(int index) {
//	if (!this->m_channels.contains(index)) {
//		qCritical(CAT_MAX14906) << "No such channel index, returning \"false\" (index = " << index << ")";
//		return false;
//	}
//
//	return this->m_channels[index].first;
//}

void DioReaderThread::singleRun() {
	this->run();
}
