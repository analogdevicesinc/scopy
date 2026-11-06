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

#include "fakebackendplugin.h"

#include "fakectxbackend.h"

#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QVBoxLayout>

#include <component/attribute.h>
#include <component/controller.h>
#include <component/device.h>

Q_LOGGING_CATEGORY(CAT_FAKEBACKEND, "FakeBackendPlugin");

using namespace scopy;

namespace {
// Number of permanent fake devices (fake:0 … fake:N-1), plus fake:hotplug. 0 disables the backend.
constexpr int FAKE_DEVICE_COUNT = 2;
} // namespace

void FakeBackendPlugin::init()
{
	// Runs once, on the original instance, before any device exists. The backend stays registered
	// for the whole session: plugin libraries are not unloaded.
	if(FAKE_DEVICE_COUNT <= 0) {
		return;
	}
	component::Controller *c = component::Controller::GetInstance();
	if(c->registerProvider(std::make_shared<FakeProvider>(FAKE_DEVICE_COUNT)) &&
	   c->registerBackend(std::make_shared<FakeCtxBackend>())) {
		qInfo(CAT_FAKEBACKEND) << "registered provider" << FakeProvider::KIND << "with" << FAKE_DEVICE_COUNT
				       << "devices";
	}
}

bool FakeBackendPlugin::compatible(QString m_param, QString category)
{
	component::ContextHandle ctx = component::Controller::context(m_param, FakeCtxBackend::ID);
	return ctx && ctx->findChild<component::Device *>("fakedev", Qt::FindDirectChildrenOnly);
}

void FakeBackendPlugin::loadToolList()
{
	m_toolList.append(SCOPY_NEW_TOOLMENUENTRY("fakebackend_tool", "FakeDevice", ":/gui/icons/home.svg"));
}

void FakeBackendPlugin::refresh()
{
	component::ContextHandle ctx = component::Controller::context(m_param, FakeCtxBackend::ID);
	if(!ctx || !m_label) {
		return;
	}
	QStringList lines{"uri: " + ctx->uri()};
	const QList<component::Attribute *> attrs = ctx->findChildren<component::Attribute *>();
	for(component::Attribute *attr : attrs) {
		if(attr->readCapability()) {
			QCoro::waitFor(attr->readCapability()->readAsync());
		}
		lines.append(attr->parent()->objectName() + "/" + attr->name() + ": " + attr->cachedValue());
	}
	m_label->setText(lines.join("\n"));
}

bool FakeBackendPlugin::onConnect()
{
	QWidget *tool = new QWidget();
	auto *lay = new QVBoxLayout(tool);
	m_label = new QLabel(tool);
	auto *btn = new QPushButton("Read", tool);
	lay->addWidget(m_label);
	lay->addWidget(btn);
	lay->addStretch();
	connect(btn, &QPushButton::clicked, this, &FakeBackendPlugin::refresh);
	refresh();

	m_toolList[0]->setEnabled(true);
	m_toolList[0]->setRunBtnVisible(false);
	m_toolList[0]->setTool(tool);
	return true;
}

bool FakeBackendPlugin::onDisconnect()
{
	m_toolList[0]->setEnabled(false);
	delete m_toolList[0]->tool();
	m_toolList[0]->setTool(nullptr);
	m_label = nullptr;
	return true;
}

void FakeBackendPlugin::initMetadata()
{
	loadMetadata(
		R"plugin(
	{
	   "priority":10,
	   "category":[
	      "fake"
	   ]
	}
)plugin");
}

#include "moc_fakebackendplugin.cpp"
