#ifndef JSFILEIO_H
#define JSFILEIO_H

#include <QObject>

namespace adiscope {
class JsFileIo : public QObject {
	Q_OBJECT

public:
	JsFileIo(QObject *parent = 0);

	Q_INVOKABLE void writeToFile(const QString &data, const QString &path);
	Q_INVOKABLE void appendToFile(const QString &data, const QString &path);
	Q_INVOKABLE QString readAll(const QString &path);
	Q_INVOKABLE QString readLine(const QString &path, const int lineNumber);
};

} /* namespace adiscope */

#endif // JSFILEIO_H
