#ifndef FILEREGISTERREADSTRATEGY_HPP
#define FILEREGISTERREADSTRATEGY_HPP

#include "iregisterreadstrategy.hpp"

class QString;
class FileRegisterReadStrategy: public IRegisterReadStrategy
{
public:
    FileRegisterReadStrategy(QString path);
    void read(uint32_t address);
Q_SIGNALS:

private:
    QString path;
};

#endif // FILEREGISTERREADSTRATEGY_HPP
