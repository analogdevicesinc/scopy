#ifndef SEARCH_HPP
#define SEARCH_HPP

#include <QObject>
#include "scopyregmap_export.h"

class RegisterModel;
class SCOPYREGMAP_EXPORT Search : public QObject
{
	Q_OBJECT
public:
	explicit Search(QObject *parent = nullptr);

	static QList<uint32_t> searchForRegisters(QMap<uint32_t, RegisterModel*> *registers, QString searchParam);
Q_SIGNALS:

};

#endif // SEARCH_HPP
