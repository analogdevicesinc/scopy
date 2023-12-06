% if 'plugin_name' in config:
#include "${config['plugin_name']}.h"

#include <QLoggingCategory>
#include <QLabel>

% if 'tools' in config:
% for i in config['tools']:
% if 'file_name' in i:
#include "${i['file_name']}.h"
% endif
% endfor
% endif 

% if 'class_name' in config:
Q_LOGGING_CATEGORY(CAT_${config['plugin_name'].upper()}, "${config['class_name']}")
% if 'namespace' in config:
using namespace ${config['namespace']};

bool ${config['class_name']}::compatible(QString m_param, QString category)
{
	// This function defines the characteristics according to which the
	// plugin is compatible with a specific device
	bool ret = true;
	return ret;
}

bool ${config['class_name']}::loadPage()
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
			qWarning(CAT_${config['plugin_name'].upper()}) << "Could not read attribute with index:" << i;
			continue;
		}

		m_infoPage->update(name, value);
	}
	cp->close(m_param);
	m_page->ensurePolished();*/
	return true;
}

bool ${config['class_name']}::loadIcon()
{
	SCOPY_PLUGIN_ICON(":/gui/icons/adalm.svg");
	return true;
}

void ${config['class_name']}::loadToolList()
{
% if 'tools' in config:
% for i in config['tools']:
	m_toolList.append(
		SCOPY_NEW_TOOLMENUENTRY("${i['id']}", "${i['tool_name']}", ":/gui/icons/scopy-default/icons/gear_wheel.svg"));
% endfor
% endif
}

void ${config['class_name']}::unload() { /*delete m_infoPage;*/ }

QString ${config['class_name']}::description() { return "Write the plugin description here"; }

bool ${config['class_name']}::onConnect()
{
	// This method is called when you try to connect to a device and the plugin is 
	// compatible to that device 
	// In case of success the function must return true and false otherwise 
    <% count = 0%>\
% if 'tools' in config:
% for i in config['tools']:
	<% tool_variable_name = i['class_name'].replace(i['class_name'][0], i['class_name'][0].lower(), 1)%>
	${i['class_name']} *${tool_variable_name} = new ${i['class_name']}();
	m_toolList[${count}]->setTool(${tool_variable_name});
	m_toolList[${count}]->setEnabled(true);
	m_toolList[${count}]->setRunBtnVisible(true);
    <% count += 1%>\
% endfor
% endif
	return true;
}

bool ${config['class_name']}::onDisconnect()
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

void ${config['class_name']}::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":100,
	   "category":[
% if 'device_category' in config:
	      "${config['device_category']}"
% endif
	   ],
	   "exclude":[""]
	}
)plugin");
}
% endif
% endif
% endif