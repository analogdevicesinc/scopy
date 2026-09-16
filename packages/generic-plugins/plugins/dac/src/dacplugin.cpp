/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "dacplugin.h"
#include "dac_logging_categories.h"
#include "dacinstrument.h"
#include "dacutils.h"
#include "scopy-dac_config.h"

#include <QLabel>
#include <deviceiconbuilder.h>
#include <menusectionwidget.h>
#include <preferenceshelper.h>
#include <style.h>

#include <component/controller.h>
#include <component/context.h>
#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/attributereader.h>
#include <component/backends/iio/iiosamplecodec.h>

#include <gui/infopage.h>
#include <pluginbase/preferences.h>

#include <qcoro/qcorotask.h>

using namespace scopy;
using namespace scopy::dac;

void DACPlugin::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->init("dac_reset_dds_on_connect", false);
	p->init("dac_reset_dds_on_disconnect", true);
}

bool DACPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout(m_preferencesPage);

	MenuSectionCollapseWidget *generalSection = new MenuSectionCollapseWidget(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET);
	generalSection->contentLayout()->setSpacing(10);
	layout->addWidget(generalSection);
	layout->setSpacing(0);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	auto resetOnConnect = PREFERENCE_CHECK_BOX(p, "dac_reset_dds_on_connect", "Reset DDS on connect",
						   "Reset all DDS settings when connecting to the device. "
						   "Disable if another application is actively using the DDS.",
						   generalSection);

	auto resetOnDisconnect = PREFERENCE_CHECK_BOX(p, "dac_reset_dds_on_disconnect", "Reset DDS on disconnect",
						      "Reset all DDS settings when disconnecting from the device. "
						      "Disable if another application needs the DDS to keep running.",
						      generalSection);

	generalSection->contentLayout()->addWidget(resetOnConnect);
	generalSection->contentLayout()->addWidget(resetOnDisconnect);

	return true;
}

bool DACPlugin::compatible(QString m_param, QString category)
{
	qDebug(CAT_DAC) << "compatible";
	bool ret = false;
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return ret;
	}

	const QList<component::Channel *> channels = ctx->findChildren<component::Channel *>();
	for(component::Channel *chn : channels) {
		if(!chn->isOutput()) {
			continue;
		}
		if(chn->findChild<component::iio::IIOSampleCodec *>() || DacUtils::checkDdsChannel(chn)) {
			ret = true;
			break;
		}
	}

	return ret;
}

bool DACPlugin::loadPage()
{
	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);

	InfoPage *infoPage = new InfoPage(m_page);
	lay->addWidget(infoPage);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		return false;
	}
	const QList<component::Attribute *> attributes =
		ctx->findChildren<component::Attribute *>(Qt::FindDirectChildrenOnly);
	for(component::Attribute *attr : attributes) {
		if(!attr->readCapability()) {
			continue;
		}
		QCoro::waitFor(attr->readCapability()->readAsync());
		infoPage->update(attr->name(), attr->cachedValue());
	}

	return true;
}

bool DACPlugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("DAC");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
	return true;
}

void DACPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("dac", "DAC",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_signal_generator.svg"));
}

void DACPlugin::unload()
{
	if(m_page) {
		delete m_page;
	}
}

QString DACPlugin::description() { return "Tool for generic IIO DAC control."; }

QString DACPlugin::pkgName() { return DAC_PKG_NAME; }

QString DACPlugin::about()
{
	QString content = "DAC plugin";
	return content;
}

bool DACPlugin::onConnect()
{
	m_context = component::Controller::context(m_param);
	if(!m_context) {
		return false;
	}
	dac = new DacInstrument(m_context.get());
	m_toolList[0]->setTool(dac);
	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(true);
	connect(m_toolList.last(), &ToolMenuEntry::runToggled, dynamic_cast<DacInstrument *>(dac),
		&DacInstrument::runToggled);
	connect(dynamic_cast<DacInstrument *>(dac), &DacInstrument::running, m_toolList[0], &ToolMenuEntry::setRunning);
	return true;
}

bool DACPlugin::onDisconnect()
{
	qDebug(CAT_DAC) << "disconnect";
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
	m_context = {};
	return true;
}

void DACPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
		"iio",
		"dac"
	   ]
	}
)plugin");
}

QString DACPlugin::version() { return "0.1"; }

#include "moc_dacplugin.cpp"
