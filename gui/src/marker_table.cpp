/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "marker_table.hpp"
#include "ui_marker_table.h"
#include "plot_utils.hpp"

#include <QStandardItemModel>
#include <QStyledItemDelegate>

using namespace scopy;

/*
 * Class FrequencyDelegate
 */
class FrequencyDelegate : public QStyledItemDelegate
{
public:
	FrequencyDelegate(QObject *parent = 0): QStyledItemDelegate(parent)
	{
	}

	virtual QString displayText(const QVariant &value, const QLocale &locale) const
	{
		Q_UNUSED(locale)

		double freq = value.toDouble();
		return formatter.format(freq, "Hz", 3);
	}
private:
	scopy::MetricPrefixFormatter formatter;
};

/*
 * Class ChannelDelegate
 */
class ChannelDelegate : public QStyledItemDelegate
{
public:
	ChannelDelegate(QObject *parent = 0): QStyledItemDelegate(parent)
	{
	}

	virtual QString displayText(const QVariant &value, const QLocale &locale) const
	{
		Q_UNUSED(locale)

		int mkId = value.toInt();
		return QString::number(mkId + 1);
	}
};

/*
 * Class MarkerTable
 */

MarkerTable::MarkerTable(QWidget *parent) :
QWidget(parent),
ui(new Ui::MarkerTable)
{
	ui->setupUi(this);

	model = new QStandardItemModel(0, Columns::NUM_COLUMNS, this);
	model->setHeaderData(COL_ID, Qt::Horizontal, QVariant(tr("Id")));
	model->setHeaderData(COL_NAME, Qt::Horizontal, QVariant(tr("Marker")));
	model->setHeaderData(COL_CH, Qt::Horizontal, QVariant(tr("Channel")));
	model->setHeaderData(COL_FREQ, Qt::Horizontal, QVariant(tr("Frequency")));
	model->setHeaderData(COL_MAGN, Qt::Horizontal, QVariant(tr("Magnitude")));
	model->setHeaderData(COL_TYPE, Qt::Horizontal, QVariant(tr("Type")));

	ui->tableView->setModel(model);
	ui->tableView->setSortingEnabled(true);
	ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);
	ui->tableView->horizontalHeader()->setSectionResizeMode(
		QHeaderView::ResizeToContents);
	ui->tableView->horizontalHeader()->setHighlightSections(false);
	ui->tableView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->hideColumn(COL_ID);
	ui->tableView->setShowGrid(false);
	ui->tableView->setSelectionMode(QAbstractItemView::NoSelection);
	ui->tableView->setStyleSheet("font-size: 14px;");

	FrequencyDelegate *freqDelegate = new FrequencyDelegate(this);
	ui->tableView->setItemDelegateForColumn(COL_FREQ, freqDelegate);

	ChannelDelegate *chnDelegate = new ChannelDelegate(this);
	ui->tableView->setItemDelegateForColumn(COL_CH, chnDelegate);
}

MarkerTable::~MarkerTable()
{
	delete ui;
}

int MarkerTable::rowOfMarker(int mkIdx, int chIdx) const
{
	int markerRow = -1;

	for (int r = 0; r < model->rowCount(); r++) {
		int id = model->item(r, COL_ID)->data(Qt::DisplayRole).toInt();
		int ch = model->item(r, COL_CH)->data(Qt::DisplayRole).toInt();

		if (id == mkIdx && ch == chIdx) {
			markerRow = r;
			break;
		}
	}

	return markerRow;
}

void MarkerTable::addMarker(int mkIdx, int chIdx, const QString& name,
		double frequency, double magnitude, const QString& type)
{
	model->insertRow(0);
	model->setData(model->index(0, COL_ID), mkIdx);
	model->setData(model->index(0, COL_NAME), name);
	model->setData(model->index(0, COL_CH), chIdx);
	model->setData(model->index(0, COL_FREQ), frequency);
	model->setData(model->index(0, COL_MAGN), magnitude);
	model->setData(model->index(0, COL_TYPE), type);
}

void MarkerTable::removeMarker(int mkIdx, int chIdx)
{
	int row = rowOfMarker(mkIdx, chIdx);

	if (row < 0) {
		return;
	}

	model->removeRow(row);
}

void MarkerTable::updateMarker(int mkIdx, int chIdx, double frequency,
	double magnitude, const QString &type)
{
	int row = rowOfMarker(mkIdx, chIdx);

	if (row < 0) {
		return;
	}

	model->item(row, COL_FREQ)->setData(frequency, Qt::DisplayRole);
	model->item(row, COL_MAGN)->setData(magnitude, Qt::DisplayRole);
	model->item(row, COL_TYPE)->setData(type, Qt::DisplayRole);
}

bool MarkerTable::isMarker(int mkIdx, int chIdx)
{
	if (rowOfMarker(mkIdx, chIdx) > 0) {
		return true;
	}
	return false;
}
