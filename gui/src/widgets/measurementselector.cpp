#include "widgets/measurementselector.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMap>
#include <stylehelper.h>

using namespace scopy;
MeasurementSelector::MeasurementSelector(QWidget *parent) : QWidget(parent)
{
	lay = new QVBoxLayout(this);
	setLayout(lay);
	lay->setMargin(0);
	lay->setSpacing(6);
}

MeasurementSelector::~MeasurementSelector()
{

}

void MeasurementSelector::addMeasurement(QString name, QString icon)
{
	MeasurementSelectorItem *item= new MeasurementSelectorItem(name, icon ,this);
	lay->addWidget(item);
	m_map.insert(name,item);
}

void MeasurementSelector::removeMeasurement(QString name)
{
	lay->removeWidget(m_map.value(name));
	m_map.remove(name);
}

MeasurementSelectorItem* MeasurementSelector::measurement(QString name) {
	return m_map.value(name);
}

MeasurementSelectorItem::MeasurementSelectorItem(QString name, QString iconPath, QWidget *parent) : QWidget(parent)
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	setLayout(lay);


	m_icon = new QLabel("",this);
	m_name = new QLabel(name, this);


	m_measureCheckbox = new QCheckBox(this);
	m_statsCheckbox = new QCheckBox(this);

	lay->addWidget(m_icon);
	lay->addWidget(m_name);
	lay->addWidget(m_measureCheckbox);
	lay->addWidget(m_statsCheckbox);

	lay->setStretch(0,1);
	lay->setStretch(1,5);
	lay->setStretch(2,1);
	lay->setStretch(3,1);
	StyleHelper::MeasurementSelectorItemWidget(iconPath, this, "MeasurementSelectorItem" + name);

}

MeasurementSelectorItem::~MeasurementSelectorItem()
{

}

QCheckBox *MeasurementSelectorItem::measureCheckbox() const
{
	return m_measureCheckbox;
}

QCheckBox *MeasurementSelectorItem::statsCheckbox() const
{
	return m_statsCheckbox;
}
