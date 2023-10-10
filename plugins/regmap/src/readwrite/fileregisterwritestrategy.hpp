#ifndef FILEREGISTERWRITESTRATEGY_HPP
#define FILEREGISTERWRITESTRATEGY_HPP

#include "iregisterwritestrategy.hpp"

class QString;
namespace scopy::regmap {
class FileRegisterWriteStrategy : public IRegisterWriteStrategy
{
public:
	FileRegisterWriteStrategy(QString path);

private:
	QString path;

	// IRegisterWriteStrategy interface
public:
	void write(uint32_t address, uint32_t val);
};
} // namespace scopy::regmap
#endif // FILEREGISTERWRITESTRATEGY_HPP
