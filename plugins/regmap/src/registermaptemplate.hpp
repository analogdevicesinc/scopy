#ifndef REGISTERMAPTEMPLATE_HPP
#define REGISTERMAPTEMPLATE_HPP

#include <QObject>
#include "scopyregmap_export.h"

namespace scopy::regmap{
class RegisterModel;
class SCOPYREGMAP_EXPORT RegisterMapTemplate : public QObject
{
	Q_OBJECT
public:
	explicit RegisterMapTemplate(QObject *parent = nullptr);
	~RegisterMapTemplate();

	QMap<uint32_t, RegisterModel *> *getRegisterList() const;
	RegisterModel* getRegisterTemplate(uint32_t address);
	void setRegisterList(QMap<uint32_t, RegisterModel *> *newRegisterList);
	RegisterModel* getDefaultTemplate();

private:
	QMap<uint32_t, RegisterModel*> *registerList;
Q_SIGNALS:

};
}
#endif // REGISTERMAPTEMPLATE_HPP
