#ifndef BITFIELDMODEL_HPP
#define BITFIELDMODEL_HPP

#include <QMap>
#include <QObject>

class BitFieldModel : public QObject
{
	Q_OBJECT
public:
	BitFieldModel(QString name, int width,int regOffset, QString description, QObject *parent = nullptr);

	BitFieldModel(   QString name,
			 QString access,
			 int defaultValue,
			 QString description,
			 QString visibility,
			 int width,
			 QString notes,
			 int bitOffset,
			 int regOffset,
			 int sliceWidth,
			 QMap<QString,QString> *options,
			 QObject *parent = nullptr);
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
	QMap<QString, QString>* getOptions() const;

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
	QMap<QString,QString> *options = nullptr;

	bool reserved;

Q_SIGNALS:

};

#endif // BITFIELDMODEL_HPP
