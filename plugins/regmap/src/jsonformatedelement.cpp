#include "jsonformatedelement.hpp"


JsonFormatedElement::JsonFormatedElement(QString fileName, QList<QString> *compatibleDevices, bool isAxiCompatible, bool useNameAsDescription)
    : fileName(fileName),
    compatibleDevices(compatibleDevices),
    isAxiCompatible(isAxiCompatible),
    useNameAsDescription(useNameAsDescription)
{

}

QString JsonFormatedElement::getFileName() const
{
    return fileName;
}

QList<QString>* JsonFormatedElement::getCompatibleDevices() const
{
    return compatibleDevices;
}

void JsonFormatedElement::addCompatibleDevice(QString device)
{
    compatibleDevices->push_back(device);
}

bool JsonFormatedElement::getIsAxiCompatible() const
{
    return isAxiCompatible;
}

bool JsonFormatedElement::getUseNameAsDescription() const
{
    return useNameAsDescription;
}
