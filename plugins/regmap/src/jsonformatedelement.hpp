#ifndef JSONFORMATEDELEMENT_HPP
#define JSONFORMATEDELEMENT_HPP

#include <QObject>

class JsonFormatedElement
{
public:
    JsonFormatedElement(QString fileName, QList<QString> *compatibleDevices = new QList<QString>(), bool isAxiCompatible = false, bool useRegisterDescriptionAsName = false, bool useBifieldDescriptionAsName = false);
    QString getFileName() const;

    QList<QString> *getCompatibleDevices() const;
    void addCompatibleDevice(QString device);
    bool getIsAxiCompatible() const;
    bool getUseRegisterNameAsDescription() const;
    bool getUseBifieldNameAsDescription() const;

private :
    QString fileName;
    QList<QString> *compatibleDevices;
    bool isAxiCompatible;
    bool useRegisterDescriptionAsName;
    bool useBifieldDescriptionAsName;

};

#endif // JSONFORMATEDELEMENT_HPP
