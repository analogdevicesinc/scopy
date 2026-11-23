/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "dcdebuggerplugin.h"

#include "dcdebugger_api.h"

#include "dcdebuggertool.h"
#include "nodepanel.h"
#include "sidesplitter.h"
#include "log/commandlogview.h"
#include "readpolicy.h"

#include <QLabel>
#include <QLoggingCategory>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <component/commandmonitor.h>
#include <component/connection.h>
#include <common/scopyconfig.h>
#include <component/context.h>
#include <core/deviceimpl.h>
#include <deviceiconbuilder.h>
#include <pluginbase/preferences.h>
#include <gui/preferenceshelper.h>
#include <scopy-dcdebugger_config.h>
#include <style.h>
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>

Q_LOGGING_CATEGORY(CAT_DCDEBUGGERPLUGIN, "DCDebuggerPlugin")

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
const char *const TOOL_ID = "dcdebugger";
} // namespace

void DCDebuggerPlugin::init()
{
	CommandLogView::initPreferences();
	ReadPolicy::initPreferences();
	viewprefs::initPreferences();
}

bool DCDebuggerPlugin::loadPreferencesPage()
{
	Preferences *p = Preferences::GetInstance();

	m_preferencesPage = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_preferencesPage);

	MenuSectionWidget *generalWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *generalSection = new MenuCollapseSection(
		"General", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, generalWidget);
	generalWidget->contentLayout()->setSpacing(10);
	generalWidget->contentLayout()->addWidget(generalSection);
	generalSection->contentLayout()->setSpacing(10);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->addWidget(generalWidget);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	generalSection->contentLayout()->addWidget(PREFERENCE_COMBO(
		p, ReadPolicy::PREF, "Read attributes on selection",
		"Choose when selecting a node or expanding a group reads its attributes from the device. \"First "
		"selection only\" reads an attribute once, if it has no cached value yet; \"On every selection\" "
		"reads them each time. The Refresh button and the per-attribute read buttons always read.",
		ReadPolicy::options(), generalSection));

	MenuSectionWidget *viewsWidget = new MenuSectionWidget(m_preferencesPage);
	MenuCollapseSection *viewsSection = new MenuCollapseSection("Views", MenuCollapseSection::MHCW_NONE,
								    MenuCollapseSection::MHW_BASEWIDGET, viewsWidget);
	viewsWidget->contentLayout()->setSpacing(10);
	viewsWidget->contentLayout()->addWidget(viewsSection);
	viewsSection->contentLayout()->setSpacing(10);
	lay->insertWidget(1, viewsWidget);

	const QList<QPair<const char *, QString>> views = {{viewprefs::ACQUISITION, "Acquisition"},
							   {viewprefs::GENERATOR, "Generator"},
							   {viewprefs::MONITOR, "Monitor"},
							   {viewprefs::REGISTERS, "Registers"}};
	for(const auto &[id, name] : views) {
		viewsSection->contentLayout()->addWidget(PREFERENCE_CHECK_BOX(
			p, id, "Enable " + name + " view",
			"Show the " + name + " view in the node panel. Applies to panels opened afterwards.",
			viewsSection));
	}
	return true;
}

bool DCDebuggerPlugin::compatible(QString m_param, QString category)
{
	Q_UNUSED(category);
	// Any view of the connection is enough: the tool browses all of them.
	component::Controller *c = component::Controller::GetInstance();
	for(const component::BackendId &id : c->backends()) {
		if(component::Controller::context(m_param, id)) {
			return true;
		}
	}
	return false;
}

bool DCDebuggerPlugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	logo->setPixmap(pixmap.scaledToHeight(14, Qt::SmoothTransformation));

	QLabel *footer = new QLabel("DC DEBUG");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
	return true;
}

void DCDebuggerPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY(TOOL_ID, "DC Debugger",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_debugger.svg"));
}

QString DCDebuggerPlugin::description() { return DCDEBUGGER_PLUGIN_DESCRIPTION; }

QString DCDebuggerPlugin::pkgName() { return DCDEBUGGER_PKG_NAME; }

QString DCDebuggerPlugin::version() { return "0.1"; }

void DCDebuggerPlugin::saveSettings(QSettings &s)
{
	if(m_tool) {
		m_tool->storeLayout();
	}
	if(m_api) {
		m_api->save(s);
	}
}

void DCDebuggerPlugin::loadSettings(QSettings &s)
{
	// After onConnect (which already read the same file): the tool's own splitters and pane.
	if(m_api) {
		m_api->load(s);
	}
	if(m_tool) {
		m_tool->restoreLayout();
	}
}

component::Connection *DCDebuggerPlugin::resolveConnection() const
{
	if(m_device && m_device->connection()) {
		return m_device->connection();
	}
	component::Controller *c = component::Controller::GetInstance();
	for(const component::BackendId &id : c->backends()) {
		component::ContextHandle ctx = component::Controller::context(m_param, id);
		if(ctx && ctx.connection()) {
			return ctx.connection();
		}
	}
	return nullptr;
}

void DCDebuggerPlugin::acquireViews(component::Connection *conn)
{
	// The device only builds the views some plugin declares; hold every compatible one.
	for(const component::BackendId &id : conn->backends()) {
		component::ContextHandle h = component::Controller::connectCtx(m_param, id);
		if(!h) {
			qWarning(CAT_DCDEBUGGERPLUGIN) << m_param << "view" << id << "could not be built";
			continue;
		}
		m_views.push_back(std::move(h));
	}
}

void DCDebuggerPlugin::releaseViews() { m_views.clear(); }

bool DCDebuggerPlugin::onConnect()
{
	m_conn = resolveConnection();
	if(!m_conn) {
		qWarning(CAT_DCDEBUGGERPLUGIN) << m_param << "has no device-controller connection";
		return false;
	}
	acquireViews(m_conn);
	// The command log state lives in the connection's monitor (off on every new connection).

	// Before the tool: the views restore their layout ("layout" of the API) when built, and the
	// core loads the settings only after onConnect.
	m_api = DCDebugger_API::attach(this);
	if(Preferences::get("general_save_session").toBool()) {
		QSettings s(scopy::config::settingsFolderPath() + "/" + name() + ".ini", QSettings::IniFormat);
		m_api->load(s);
	}
	m_tool = new DCDebuggerTool(m_conn);
	ToolMenuEntry *tme = ToolMenuEntry::findToolMenuEntryById(m_toolList, TOOL_ID);
	tme->setTool(m_tool);
	tme->setEnabled(true);
	tme->setRunBtnVisible(false);

	// Views built or dropped later (late backend registration) show up on the next rebuild.
	connect(m_conn, &component::Connection::contextCreated, m_tool, &DCDebuggerTool::rebuild, Qt::QueuedConnection);
	connect(m_conn, &component::Connection::contextDestroyed, m_tool, &DCDebuggerTool::rebuild,
		Qt::QueuedConnection);
	return true;
}

bool DCDebuggerPlugin::onDisconnect()
{
	DCDebugger_API::detach(this);
	m_api = nullptr;
	// Drop every reference into the tree before the views go away.
	if(m_tool) {
		m_tool->clear();
	}
	for(ToolMenuEntry *tme : std::as_const(m_toolList)) {
		tme->setEnabled(false);
		tme->setRunBtnVisible(false);
		tme->setRunning(false);
		if(tme->tool()) {
			tme->tool()->deleteLater();
		}
		tme->setTool(nullptr);
	}
	// The connection may outlive this device (shared): stop tracing it, nothing shows the log.
	if(m_conn && m_conn->monitor()) {
		m_conn->monitor()->setEnabled(false);
	}
	releaseViews();
	m_conn = nullptr;
	return true;
}

void DCDebuggerPlugin::initMetadata()
{
	// No category: offered on every device; compatible() checks for a component view.
	loadMetadata(
		R"plugin(
	{
	   "priority":3
	}
)plugin");
}

#include "moc_dcdebuggerplugin.cpp"
