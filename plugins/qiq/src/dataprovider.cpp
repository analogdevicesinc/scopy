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
	, m_cliProcess(new QProcess())
{
	m_processFw = new QFutureWatcher<void>(this);
	initOutputFile();
	setupConnections();
	connect(m_cliProcess, &QProcess::readyReadStandardOutput, this, [this]() {
		QString output = m_cliProcess->readAllStandardOutput();
		if(output.contains("FAIL")) {
			Q_EMIT stopAcq();
		}
		if(output.contains("OK")) {
			if(!m_data) {
				mapFile();
			}
			DEBUGTIMER_LOG(m_debugTimer, "C program command:");
			readProcessedData();
		}
	});
	connect(m_cliProcess, &QProcess::readyReadStandardError, this, [this]() {
		QString output = m_cliProcess->readAllStandardError();
		qInfo() << "Error:" << output;
	});
}

DataProvider::~DataProvider()
{
	if(m_data) {
		m_file.unmap(m_data);
	}
	m_file.remove();
	if(m_cliProcess) {
		m_cliProcess->kill();
	}
}

void DataProvider::processData(const QString &inputFile)
{
	m_debugTimer.startTimer();
	QString cmd("p " + QString::number(m_chnls));
	m_cliProcess->write(cmd.toStdString().c_str());
	m_cliProcess->write("\n");
}

void DataProvider::setScriptPath(const QString &newScriptPath) { m_scriptPath = newScriptPath; }

void DataProvider::readProcessedData()
{
	DebugTimer timer(scopy::config::settingsFolderPath() + QDir::separator() + "benchmark.csv");
	if(!m_data || m_size < 4) {
		qWarning() << "Invalid file";
		return;
	}

	int numSamples = m_size / sizeof(short);

	const short *samples = reinterpret_cast<const short *>(m_data);

	QVector<QVector<double>> processedData(CHNLS);

	for(int i = 0; i < CHNLS; i++) {
		for(int j = 0; j < numSamples; j++) {
			processedData[i].push_back(samples[j]);
		}
	}

	DEBUGTIMER_LOG(timer, "Read processed data:");
	Q_EMIT dataReady(processedData);
}

void DataProvider::initOutputFile()
{
	QString fileName = "processed_data_" + QString::number(m_uuid++) + ".bin";
	m_outputFile = scopy::config::settingsFolderPath() + QDir::separator() + fileName;
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
	});
	m_processFw->setFuture(f);
}

void DataProvider::runProcess(int chnls)
{
	m_chnls = chnls;
	if(m_data) {
		m_file.unmap(m_data);
		m_data = nullptr;
	}
	if(m_cliProcess->state() == QProcess::Running) {
		m_cliProcess->kill();
		m_cliProcess->waitForFinished();
	}

	QString program = m_cliPath;
	m_cliProcess->start(program,
			    QStringList() << scopy::config::settingsFolderPath() + QDir::separator() + "device_data.bin"
					  << m_outputFile);

	if(!m_cliProcess->waitForStarted()) {
		qWarning() << "Running C error!";
		return;
	}

	qInfo() << "Process started";
	qInfo() << m_cliProcess->readAllStandardOutput();
	qInfo() << m_cliProcess->readAllStandardError();
}

void DataProvider::setCliPath(const QString &newCliPath) { m_cliPath = newCliPath; }
