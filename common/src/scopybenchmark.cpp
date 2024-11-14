#include "scopybenchmark.h"
#include <QDate>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_BENCHMARK, "Benchmark")
using namespace scopy;

ScopyBenchmark::ScopyBenchmark() {}

ScopyBenchmark::~ScopyBenchmark() {}

void ScopyBenchmark::startTimer() { m_timer.start(); }

void ScopyBenchmark::restartTimer() { m_timer.restart(); }

void ScopyBenchmark::log(const QString &msg, const char *function, const char *file, int line)
{
	QMessageLogger(file, line, function).info(CAT_BENCHMARK) << function << msg << m_timer.elapsed() << "ms";
}

void ScopyBenchmark::log(const QString &filePath, const QString &msg, const char *function, const char *file, int line)
{
	QFile f(filePath);
	if(f.open(QIODevice::WriteOnly | QIODevice::Append)) {
		QTextStream stream(&f);
		stream << QDateTime::currentDateTime().toString("dd:MM:yyyy hh:mm:ss.zzz") << "\t" << file << ":"
		       << line << "\t" << function << "\t" << msg << "\t" << m_timer.elapsed() << "\t"
		       << "ms"
		       << "\n";
	}
}
