#ifndef IREGISTERWRITESTRATEGY_HPP
#define IREGISTERWRITESTRATEGY_HPP

#include <QObject>

namespace scopy::regmap {
class IRegisterWriteStrategy : public QObject
{
	Q_OBJECT
public:
	virtual void write(uint32_t address, uint32_t val) = 0;

Q_SIGNALS:
	void writeError(const char *err);
	void writeSuccess(uint32_t address);
};
} // namespace scopy::regmap
#endif // IREGISTERWRITESTRATEGY_HPP
