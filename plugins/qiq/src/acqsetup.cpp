#include "acqsetup.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <filebrowserwidget.h>

using namespace scopy::qiqplugin;

AcqSetup::AcqSetup(QWidget *parent)
	: QWidget(parent)
	, m_enChnls(0)
{
	// when acquisition is enabled this menu must to be disabled
	QVBoxLayout *lay = new QVBoxLayout(this);

	// scroll
	m_chnlsW = new QWidget(this);
	QScrollArea *scroll = new QScrollArea(this);
	m_chnlsW->setLayout(new QVBoxLayout);
	scroll->setWidget(m_chnlsW);
	scroll->setWidgetResizable(true);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_devicesCb = new MenuCombo("Available devices", this);

	QWidget *srW = new QWidget(this);
	srW->setLayout(new QVBoxLayout(this));

	QLabel *sampleLabel = new QLabel("Samples count");
	m_sampleRateEdit = new MenuLineEdit(this);
	srW->layout()->addWidget(sampleLabel);
	srW->layout()->addWidget(m_sampleRateEdit);

	m_fileBrowser = new FileBrowserWidget(FileBrowserWidget::OPEN_FILE, this);
	m_fileBrowser->lineEdit()->setPlaceholderText("Select cli program...");

	QPushButton *confBtn = new QPushButton("Configure");
	confBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	connect(confBtn, &QPushButton::clicked, this, &AcqSetup::onConfigPressed);

	lay->addWidget(m_devicesCb);
	lay->addWidget(scroll);
	lay->addWidget(srW);
	lay->addWidget(m_fileBrowser);
	lay->addWidget(confBtn);
}

AcqSetup::~AcqSetup() {}

void AcqSetup::onConfigPressed()
{
	bool ok = false;
	QString deviceName = m_devicesCb->combo()->currentText();
	int sampleRate = m_sampleRateEdit->edit()->text().toInt(&ok);
	QStringList chnlsList;
	if(!ok) {
		sampleRate = 1024;
	}
	for(auto it = m_chnlMap.begin(); it != m_chnlMap.end(); ++it) {
		if(it.value()->onOffswitch()->isChecked()) {
			chnlsList.append(it.key());
		}
	}
	Q_EMIT configPressed({sampleRate, deviceName, m_fileBrowser->lineEdit()->text(), chnlsList});
}

void AcqSetup::initChnlList(const QString &devName)
{
	const QStringList chnls = m_map[devName];
	for(const QString &chnlId : chnls) {
		MenuOnOffSwitch *chnlSwitch = new MenuOnOffSwitch(chnlId);
		chnlSwitch->onOffswitch()->setChecked(false);
		m_chnlMap.insert(chnlId, chnlSwitch);
		m_chnlsW->layout()->addWidget(chnlSwitch);
	}
}

void AcqSetup::init(QMap<QString, QStringList> map)
{
	m_map = map;
	for(auto it = map.begin(); it != map.end(); ++it) {
		m_devicesCb->combo()->addItem(it.key());
	}
	initChnlList(m_devicesCb->combo()->currentText());
	connect(m_devicesCb->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
		clearChnlsList();
		initChnlList(m_devicesCb->combo()->currentText());
	});
}

void AcqSetup::clearChnlsList()
{
	for(MenuOnOffSwitch *ch : qAsConst(m_chnlMap)) {
		m_chnlsW->layout()->removeWidget(ch);
		ch->deleteLater();
	}
	m_chnlMap.clear();
}
