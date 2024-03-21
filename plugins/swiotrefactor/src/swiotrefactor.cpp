#include "swiotrefactor.h"

#include <QLoggingCategory>
#include <QLabel>

#include "max14906/max14906.h"
#include "faults/faults.h"

Q_LOGGING_CATEGORY(CAT_SWIOTREFACTOR, "SWIOTREFACTORPlugin")
using namespace scopy::swiotrefactor;

bool SWIOTREFACTORPlugin::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	bool ret = true;
	return ret;
}

bool SWIOTREFACTORPlugin::loadPage()
{
	// Here you must write the code for the plugin info page
	// Below is an example for an iio device
	/*m_page = new QWidget();
	m_page->setLayout(new QVBoxLayout(m_page));
	m_page->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_infoPage = new InfoPage(m_page);
	m_infoPage->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	m_page->layout()->addWidget(m_infoPage);
	m_page->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	auto cp = ContextProvider::GetInstance();
	struct iio_context *context = cp->open(m_param);
	ssize_t attributeCount = iio_context_get_attrs_count(context);
	for(int i = 0; i < attributeCount; ++i) {
		const char *name;
		const char *value;
		int ret = iio_context_get_attr(context, i, &name, &value);
		if(ret < 0) {
			qWarning(CAT_SWIOTREFACTOR) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();*/
	return true;
}

bool SWIOTREFACTORPlugin::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void SWIOTREFACTORPlugin::loadToolList()
{
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("max14906", "Max14906", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("faults", "Faults", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
}

void SWIOTREFACTORPlugin::unload()
{ /*delete m_infoPage;*/
}

QString SWIOTREFACTORPlugin::description() { return "Write the plugin description here"; }

bool SWIOTREFACTORPlugin::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is
	// compatible to that device
	// In case of success the function must return true and false otherwise

	Max14906 *max14906 = new Max14906(m_param, m_toolList[0]);
	m_toolList[0]->setTool(max14906);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);

	Faults *faults = new Faults(m_param, m_toolList[1]);
	m_toolList[1]->setTool(faults);
	m_toolList[1]->setEnabled(true);
	m_toolList[1]->setRunBtnVisible(true);
	return true;
}

bool SWIOTREFACTORPlugin::onDisconnect()
{
	// This method is called when the disconnect button is pressed
	// It must remove all connections that were established on the connection
	for(auto &tool : m_toolList) {
		tool->setEnabled(false);
		tool->setRunning(false);
		tool->setRunBtnVisible(false);
		QWidget *w = tool->tool();
		if(w) {
			tool->setTool(nullptr);
			delete(w);
		}
	}
	return true;
}

void SWIOTREFACTORPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
	      "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}
