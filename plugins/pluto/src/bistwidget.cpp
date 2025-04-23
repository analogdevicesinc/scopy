#include "bistwidget.h"

#include <style.h>
#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

BistWidget::BistWidget(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	Style::setStyle(this, style::properties::widget::border_interactive);

	m_layout = new QVBoxLayout(this);
	setLayout(m_layout);

	// Get connection to device
	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	QHBoxLayout *hLayout1 = new QHBoxLayout();

	// bist_tone
	m_bistTone = IIOWidgetBuilder(this).device(m_device).attribute("bist_tone").title("Bist TONE").buildSingle();
	hLayout1->addWidget(m_bistTone);

	// bist_prbs
	IIOWidget *bistPrbs =
		IIOWidgetBuilder(this).device(m_device).attribute("bist_prbs").title("Bist PRBS").buildSingle();
	hLayout1->addWidget(bistPrbs);

	// loopback
	IIOWidget *loopback =
		IIOWidgetBuilder(this).device(m_device).attribute("loopback").title("LoopBack").buildSingle();
	hLayout1->addWidget(loopback);

	m_layout->addLayout(hLayout1);

	// tone_level
	m_toneLevel = IIOWidgetBuilder(this).device(m_device).attribute("tone_level").title("Level").buildSingle();
	m_layout->addWidget(m_toneLevel);

	// bist_tone_frequency
	m_toneFrequency = IIOWidgetBuilder(this)
				  .device(m_device)
				  .attribute("bist_tone_frequency")
				  .title("Frequency")
				  .buildSingle();
	m_layout->addWidget(m_toneFrequency);

	m_layout->addWidget(new QLabel("Channel Mask", this));

	QHBoxLayout *hLayout2 = new QHBoxLayout();
	// C2-Q
	m_c2q = new MenuOnOffSwitch("C2-Q", this, false);
	hLayout2->addWidget(m_c2q);

	//  C2-I
	m_c2i = new MenuOnOffSwitch("C2-I", this, false);
	hLayout2->addWidget(m_c2i);

	//  C1-Q
	m_c1q = new MenuOnOffSwitch("C1-Q", this, false);
	hLayout2->addWidget(m_c1q);

	//  C1-I
	m_c1i = new MenuOnOffSwitch("C1-I", this, false);
	hLayout2->addWidget(m_c1i);

	connect(m_c2q->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);
	connect(m_c2i->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);
	connect(m_c1q->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);
	connect(m_c1i->onOffswitch(), &QAbstractButton::toggled, this, &BistWidget::updateBistTone);

	m_layout->addLayout(hLayout2);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

scopy::pluto::BistWidget::~BistWidget() { ConnectionProvider::close(m_uri); }

void BistWidget::updateBistTone()
{
	// sprintf(temp, "%u %u %u %u", mode, freq, level * 6,
	// 	(c2q << 3) | (c2i << 2) | (c1q << 1) | c1i);

	int c2qVal = m_c2q->onOffswitch()->isChecked() ? 1 : 0;
	int c2iVal = m_c2i->onOffswitch()->isChecked() ? 1 : 0;
	int c1qVal = m_c1q->onOffswitch()->isChecked() ? 1 : 0;
	int c1iVal = m_c1i->onOffswitch()->isChecked() ? 1 : 0;

	unsigned int bitmask = (c2qVal << 3) | (c2iVal << 2) | (c1qVal << 1) | c1iVal;

	auto tone = m_bistTone->read().first;
	auto freq = m_toneFrequency->read().first;
	auto level = m_toneLevel->read().first.toInt() * 6;

	Connection *conn = ConnectionProvider::GetInstance()->open(m_uri);
	// iio:device0: ad9361-phy
	iio_device *m_device = iio_context_find_device(conn->context(), "ad9361-phy");

	// Format the configuration string
	QString config = QString("%1 %2 %3 %4").arg(tone).arg(freq).arg(level).arg(bitmask);

	int ret = iio_device_debug_attr_write(m_device, "bist_tone", config.toStdString().c_str());
	if(ret < 0) {
		qWarning() << "FIR Filter channel enablement failed: " << QString::fromLocal8Bit(strerror(ret * (-1)));
	}
	// if (m_dev2)
	// 	iio_device_debug_attr_write(m_dev2, "bist_tone", temp);

	Q_EMIT bistToneUpdated();
}
