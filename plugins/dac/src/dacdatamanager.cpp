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
	m_layout = new QHBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setSpacing(10);
	setLayout(m_layout);

	QVBoxLayout *leftHeader = new QVBoxLayout(this);
//	leftHeader->setMargin(0);
//	leftHeader->setSpacing(10);

	dacAddonStack = new MapStackedWidget(this);

	QLabel *name = new QLabel(model->getName(), this);
	m_mode = new MenuCombo("MODE", this);
	auto cb = m_mode->combo();
	connect(cb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		auto mode = cb->itemData(idx).toInt();
		dacAddonStack->show(QString::number(mode));
	});

	setupDacMode("Disabled", DAC_DISABLED);
	setupDacMode("Buffer", DAC_BUFFER);
	setupDacMode("DDS", DAC_DDS);

	leftHeader->addWidget(name, Qt::AlignLeft);
	leftHeader->addWidget(m_mode, Qt::AlignLeft);

	m_color = StyleHelper::getColor(
		"CH" + QString::number(QRandomGenerator::global()->bounded(0, 7)));
//	setStyleSheet("background-color:" + m_color.name());

	m_layout->addLayout(leftHeader, 1);
	m_layout->addWidget(dacAddonStack);

	cb->setCurrentIndex(DAC_BUFFER);

//	m_layout->addWidget(mapstackedwidget-containing-configs-for-buffer-and-dds);
}

// tbd should we have a DacAddon on which
// DacBuffer and DacDds Addon are based of?
// and for the disabled part we have a simple
// DacAddon, just empty.
// m_dacBuffer = new DacDataBuffer();
// move this into a setupDacFunction
// and save the modeCombo to a member
// such that you can add items later on

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
