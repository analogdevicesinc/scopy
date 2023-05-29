#ifndef UTILS_HPP
#define UTILS_HPP

#include <QObject>

class QDir;
class QPushButton;
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

private:
    static const int bitsPerRow = 7;

signals:

};
}
#endif // UTILS_HPP
