#ifndef IREGISTERREADSTRATEGY_HPP
#define IREGISTERREADSTRATEGY_HPP

#include <QObject>

namespace scopy::regmap {
class IRegisterReadStrategy : public QObject
{
	Q_OBJECT
public:
	virtual void read(uint32_t address) = 0;

Q_SIGNALS:
	void readDone(uint32_t address, uint32_t value);
	void readError(const char *err);
};
} // namespace scopy::regmap
#endif // IREGISTERREADSTRATEGY_HPP
