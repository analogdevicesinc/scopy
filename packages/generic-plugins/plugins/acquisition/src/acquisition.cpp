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

#include "acquisition.h"

#include "acqinstrument.h"
#include "acqinstrumentcontroller.h"
#include "acquisitioninfopage.h"
#include "scopy-acquisition_config.h"

#include <QLabel>
#include <QLoggingCategory>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <component/context.h>
#include <gui/deviceiconbuilder.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

Q_LOGGING_CATEGORY(CAT_ACQUISITION, "AcquisitionPlugin")

using namespace scopy;
using namespace scopy::acquisition;

bool AcquisitionPlugin::compatible(QString param, QString category)
{
	Q_UNUSED(category)
	// Compatible with any device the device controller could build a context for,
	// whatever its backend or component tree - same spirit as the debugger plugin,
	// but decided purely from the controller instead of libiio.
	component::ContextHandle ctx = component::Controller::context(param);
	return static_cast<bool>(ctx);
}

bool AcquisitionPlugin::loadPage()
{
	// The device page is built before connecting, while the Device itself holds the
	// context (IIODeviceImpl acquires it in its constructor), so acquiring the
	// existing one here is enough - no connect of our own.
	component::ContextHandle ctx = component::Controller::context(m_param);
	if(!ctx) {
		qWarning(CAT_ACQUISITION)
			<< "no device-controller context for" << m_param << "- skipping the info page";
		return false;
	}

	m_page = new QWidget();
	QVBoxLayout *lay = new QVBoxLayout(m_page);
	lay->addWidget(new AcquisitionInfoPage(ctx.get(), m_page));
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));

	return true;
}

bool AcquisitionPlugin::loadIcon()
{
	QLabel *logo = new QLabel();
	QPixmap pixmap(":/gui/icons/scopy-default/icons/logo_analog.svg");
	int pixmapHeight = 14;
	pixmap = pixmap.scaledToHeight(pixmapHeight, Qt::SmoothTransformation);
	logo->setPixmap(pixmap);

	QLabel *footer = new QLabel("ACQUISITION");
	Style::setStyle(footer, style::properties::label::deviceIcon, true);

	m_icon = DeviceIconBuilder().shape(DeviceIconBuilder::SQUARE).headerWidget(logo).footerWidget(footer).build();
	return true;
}

void AcquisitionPlugin::loadToolList()
{
	// The disabled placeholder shown before connecting; newInstrument() replaces it with the
	// real entry once there is a context to build the instrument from.
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("acq", "Acquisition",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_oscilloscope.svg"));
}

void AcquisitionPlugin::unload() {}

void AcquisitionPlugin::newInstrument()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("acq", "Acquisition",
						  ":/gui/icons/" + Style::getAttribute(json::theme::icon_theme_folder) +
							  "/icons/tool_oscilloscope.svg"));
	auto tme = m_toolList.last();
	tme->setEnabled(true);
	tme->setRunBtnVisible(true);

	auto *ctrl = new adc::AcqInstrumentController(tme, this);
	m_ctrls.append(ctrl);

	Q_EMIT toolListChanged();
	tme->setTool(ctrl->ui());

	// Only construct sub-widgets after setting a parent, to avoid widget repolishing.
	ctrl->init(m_context.get());
}

void AcquisitionPlugin::deleteInstrument(ToolMenuEntry *tool)
{
	tool->setEnabled(false);
	tool->setRunning(false);
	tool->setRunBtnVisible(false);

	adc::AcqInstrumentController *found = nullptr;
	for(adc::AcqInstrumentController *ctrl : std::as_const(m_ctrls)) {
		if(ctrl->ui() == tool->tool()) {
			found = ctrl;
			break;
		}
	}

	QWidget *w = tool->tool();
	tool->setTool(nullptr);
	if(found) {
		found->stop();
		m_ctrls.removeAll(found);
		// Deletes the instrument too: the controller's destructor owns it.
		delete found;
	} else {
		delete w;
	}

	m_toolList.removeAll(tool);
	tool->deleteLater();
	Q_EMIT toolListChanged();
}

bool AcquisitionPlugin::onConnect()
{
	m_context = component::Controller::context(m_param);
	if(!m_context) {
		qWarning(CAT_ACQUISITION) << "no device-controller context for" << m_param;
		return false;
	}

	// Drop the placeholders loadToolList() created, then build the real instrument.
	// Indexed from the front rather than ranged-for: deleteInstrument() removes from m_toolList.
	while(!m_toolList.isEmpty()) {
		deleteInstrument(m_toolList.first());
	}
	newInstrument();
	return true;
}

bool AcquisitionPlugin::onDisconnect()
{
	while(!m_toolList.isEmpty()) {
		deleteInstrument(m_toolList.first());
	}

	// Released last: every source block the instrument built is parented inside it and holds
	// devices and streams off the Context, so dropping the refcount first could delete them
	// from under a live block.
	m_context = {};

	loadToolList();
	Q_EMIT toolListChanged();
	return true;
}

void AcquisitionPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
	      "iio"
	   ],
	   "exclude":[""]
	}
)plugin");
}

QString AcquisitionPlugin::description() { return "Device controller based acquisition plugin"; }

QString AcquisitionPlugin::pkgName() { return ACQUISITION_PKG_NAME; }

QString AcquisitionPlugin::about() { return "Device controller based acquisition plugin"; }

QString AcquisitionPlugin::version() { return "0.1"; }

#include "moc_acquisition.cpp"
