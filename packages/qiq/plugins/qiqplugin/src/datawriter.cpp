#include "datawriter.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_DATA_WRITER, "DataWriter");

using namespace scopy::qiqplugin;

DataWriter::DataWriter(QObject *parent)
	: QObject(parent)
{}

DataWriter::~DataWriter() { unmap(); }

uchar *DataWriter::mappedData() { return m_data; }

bool DataWriter::openFile(const QString &path, int64_t dataSize)
{
	m_file.setFileName(path);
	if(!m_file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
		return false;
	}
	if(m_file.size() < dataSize) {
		if(!m_file.resize(dataSize)) {
			m_file.close();
			return false;
		}
	}
	m_dataSize = dataSize;
	m_data = m_file.map(0, m_dataSize);
	if(!m_data) {
		return false;
	}
	m_file.close();
	return true;
}

void DataWriter::unmap()
{
	if(m_data) {
		m_file.unmap(m_data);
		m_data = nullptr;
	}
	m_file.close();
}
