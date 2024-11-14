#ifndef SCOPYBENCHMARK_H
#define SCOPYBENCHMARK_H

#include <QElapsedTimer>
#include <QString>
#include "scopy-common_export.h"

#define CONSOLE_LOG(logger, msg) logger.log(msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)
#define FILE_LOG(logger, msg, path) logger.log(path, msg, __PRETTY_FUNCTION__, __FILE__, __LINE__)

namespace scopy {
class SCOPY_COMMON_EXPORT ScopyBenchmark
{
public:
	ScopyBenchmark();
	~ScopyBenchmark();

	void startTimer();
	void restartTimer();

	void log(const QString &msg, const char *function, const char *file, int line);
	void log(const QString &filePath, const QString &msg, const char *function, const char *file, int line);

private:
	QElapsedTimer m_timer;
};

} // namespace scopy

#endif // SCOPYBENCHMARK_H
