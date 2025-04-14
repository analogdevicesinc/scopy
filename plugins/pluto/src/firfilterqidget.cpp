#include "firfilterqidget.h"

#include <QFileDialog>
#include <QLabel>
#include <style.h>

#include <pluginbase/preferences.h>
#include <QDebug>
#include <qloggingcategory.h>

using namespace scopy;
using namespace pluto;
Q_LOGGING_CATEGORY(CAT_FIR_FILTER, "FirFilter")

FirFilterQidget::FirFilterQidget(iio_device *dev1, iio_device *dev2, QWidget *parent)
	: m_dev1(dev1)
	, m_dev2(dev2)
	, QWidget{parent}
{

	m_layout = new QVBoxLayout(this);
	setLayout(m_layout);

	QLabel *label = new QLabel("Filter FIR configuraion:", this);

	// TODO REPLACE WITH FILE CHOSE WIDGET
	m_choseFileBtn = new QPushButton("(None)", this);

	Style::setStyle(m_choseFileBtn, style::properties::button::basicButton);

	connect(m_choseFileBtn, &QPushButton::clicked, this, &FirFilterQidget::chooseFile);

	MenuOnOffSwitch *autoFilter = new MenuOnOffSwitch("Auto Filter", this, false);

	connect(autoFilter->onOffswitch(), &QAbstractButton::toggled, this, &FirFilterQidget::autofilterToggled);

	m_applyRxTxFilter = new MenuOnOffSwitch("Enable RX and TX FIR Filters", this, false);
	m_applyRxFilter = new MenuOnOffSwitch("Enable RX FIR Filters", this, false);
	m_applyTxFilter = new MenuOnOffSwitch("Enable TX FIR Filters", this, false);
	m_disableAllFilters = new MenuOnOffSwitch("Disable all FIR Filters", this, false);

	connect(m_applyRxTxFilter->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		toggleDeviceFilter(m_dev1, toggled);

		if(m_dev2 != nullptr) {
			toggleDeviceFilter(m_dev2, toggled);
		}
	});

	connect(m_disableAllFilters->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		toggleDeviceFilter(m_dev1, toggled);

		if(m_dev2 != nullptr) {
			toggleDeviceFilter(m_dev2, toggled);
		}
	});

	connect(m_applyTxFilter->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		toggleChannelFilter(m_dev1, true, toggled);
		if(m_dev2 != nullptr) {
			toggleChannelFilter(m_dev2, true, toggled);
		}
	});

	connect(m_applyRxFilter->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		toggleChannelFilter(m_dev1, false, toggled);
		if(m_dev2 != nullptr) {
			toggleChannelFilter(m_dev2, false, toggled);
		}
	});

	// QButtonGroup *filterButtons = new QButtonGroup(this);
	// filterButtons->setExclusive(true);
	// navigationButtons->addButton(bistBtn);

	m_layout->addWidget(label);
	m_layout->addWidget(autoFilter);
	m_layout->addWidget(m_choseFileBtn);
	m_layout->addWidget(m_applyRxTxFilter);
	m_layout->addWidget(m_applyRxFilter);
	m_layout->addWidget(m_applyTxFilter);
	m_layout->addWidget(m_disableAllFilters);

	hideAllFilters();
}

void FirFilterQidget::chooseFile()
{
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString selectedFilter;
	// Is there a way to open file dialog at our asset file ?
	QString filename = QFileDialog::getOpenFileName(
		this, tr("Export"), "", tr("Filter Files (*.ftr);;All Files(*)"), &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if(!filename.isEmpty()) {
		m_choseFileBtn->setText(filename);

		// TODO implement fir filter functionality
		applyFirFilter(filename);
	}
}

void FirFilterQidget::applyFirFilter(QString path)
{
	if(path.isEmpty()) {
		qWarning(CAT_FIR_FILTER) << "No file path provided";
		return;
	}

	QFile file(path);

	bool rx = false, tx = false;

	if(file.open(QIODevice::ReadOnly)) {

		QTextStream textStream(&file);
		auto line = textStream.readLine();

		int ret = -ENOMEM;

		// Skip comment lines
		while(!textStream.atEnd()) {
			line = textStream.readLine();

			// Determine RX or TX
			if(line.contains("RX", Qt::CaseInsensitive)) {
				rx = true;
			} else if(line.contains("TX", Qt::CaseInsensitive)) {
				tx = true;
			}

			// if we got both rx and tx we can stop searching
			if(rx && tx)
				break;
		}

		// Read the entire file content
		QByteArray buffer = file.readAll();
		file.close();

		// Write configuration to the device(s)
		ret = iio_device_attr_write_raw(m_dev1, "filter_fir_config", buffer.data(), buffer.size());

		if(m_dev2 != nullptr) {
			int ret2 = iio_device_attr_write_raw(m_dev2, "filter_fir_config", buffer.data(), buffer.size());
			ret = std::min(ret, ret2);
		}

		if(ret < 0) {
			qWarning(CAT_FIR_FILTER)
				<< "FIR Filter Configuration Failed" << QString::fromLocal8Bit(strerror(ret * (-1)));
		}
	} else {
		qWarning(CAT_FIR_FILTER) << "Can't open file: " << path;
		return;
	}

	refreshVisibleFilters();
	if(rx && tx) {
		m_applyRxTxFilter->setVisible(true);
	} else if(rx) {
		m_applyRxFilter->setVisible(true);
	} else if(tx) {
		m_applyTxFilter->setVisible(true);
	} else {
		m_disableAllFilters->setVisible(false);
	}
}

void FirFilterQidget::refreshVisibleFilters()
{

	m_applyRxTxFilter->setVisible(false);
	m_applyRxFilter->setVisible(false);
	m_applyTxFilter->setVisible(false);
	m_disableAllFilters->setVisible(true);
}

void FirFilterQidget::hideAllFilters()
{
	m_applyRxTxFilter->setVisible(false);
	m_applyRxFilter->setVisible(false);
	m_applyTxFilter->setVisible(false);
	m_disableAllFilters->setVisible(false);
}

void FirFilterQidget::toggleDeviceFilter(iio_device *dev, bool toggled)
{
	int ret = -ENOMEM;
	// iio-osc uses in_out_voltage_filter_fir_en find out why
	iio_channel *chn = iio_device_find_channel(dev, "out", false);
	if(chn) {
		ret = iio_channel_attr_write_bool(chn, "voltage_filter_fir_en", toggled);
		if(ret < 0) {
			qWarning(CAT_FIR_FILTER)
				<< "FIR Filter enablement failed: " << QString::fromLocal8Bit(strerror(ret * (-1)));
		} else {
			Q_EMIT filterChangeWasMade();
		}
	}
}

void FirFilterQidget::toggleChannelFilter(iio_device *dev, bool isTx, bool toggled)
{
	iio_channel *chn = iio_device_find_channel(dev, "voltage0", isTx);
	if(chn) {
		int ret = -ENOMEM;
		ret = iio_channel_attr_write_bool(chn, "filter_fir_en", toggled);
		if(ret < 0) {
			qWarning(CAT_FIR_FILTER) << "FIR Filter channel enablement failed: "
						 << QString::fromLocal8Bit(strerror(ret * (-1)));
		} else {
			Q_EMIT filterChangeWasMade();
		}
	}
}
