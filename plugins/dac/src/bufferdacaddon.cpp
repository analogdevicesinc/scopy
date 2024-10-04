#include "bufferdacaddon.h"
#include "dacdatamodel.h"
#include "txnode.h"
#include "databufferbuilder.h"
#include "databuffer.h"
#include "dac_logging_categories.h"
#include "scopy-dac_config.h"

#include <menusectionwidget.h>
#include <menucollapsesection.h>
#include <menulineedit.h>
#include <menucontrolbutton.h>
#include <menuheader.h>

#include <iio-widgets/iiowidgetbuilder.h>
#include <iio-widgets/datastrategy/channelattrdatastrategy.h>

#include <QDateTime>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QString>

using namespace scopy;
using namespace scopy::dac;
using namespace scopy::gui;
BufferDacAddon::BufferDacAddon(DacDataModel *model, QWidget *parent)
	: DacAddon(parent)
	, m_model(model)
	, m_dataBuffer(nullptr)
	, m_optionalGuiStrategy(nullptr)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto m_layout = new QVBoxLayout();
	m_layout->setMargin(0);
	m_layout->setSpacing(0);
	setLayout(m_layout);

	QScrollArea *scroll = new QScrollArea();
	QWidget *w = new QWidget(scroll);
	auto scrollLayout = new QVBoxLayout(w);
	scrollLayout->setMargin(0);
	scrollLayout->setSpacing(5);
	w->setLayout(scrollLayout);
	scroll->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	scroll->setWidgetResizable(true);
	scroll->setWidget(w);

	// Start buffer configuration section
	MenuSectionWidget *bufferConfigContainer = new MenuSectionWidget(this);
	MenuCollapseSection *bufferConfigSection =
		new MenuCollapseSection("FILE CONFIGURATION", MenuCollapseSection::MHCW_ARROW, bufferConfigContainer);
	bufferConfigSection->contentLayout()->setSpacing(10);

	// Buffersize section
	MenuSectionWidget *buffersizeContainer = new MenuSectionWidget(bufferConfigContainer);
	buffersizeContainer->setProperty("tutorial_name", "BUFFERSIZE");
	m_bufferSizeSpin =
		new MenuSpinbox("Buffer size", 0, "samples", 16, 16 * 1024 * 1024, true, false, buffersizeContainer);
	StyleHelper::BackgroundWidget(m_bufferSizeSpin);
	connect(m_bufferSizeSpin, &MenuSpinbox::valueChanged, this,
		[&](double value) { m_model->setBuffersize(value); });
	buffersizeContainer->contentLayout()->setSpacing(0);
	buffersizeContainer->contentLayout()->setMargin(0);
	buffersizeContainer->contentLayout()->addWidget(m_bufferSizeSpin);

	// Kernel buffers section
	MenuSectionWidget *kernelContainer = new MenuSectionWidget(this);
	kernelContainer->setProperty("tutorial_name", "KERNEL_BUFFERS");
	m_kernelCountSpin = new MenuSpinbox("Kernel buffers", 0, "", 1, 64, true, false, buffersizeContainer);
	m_kernelCountSpin->setIncrementMode(MenuSpinbox::IS_FIXED);
	StyleHelper::BackgroundWidget(m_kernelCountSpin);
	connect(m_kernelCountSpin, &MenuSpinbox::valueChanged, this,
		[&](double value) { m_model->setKernelBuffersCount(value); });
	kernelContainer->contentLayout()->setSpacing(0);
	kernelContainer->contentLayout()->setMargin(0);
	kernelContainer->contentLayout()->addWidget(m_kernelCountSpin);

	// Decimation section - hidden for now
	MenuSpinbox *decimationSpin = new MenuSpinbox("Decimation", 0, "", 1, 1000, true, false, bufferConfigSection);
	StyleHelper::BackgroundWidget(decimationSpin);
	connect(decimationSpin, &MenuSpinbox::valueChanged, this, [&](double value) { m_model->setDecimation(value); });
	decimationSpin->setValue(1);
	decimationSpin->hide();

	bufferConfigSection->contentLayout()->addWidget(buffersizeContainer);
	bufferConfigSection->contentLayout()->addWidget(kernelContainer);
	bufferConfigSection->contentLayout()->addWidget(decimationSpin);
	bufferConfigContainer->contentLayout()->addWidget(bufferConfigSection);

	// Run and file section
	QWidget *runConfigContainer = new QWidget(this);
	auto runConfigLay = new QHBoxLayout(this);
	runConfigLay->setSpacing(10);
	runConfigLay->setMargin(0);
	runConfigContainer->setLayout(runConfigLay);

	// Run button section
	m_runBtn = new RunBtn(runConfigContainer);
	m_runBtn->setEnabled(false);
	m_runBtn->setProperty("tutorial_name", "RUN_BUTTON");
	connect(m_runBtn, &QPushButton::toggled, this, &BufferDacAddon::runBtnToggled);
	connect(
		m_model, &DacDataModel::updateBuffersize, this,
		[&](unsigned int val) { m_bufferSizeSpin->setValue(val); }, Qt::QueuedConnection);
	connect(
		m_model, &DacDataModel::updateKernelBuffers, this,
		[&](unsigned int val) { m_kernelCountSpin->setValue(val); }, Qt::QueuedConnection);
	connect(
		m_model, &DacDataModel::invalidRunParams, this, [this]() { m_runBtn->setChecked(false); },
		Qt::QueuedConnection);

	// Cyclic buffer section
	MenuSectionWidget *cyclicContainer = new MenuSectionWidget(this);
	cyclicContainer->setProperty("tutorial_name", "CYCLIC_BUTTON");
	m_cyclicBtn = new MenuOnOffSwitch("Cyclic", cyclicContainer);
	connect(m_cyclicBtn->onOffswitch(), &QPushButton::toggled, this, [=, this](bool toggled) {
		bufferConfigContainer->setVisible(!toggled);
		m_model->setCyclic(toggled);
	});
	connect(this, &BufferDacAddon::toggleCyclic, this,
		[=, this](bool toggled) { m_cyclicBtn->onOffswitch()->setChecked(toggled); });
	cyclicContainer->contentLayout()->addWidget(m_cyclicBtn);
	cyclicContainer->setFixedHeight(48);
	m_cyclicBtn->onOffswitch()->setChecked(true);

	// File size and file truncate section
	MenuSectionWidget *filesizeContainer = new MenuSectionWidget(this);
	filesizeContainer->setProperty("tutorial_name", "FILESIZE");
	m_fileSizeSpin =
		new MenuSpinbox("File size", 0, "samples", 16, 16 * 1024 * 1024, false, false, filesizeContainer);
	m_fileSizeSpin->setLineVisible(false);
	m_fileSizeSpin->setIncrementMode(MenuSpinbox::IS_FIXED);
	StyleHelper::BackgroundWidget(m_fileSizeSpin);
	connect(
		m_fileSizeSpin, &MenuSpinbox::valueChanged, this, [&](double value) { m_model->setFilesize(value); },
		Qt::QueuedConnection);
	m_fileSizeSpin->setScaleRange(1, 16 * 1024 * 1024);
	m_fileSizeSpin->setValue(16);
	filesizeContainer->contentLayout()->addWidget(m_fileSizeSpin);
	filesizeContainer->setFixedHeight(48);

	runConfigLay->addWidget(cyclicContainer);
	runConfigLay->addWidget(filesizeContainer);
	runConfigLay->addWidget(m_runBtn);

	// File browser section
	MenuSectionWidget *fileBrowserSection = new MenuSectionWidget(this);
	fileBrowserSection->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	QLabel *fileLbl = new QLabel("Choose file");
	StyleHelper::MenuSmallLabel(fileLbl);

	// Determine default search dir
	QString defaultDir = DAC_CSV_BUILD_PATH;
#ifdef Q_OS_WINDOWS
	defaultDir = DAC_CSV_PATH_LOCAL;
#elif defined __APPLE__
	defaultDir = QCoreApplication::applicationDirPath() + "/plugins/csv";
#elif defined(__appimage__)
	defaultDir = QCoreApplication::applicationDirPath() + "/../lib/scopy/plugins/csv";
#else
	defaultDir = DAC_CSV_SYSTEM_PATH;
#endif

	fm = new FileBrowser(fileBrowserSection);
	fm->setDefaultDir(defaultDir);
	fm->setProperty("tutorial_name", "FILE_MANAGER");
	connect(fm, &FileBrowser::load, this, &BufferDacAddon::load);
	fileBrowserSection->contentLayout()->addWidget(fileLbl);
	fileBrowserSection->contentLayout()->addWidget(fm);

	// Channel list section
	MenuSectionWidget *channelsSection = new MenuSectionWidget(this);
	channelsSection->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	QLabel *channelsLbl = new QLabel("Channels");
	StyleHelper::MenuSmallLabel(channelsLbl);

	QScrollArea *scrollArea = new QScrollArea(channelsSection);
	scrollArea->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	scrollArea->setProperty("tutorial_name", "CHANNEL_SELECTOR");
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	StyleHelper::BackgroundWidget(scrollArea);
	QWidget *channelsContainer = new QWidget(scrollArea);
	channelsContainer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	QVBoxLayout *channelsContainerLayout = new QVBoxLayout();
	channelsContainerLayout->setMargin(0);
	channelsContainerLayout->setSpacing(0);
	channelsContainer->setLayout(channelsContainerLayout);

	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(channelsContainer);

	channelsSection->contentLayout()->addWidget(channelsLbl);
	channelsSection->contentLayout()->addWidget(scrollArea);

	m_optionalGuiStrategy = new QWidget(this);
	m_optionalGuiStrategy->setProperty("tutorial_name", "DATA_CONFIG");

	// Console log section
	MenuSectionWidget *logSection = new MenuSectionWidget(this);
	logSection->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	logSection->setProperty("tutorial_name", "CONSOLE_LOG");
	m_logText = new QTextBrowser(logSection);
	m_logText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_logText->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
	m_logText->setTabStopDistance(30);
	QFont mono("Monospace");
	mono.setStyleHint(QFont::Monospace);
	m_logText->setFont(mono);
	m_logText->setReadOnly(true);
	logSection->contentLayout()->addWidget(m_logText);
	connect(
		this, &BufferDacAddon::log, this,
		[this](QString log) {
			m_logText->append(">>> " + QDateTime::currentDateTime().toString() + " " + log);
		},
		Qt::QueuedConnection);
	connect(m_model, &DacDataModel::log, this, &BufferDacAddon::log);

	// Setup buttons for buffer channels
	int i = 0;
	auto lst = m_model->getBufferTxs();
	for(TxNode *node : lst) {
		MenuControlButton *chnBtn = new MenuControlButton(this);
		chnBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
		chnBtn->setName(node->getUuid());
		chnBtn->setCheckBoxStyle(MenuControlButton::CS_CIRCLE);
		auto color = StyleHelper::getColor("CH" + QString::number(i % 7));
		chnBtn->setColor(color);
		node->setColor(color);
		chnBtn->setDoubleClickToOpenMenu(true);
		chnBtn->setOpenMenuChecksThis(true);
		chnBtn->setCheckable(true);
		chnBtn->setChecked(false);
		chnBtn->button()->setVisible(false);
		connect(chnBtn->checkBox(), &QCheckBox::toggled, this,
			[=, this](bool toggled) { m_model->enableBufferChannel(node->getUuid(), toggled); });
		channelsContainerLayout->addWidget(chnBtn);

		QWidget *chnMenu = createMenu(node);
		m_channelBtns.insert(node->getUuid(), chnBtn);
		m_channelMenus.insert(node->getUuid(), chnMenu);
		connect(chnBtn, &QPushButton::clicked, this, [=, this](bool b) {
			if(b) {
				Q_EMIT requestChannelMenu(node->getUuid());
			}
		});
		i++;
	}
	channelsContainerLayout->addSpacerItem(
		new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

	// Insert all sections into layouts
	QWidget *topSection = new QWidget(this);
	auto topLayout = new QHBoxLayout();
	topSection->setLayout(topLayout);
	topSection->setFixedHeight(190);
	topLayout->setMargin(0);
	topLayout->setSpacing(5);
	topLayout->addWidget(fileBrowserSection);
	topLayout->addWidget(channelsSection);

	scrollLayout->addWidget(topSection);
	scrollLayout->addWidget(runConfigContainer);
	scrollLayout->addWidget(bufferConfigContainer);
	scrollLayout->addWidget(m_optionalGuiStrategy);
	scrollLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	scrollLayout->addWidget(logSection, 3, Qt::AlignBottom);
	m_layout->addWidget(scroll);
}

BufferDacAddon::~BufferDacAddon() {}

void BufferDacAddon::load(QString path)
{
	if(m_dataBuffer) {
		// handle delete and cleanup
		delete m_dataBuffer;
		m_dataBuffer = nullptr;
	}

	m_dataBuffer = DataBufferBuilder()
			       .dataStrategy(DataBufferBuilder::FileStrategy)
			       .guiStrategy(DataBufferBuilder::FileGuiStrategy)
			       .file(path)
			       .parent(this)
			       .build();

	if(!m_dataBuffer) {
		onLoadFailed();
		return;
	}

	connect(m_dataBuffer, SIGNAL(loadFinished()), this, SLOT(onLoadFinished()), Qt::QueuedConnection);
	connect(m_dataBuffer, SIGNAL(loadFailed()), this, SLOT(onLoadFailed()), Qt::QueuedConnection);
	connect(m_dataBuffer, SIGNAL(dataUpdated()), this, SLOT(dataReload()), Qt::QueuedConnection);

	QMetaObject::invokeMethod(m_dataBuffer, "loadData", Qt::QueuedConnection);
}

void BufferDacAddon::onLoadFailed()
{
	Q_EMIT log("Failed to load " + fm->getFilePath());
	m_model->reset();
	m_runBtn->setEnabled(false);
}

void BufferDacAddon::onLoadFinished()
{
	Q_EMIT log("Successfully loaded " + fm->getFilePath());
	m_runBtn->setEnabled(true);
	updateGuiStrategyWidget();
	auto data = m_dataBuffer->getDataBufferStrategy()->data();
	if(data.size()) {
		enableFirstChannels(data[0].size());
	}
}

void BufferDacAddon::enableFirstChannels(int channelCount)
{
	int i = 0;
	for(auto btn : qAsConst(m_channelBtns)) {
		btn->checkBox()->setChecked(i < channelCount);
		i++;
	}
}

void BufferDacAddon::updateGuiStrategyWidget()
{
	if(m_optionalGuiStrategy->layout()) {
		delete m_optionalGuiStrategy->layout();
	}
	auto optGuiStrLay = new QHBoxLayout(m_optionalGuiStrategy);
	optGuiStrLay->setMargin(0);
	optGuiStrLay->setSpacing(0);
	m_optionalGuiStrategy->setLayout(optGuiStrLay);

	if(m_dataBuffer) {
		auto guiStrWidget = m_dataBuffer->getDataGuiStrategyInterface()->ui();
		if(guiStrWidget) {
			optGuiStrLay->addWidget(guiStrWidget);
		}

		auto fileSize = m_dataBuffer->getDataBufferStrategy()->data().size();
		m_bufferSizeSpin->setMaxValue(fileSize);
		m_fileSizeSpin->setMaxValue(fileSize);
		m_bufferSizeSpin->setValue(fileSize);
		m_fileSizeSpin->setValue(fileSize);
	}
}

void BufferDacAddon::enable(bool enable) { m_model->enableBuffer(enable); }

void BufferDacAddon::setRunning(bool toggled)
{
	if(!m_runBtn->isEnabled()) {
		Q_EMIT running(false);
	} else {
		bool isRunning = m_runBtn->isChecked();
		if(toggled != isRunning) {
			m_runBtn->setChecked(toggled);
		}
	}
}

void BufferDacAddon::runBtnToggled(bool toggled)
{
	if(!m_dataBuffer) {
		return;
	}
	if(toggled) {
		dataReload();
		m_model->start();
	} else {
		m_model->stop();
	}
	m_isRunning = toggled;
	Q_EMIT running(toggled);
}

void BufferDacAddon::dataReload()
{
	auto data = m_dataBuffer->getDataBufferStrategy()->data();
	m_model->setData(data);
}

void BufferDacAddon::forwardSamplingFrequencyChange(QDateTime timestamp, QString oldData, QString newData, int retCode,
						    bool readOp)
{
	if(retCode < 0) {
		return;
	}
	bool ok;
	unsigned int val = newData.toUInt(&ok);
	if(ok) {
		m_model->setSamplingFrequency(val);
	}
}

void BufferDacAddon::detectSamplingFrequency(IIOWidget *w)
{
	if(w->getRecipe().data.contains("sampling_frequency")) {
		connect(dynamic_cast<ChannelAttrDataStrategy *>(w->getDataStrategy()),
			&ChannelAttrDataStrategy::emitStatus, this, &BufferDacAddon::forwardSamplingFrequencyChange);
		w->readAsync();
	}
}

QWidget *BufferDacAddon::createAttrMenu(TxNode *node, QWidget *parent)
{
	MenuSectionWidget *attrContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrContainer);
	QList<IIOWidget *> attrWidgets = IIOWidgetBuilder(attr).channel(node->getChannel()).buildAll();

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10);
	layout->setMargin(0);

	for(IIOWidget *w : qAsConst(attrWidgets)) {
		layout->addWidget(w);
		detectSamplingFrequency(w);
	}

	attr->contentLayout()->addLayout(layout);
	attrContainer->contentLayout()->addWidget(attr);
	attr->header()->setChecked(true);
	return attrContainer;
}

QWidget *BufferDacAddon::createMenu(TxNode *node)
{
	QWidget *w = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout();

	QScrollArea *scroll = new QScrollArea();
	QWidget *wScroll = new QWidget(scroll);
	QVBoxLayout *layScroll = new QVBoxLayout(wScroll);
	layScroll->setMargin(0);
	layScroll->setSpacing(10);

	wScroll->setLayout(layScroll);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	scroll->setWidget(wScroll);

	lay->setMargin(0);
	lay->setSpacing(10);
	w->setLayout(lay);

	MenuHeaderWidget *header = new MenuHeaderWidget(node->getUuid(), node->getColor(), w);
	QWidget *attrMenu = createAttrMenu(node, w);

	lay->addWidget(header);
	lay->addWidget(scroll);
	layScroll->addWidget(attrMenu);

	layScroll->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	return w;
}
