#ifndef BITFIELDMODEL_HPP
#define BITFIELDMODEL_HPP

#include "bitfieldoption.hpp"

#include <QMap>
#include <QObject>

namespace scopy::regmap {
class BitFieldModel : public QObject
{
	Q_OBJECT
public:
	BitFieldModel(QString name, int width, int regOffset, QString description, QObject *parent = nullptr);

	BitFieldModel(QString name, QString access, int defaultValue, QString description, QString visibility,
		      int width, QString notes, int bitOffset, int regOffset, int sliceWidth,
		      QVector<BitFieldOption *> *options, QObject *parent = nullptr);
	~BitFieldModel();

	QString getName() const;
	QString getAccess() const;
	int getDefaultValue() const;
	QString getDescription() const;
	QString getVisibility() const;
	int getWidth() const;
	QString getNotes() const;
	int getBitOffset() const;
	int getRegOffset() const;
	int getSliceWidth() const;
	QVector<BitFieldOption *> *getOptions() const;

private:
	QString name;
	QString access;
	int defaultValue;
	QString description;
	QString visibility;
	int width;
	QString notes;
	int bitOffset;
	int regOffset;
	int sliceWidth;
	QVector<BitFieldOption *> *options = nullptr;

	bool reserved;

Q_SIGNALS:
};
} // namespace scopy::regmap
#endif // BITFIELDMODEL_HPP
