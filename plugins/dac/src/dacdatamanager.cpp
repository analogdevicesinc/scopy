#include "dacdatamanager.h"
#include "dacdatamodel.h"
#include "ddsdacaddon.h"
#include "bufferdacaddon.h"

#include <stylehelper.h>
#include <gui/widgets/menucombo.h>
#include <QRandomGenerator>
#include <QLabel>

using namespace scopy;

DacDataManager::DacDataManager(DacDataModel *model, QWidget *parent)
	: QWidget(parent)
	, m_model(model)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(10);
	setLayout(m_layout);

	QWidget *header = new QWidget(this);
	QVBoxLayout *headerLay = new QVBoxLayout(this);
	header->setLayout(headerLay);
	StyleHelper::BackgroundWidget(header);
	headerLay->setMargin(10);
	headerLay->setSpacing(0);

	dacAddonStack = new MapStackedWidget(this);

//	QLabel *name = new QLabel(model->getName().toUpper(), this);
	m_mode = new MenuCombo("MODE", this);
	StyleHelper::IIOComboBox(m_mode->combo());
	auto cb = m_mode->combo();
	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		dacAddonStack->show(QString::number(mode));
	});

	setupDacMode("Disabled", DAC_DISABLED);
	setupDacMode("Buffer", DAC_BUFFER);
	setupDacMode("DDS", DAC_DDS);

	headerLay->addWidget(m_mode);
//	leftHeaderLay->addWidget(name);
//	leftHeaderLay->addWidget(m_mode);
//	leftHeaderLay->addItem(new QSpacerItem(40, 40, QSizePolicy::Expanding));

	m_color = StyleHelper::getColor(
		"CH" + QString::number(QRandomGenerator::global()->bounded(0, 7)));

	m_layout->addWidget(header);
	m_layout->addWidget(dacAddonStack);
	m_layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

	cb->setCurrentIndex(DAC_BUFFER);
}

DacDataManager::~DacDataManager()
{

}

QString DacDataManager::getName() const
{
	return m_model->getName();
}

QWidget *DacDataManager::getWidget()
{

}

QColor DacDataManager::getColor() const
{
	return m_color;
}

void DacDataManager::setColor(QColor newColor)
{
	m_color = newColor;
}

void DacDataManager::setupBufferDac()
{
//	if (m_dacBuffer) {

//	}
}

void DacDataManager::setupDdsDac()
{

}

void DacDataManager::setupDacMode(QString mode_name, unsigned int mode)
{
	if (!m_model->isDds() && (mode == DAC_DDS))
		return;
	if (!m_model->isBufferCapable() && (mode == DAC_BUFFER))
		return;
	DacAddon *dac;
	switch (mode) {
	case DAC_DDS:
		dac = new DdsDacAddon(m_model, this);
		break;
	case DAC_BUFFER:
		dac = new BufferDacAddon(m_model, this);
		break;
	default:
		dac = new DacAddon("green", this);
		break;
	}
	m_dacAddons.append(dac);
	dacAddonStack->add(QString::number(mode), dac);
	m_mode->combo()->addItem(mode_name, mode);
}
