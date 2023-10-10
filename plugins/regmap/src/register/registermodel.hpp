#ifndef REGISTERMODEL_HPP
#define REGISTERMODEL_HPP

#include <QObject>

namespace scopy::regmap {
class BitFieldModel;

class RegisterModel : public QObject
{
	Q_OBJECT
public:
	RegisterModel(QString name, uint32_t address, QString description, bool exists, int width, QString notes,
		      QVector<BitFieldModel *> *bitFields);

	QString getName() const;
	uint32_t getAddress() const;
	QString getDescription() const;
	bool getExists() const;
	int getWidth() const;
	QString getNotes() const;
	QVector<BitFieldModel *> *getBitFields() const;

private:
	QString name;
	uint32_t address;
	QString description;
	bool exists;
	int width;
	QString notes;
	QVector<BitFieldModel *> *bitFields;
Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // REGISTERMODEL_HPP
