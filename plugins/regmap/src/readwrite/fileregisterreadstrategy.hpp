#ifndef FILEREGISTERREADSTRATEGY_HPP
#define FILEREGISTERREADSTRATEGY_HPP

#include "iregisterreadstrategy.hpp"

class QString;
namespace scopy::regmap {
class FileRegisterReadStrategy : public IRegisterReadStrategy
{
public:
	FileRegisterReadStrategy(QString path);
	void read(uint32_t address);
Q_SIGNALS:

private:
	QString path;
};
} // namespace scopy::regmap
#endif // FILEREGISTERREADSTRATEGY_HPP
