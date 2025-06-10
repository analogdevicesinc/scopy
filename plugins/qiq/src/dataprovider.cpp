#include "dataprovider.h"
#include "scopy-qiqplugin_config.h"
#include <QDir>
#include <qdebug.h>
#include <qtconcurrentrun.h>

using namespace scopy::qiqplugin;

int DataProvider::m_uuid{0};

DataProvider::DataProvider(QObject *parent)
	: QObject(parent)
	, m_data(nullptr)
{
	m_processFw = new QFutureWatcher<void>(this);
	initOutputFile();
	setupConnections();
}

DataProvider::~DataProvider()
{
	if(m_data) {
		m_file.unmap(m_data);
	}
	m_file.remove();
}

void DataProvider::processData(const QString &inputFile)
{
	m_debugTimer.startTimer();
	runPython(QStringList() << "--input" << inputFile << "--output" << m_outputFile);
}

void DataProvider::setScriptPath(const QString &newScriptPath) { m_scriptPath = newScriptPath; }

void DataProvider::readProcessedData()
{
	DebugTimer timer("/home/andrei/Desktop/benchmark.csv");
	if(!m_data || m_size < 4) {
		qWarning() << "Invalid file";
		return;
	}

	int numSamples = m_size / sizeof(Sample);

	const Sample *samples = reinterpret_cast<const Sample *>(m_data);

	QVector<QVector<double>> processedData(CHNL_NUMBER);

	for(int i = 0; i < numSamples; ++i) {
		processedData[0].push_back(samples[i].ch1);
		processedData[1].push_back(samples[i].ch2);
	}

	DEBUGTIMER_LOG(timer, "Read processed data:");
	Q_EMIT dataReady(processedData);
}

void DataProvider::initOutputFile()
{
	QString fileName = "processed_data_" + QString::number(m_uuid++) + ".bin";
	m_outputFile = QString(QIQPLUGIN_RES_PATH) + QDir::separator() + fileName;
	m_file.setFileName(m_outputFile);
	if(!m_file.open(QIODevice::ReadWrite)) {
		qInfo() << "Cannot create file: " << m_outputFile;
		return;
	}
	m_file.close();
}

void DataProvider::setupConnections()
{
	connect(m_processFw, &QFutureWatcher<void>::finished, this, [this]() {
		if(!m_data) {
			mapFile();
		}
		DEBUGTIMER_LOG(m_debugTimer, "Running process data script:");
		readProcessedData();
	});
}

bool DataProvider::mapFile()
{
	if(!m_file.open(QIODevice::ReadOnly)) {
		qInfo() << "Cannot open file: " << m_outputFile;
		return false;
	}
	m_size = m_file.size();
	m_data = m_file.map(0, m_file.size());
	m_file.close();
	return true;
}

void DataProvider::runPython(const QStringList args)
{
	if(m_scriptPath.isEmpty()) {
		qWarning() << "No script provided!";
		return;
	}
	if(m_processFw->isRunning()) {
		return;
	}

	QFuture<void> f = QtConcurrent::run([this, args]() {
		QString program = "python3";

		QProcess process;
		process.start(program, QStringList() << m_scriptPath << args);

		if(!process.waitForStarted()) {
			qDebug() << "Running error!";
			return;
		}

		process.waitForFinished(30000);

		// qInfo() << "Output:" << process.readAllStandardOutput();
		// qInfo() << "Errors:" << process.readAllStandardError();
	});
	m_processFw->setFuture(f);
}
