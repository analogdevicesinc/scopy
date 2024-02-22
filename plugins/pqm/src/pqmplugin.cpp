#include "pqmplugin.h"
#include "qlabel.h"

#include <QLoggingCategory>
#include <QPushButton>
#include <QVBoxLayout>
#include <acquisitionmanager.h>
#include <harmonicsinstrument.h>
#include <rmsinstrument.h>
#include <settingsinstrument.h>
#include <waveforminstrument.h>

#include <pluginbase/preferences.h>
#include <iioutil/connectionprovider.h>

Q_LOGGING_CATEGORY(CAT_PQMPLUGIN, "PQMPlugin");
using namespace scopy::pqm;

void PQMPlugin::preload() { m_pqmController = new PqmController(m_param); }

bool PQMPlugin::compatible(QString m_param, QString category)
{
	bool ret = false;
	auto &&cp = ConnectionProvider::GetInstance();

	Connection *conn = cp->open(m_param);
	if(!conn) {
		qInfo(CAT_PQMPLUGIN) << "The context is not compatible with the PQMPlugin!";
		return ret;
	}
	iio_device *pqmDevice = iio_context_find_device(conn->context(), "pqm");
	if(pqmDevice) {
		ret = true;
	}
	cp->close(m_param);

	return ret;
}

bool PQMPlugin::loadPage()
{

	m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new InfoPage(m_page);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	auto cp = ConnectionProvider::GetInstance();
	Connection *conn = cp->open(m_param);
	if(conn == nullptr)
		return false;
	struct iio_context *context = conn->context();
	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_PQMPLUGIN) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();

	return true;
}

bool PQMPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void PQMPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("pqmrms", "Rms", ":/gui/icons/scopy-default/icons/tool_network_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("pqmharmonics", "Harmonics",
						  ":/gui/icons/scopy-default/icons/tool_spectrum_analyzer.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("pqmwaveform", "Waveform",
						  ":/gui/icons/scopy-default/icons/tool_oscilloscope.svg"));
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("pqmsettings", "Settings",
						  ":/gui/icons/scopy-default/icons/tool_debugger.svg"));
}

void PQMPlugin::unload()
{
	delete m_infoPage;
	if(m_pqmController) {
		delete m_pqmController;
		m_pqmController = nullptr;
	}
}

QString PQMPlugin::description() { return "Adds functionality specific to PQM board"; }

bool PQMPlugin::onConnect()
{
	Connection *conn = ConnectionProvider::GetInstance()->open(m_param);

	if(!conn) {
		return false;
	}
	struct iio_context *ctx = conn->context();
	connect(m_pqmController, &PqmController::pingFailed, this, &PQMPlugin::disconnectDevice);
	m_pqmController->startPingTask(ctx);

	m_acqManager = new AcquisitionManager(ctx, this);

	RmsInstrument *rms = new RmsInstrument();
	m_toolList[0]->setTool(rms);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
	connect(m_acqManager, &AcquisitionManager::pqmAttrsAvailable, rms, &RmsInstrument::onAttrAvailable);

	HarmonicsInstrument *harmonics = new HarmonicsInstrument();
	m_toolList[1]->setTool(harmonics);
	m_toolList[1]->setEnabled(true);
	m_toolList[1]->setRunBtnVisible(true);
	connect(m_acqManager, &AcquisitionManager::pqmAttrsAvailable, harmonics, &HarmonicsInstrument::onAttrAvailable);

	WaveformInstrument *waveform = new WaveformInstrument();
	m_toolList[2]->setTool(waveform);
	m_toolList[2]->setEnabled(true);
	m_toolList[2]->setRunBtnVisible(true);
	connect(m_acqManager, &AcquisitionManager::bufferDataAvailable, waveform,
		&WaveformInstrument::onBufferDataAvailable);

	SettingsInstrument *settings = new SettingsInstrument();
	m_toolList[3]->setTool(settings);
	m_toolList[3]->setEnabled(true);
	m_toolList[3]->setRunBtnVisible(false);
	connect(m_acqManager, &AcquisitionManager::pqmAttrsAvailable, settings,
		&SettingsInstrument::attributeAvailable);
	connect(settings, &SettingsInstrument::setAttributes, m_acqManager, &AcquisitionManager::setConfigAttr);

	for(auto &tool : m_toolList) {
		if(tool->runBtnVisible()) {
			connect(tool, SIGNAL(runClicked(bool)), tool->tool(), SIGNAL(runTme(bool)));
			connect(tool->tool(), SIGNAL(enableTool(bool)), tool, SLOT(setRunning(bool)));
		}
		connect(tool->tool(), SIGNAL(enableTool(bool, QString)), m_acqManager,
			SLOT(toolEnabled(bool, QString)));
	}
	return true;
}

bool PQMPlugin::onDisconnect()
{
	m_pqmController->stopPingTask();
	disconnect(m_pqmController);
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			disconnect(tool);
			disconnect(tool->tool());
			tool->setTool(nullptr);
			delete(w);
		}
	}
	ResourceManager::close("pqm");
	disconnect(m_acqManager);
	delete m_acqManager;
	m_acqManager = nullptr;

	ConnectionProvider *cp = ConnectionProvider::GetInstance();
	cp->close(m_param);
	return true;
}

void PQMPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":["*"]
	}
)plugin");
}
