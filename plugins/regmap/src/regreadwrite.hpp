#ifndef REGREADWRITE_HPP
#define REGREADWRITE_HPP

#include "scopy-regmapplugin_export.h"

#include <iio.h>

#include <QObject>

// TODO set the right buf size
#define BUF_SIZE 16384

namespace scopy::regmap {
class SCOPY_REGMAPPLUGIN_EXPORT RegReadWrite : public QObject
{
	Q_OBJECT

public:
	explicit RegReadWrite(struct iio_device *dev, QObject *parent = nullptr);

	~RegReadWrite();

	void read(uint32_t address);
	void write(uint32_t address, uint32_t val);

Q_SIGNALS:
	void readDone(uint32_t address, uint32_t value);
	void readError(const char *err);
	void writeError(const char *err);
	void writeSuccess(uint32_t address);

private:
	struct iio_device *dev;
};
} // namespace scopy::regmap
#endif // REGREADWRITE_HPP
