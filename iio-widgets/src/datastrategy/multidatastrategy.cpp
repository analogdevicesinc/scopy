#include "datastrategy/multidatastrategy.h"

Q_LOGGING_CATEGORY(CAT_MULTI_DATA_STRATEGY, "MultiDataStrategy")
using namespace scopy;

MultiDataStrategy::MultiDataStrategy(QList<DataStrategyInterface *> strategies, QWidget *parent)
	: QWidget(parent)
	, m_dataStrategies(strategies)
	, m_data("")
	, m_optionalData("")
	, m_returnCode(0)
{
	for(DataStrategyInterface *ds : m_dataStrategies) {
		QWidget *widgetDS = dynamic_cast<QWidget *>(ds);
		if(!widgetDS) {
			qWarning(CAT_MULTI_DATA_STRATEGY) << "Data strategy not valid.";
			continue;
		}

		m_expectedSignals.insert(widgetDS);
		connect(widgetDS, SIGNAL(sendData(QString, QString)), this, SLOT(receiveSingleRead(QString, QString)));

		// Forward signals from all DS. Caution: these signals are not aggregated.
		connect(widgetDS, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
			SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)));
		connect(widgetDS, SIGNAL(aboutToWrite(QString, QString)), this, SIGNAL(aboutToWrite(QString, QString)));
	}
}

void MultiDataStrategy::addDataStrategy(DataStrategyInterface *ds)
{
	m_dataStrategies.append(ds);
	m_expectedSignals.insert(dynamic_cast<QObject *>(ds));

	connect(dynamic_cast<QWidget *>(ds), SIGNAL(sendData(QString, QString)), this,
		SLOT(receiveSingleRead(QString, QString)));

	// Forward signals from all DS. Caution: these signals are not aggregated.
	connect(dynamic_cast<QWidget *>(ds), SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)));
	connect(dynamic_cast<QWidget *>(ds), SIGNAL(aboutToWrite(QString, QString)), this,
		SIGNAL(aboutToWrite(QString, QString)));
}

void MultiDataStrategy::removeDataStrategy(DataStrategyInterface *ds)
{
	m_dataStrategies.removeOne(ds);
	m_expectedSignals.remove(dynamic_cast<QObject *>(ds));
	disconnect(dynamic_cast<QWidget *>(ds), SIGNAL(sendData(QString, QString)), this,
		   SLOT(receiveSingleRead(QString, QString)));
	disconnect(dynamic_cast<QWidget *>(ds), SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		   SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)));
	disconnect(dynamic_cast<QWidget *>(ds), SIGNAL(aboutToWrite(QString, QString)), this,
		   SIGNAL(aboutToWrite(QString, QString)));
}

void MultiDataStrategy::removeDataStrategyByIndex(int index)
{
	QWidget *ds = dynamic_cast<QWidget *>(m_dataStrategies.at(index));
	m_expectedSignals.remove(dynamic_cast<QObject *>(m_dataStrategies.at(index)));
	m_dataStrategies.removeAt(index);
	disconnect(ds, SIGNAL(sendData(QString, QString)), this, SLOT(receiveSingleRead(QString, QString)));
	disconnect(ds, SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		   SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)));
	disconnect(ds, SIGNAL(aboutToWrite(QString, QString)), this, SIGNAL(aboutToWrite(QString, QString)));
}

QList<DataStrategyInterface *> MultiDataStrategy::dataStrategies() { return m_dataStrategies; }

QString MultiDataStrategy::data() { return m_data; }

QString MultiDataStrategy::optionalData() { return m_optionalData; }

int MultiDataStrategy::write(QString data)
{
	int res = 0;
	for(DataStrategyInterface *ds : m_dataStrategies) {
		int currentRet = ds->write(data);
		if(currentRet < 0) {
			res = currentRet;
		}
	}

	return res;
}

QPair<QString, QString> MultiDataStrategy::read()
{
	QPair<QString, QString> initialRead;
	for(int i = 0; i < m_dataStrategies.size(); ++i) {
		if(i == 0) {
			initialRead = m_dataStrategies[i]->read();
		} else {
			if(initialRead != m_dataStrategies[i]->read()) {
				initialRead = {"DIFFERENT RESULTS", ""};
			}
		}
	}

	return initialRead;
}

void MultiDataStrategy::writeAsync(QString data)
{
	for(DataStrategyInterface *ds : m_dataStrategies) {
		ds->writeAsync(data);
	}
}

void MultiDataStrategy::readAsync()
{
	for(DataStrategyInterface *ds : m_dataStrategies) {
		ds->readAsync();
	}
}

void MultiDataStrategy::receiveSingleRead(QString data, QString optionalData)
{
	QObject *sender = QObject::sender();
	m_receivedSignals.insert(sender);
	m_receivedData.append({data, optionalData});

	if(m_receivedSignals == m_expectedSignals) {
		qDebug(CAT_MULTI_DATA_STRATEGY) << "Received all signals!";
		bool ok = true;
		for(QPair<QString, QString> ss : m_receivedData) {
			if(ss.first != data || ss.second != optionalData) {
				ok = false;
				Q_EMIT sendData("DIFFERENT RESULTS", "");
			}
		}
		if(ok) {
			m_data = data;
			m_optionalData = optionalData;
			Q_EMIT sendData(data, optionalData);
		}
		m_receivedData.clear();
		m_receivedSignals.clear();
	}
}

#include "moc_multidatastrategy.cpp"