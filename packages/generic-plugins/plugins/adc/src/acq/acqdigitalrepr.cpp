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

#include "acqdigitalrepr.h"

#include "DigitalCurveItem.h"
#include "acqplotrow.h"

#include <core/acq_engine/DataStore.h>

#include <gui/axishandle.h>
#include <gui/plotaxis.h>
#include <gui/plotaxishandle.h>
#include <gui/plotwidget.h>
#include <gui/style.h>
#include <gui/style_attributes.h>

#include <qwt_plot.h>

using namespace scopy;
using namespace scopy::adc;

DigitalRepr::DigitalRepr() = default;

DigitalRepr::~DigitalRepr() { detach(); }

void DigitalRepr::attach(AcqPlotRow *row, const QString &name, const QColor &color)
{
	if(!row || !row->plot() || m_item) {
		return;
	}
	PlotAxis *yAxis = row->digitalAxis();
	if(!yAxis) {
		return;
	}

	m_plot = row->plot();
	m_row = row;
	m_color = color;

	// A draggable handle on the shared digital axis, one per track. The item reads
	// its band position off the handle every draw, so dragging the handle is what
	// reorders tracks — there is no stored ordering to keep in sync.
	m_handle = new PlotAxisHandle(m_plot.data(), yAxis);
	m_handle->handle()->setBarVisibility(BarVisibility::ON_HOVER);
	m_handle->handle()->setColor(color.isValid() ? color : Style::getColor(json::theme::content_silent));
	// NORTH_OR_WEST on a YLeft axis is the left edge, where the analog Y labels are.
	// Annotation bands use the east side, so the two never collide.
	m_handle->handle()->setHandlePos(HandlePos::NORTH_OR_WEST);
	m_plot->addPlotAxisHandle(m_handle);

	// AxisHandle parents itself to the canvas and installs an event filter there to
	// resize itself. Do not reparent, resize or raise it — that breaks the filter and
	// the handle gets clipped on a horizontal resize. Only the explicit show is
	// needed, the same call PlotCursors makes.
	m_handle->handle()->setVisible(true);
	m_handle->setPosition(row->nextDigitalSlot());

	// A drag moves the band, but the item only repositions on a draw, so the plot has
	// to be told. The manager's frame timer would eventually do it, but not while
	// stopped — which is exactly when a reader reorders tracks.
	QObject::connect(m_handle.data(), &PlotAxisHandle::scalePosChanged, m_handle.data(), [this](double) {
		if(!m_plot.isNull() && m_plot->plot()) {
			m_plot->plot()->replot();
		}
	});

	m_item = new DigitalCurveItem(name, m_plot->xAxis(), yAxis, m_handle.data());
	m_item->setColor(color);
	m_item->setVisible(m_enabled);
	m_item->attach(m_plot->plot());
}

void DigitalRepr::detach()
{
	// The plot, and with it the canvas that owns the handle, may already be gone —
	// the repr's channel and the plot are siblings under the manager. Once the QwtPlot
	// is destroyed Qwt has auto-detached its items, so detaching again is a
	// use-after-free. Same discipline as DigitalTrackManager's destructor.
	if(m_plot.isNull()) {
		m_item = nullptr;
		m_handle = nullptr;
		m_row = nullptr;
		return;
	}

	if(m_item) {
		DigitalCurveItem *item = m_item;
		m_item = nullptr; // before the calls, so a re-entrant detach is a no-op
		item->detach();
		delete item;
	}

	if(!m_handle.isNull()) {
		PlotAxisHandle *h = m_handle.data();
		m_handle = nullptr;
		m_plot->removePlotAxisHandle(h);
		// deleteLater, not delete: the handle's own event filter may be mid-dispatch
		// on the canvas when a channel is removed from a UI callback.
		h->deleteLater();
	}

	m_row = nullptr;
}

std::size_t DigitalRepr::claimDepth(int plotSize, std::size_t bufferSize) const
{
	// Same window as a curve: a digital track is one bit per sample over the same
	// visible span.
	return scopy::acq::DataStore::depthForWindow(static_cast<std::size_t>(qMax(1, plotSize)), bufferSize);
}

void DigitalRepr::pull(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, int plotSize)
{
	if(!store || !m_item || m_plot.isNull()) {
		return;
	}

	// toBits owns its result, so unlike CurveRepr's FloatView there is no aliasing to
	// manage here — and setSamples copies into the item anyway. The cost is one
	// allocation per cycle, against a quint8 per sample.
	const QVector<quint8> bits = scopy::acq::toBits(store->window(key, plotSize));
	if(bits.isEmpty()) {
		return;
	}

	// The count the item lays samples out against, not the count it has. Telling it
	// plotSize rather than bits.size() is what keeps a partial window (a fresh run,
	// or plotSize > one buffer) aligned with the analog curves instead of stretched to
	// fill the axis.
	m_item->setSampleCount(static_cast<quint64>(qMax(1, plotSize)));
	m_item->setSamples(bits);
}

void DigitalRepr::reset()
{
	if(m_item && !m_plot.isNull()) {
		m_item->clear();
	}
}

QWidget *DigitalRepr::createSettingsWidget(QWidget *parent)
{
	// Nothing to offer yet. The band height is fixed in canvas pixels, the vertical
	// position is the handle (dragged on the plot, not from a menu), and there is no
	// Y range to set. Colour and name are on the generic CHANNEL section above.
	Q_UNUSED(parent)
	return nullptr;
}

void DigitalRepr::setEnabled(bool en)
{
	m_enabled = en;
	if(m_plot.isNull()) {
		return;
	}
	if(m_item) {
		m_item->setVisible(en);
	}
	// The handle goes with the band: a lone handle for an invisible track is a control
	// with nothing to control.
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->setVisible(en);
	}
}

void DigitalRepr::setColor(const QColor &c)
{
	m_color = c;
	if(m_plot.isNull()) {
		return;
	}
	if(m_item) {
		m_item->setColor(c);
	}
	if(!m_handle.isNull() && m_handle->handle()) {
		m_handle->handle()->setColor(c);
	}
}
