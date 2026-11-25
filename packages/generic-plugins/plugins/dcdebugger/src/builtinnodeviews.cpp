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

#include "acquisition/acquisitionview.h"
#include "acquisition/rawacquisitionview.h"
#include "generation/generatorview.h"
#include "monitor/channelmonitorview.h"
#include "monitor/monitorview.h"
#include "nodepanel.h"
#include "nodeviewregistry.h"
#include "ping/pingview.h"
#include "registers/registerview.h"

#include <component/inputstream.h>
#include <component/outputstream.h>
#include <component/ping.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {

bool isContextOrDevice(NodeKind kind) { return kind == NodeKind::Context || kind == NodeKind::Device; }

class AcquisitionViewFactory : public NodeViewFactory
{
public:
	QString id() const override { return QStringLiteral("acquisition"); }
	QString title() const override { return QStringLiteral("Acquisition"); }
	bool supports(QObject *node, NodeKind) const override { return qobject_cast<component::InputStream *>(node); }
	QWidget *create(QObject *node, NodeKind, NodePanel *panel) override
	{
		return new AcquisitionView(qobject_cast<component::InputStream *>(node), panel);
	}
};

// Simplified replacement of AcquisitionViewFactory, side by side until the old view is removed.
class RawAcquisitionViewFactory : public NodeViewFactory
{
public:
	QString id() const override { return QStringLiteral("rawacquisition"); }
	QString title() const override { return QStringLiteral("Raw Acquisition"); }
	bool supports(QObject *node, NodeKind) const override { return qobject_cast<component::InputStream *>(node); }
	QWidget *create(QObject *node, NodeKind, NodePanel *panel) override
	{
		return new RawAcquisitionView(qobject_cast<component::InputStream *>(node), panel);
	}
};

class GeneratorViewFactory : public NodeViewFactory
{
public:
	QString id() const override { return QStringLiteral("generator"); }
	QString title() const override { return QStringLiteral("Generator"); }
	bool supports(QObject *node, NodeKind) const override { return qobject_cast<component::OutputStream *>(node); }
	QWidget *create(QObject *node, NodeKind, NodePanel *panel) override
	{
		return new GeneratorView(qobject_cast<component::OutputStream *>(node), panel);
	}
};

class PingViewFactory : public NodeViewFactory
{
public:
	QString id() const override { return QStringLiteral("ping"); }
	QString title() const override { return QStringLiteral("Ping"); }
	bool supports(QObject *node, NodeKind) const override { return qobject_cast<component::Ping *>(node); }
	QWidget *create(QObject *node, NodeKind, NodePanel *panel) override
	{
		auto *view = new PingView(qobject_cast<component::Ping *>(node), panel);
		QObject::connect(panel, &NodePanel::refreshRequested, view, &PingView::refresh);
		QObject::connect(panel, &NodePanel::selected, view, &PingView::refresh);
		return view;
	}
};

class MonitorViewFactory : public NodeViewFactory
{
public:
	QString id() const override { return QStringLiteral("monitor"); }
	QString title() const override { return QStringLiteral("Monitor"); }
	bool supports(QObject *node, NodeKind kind) const override
	{
		return isContextOrDevice(kind) && !MonitorView::monitorableChannels(node).isEmpty();
	}
	QWidget *create(QObject *node, NodeKind, NodePanel *panel) override { return new MonitorView(node, panel); }
};

// Table-only replacement of MonitorViewFactory, side by side until the old view is removed.
class ChannelMonitorViewFactory : public NodeViewFactory
{
public:
	QString id() const override { return QStringLiteral("channelmonitor"); }
	QString title() const override { return QStringLiteral("Channel Monitor"); }
	bool supports(QObject *node, NodeKind kind) const override
	{
		return kind == NodeKind::Device && !ChannelMonitorView::inputChannels(node).isEmpty();
	}
	QWidget *create(QObject *node, NodeKind, NodePanel *panel) override
	{
		return new ChannelMonitorView(node, panel);
	}
};

class RegisterViewFactory : public NodeViewFactory
{
public:
	QString id() const override { return QStringLiteral("registers"); }
	QString title() const override { return QStringLiteral("Registers"); }
	bool supports(QObject *node, NodeKind kind) const override
	{
		return isContextOrDevice(kind) && !RegisterView::targetsOf(node).isEmpty();
	}
	QWidget *create(QObject *node, NodeKind, NodePanel *panel) override { return new RegisterView(node, panel); }
};

} // namespace

void scopy::dcdebugger::registerBuiltinNodeViews(NodeViewRegistry *registry)
{
	// The node's own instrument first, then what its children offer.
	registry->registerFactory(std::make_unique<AcquisitionViewFactory>(), 100);
	registry->registerFactory(std::make_unique<RawAcquisitionViewFactory>(), 95);
	registry->registerFactory(std::make_unique<GeneratorViewFactory>(), 100);
	registry->registerFactory(std::make_unique<PingViewFactory>(), 100);
	registry->registerFactory(std::make_unique<MonitorViewFactory>(), 50);
	registry->registerFactory(std::make_unique<ChannelMonitorViewFactory>(), 45);
	registry->registerFactory(std::make_unique<RegisterViewFactory>(), 40);
}
