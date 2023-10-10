#ifndef JSONFORMATEDELEMENT_HPP
#define JSONFORMATEDELEMENT_HPP

#include <QObject>

namespace scopy::regmap {
class JsonFormatedElement
{
public:
	JsonFormatedElement(QString fileName, QList<QString> *compatibleDevices = new QList<QString>(),
			    bool isAxiCompatible = false, bool useRegisterDescriptionAsName = false,
			    bool useBifieldDescriptionAsName = false);
	QString getFileName() const;

	QList<QString> *getCompatibleDevices() const;
	void addCompatibleDevice(QString device);
	bool getIsAxiCompatible() const;
	bool getUseRegisterNameAsDescription() const;
	bool getUseBifieldNameAsDescription() const;
	QString toString();

private:
	QString fileName;
	QList<QString> *compatibleDevices;
	bool isAxiCompatible = false;
	bool useRegisterDescriptionAsName;
	bool useBifieldDescriptionAsName;
};
} // namespace scopy::regmap
#endif // JSONFORMATEDELEMENT_HPP
