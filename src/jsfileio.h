#ifndef JSFILEIO_H
#define JSFILEIO_H

#include <QObject>

namespace adiscope {
class JsFileIo: public QObject
{
	Q_OBJECT

public:
	JsFileIo(QObject *parent = 0);

	Q_INVOKABLE void writeToFile(const QString& data, const QString& path);
};

} /* namespace adiscope */

#endif // JSFILEIO_H
