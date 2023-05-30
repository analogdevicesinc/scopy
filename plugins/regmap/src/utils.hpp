#ifndef UTILS_HPP
#define UTILS_HPP

#include <QObject>

class QDir;
class QPushButton;

class JsonFormatedElement;
namespace scopy::regmap {
class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr);

    static QString convertToHexa(uint32_t value, int size);
    static void applyScopyButtonStyle(QPushButton *button);
    static QDir setXmlPath();

    static int getBitsPerRow();

    static void applyJsonConfig();
    static QList<QString>* getTemplate(QString devName);

private:
    static const int bitsPerRow = 7;
    static QMap<QString, JsonFormatedElement*>* spiJson;
    static QMap<QString, JsonFormatedElement*>* axiJson;
    static void getConfigurationFromJson(QString filePath);
    static void populateJsonTemplateMap(QJsonArray jsonArray, bool spi);
    static void generateJsonTemplate(QString filePath);

signals:

};
}
#endif // UTILS_HPP
