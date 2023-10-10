#ifndef UTILS_HPP
#define UTILS_HPP

#include <QObject>

class QDir;
class QPushButton;
class QLayout;

namespace scopy::regmap {
class JsonFormatedElement;

class Utils : public QObject
{
	Q_OBJECT
public:
	explicit Utils(QObject *parent = nullptr);

	static QString convertToHexa(uint32_t value, int size);
	static void removeLayoutMargins(QLayout *layout);
	static QDir setXmlPath();

	static int getBitsPerRow();
	static int getBitsPerRowDetailed();

	static JsonFormatedElement *getJsonTemplate(QString xml);

	static void applyJsonConfig();
	static QString getTemplate(QString devName);

private:
	static const int bitsPerRow = 7;
	static const int bitsPerRowDetailed = 3;
	static QMap<QString, JsonFormatedElement *> *spiJson;
	static QMap<QString, JsonFormatedElement *> *axiJson;
	static void getConfigurationFromJson(QString filePath);
	static void populateJsonTemplateMap(QJsonArray jsonArray, bool isAxi);
	static void generateJsonTemplate(QString filePath);

signals:
};
} // namespace scopy::regmap
#endif // UTILS_HPP
