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

#ifndef ACQPLOTRAIL_H
#define ACQPLOTRAIL_H

#include "acqplotkind.h"

#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_engine/datakey.h>
#include <core/acq_engine/samplebuffer.h>
// Not forward-declared: QPointer needs the complete type, and the shell is what the rail
// rows and menu pages are built on.
#include <gui/instrumenttemplate.h>

#include <QMap>
#include <QObject>
#include <QPointer>
#include <QString>

namespace scopy {
class MenuCombo;

namespace acq {
class DataStore;
}

namespace adc {

class AcqChannel;
class AcqPlot;

// The "Plots" rail group and every menu page behind it: the add-plot row, one expandable row
// per plot with its PLOT/VIEW/ADD CHANNEL page, and one nested row per channel with the
// channel's own settings page.
//
// Split out of AcqPlotManager because none of it reads or draws a sample. It builds widgets
// and reports what the reader pressed; the manager decides what that means. So the requests
// leave as signals — addPlotRequested, addChannelRequested, removePlotRequested — and nothing
// here creates, destroys or mutates a plot or a channel.
//
// Rows and pages are owned by the shell. Plots and channels are borrowed and used only as the
// keys of the two maps, which is why a stale pointer is never dereferenced: the manager
// unregisters before it destroys.
class AcqPlotRail : public QObject
{
	Q_OBJECT
public:
	// All three borrowed. `store` and `engine` are the key set the source pickers offer —
	// declared by the engine, written into the store — and may be null. The group and its
	// add-plot row are built here rather than on the first plot: that row is the only way to
	// make one.
	AcqPlotRail(InstrumentTemplate *shell, scopy::acq::DataStore *store, scopy::acq::AcquisitionEngine *engine,
		    QObject *parent = nullptr);

	// A rail row for the plot plus its menu page, keyed on plot->menuId(). The row is the
	// container the plot's channel rows nest under.
	void registerPlot(AcqPlot *p);
	void unregisterPlot(AcqPlot *p);

	// A rail row under the channel's plot's row plus the channel's settings page, both keyed
	// on ch->menuId(). A channel whose plot has no row gets none rather than one in the
	// wrong place.
	void registerChannel(AcqChannel *ch);
	void unregisterChannel(AcqChannel *ch);

	// Refill every plot's ADD CHANNEL pickers: a stream that appears after a page was built
	// must be addable without rebuilding the page.
	void refreshKeyCombos();

	// Fills `combo` with the available streams — declared ∪ written, sorted — keeping the
	// current selection where it still exists. `withSampleIndex` prepends the sample-index
	// entry, which is what an X picker wants and a Y picker does not.
	//
	// Public because the trigger's axis-source combo lives on the controller and reads the
	// same key set.
	void populateKeyCombo(MenuCombo *combo, bool withSampleIndex) const;
	// The key a source combo currently names, empty when nothing is selected.
	static scopy::acq::DataKey keyFromCombo(const MenuCombo *combo);

Q_SIGNALS:
	// The reader pressed Create on the add-plot page. The name is empty when the field was
	// left blank — naming an unnamed plot is the manager's business, since only it knows
	// which uuid is next.
	void addPlotRequested(const QString &name, AcqPlotKind kind);

	// Add channel on a plot's page. An empty xKey means "the producer's recommendation if it
	// made one, otherwise the plot's sample-index ramp".
	void addChannelRequested(AcqPlot *p, scopy::acq::ReprKind kind, const scopy::acq::DataKey &yKey,
				 const scopy::acq::DataKey &xKey);

	// Delete plot. The uuid and not the pointer: the handler destroys the plot, and with it
	// the page the button that emitted this lives on.
	void removePlotRequested(quint32 uuid);

private:
	// What is held for one plot: its rail row, which is also the container its channel rows
	// nest under, and the two source pickers on its ADD CHANNEL section.
	struct Entry
	{
		CollapsableMenuControlButton *row{nullptr};
		QPointer<MenuCombo> yCombo;
		QPointer<MenuCombo> xCombo;
	};

	// The add-plot row and its page. One per rail, built by the constructor.
	void createAddPlotRow();
	QWidget *createPlotPage(AcqPlot *p, Entry &entry);
	// The VIEW section on a plot's page: labels, legend, and the cursors switch with the
	// CursorSettings page it drops down. All per plot, all held by the AcqPlot.
	QWidget *createViewSection(AcqPlot *p, QWidget *parent);

	QPointer<InstrumentTemplate> m_shell;
	QPointer<scopy::acq::DataStore> m_store;
	QPointer<scopy::acq::AcquisitionEngine> m_engine;

	MenuSectionCollapseWidget *m_group{nullptr};
	QMap<AcqPlot *, Entry> m_entries;
	QMap<AcqChannel *, MenuControlButton *> m_channelRows;
};

} // namespace adc
} // namespace scopy

#endif // ACQPLOTRAIL_H
