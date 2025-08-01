#include "buffermenu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QScrollArea>
#include <menuonoffswitch.h>

using namespace scopy::qiqplugin;

BufferMenu::BufferMenu(QWidget *parent)
	: QWidget(parent)
{
	setupUI();
}

BufferMenu::~BufferMenu() {}

void BufferMenu::setupUI()
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);

	// Buffer size
	m_bufferSizeSpin = new MenuSpinbox("Buffer Size", 16, "samples", 16, 4000000, true, false);
	m_bufferSizeSpin->setScaleRange(1, 1e6);
	layout->addWidget(m_bufferSizeSpin);

	// Device selection
	m_deviceCombo = new MenuCombo("Device");
	layout->addWidget(m_deviceCombo);

	// Channel list
	layout->addWidget(new QLabel("Channels:"));
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setMinimumHeight(80);
	m_chnList = new QWidget();
	m_chnList->setLayout(new QVBoxLayout());
	scrollArea->setWidget(m_chnList);
	scrollArea->setWidgetResizable(true);
	layout->addWidget(scrollArea);

	// Connections
	connect(m_deviceCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&BufferMenu::updateChnList);
	connect(m_deviceCombo->combo(), QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&BufferMenu::onParamsChanged);
	connect(m_bufferSizeSpin, &MenuSpinbox::valueChanged, this, &BufferMenu::onParamsChanged);
}

void BufferMenu::setAvailableChannels(const QMap<QString, QStringList> &channels)
{
	m_availableChannels = channels;

	m_deviceCombo->combo()->clear();
	for(auto it = channels.begin(); it != channels.end(); ++it) {
		m_deviceCombo->combo()->addItem(it.key());
	}
	updateChnList();
}

void BufferMenu::updateChnList()
{
	m_enChannels.clear();
	// Clear existing checkboxes
	QLayout *layout = m_chnList->layout();
	while(QLayoutItem *item = layout->takeAt(0)) {
		if(item->widget()) {
			item->widget()->deleteLater();
		}
		delete item;
	}
	// Add new checkboxes for current device
	QString currentDevice = m_deviceCombo->combo()->currentText();
	if(m_availableChannels.contains(currentDevice)) {
		const QStringList &channels = m_availableChannels[currentDevice];
		for(const QString &channel : channels) {
			MenuOnOffSwitch *onOffSwitch = new MenuOnOffSwitch(channel);
			layout->addWidget(onOffSwitch);
			connect(onOffSwitch->onOffswitch(), &QCheckBox::toggled, this, [this, channel](bool checked) {
				if(checked) {
					m_enChannels.append(channel);
				} else {
					m_enChannels.removeAll(channel);
				}
				onParamsChanged();
			});
		}
	}
}

void BufferMenu::onParamsChanged()
{
	BufferParams params;
	params.samplesCount = m_bufferSizeSpin->value();
	params.deviceName = m_deviceCombo->combo()->currentText();
	params.enChnls = m_enChannels;

	Q_EMIT bufferParamsChanged(params);
}
