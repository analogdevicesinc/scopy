#ifndef JSONFORMATEDELEMENT_HPP
#define JSONFORMATEDELEMENT_HPP

#include <QObject>

class JsonFormatedElement
{
public:
    JsonFormatedElement(QString fileName, QList<QString> *compatibleDevices = new QList<QString>(), bool isAxiCompatible = false, bool useNameAsDescription = false);
    QString getFileName() const;

    QList<QString> *getCompatibleDevices() const;
    void addCompatibleDevice(QString device);

    bool getIsAxiCompatible() const;

    bool getUseNameAsDescription() const;

private :
    QString fileName;
    QList<QString> *compatibleDevices;
    bool isAxiCompatible;
    bool useNameAsDescription;

};

#endif // JSONFORMATEDELEMENT_HPP
