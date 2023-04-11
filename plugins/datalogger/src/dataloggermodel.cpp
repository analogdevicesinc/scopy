#include "dataloggermodel.h"

using namespace adiscope;

DataLoggerModel::DataLoggerModel():
	m_path(""),
	m_timer(new QTimer())
{
	QObject::connect(m_timer, &QTimer::timeout , this, [=](){
		QtConcurrent::run(this, &DataLoggerModel::logData);
	});
}

void DataLoggerModel::setPath(QString path)
{
	m_path = path;
}

void DataLoggerModel::setTimerInterval(double interval)
{
	m_timer->setInterval(interval);
}

void DataLoggerModel::startLogger(bool overwrite)
{
	m_timer->start();
	//when overwrite also create file header
	if (overwrite) {
		FileManager *fm = new FileManager("Data Logger");
		fm->open(m_path, FileManager::EXPORT);

		QStringList chNames;
		if (!m_channels.empty()) {
			for (auto ch: m_channels.keys()) {
				chNames.push_back(ch);
			}
		}
		QMap<QString,QVector<QString>> dataToLog;

		dataToLog["Timestamp"]= {};
		if (!m_channels.empty()) {
			for (auto ch: m_channels.keys()) {
				dataToLog[ch]= {};
			}
		}
		fm->writeToFile(overwrite,dataToLog);
	}
}

void DataLoggerModel::stopLogger()
{
	if (m_timer->isActive()) {
		m_timer->stop();
	}
}

void DataLoggerModel::createChannel(QString name, CHANNEL_DATA_TYPE type, CHANNEL_FILTER filter)
{
	m_channels[name].type = type;
	m_channels[name].filter = filter;
}

void DataLoggerModel::destroyChannel(QString name)
{
	m_channels.remove(name);
}

void DataLoggerModel::resetChannel(QString name)
{
	m_channels[name].values.clear();
}

void DataLoggerModel::receiveValue(QString name, QString value)
{
	m_channels[name].values.push_back(value);
}

void DataLoggerModel::logData()
{
	QMap<QString,QVector<QString>> dataToLog;
	QString data = "";

	for (auto ch: m_channels.keys()) {
		if (ch == "Timestamp") {
			continue;
		}
		data = "";
		//apply filter on stored data
		if (m_channels[ch].filter == CHANNEL_FILTER::LAST_VALUE && !m_channels[ch].values.empty()) {
			data += m_channels[ch].values.last();
			dataToLog[ch].push_back(data);
			resetChannel(ch);
			continue;
		}
		if (m_channels[ch].filter == CHANNEL_FILTER::AVERAGE && !m_channels[ch].values.empty()) {
			double avg = computeAvg(m_channels[ch].values);
			dataToLog[ch].push_back(QString::number(avg));
			resetChannel(ch);
			continue;
		}
		if (m_channels[ch].filter == CHANNEL_FILTER::ALL && !m_channels[ch].values.empty()) {
			for(auto v : m_channels[ch].values){
				data += v + " ";
			}
			dataToLog[ch].push_back(data);
			resetChannel(ch);
		}
		resetChannel(ch);
	}

	FileManager *fm = new FileManager("Data Logger");
	fm->open(m_path, FileManager::EXPORT);

	QStringList chNames;
	if (!m_channels.empty()) {
		for (auto ch: m_channels.keys()) {
			chNames.push_back(ch);
		}
	}
	dataToLog["Timestamp"]={QDateTime::currentDateTime().time().toString()};
	fm->writeToFile(false,dataToLog);
}

double DataLoggerModel::computeAvg(QVector<QString> values)
{
	double result = 0;
	for (QString v : values) {
		assert(isNumber(v));
		result += v.toDouble();
	}
	result = result / values.size();
	return result;
}

bool DataLoggerModel::isNumber(const QString& str)
{
	for (QChar const &c : str) {
		if (!( c.isDigit() || c == ".")) return false;
	}
	return true;
}

DataLoggerModel::~DataLoggerModel()
{
	delete m_timer;
}
