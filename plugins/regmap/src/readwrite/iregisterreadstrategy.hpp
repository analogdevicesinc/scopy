#ifndef IREGISTERREADSTRATEGY_HPP
#define IREGISTERREADSTRATEGY_HPP

#include <QObject>

class IRegisterReadStrategy : public QObject
{
	Q_OBJECT
public:

	virtual void read(uint32_t address) = 0;

Q_SIGNALS:
	void readDone(uint32_t address, uint32_t value);
	void readError(const char *err);
};

#endif // IREGISTERREADSTRATEGY_HPP
