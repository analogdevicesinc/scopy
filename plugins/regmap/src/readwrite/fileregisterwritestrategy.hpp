#ifndef FILEREGISTERWRITESTRATEGY_HPP
#define FILEREGISTERWRITESTRATEGY_HPP

#include "iregisterwritestrategy.hpp"

class QString;
class FileRegisterWriteStrategy: public IRegisterWriteStrategy
{
public:
	FileRegisterWriteStrategy(QString path);

private:
    QString path;

	// IRegisterWriteStrategy interface
public:
	void write(uint32_t address, uint32_t val);
};

#endif // FILEREGISTERWRITESTRATEGY_HPP
