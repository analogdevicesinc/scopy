#include "outputconfig.h"

#include <QFile>
#include <mapkeys.h>

using namespace scopy::qiqplugin;

OutputConfig::OutputConfig() {}

OutputConfig::~OutputConfig() {}

bool OutputConfig::isValid() const { return !m_outputFile.isEmpty() && QFile::exists(m_outputFile); }

void OutputConfig::fromVariantMap(const QVariantMap &params)
{
	m_outputFile = params.value(KeysOutputConfig::OUTPUT_FILE, "").toString();
	m_outputFileFormat = params.value(KeysOutputConfig::OUTPUT_FILE_FORMAT, "").toString();
	m_enabledAnalysis = params.value(KeysOutputConfig::ENABLED_ANALYSIS, {}).toStringList();
}

QVariantMap OutputConfig::toVariantMap() const
{
	QVariantMap map;
	map[KeysOutputConfig::OUTPUT_FILE] = m_outputFile;
	map[KeysOutputConfig::OUTPUT_FILE_FORMAT] = m_outputFileFormat;
	map[KeysOutputConfig::ENABLED_ANALYSIS] = m_enabledAnalysis;
	return map;
}

QString OutputConfig::outputFile() const { return m_outputFile; }
void OutputConfig::setOutputFile(const QString &file) { m_outputFile = file; }

QString OutputConfig::outputFileFormat() const { return m_outputFileFormat; }
void OutputConfig::setOutputFileFormat(const QString &format) { m_outputFileFormat = format; }

QStringList OutputConfig::enabledAnalysis() const { return m_enabledAnalysis; }
void OutputConfig::setEnabledAnalysis(const QStringList &list) { m_enabledAnalysis = list; }
