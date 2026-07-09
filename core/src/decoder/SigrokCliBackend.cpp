#include "decoder/SigrokCliBackend.h"

#include "decoder/SigrokCliCatalog.h"

#include <QLoggingCategory>
#include <QProcess>

#include <algorithm>
#include <cmath>

Q_LOGGING_CATEGORY(CAT_SIGROK_BACKEND, "SigrokCliBackend")

namespace scopy {
namespace decoder {

SigrokCliBackend::SigrokCliBackend(SigrokCliCatalog *catalog)
	: m_catalog(catalog)
{}

SigrokCliBackend::~SigrokCliBackend() = default;

QStringList SigrokCliBackend::buildArgs(const DecoderConfig &cfg) const
{
	QStringList args;
	args << "-i" << "-";
	args << "-I"
	     << QString("binary:numchannels=%1:samplerate=%2")
			.arg(cfg.numChannels)
			.arg(static_cast<qint64>(cfg.sampleRate));

	QString pSpec = QString::fromStdString(cfg.decoderId);
	for(const auto &ch : cfg.channels)
		pSpec += QString(":%1=%2")
				 .arg(QString::fromStdString(ch.role))
				 .arg(ch.bitIndex);
	for(const auto &kv : cfg.options)
		pSpec += QString(":%1=%2")
				 .arg(QString::fromStdString(kv.first))
				 .arg(QString::fromStdString(kv.second));
	args << "-P" << pSpec;

	args << "--protocol-decoder-samplenum"
	     << "--protocol-decoder-ann-class";
	return args;
}

void SigrokCliBackend::parseStdout(const QByteArray &buf,
                                   std::vector<AnnotationC> &out) const
{
	const QList<QByteArray> lines = buf.split('\n');
	for(const QByteArray &raw : lines) {
		const QString s = QString::fromUtf8(raw).trimmed();
		if(s.isEmpty()) continue;

		// Format (with --protocol-decoder-samplenum --protocol-decoder-ann-class):
		//   "<start>-<end> <decoder>: <class>: <text>"
		const int firstSpace = s.indexOf(' ');
		if(firstSpace < 0) continue;

		const QString range = s.left(firstSpace);
		QString rest        = s.mid(firstSpace + 1);

		const int dash = range.indexOf('-');
		if(dash < 0) continue;

		bool okStart = false, okEnd = false;
		const quint64 start = range.left(dash).toULongLong(&okStart);
		const quint64 end   = range.mid(dash + 1).toULongLong(&okEnd);
		if(!okStart || !okEnd) continue;

		const int decColon = rest.indexOf(": ");
		if(decColon < 0) continue;
		const QString decoder = rest.left(decColon);
		rest                  = rest.mid(decColon + 2);

		const int classColon = rest.indexOf(": ");
		QString klass, text;
		if(classColon >= 0) {
			klass = rest.left(classColon);
			text  = rest.mid(classColon + 2);
		} else {
			text = rest;
		}

		AnnotationC ann;
		ann.start    = start;
		ann.end      = end;
		ann.decoder  = decoder.toStdString();
		ann.klass    = klass.toStdString();
		ann.text     = text.toStdString();
		ann.severity = 0;
		out.push_back(std::move(ann));
	}
}

bool SigrokCliBackend::decode(const DecoderConfig &cfg,
                              const uint8_t *data, std::size_t nSamples,
                              std::vector<AnnotationC> &out)
{
	out.clear();
	m_lastError.clear();

	if(!data || nSamples == 0) {
		qCDebug(CAT_SIGROK_BACKEND) << "decode(): empty input, skipping";
		return true;
	}

	const QString exe = m_catalog ? m_catalog->resolveCli() : QString{};
	if(exe.isEmpty()) {
		m_lastError = "sigrok-cli executable not found";
		qCWarning(CAT_SIGROK_BACKEND) << m_lastError.c_str();
		return false;
	}

	const int unitsize = std::max(1, static_cast<int>(std::ceil(cfg.numChannels / 8.0)));
	const qint64 bytes = static_cast<qint64>(nSamples) * unitsize;

	const QStringList args = buildArgs(cfg);
	m_lastCmdLine          = exe + " " + args.join(' ');
	qCInfo(CAT_SIGROK_BACKEND) << "decode(): exe=" << exe;
	qCInfo(CAT_SIGROK_BACKEND) << "decode(): argv=" << args;
	qCInfo(CAT_SIGROK_BACKEND) << "decode(): nSamples=" << nSamples
				   << "unitsize=" << unitsize
				   << "bytes=" << bytes;

	QProcess proc;
	proc.setProcessChannelMode(QProcess::SeparateChannels);
	proc.start(exe, args);
	if(!proc.waitForStarted(2000)) {
		m_lastError = "failed to start sigrok-cli: "
			      + proc.errorString().toStdString();
		qCWarning(CAT_SIGROK_BACKEND) << QString::fromStdString(m_lastError);
		return false;
	}

	const qint64 wrote = proc.write(reinterpret_cast<const char *>(data), bytes);
	if(wrote != bytes) {
		qCWarning(CAT_SIGROK_BACKEND)
			<< "decode(): write short: wrote=" << wrote
			<< "wanted=" << bytes;
	}
	proc.waitForBytesWritten(2000);
	proc.closeWriteChannel(); // EOF — flushes the decoder

	if(!proc.waitForFinished(10000)) {
		m_lastError = "sigrok-cli timed out";
		qCWarning(CAT_SIGROK_BACKEND) << QString::fromStdString(m_lastError);
		proc.kill();
		proc.waitForFinished(500);
		return false;
	}

	const QByteArray stdoutBuf = proc.readAllStandardOutput();
	const QByteArray stderrBuf = proc.readAllStandardError();

	qCInfo(CAT_SIGROK_BACKEND)
		<< "decode(): exit=" << proc.exitCode()
		<< "status=" << proc.exitStatus()
		<< "stdout=" << stdoutBuf.size() << "bytes"
		<< "stderr=" << stderrBuf.size() << "bytes";

	if(!stderrBuf.isEmpty()) {
		qCWarning(CAT_SIGROK_BACKEND)
			<< "decode(): stderr:" << stderrBuf.trimmed();
	}

	if(proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
		m_lastError = stderrBuf.trimmed().toStdString();
		if(m_lastError.empty())
			m_lastError = "sigrok-cli exited with non-zero status";
		return false;
	}

	parseStdout(stdoutBuf, out);
	qCInfo(CAT_SIGROK_BACKEND) << "decode(): parsed" << out.size() << "annotations";
	return true;
}

} // namespace decoder
} // namespace scopy
