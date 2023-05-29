#include "configchannelview.h"
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

ConfigChannelView::ConfigChannelView(int channelIndex, QWidget *parent) :
	QWidget(parent),
	channelLabel(new QLabel(this)),
	deviceOptions(new QComboBox(this)),
	functionOptions(new QComboBox(this)),
	enabledCheckBox(new QCheckBox(this)),
	m_channelEnabled(false),
	m_channelIndex(channelIndex)
{
	this->enabledCheckBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	this->channelLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	channelLabel->setText(QString::fromStdString("Channel ") + QString::number(m_channelIndex + 1));
	enabledCheckBox->toggled(true);
	this->connectSignalsAndSlots();
}

ConfigChannelView::~ConfigChannelView()
{

}

void ConfigChannelView::connectSignalsAndSlots()
{
	QObject::connect(enabledCheckBox, &QCheckBox::stateChanged, this, [this] () {
		this->setChannelEnabled(enabledCheckBox->isChecked());
		Q_EMIT enabledChanged(m_channelIndex, enabledCheckBox->isChecked());
	});

	QObject::connect(deviceOptions, &QComboBox::textActivated, this, [this] (const QString& text) {
		Q_EMIT deviceChanged(m_channelIndex, text);
	});

	QObject::connect(functionOptions, &QComboBox::textActivated, this, [this] (const QString& text) {
		Q_EMIT functionChanged(m_channelIndex, text);
	});
}

bool ConfigChannelView::isChannelEnabled() const {
	return m_channelEnabled;
}

void ConfigChannelView::setChannelEnabled(bool mChannelEnabled) {
	m_channelEnabled = mChannelEnabled;
	enabledCheckBox->setChecked(m_channelEnabled);

	if (m_channelEnabled) {
		deviceOptions->setEnabled(true);
		functionOptions->setEnabled(true);
	} else {
		deviceOptions->setEnabled(false);
		functionOptions->setEnabled(false);
	}
}

const QString &ConfigChannelView::getSelectedDevice() const {
	return m_selectedDevice;
}

void ConfigChannelView::setSelectedDevice(const QString &mSelectedDevice) {
	m_selectedDevice = mSelectedDevice;
	int index = deviceOptions->findText(m_selectedDevice);
	deviceOptions->setCurrentIndex(index);
}

const QStringList &ConfigChannelView::getDeviceAvailable() const {
	return m_deviceAvailable;
}

void ConfigChannelView::setDeviceAvailable(const QStringList &mDeviceAvailable) {
	m_deviceAvailable = mDeviceAvailable;
	deviceOptions->clear();
	for (const QString& device : m_deviceAvailable) {
		deviceOptions->addItem(device);
	}
}

const QString &ConfigChannelView::getSelectedFunction() const {
	return m_selectedFunction;
}

void ConfigChannelView::setSelectedFunction(const QString &mSelectedFunction) {
	m_selectedFunction = mSelectedFunction;
	int index = functionOptions->findText(m_selectedFunction);
	functionOptions->setCurrentIndex(index);
}

const QStringList &ConfigChannelView::getFunctionAvailable() const {
	return m_functionAvailable;
}

void ConfigChannelView::setFunctionAvailable(const QStringList &mFunctionAvailable) {
	m_functionAvailable = mFunctionAvailable;
	functionOptions->clear();
	for (const QString& device : m_functionAvailable) {
		functionOptions->addItem(device);
	}
}

QLabel *ConfigChannelView::getChannelLabel() const {
	return channelLabel;
}

QComboBox *ConfigChannelView::getDeviceOptions() const {
	return deviceOptions;
}

QComboBox *ConfigChannelView::getFunctionOptions() const {
	return functionOptions;
}

QCheckBox *ConfigChannelView::getEnabledCheckBox() const {
	return enabledCheckBox;
}

#include "moc_configchannelview.cpp"
