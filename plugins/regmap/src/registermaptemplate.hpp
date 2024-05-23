#ifndef REGISTERMAPTEMPLATE_HPP
#define REGISTERMAPTEMPLATE_HPP

#include "scopy-regmap_export.h"

#include <QObject>

namespace scopy::regmap {
class RegisterModel;
class SCOPY_REGMAP_EXPORT RegisterMapTemplate : public QObject
{
	Q_OBJECT
public:
	explicit RegisterMapTemplate(QObject *parent = nullptr);
	~RegisterMapTemplate();

	QMap<uint32_t, RegisterModel *> *getRegisterList() const;
	RegisterModel *getRegisterTemplate(uint32_t address);
	void setRegisterList(QMap<uint32_t, RegisterModel *> *newRegisterList);
	RegisterModel *getDefaultTemplate(uint32_t address);

	int bitsPerRow() const;
	void setBitsPerRow(int newBitsPerRow);

private:
	QMap<uint32_t, RegisterModel *> *registerList;
	int m_bitsPerRow = 8;
Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // REGISTERMAPTEMPLATE_HPP
