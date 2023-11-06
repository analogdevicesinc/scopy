#include "datastrategy/filedemodatastrategy.h"
#include <utility>
#include <QFile>

using namespace scopy::attr;

FileDemoDataStrategy::FileDemoDataStrategy(AttributeFactoryRecipe recipe, QObject *parent)
{
	m_recipe = std::move(recipe);
	setParent(parent);
}

void FileDemoDataStrategy::save(QString data)
{
	QFile saveFile("/home/andrei-fabian/adi/newscopy/the_office.txt");
	saveFile.open(QIODevice::WriteOnly);
	data += "\n";
	saveFile.write(data.toLatin1());
	saveFile.close();
}

void FileDemoDataStrategy::requestData()
{
	Q_EMIT sendData("andrei", "adi alexandra andrei#1 andrei#2 andrei#3 cristi ionut bogdan");
}

#include "moc_filedemodatastrategy.cpp"
