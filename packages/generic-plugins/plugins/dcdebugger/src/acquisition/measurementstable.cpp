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

#include "measurementstable.h"

#include <QHeaderView>
#include <QPixmap>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

#include <cmath>

#include <gui/docking/dockablearea.h>
#include <gui/docking/dockwrapper.h>
#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

QString scopy::dcdebugger::siFormat(double v, const QString &unit, int digits)
{
	if(!std::isfinite(v)) {
		return QStringLiteral("—");
	}
	static const struct
	{
		double scale;
		const char *prefix;
	} prefixes[] = {{1e9, "G"}, {1e6, "M"}, {1e3, "k"}, {1, ""}, {1e-3, "m"}, {1e-6, "µ"}, {1e-9, "n"}};
	const double a = std::abs(v);
	for(const auto &p : prefixes) {
		if(a >= p.scale || p.scale == 1e-9) {
			if(a == 0) {
				break;
			}
			return QString::number(v / p.scale, 'g', digits) + " " + QString::fromUtf8(p.prefix) + unit;
		}
	}
	return QStringLiteral("0 ") + unit;
}

MeasurementsTable::MeasurementsTable(QWidget *parent)
	: QTableWidget(parent)
{
	const QStringList headers = {"Channel", "Min", "Max", "Mean", "RMS", "AC RMS", "Pk-Pk", "Freq", "Peak"};
	setColumnCount(headers.size());
	setHorizontalHeaderLabels(headers);
	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setVisible(false);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionMode(QAbstractItemView::NoSelection);
	setFocusPolicy(Qt::NoFocus);
	Style::setStyle(this, style::properties::widget::tableViewWidget);
}

void MeasurementsTable::setRows(const QList<MeasurementRow> &rows, const QString &unit)
{
	setRowCount(rows.size());
	for(int r = 0; r < rows.size(); ++r) {
		const MeasurementRow &row = rows[r];
		const Measurements &m = row.values;
		QPixmap swatch(10, 10);
		swatch.fill(row.color);
		auto *name = new QTableWidgetItem(QIcon(swatch), row.channel);
		setItem(r, 0, name);
		const QStringList cells = {siFormat(m.min, unit),
					   siFormat(m.max, unit),
					   siFormat(m.mean, unit),
					   siFormat(m.rms, unit),
					   siFormat(m.acRms, unit),
					   siFormat(m.pkpk, unit),
					   m.frequency < 0 ? QStringLiteral("—") : siFormat(m.frequency, "Hz"),
					   row.peak.isEmpty() ? QStringLiteral("—") : row.peak};
		for(int c = 0; c < cells.size(); ++c) {
			auto *item = new QTableWidgetItem(cells[c]);
			item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			setItem(r, c + 1, item);
		}
	}
}

PlotMeasurementsArea::PlotMeasurementsArea(QWidget *plot, QTableWidget *table, QWidget *parent)
	: QWidget(parent)
	, m_table(table)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);

	DockableAreaInterface *area = createDockableArea(this);
#ifdef USE_KDDOCKWIDGETS
	if(auto *mw = dynamic_cast<KDDockWidgets::QtWidgets::MainWindow *>(area)) {
		lay->addWidget(mw);
		DockWrapperInterface *plotDock = createDockWrapper("Plot");
		plotDock->setInnerWidget(plot);
		area->addDockWrapper(plotDock, DockableAreaInterface::Direction_TOP);
		DockWrapperInterface *tableDock = createDockWrapper("Measurements");
		tableDock->setInnerWidget(table);
		m_docks = {plotDock, tableDock};
		// Added directly, not through the area: only this way it gets its initial height.
		auto *dw = dynamic_cast<KDDockWidgets::QtWidgets::DockWidget *>(tableDock);
		dw->setAffinities({mw->uniqueName()});
		mw->addDockWidget(dw, KDDockWidgets::Location_OnBottom, nullptr,
				  KDDockWidgets::InitialOption(QSize(0, tableHeight())));
		return;
	}
#endif
	delete area->asWidget();
	m_splitter = new QSplitter(Qt::Vertical, this);
	m_splitter->setChildrenCollapsible(true);
	m_splitter->addWidget(plot);
	m_splitter->addWidget(table);
	m_splitter->setStretchFactor(0, 1);
	m_splitter->setStretchFactor(1, 0);
	lay->addWidget(m_splitter);
}

PlotMeasurementsArea::~PlotMeasurementsArea()
{
	// A floating dock is a window of its own, not a child of this area: it would outlive it.
	for(DockWrapperInterface *dock : std::as_const(m_docks)) {
		delete dynamic_cast<QWidget *>(dock);
	}
}

void PlotMeasurementsArea::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	if(m_splitter && !m_sized) {
		// After the first layout pass, once the splitter has its real height.
		QTimer::singleShot(0, this, &PlotMeasurementsArea::applyInitialSizes);
	}
}

int PlotMeasurementsArea::tableHeight() const
{
	return m_table->horizontalHeader()->sizeHint().height() +
		DefaultRows * m_table->verticalHeader()->defaultSectionSize() + 2 * m_table->frameWidth();
}

void PlotMeasurementsArea::applyInitialSizes()
{
	const int total = m_splitter->height() - m_splitter->handleWidth();
	if(m_sized || total <= 0) {
		return;
	}
	m_sized = true;
	const int table = std::min(tableHeight(), total / 2);
	m_splitter->setSizes({total - table, table});
}

#include "moc_measurementstable.cpp"
