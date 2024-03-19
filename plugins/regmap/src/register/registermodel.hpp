#ifndef REGISTERMODEL_HPP
#define REGISTERMODEL_HPP

#include <QObject>
#include <registermaptemplate.hpp>

namespace scopy::regmap {
class BitFieldModel;

class RegisterModel : public QObject
{
	Q_OBJECT
public:
	RegisterModel(QString name, uint32_t address, QString description, bool exists, int width, QString notes,
		      QVector<BitFieldModel *> *bitFields, RegisterMapTemplate *registerMapTemplate);

	QString getName() const;
	uint32_t getAddress() const;
	QString getDescription() const;
	bool getExists() const;
	int getWidth() const;
	QString getNotes() const;
	QVector<BitFieldModel *> *getBitFields() const;

	RegisterMapTemplate *registerMapTemaplate() const;
	void setRegisterMapTemaplate(RegisterMapTemplate *newRegisterMapTemaplate);

private:
	QString name;
	uint32_t address;
	QString description;
	bool exists;
	int width;
	QString notes;
	QVector<BitFieldModel *> *bitFields;
	RegisterMapTemplate *m_registerMapTemaplate = nullptr;

Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // REGISTERMODEL_HPP
