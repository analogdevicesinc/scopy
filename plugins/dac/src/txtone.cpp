#include "txtone.h"
#include "txnode.h"

#include <iio-widgets/iiowidgetbuilder.h>

using namespace scopy;

// the tone is update emitter only when DDS mode is One Tone and
// the tone is the 1st tone of the channel (either "A", either "F1").
// or
// when DDS mode is Two Tone and
// the tone is from the first channel
// bool isUpdateEmitter,
TxTone::TxTone(TxNode *node, unsigned int idx, QWidget *parent)
	: QWidget(parent)
	, m_node(node)
	, m_idx(idx)
{
	QHBoxLayout *toneLay = new QHBoxLayout(this);
	toneLay->setMargin(0);
	toneLay->setSpacing(10);
	this->setLayout(toneLay);

	QLabel *name = new QLabel(this);
	name->setText("Tone " + QString::number(idx) + " : " + m_node->getUuid());
	StyleHelper::MenuComboLabel(name);

	m_frequency = IIOWidgetBuilder()
			.channel(m_node->getChannel())
			.attribute("frequency").uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
			.parent(this).buildSingle();

	m_scale = IIOWidgetBuilder()
			.channel(m_node->getChannel())
			.attribute("scale").uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
			.parent(this).buildSingle();

	m_phase = IIOWidgetBuilder()
			.channel(m_node->getChannel())
			.attribute("phase").uiStrategy(IIOWidgetBuilder::UIS::EditableUi)
			.parent(this).buildSingle();

	connect(dynamic_cast<QWidget *>(m_frequency->getDataStrategy()),
		SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SLOT(forwardFreqChange(QDateTime, QString, QString, int, bool)));

	connect(dynamic_cast<QWidget *>(m_scale->getDataStrategy()),
		SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SLOT(forwardScaleChange(QDateTime, QString, QString, int, bool)));

	connect(dynamic_cast<QWidget *>(m_phase->getDataStrategy()),
		SIGNAL(emitStatus(QDateTime, QString, QString, int, bool)), this,
		SLOT(forwardPhaseChange(QDateTime, QString, QString, int, bool)));

	toneLay->addWidget(name);
	toneLay->addWidget(m_frequency);
	toneLay->addWidget(m_scale);
	toneLay->addWidget(m_phase);
}

TxTone::~TxTone()
{

}

void TxTone::updateFrequency(QString frequency)
{
	m_frequency->write(frequency);
	m_frequency->read();
}

void TxTone::updateScale(QString scale)
{
	m_scale->write(scale);
	m_scale->read();
}

void TxTone::updatePhase(QString phase)
{
	m_phase->write(phase);
	m_phase->read();
}

void TxTone::forwardScaleChange(QDateTime timestamp, QString oldData, QString newData, int retCode,
			       bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	Q_EMIT scaleUpdated(m_idx, newData);
}

void TxTone::forwardPhaseChange(QDateTime timestamp, QString oldData, QString newData, int retCode,
			       bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	Q_EMIT phaseUpdated(m_idx, newData);
}

void TxTone::forwardFreqChange(QDateTime timestamp, QString oldData, QString newData, int retCode,
			       bool readOp)
{
	if(retCode < 0 || readOp) {
		return;
	}
	Q_EMIT frequencyUpdated(m_idx, newData);
}

void TxTone::read()
{
	m_frequency->read();
	m_scale->read();
	m_phase->read();
}
