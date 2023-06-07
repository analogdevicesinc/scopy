#include "faults.h"
#include <gui/tool_view_builder.hpp>

#include <QTimer>
#include <QThread>

#include "src/runtime/max14906/max14906.h"
#include "src/swiot_logging_categories.h"
#include <gui/channel_manager.hpp>

using namespace scopy::swiot;

#define POLLING_INTERVAL 1000

Faults::Faults(struct iio_context *ctx, ToolMenuEntry *tme, QWidget *parent) :
	QWidget(parent),
	ctx(ctx),
	ui(new Ui::Faults),
	m_faultsPage(new FaultsPage(ctx, this)),
	m_statusLabel(new QLabel(this)),
	m_statusContainer(new QWidget(this)),
	timer(new QTimer()),
	ad74413r_numeric(0),
	max14906_numeric(0),
	m_backButton(Faults::createBackButton()),
	thread(new QThread(this)),
	m_tme(tme) {
	qInfo(CAT_SWIOT_FAULTS) << "Initialising SWIOT faults page.";

	ui->setupUi(this);

	this->setupDynamicUi(parent);
	this->connectSignalsAndSlots();
}

Faults::~Faults() {
	if (this->thread->isRunning()) {
		this->thread->quit();
		this->thread->wait();
	}
	delete thread;
	delete ui;
}

void Faults::setupDynamicUi(QWidget *parent) {
	scopy::gui::ToolViewRecipe recipe;
	recipe.helpBtnUrl = "";
	recipe.hasRunBtn = true;
	recipe.hasSingleBtn = true;
	recipe.hasPairSettingsBtn = false;
	recipe.hasPrintBtn = false;
	recipe.hasChannels = false;
	recipe.hasHelpBtn = false;

	this->m_toolView = gui::ToolViewBuilder(recipe, nullptr, parent).build();

	m_statusContainer->setLayout(new QHBoxLayout(m_statusContainer));
	m_statusContainer->layout()->setSpacing(0);
	m_statusContainer->layout()->setContentsMargins(0,0,0,0);
	m_statusLabel->setText("The external power supply is not connected. The MAX14906 chip will not be used at full capacity.");
	m_statusLabel->setWordWrap(true);
	m_statusContainer->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 60); border: 1px solid rgba(0, 0, 0, 30); font-size: 11pt");

	auto exclamationLabel = new QPushButton(m_statusContainer);
	exclamationLabel->setIcon(QIcon::fromTheme(":/swiot/warning.svg"));
	exclamationLabel->setIconSize(QSize(32, 32));
	exclamationLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_statusContainer->layout()->addWidget(exclamationLabel);
	m_statusContainer->layout()->addWidget(m_statusLabel);

	m_toolView->addPlotInfoWidget(m_statusContainer);
	this->m_toolView->addFixedCentralWidget(m_faultsPage, 0, 0, -1, -1);

	this->ui->mainLayout->addWidget(m_toolView);
	this->m_toolView->getGeneralSettingsBtn()->setChecked(true);

	this->m_toolView->addTopExtraWidget(m_backButton);
}

void Faults::connectSignalsAndSlots() {
	QObject::connect(this->m_toolView->getRunBtn(), &QPushButton::toggled, this,
			 &Faults::runButtonClicked);
	QObject::connect(this->m_toolView->getSingleBtn(), &QPushButton::clicked, this,
			 &Faults::singleButtonClicked);
	QObject::connect(m_backButton, &QPushButton::clicked, this, [this]() {
		Q_EMIT backBtnPressed();
	});

	QObject::connect(this->timer, &QTimer::timeout, this, &Faults::pollFaults);
	QObject::connect(this->thread, &QThread::started, this, [&]() {
		qDebug(CAT_SWIOT_FAULTS) << "Faults reader thread started";
		this->timer->start(POLLING_INTERVAL);
	});

	QObject::connect(this->m_tme, &ToolMenuEntry::runToggled, this->m_toolView->getRunBtn(),
			 &QPushButton::setChecked);
}

void Faults::runButtonClicked() {
	qDebug(CAT_SWIOT_FAULTS) << "Run button clicked";
	this->m_toolView->getSingleBtn()->setChecked(false);
	if (this->m_toolView->getRunBtn()->isChecked()) {
		qDebug(CAT_SWIOT_FAULTS) << "thread started";
		this->thread->start();
		if (!this->m_tme->running()) {
			this->m_tme->setRunning(true);
		}
	} else {
		if (this->thread->isRunning()) {
			qDebug(CAT_SWIOT_FAULTS) << "thread stopped";
			this->thread->quit();
			this->thread->wait();
		}
		if (this->m_tme->running()) {
			this->m_tme->setRunning(false);
		}
		this->timer->stop();
	}
}

void Faults::singleButtonClicked() {
	qDebug(CAT_SWIOT_FAULTS) << "Single button clicked";
	this->m_toolView->getRunBtn()->setChecked(false);
	this->timer->stop();
	this->pollFaults();
	this->m_toolView->getSingleBtn()->setChecked(false);
}

void Faults::pollFaults() {
	qDebug(CAT_SWIOT_FAULTS) << "Polling faults...";
	this->m_faultsPage->update();
}

QPushButton *Faults::createBackButton() {
	auto *backButton = new QPushButton();
	backButton->setObjectName(QString::fromUtf8("backButton"));
	backButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
						    "  width: 95px;\n"
						    "  height: 40px;\n"
						    "\n"
						    "  font-size: 12px;\n"
						    "  text-align: center;\n"
						    "  font-weight: bold;\n"
						    "  padding-left: 15px;\n"
						    "  padding-right: 15px;\n"
						    "}"));
	backButton->setProperty("blue_button", QVariant(true));
	backButton->setText("Config");
	return backButton;
}

void Faults::externalPowerSupply(bool ps) {
	if (ps) {
		m_statusContainer->hide();
	} else {
		m_statusContainer->show();
		m_statusLabel->show();
	}
}

#include "moc_faults.cpp"
