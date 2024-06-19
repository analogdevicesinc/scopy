#include "settingsinstrument.h"
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menulineedit.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/tooltemplate.h>
#include <gui/animationpushbutton.h>
#include <QLoggingCategory>
#include <QBoxLayout>
#include <QDateTimeEdit>
#include <QScrollArea>
#include <QTime>

Q_LOGGING_CATEGORY(CAT_PQM_SETTINGS, "pqm_settings");
using namespace scopy::pqm;

SettingsInstrument::SettingsInstrument(QWidget *parent)
	: QWidget(parent)
	, m_readEnabled(false)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *instrumentLayout = new QVBoxLayout(this);
	setLayout(instrumentLayout);

	ToolTemplate *tool = new ToolTemplate(this);
	tool->topContainer()->setVisible(false);
	tool->centralContainer()->setVisible(true);
	tool->bottomContainer()->setVisible(true);
	tool->topContainerMenuControl()->setVisible(false);

	instrumentLayout->addWidget(tool);

	initSystemTimeSection(tool->centralContainer());
	initTimestampSection(tool->centralContainer());
	initCalibSection(tool->centralContainer());
	initConfigSection(tool->centralContainer());
}

SettingsInstrument::~SettingsInstrument() {}

void SettingsInstrument::attributeAvailable(QMap<QString, QMap<QString, QString>> configAttr)
{
	if(m_readEnabled) {
		m_readEnabled = false;
		m_pqmAttr = configAttr;
		Q_EMIT updateUi();
		Q_EMIT enableTool(false);
	}
}

void SettingsInstrument::onSetBtnPressed() { Q_EMIT setAttributes(m_pqmAttr); }

void SettingsInstrument::onReadBtnPressed(bool en)
{
	AnimationPushButton *animBtn = dynamic_cast<AnimationPushButton *>(QObject::sender());
	animBtn->startAnimation();
	m_readEnabled = true;
	Q_EMIT enableTool(true);
}

void SettingsInstrument::setDateTimeAttr(QDateTime dateTime, QString attrName)
{
	QString systemTimeValue = dateTime.toString("yyyyMMddhhmmsszzz");
	m_pqmAttr[DEVICE_NAME][attrName] = systemTimeValue;
}

void SettingsInstrument::updateCbValues(QComboBox *cb, QString attr)
{
	if(cb->count() <= 0) {
		QString availableAttr = attr + "_available";
		const QStringList values = m_pqmAttr[DEVICE_NAME][availableAttr].split(" ");
		for(const QString &val : values) {
			cb->addItem(val);
		}
	}
	cb->setCurrentText(m_pqmAttr[DEVICE_NAME][attr]);
}

QWidget *SettingsInstrument::createMenuCombo(QString name, QString attr)
{
	MenuCombo *menuCombo = new MenuCombo(name);
	connect(menuCombo->combo(), &QComboBox::currentTextChanged, this,
		[=, this](QString val) { m_pqmAttr[DEVICE_NAME][attr] = val; });
	connect(this, &SettingsInstrument::updateUi, this, [=, this]() { updateCbValues(menuCombo->combo(), attr); });
	return menuCombo;
}

QWidget *SettingsInstrument::createConfigEdit(QString name, QString attr)
{
	QWidget *configEdit = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(configEdit);
	lay->setSpacing(0);
	lay->setContentsMargins(0, 0, 0, 0);

	QLabel *label = new QLabel(name, configEdit);
	StyleHelper::MenuSmallLabel(label, name);
	MenuLineEdit *lineEdit = new MenuLineEdit(configEdit);
	lay->addWidget(label);
	lay->addWidget(lineEdit);
	connect(lineEdit->edit(), &QLineEdit::textEdited, this, [=, this](const QString val) {
		if(m_pqmAttr[DEVICE_NAME].contains(attr)) {
			m_pqmAttr[DEVICE_NAME][attr] = val;
		}
	});
	connect(this, &SettingsInstrument::updateUi, this, [=, this]() {
		if(m_pqmAttr[DEVICE_NAME].contains(attr)) {
			QString val = m_pqmAttr[DEVICE_NAME][attr];
			lineEdit->edit()->setPlaceholderText(val);
			lineEdit->edit()->setText(val);
		}
	});

	return configEdit;
}

void SettingsInstrument::initConfigSection(QWidget *parent)
{
	MenuCollapseSection *configSection =
		new MenuCollapseSection("Config values", MenuCollapseSection::MHCW_ARROW, parent);

	QWidget *configWidget = new QWidget(configSection);
	configWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	configWidget->setLayout(new QVBoxLayout());
	configWidget->layout()->setContentsMargins(0, 0, 0, 0);
	configWidget->layout()->setSpacing(6);

	MenuOnOffSwitch *currentConsel = new MenuOnOffSwitch("Enable I Consel", configWidget);
	connect(currentConsel->onOffswitch(), &QAbstractButton::toggled, this,
		[=, this](bool en) { m_pqmAttr[DEVICE_NAME]["i_consel_en"] = QString::number(en); });
	connect(this, &SettingsInstrument::updateUi, this, [=, this]() {
		currentConsel->onOffswitch()->setChecked(m_pqmAttr[DEVICE_NAME]["i_consel_en"].toInt());
	});
	configWidget->layout()->addWidget(currentConsel);

	configWidget->layout()->addWidget(createMenuCombo("V Consel", "v_consel"));
	configWidget->layout()->addWidget(createMenuCombo("Nominal Frequency (HZ)", "nominal_frequency"));
	configWidget->layout()->addWidget(createMenuCombo("Flicker Model", "flicker_model"));

	configWidget->layout()->addWidget(createConfigEdit("Nominal Voltage (V)", "nominal_voltage"));
	configWidget->layout()->addWidget(createConfigEdit("Voltage Scale", "voltage_scale"));
	configWidget->layout()->addWidget(createConfigEdit("Current Scale", "current_scale"));
	configWidget->layout()->addWidget(createConfigEdit("Dip Threshold (%)", "dip_threshold"));
	configWidget->layout()->addWidget(createConfigEdit("Dip Hysteresis (%)", "dip_hysteresis"));
	configWidget->layout()->addWidget(createConfigEdit("Swell Threshold (%)", "swell_threshold"));
	configWidget->layout()->addWidget(createConfigEdit("Swell Hysteresis (%)", "swell_hysteresis"));
	configWidget->layout()->addWidget(createConfigEdit("Intrp Threshold (%)", "intrp_threshold"));
	configWidget->layout()->addWidget(createConfigEdit("Intrp Hysteresis (%)", "intrp_hysteresis"));
	configWidget->layout()->addWidget(createConfigEdit("RVC Threshold (%)", "rvc_threshold"));
	configWidget->layout()->addWidget(createConfigEdit("RVC Hysteresis (%)", "rvc_hysteresis"));
	configWidget->layout()->addWidget(createConfigEdit("MsV Carrier Frequency (Hz)", "msv_carrier_frequency"));
	configWidget->layout()->addWidget(createConfigEdit("MsV Record Length (s)", "msv_record_length"));
	configWidget->layout()->addWidget(createConfigEdit("MsV Threshold (%)", "msv_threshold"));

	QWidget *configBtns = new QWidget(configWidget);
	configBtns->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	configBtns->setLayout(new QHBoxLayout());
	configBtns->layout()->setContentsMargins(0, 0, 0, 0);
	AnimationPushButton *configReadBtn = new AnimationPushButton();
	configReadBtn->setText("Read");
	configReadBtn->setFixedWidth(88);
	QMovie *readLoadingIcon(new QMovie(this));
	readLoadingIcon->setFileName(":/gui/loading.gif");
	readLoadingIcon->setScaledSize(QSize(20, 20));
	configReadBtn->setAnimation(readLoadingIcon);
	StyleHelper::BlueButton(configReadBtn, "configRead");
	QPushButton *configSetBtn = new QPushButton("Set");
	configSetBtn->setFixedWidth(88);
	StyleHelper::BlueButton(configSetBtn, "configSet");
	configBtns->layout()->addWidget(configReadBtn);
	configBtns->layout()->addWidget(configSetBtn);
	configBtns->layout()->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
	configWidget->layout()->addWidget(configBtns);

	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidget(configWidget);
	scrollArea->setWidgetResizable(true);
	configSection->contentLayout()->addWidget(scrollArea);

	connect(configReadBtn, &AnimationPushButton::clicked, this, &SettingsInstrument::onReadBtnPressed);
	connect(this, &SettingsInstrument::updateUi, configReadBtn, &AnimationPushButton::stopAnimation);
	connect(configSetBtn, &QPushButton::pressed, this, &SettingsInstrument::onSetBtnPressed);

	parent->layout()->addWidget(configSection);
}

void SettingsInstrument::initSystemTimeSection(QWidget *parent)
{
	MenuCollapseSection *systemTimeSection =
		new MenuCollapseSection("System time", MenuCollapseSection::MHCW_ARROW, parent);
	systemTimeSection->contentLayout()->setSpacing(6);
	QDateTimeEdit *systemTimeEdit = new QDateTimeEdit(systemTimeSection);
	systemTimeEdit->setDateTime(QDateTime::currentDateTime());
	systemTimeEdit->setDisplayFormat("dd:MM:yyyy hh:mm:ss.zzz");
	QPushButton *systemTimeBtn = new QPushButton("Set", systemTimeSection);
	systemTimeBtn->setFixedWidth(88);
	StyleHelper::BlueButton(systemTimeBtn, "systemTimeBtn");

	systemTimeSection->contentLayout()->addWidget(systemTimeEdit);
	systemTimeSection->contentLayout()->addWidget(systemTimeBtn);

	connect(systemTimeBtn, &QPushButton::clicked, this, [this, systemTimeEdit]() {
		setDateTimeAttr(systemTimeEdit->dateTime(), SYSTEM_TIME_ATTR);
		onSetBtnPressed();
	});

	parent->layout()->addWidget(systemTimeSection);
}

void SettingsInstrument::initTimestampSection(QWidget *parent)
{
	MenuCollapseSection *timestampSection =
		new MenuCollapseSection("Logging", MenuCollapseSection::MHCW_ARROW, parent);
	timestampSection->contentLayout()->setSpacing(6);

	QWidget *timestampWidget = new QWidget(timestampSection);
	timestampWidget->setLayout(new QHBoxLayout());
	timestampWidget->layout()->setContentsMargins(0, 0, 0, 0);
	timestampWidget->layout()->setSpacing(10);

	QDateTimeEdit *timestampEdit1 = new QDateTimeEdit(timestampSection);
	timestampEdit1->setDateTime(QDateTime::currentDateTime());
	timestampEdit1->setDisplayFormat("dd:MM:yyyy hh:mm:ss.zzz");

	QDateTimeEdit *timestampEdit2 = new QDateTimeEdit(timestampSection);
	timestampEdit2->setDateTime(QDateTime::currentDateTime());
	timestampEdit2->setDisplayFormat("dd:MM:yyyy hh:mm:ss.zzz");

	qInfo() << "Date time: " << timestampEdit2->dateTime().toString("yyyyMMddhhmmsszzz");

	connect(timestampEdit1, &QDateTimeEdit::dateTimeChanged, this, [=](QDateTime dateTime) {
		if(dateTime > timestampEdit2->dateTime()) {
			timestampEdit1->setDateTime(timestampEdit2->dateTime());
		}
	});

	timestampWidget->layout()->addWidget(timestampEdit1);
	timestampWidget->layout()->addWidget(timestampEdit2);

	QPushButton *timestampBtn = new QPushButton("Set interval", timestampSection);
	timestampBtn->setFixedWidth(88);
	StyleHelper::BlueButton(timestampBtn, "timestampBtn");
	timestampSection->contentLayout()->addWidget(timestampWidget);
	timestampSection->contentLayout()->addWidget(timestampBtn);

	connect(timestampBtn, &QPushButton::clicked, this, [this, timestampEdit1, timestampEdit2]() {
		setDateTimeAttr(timestampEdit1->dateTime(), LOG_START_ATTR);
		setDateTimeAttr(timestampEdit2->dateTime(), LOG_STOP_ATTR);
		onSetBtnPressed();
	});

	parent->layout()->addWidget(timestampSection);
}

void SettingsInstrument::initCalibSection(QWidget *parent)
{
	MenuCollapseSection *calibrateSection =
		new MenuCollapseSection("Calibrate", MenuCollapseSection::MHCW_ARROW, parent);
	calibrateSection->contentLayout()->setSpacing(6);

	MenuCombo *calibrateCombo = new MenuCombo("Channel Type", calibrateSection);
	calibrateSection->contentLayout()->addWidget(calibrateCombo);
	calibrateSection->contentLayout()->addWidget(createConfigEdit("Excepted RMS", "excepted_rms"));

	QPushButton *calibrateBtn = new QPushButton("Calibrate", calibrateSection);
	calibrateBtn->setFixedWidth(88);
	StyleHelper::BlueButton(calibrateBtn, "calibrateBtn");
	calibrateSection->contentLayout()->addWidget(calibrateBtn);

	parent->layout()->addWidget(calibrateSection);
}

#include "moc_settingsinstrument.cpp"
