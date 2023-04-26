#ifndef UTILS_HPP
#define UTILS_HPP

#include <QObject>

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr);

    static QString convertToHexa(uint32_t value, int size);

    static const int bitsPerRow = 8;
signals:

};

#endif // UTILS_HPP
