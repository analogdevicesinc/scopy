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
#include "measure_settings.h"
#include "oscilloscope_plot.hpp"
#include "dropdown_switch_list.h"
#include "ui_measure_settings.h"

#include <QStandardItem>
#include <QTreeView>
#include <QHeaderView>

using namespace adiscope;

static const std::map<int, QString> icons_lut = {
	{Measure::PERIOD, "://icons/measurements/period.svg"},
	{Measure::FREQUENCY, "://icons/measurements/frequency.svg"},
	{Measure::MIN, "://icons/measurements/min.svg"},
	{Measure::MAX, "://icons/measurements/max.svg"},
	{Measure::PEAK_PEAK, "://icons/measurements/peak_to_peak.svg"},
	{Measure::MEAN, "://icons/measurements/mean.svg"},
	{Measure::CYCLE_MEAN, "://icons/measurements/cycle_mean.svg"},
	{Measure::RMS, "://icons/measurements/rms.svg"},
	{Measure::CYCLE_RMS, "://icons/measurements/cycle_rms.svg"},
	{Measure::AC_RMS, "://icons/measurements/rms.svg"},
	{Measure::AREA, "://icons/measurements/area.svg"},
	{Measure::CYCLE_AREA, "://icons/measurements/cycle_area.svg"},
	{Measure::LOW, "://icons/measurements/low.svg"},
	{Measure::HIGH, "://icons/measurements/high.svg"},
	{Measure::AMPLITUDE, "://icons/measurements/amplitude.svg"},
	{Measure::MIDDLE, "://icons/measurements/middle.svg"},
	{Measure::P_OVER, "://icons/measurements/p_overshoot.svg"},
	{Measure::N_OVER, "://icons/measurements/n_overshoot.svg"},
	{Measure::RISE, "://icons/measurements/rise_time.svg"},
	{Measure::FALL, "://icons/measurements/fall_time.svg"},
	{Measure::P_WIDTH, "://icons/measurements/p_width.svg"},
	{Measure::N_WIDTH, "://icons/measurements/n_width.svg"},
	{Measure::P_DUTY, "://icons/measurements/p_duty.svg"},
	{Measure::N_DUTY, "://icons/measurements/n_duty.svg"},
};

MeasureSettings::MeasureSettings(CapturePlot *plot, QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::MeasureSettings),
	m_channelName(""),
	m_chnUnderlineColor(),
	m_horizMeasurements(new DropdownSwitchList(2, this)),
	m_vertMeasurements(new DropdownSwitchList(2, this)),
	m_emitActivated(true),
	m_emitStatsChanged(true),
	m_emitStatsDeleteAll(true),
	m_emitDeleteAll(true),
	m_are_dropdowns_filled(false),
	m_plot(plot),
	m_selectedChannel(-1),
	m_enableDisplayAll(false)
{
	QTreeView *treeView;

	m_ui->setupUi(this);
	m_ui->hLayout_hMeasurements->addWidget(m_horizMeasurements);
	m_ui->hLayout_vMeasurements->addWidget(m_vertMeasurements);

	m_horizMeasurements->setTitle("Horizontal");
	m_horizMeasurements->setColumnTitle(0, "Name");
	m_horizMeasurements->setColumnTitle(1, "Measure");
	m_horizMeasurements->setColumnTitle(2, "Stats");
	m_horizMeasurements->setMaxVisibleItems(4);
	treeView = static_cast<QTreeView *>(m_horizMeasurements->view());
	treeView->header()->resizeSection(0, 122);
	treeView->setIconSize(QSize(30, 20));

	connect(m_horizMeasurements->model(),
		SIGNAL(itemChanged(QStandardItem*)),
		SLOT(onMeasurementPropertyChanged(QStandardItem*)));

	m_vertMeasurements->setTitle("Vertical");
	m_vertMeasurements->setColumnTitle(0, "Name");
	m_vertMeasurements->setColumnTitle(1, "Measure");
	m_vertMeasurements->setColumnTitle(2, "Stats");
	m_vertMeasurements->setMaxVisibleItems(4);
	treeView = static_cast<QTreeView *>(m_vertMeasurements->view());
	treeView->header()->resizeSection(0, 122);
	treeView->setIconSize(QSize(30, 20));

	connect(m_vertMeasurements->model(),
		SIGNAL(itemChanged(QStandardItem*)),
		SLOT(onMeasurementPropertyChanged(QStandardItem*)));
}

MeasureSettings::~MeasureSettings()
{
	delete m_ui;
}

QString MeasureSettings::channelName() const
{
	return m_channelName;
}

void MeasureSettings::setChannelName(const QString& name)
{
	m_channelName = name;
	m_ui->lblChanName->setText(name);
}

QColor MeasureSettings::channelUnderlineColor() const
{
	return m_chnUnderlineColor;
}

void MeasureSettings::setChannelUnderlineColor(const QColor& color)
{
	m_chnUnderlineColor = color;
	QString stylesheet = QString("border: 2px solid %1;"
				).arg(color.name());

	m_ui->line->setStyleSheet(stylesheet);
}

bool MeasureSettings::emitActivated() const
{
	return m_emitActivated;
}
void MeasureSettings::setEmitActivated(bool en)
{
	m_emitActivated = en;
}

bool MeasureSettings::emitStatsChanged()const
{
	return m_emitStatsChanged;
}
void MeasureSettings::setEmitStatsChanged(bool en)
{
	m_emitStatsChanged = en;
}

QList<MeasurementItem> MeasureSettings::measurementSelection()
{
	return m_selectedMeasurements;
}

void MeasureSettings::onMeasurementPropertyChanged(QStandardItem *item)
{
	QAbstractItemModel *model = static_cast<QAbstractItemModel *>
		(QObject::sender());
	DropdownSwitchList *dropdown = static_cast<DropdownSwitchList *>
		(model->parent());
	bool en = item->data(Qt::EditRole).toBool();
	QStandardItem *nameItem = item->model()->item(item->row(), 0);
	int id = nameItem->data(Qt::UserRole).toInt();

	if (item->column() == 1) {
		if (m_emitActivated)
			onMeasurementActivated(m_selectedChannel, id, en);
	} else if (item->column() == 2) {
		if (m_emitStatsChanged)
			onStatisticActivated(dropdown, id, en);
	}

	// Switch from Recover to Delete All if a measurement state gets changed
	if (item->column() == 1 && m_emitActivated &&
			m_ui->button_measDeleteAll->isChecked()) {
		m_emitDeleteAll = false;
		m_ui->button_measDeleteAll->setChecked(false);
	}

	// Disable Display All if a measurement state gets changed
	if (item->column() == 1 &&m_emitActivated &&
			m_ui->button_measDisplayAll->isChecked()) {
		m_ui->button_measDisplayAll->setChecked(false);
	}

	// Switch from Recover to Delete All if a statistic state gets changed
	if (item->column() == 2 && m_emitStatsChanged &&
			m_ui->button_statsDeleteAll->isChecked()) {
		m_emitStatsDeleteAll = false;
		m_ui->button_statsDeleteAll->setChecked(false);
	}
}

void MeasureSettings::on_button_measDisplayAll_toggled(bool checked)
{
	m_ui->button_measDeleteAll->setEnabled(!checked);

	if (checked)
		displayAllMeasurements();
	else
		disableDisplayAllMeasurements();
}

void MeasureSettings::on_button_measDeleteAll_toggled(bool checked)
{
	QPushButton *button = static_cast<QPushButton*>(QObject::sender());
	if (checked)
		button->setText("Recover");
	else
		button->setText("Delete All");

	if (!m_emitDeleteAll) {
		m_emitDeleteAll = true;
	} else {
		if (checked)
			deleteAllMeasurements();
		else
			recoverAllMeasurements();
	}
}

void MeasureSettings::onChannelAdded(int chnIdx)
{
	// Use the measurements of the 1st channel to construct the dropdowns.
	// All channels have the same set of measurements.
	if (m_are_dropdowns_filled)
		return;

	auto measurements = m_plot->measurements(chnIdx);
	int h = 0;
	int v = 0;

	for (int i = 0; i < measurements.size(); i++) {
		enum MeasurementData::axisType axis = measurements[i]->axis();

		if (axis == MeasurementData::HORIZONTAL) {
			m_horizMeasurements->addDropdownElement(
					QIcon(icons_lut.at(i)),
					measurements[i]->name(), QVariant(i));
			m_measurePosInDropdowns.append(h++);
		} else if (axis == MeasurementData::VERTICAL) {
			m_vertMeasurements->addDropdownElement(
					QIcon(icons_lut.at(i)),
					measurements[i]->name(), QVariant(i));
			m_measurePosInDropdowns.append(v++);
		}
	}

	m_are_dropdowns_filled = true;
}

void MeasureSettings::onChannelRemoved(int chnIdx)
{
	deleteMeasurementsOfChannel(m_selectedMeasurements, chnIdx);
	updateMeasurementsOnChannelDel(m_selectedMeasurements, chnIdx);
	Q_EMIT measurementSelectionListChanged();

	deleteMeasurementsOfChannel(m_deleteAllBackup, chnIdx);
	deleteMeasurementsOfChannel(m_displayAllBackup, chnIdx);

	deleteStatisticsOfChannel(m_selectedStatistics, chnIdx);
	Q_EMIT statisticSelectionListChanged();

	deleteStatisticsOfChannel(m_statsDeleteAllBackup, chnIdx);
}

void MeasureSettings::setSelectedChannel(int chnIdx)
{
	if (m_selectedChannel != chnIdx) {
		m_selectedChannel = chnIdx;
		loadMeasurementStatesFromData();
		loadStatisticStatesForChannel(chnIdx);
		if (m_ui->button_measDisplayAll->isChecked()){
			disableDisplayAllMeasurements();
			displayAllMeasurements();
		}
	}
}

void MeasureSettings::loadMeasurementStatesFromData()
{
	auto measurements = m_plot->measurements(m_selectedChannel);
	int h_idx = 0;
	int v_idx = 0;

	QStandardItemModel *horiz_model =
		static_cast<QStandardItemModel *>(m_horizMeasurements->model());
	QStandardItemModel *vert_model =
		static_cast<QStandardItemModel *>(m_vertMeasurements->model());

	setEmitActivated(false);

	for (int i = 0; i < measurements.size(); i++) {
		int axis = measurements[i]->axis();
		int state = measurements[i]->enabled();

		if (axis == MeasurementData::HORIZONTAL) {
			horiz_model->item(h_idx++, 1)->setData(
					QVariant((int) state), Qt::EditRole);
		} else if (axis == MeasurementData::VERTICAL) {
			vert_model->item(v_idx++, 1)->setData(
					QVariant((int) state), Qt::EditRole);
		}
	}

	setEmitActivated(true);
}

void MeasureSettings::deleteAllMeasurements()
{
	m_deleteAllBackup = m_selectedMeasurements;
	m_selectedMeasurements.clear();
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::recoverAllMeasurements()
{
	m_selectedMeasurements = m_deleteAllBackup;
	m_deleteAllBackup.clear();
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::displayAllMeasurements()
{
	m_displayAllBackup = m_selectedMeasurements;
	m_selectedMeasurements.clear();
	auto measurements = m_plot->measurements(m_selectedChannel);
	for (int i = 0; i < measurements.size(); i++) {
		MeasurementItem item(i, measurements[i]->channel());
		m_selectedMeasurements.push_back(MeasurementItem(item));
	}
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::disableDisplayAllMeasurements()
{
	m_selectedMeasurements = m_displayAllBackup;
	Q_EMIT measurementSelectionListChanged();
	loadMeasurementStatesFromData();
}

void MeasureSettings::onMeasurementActivated(int chnIdx, int id, bool en)
{
	if (chnIdx < 0)
		return;

	auto measurements = m_plot->measurements(chnIdx);
	MeasurementItem mItem(id, measurements[id]->channel());
	if (en) {
		m_selectedMeasurements.push_back(mItem);
		Q_EMIT measurementActivated(mItem.id(), mItem.channel_id());
	} else {
		m_selectedMeasurements.removeOne(mItem);
		Q_EMIT measurementDeactivated(mItem.id(), mItem.channel_id());
	}
}

void MeasureSettings::onStatisticActivated(DropdownSwitchList *dropdown,
	int id, bool en)
{
	if (m_selectedChannel < 0)
		return;

	if (en) {
		addStatistic(id, m_selectedChannel);
		Q_EMIT statisticActivated(id, m_selectedChannel);
	} else {
		removeStatistic(id, m_selectedChannel);
		Q_EMIT statisticDeactivated(id, m_selectedChannel);
	}
}

void MeasureSettings::loadStatisticStatesForChannel(int chnIdx)
{
	const int stats_col = 2;

	setEmitStatsChanged(false);

	// Start with all selections cleared
	setAllMeasurements(stats_col, false);

	// Restore selections that are present in the selected statistics list
	for (int i = 0; i < m_selectedStatistics.size(); i++) {
		if (m_selectedStatistics[i].measurementItem.channel_id() !=
				chnIdx) {
			continue;
		}
		QStandardItemModel *model = static_cast<QStandardItemModel *>
			(m_selectedStatistics[i].dropdown->model());

		int measId = m_selectedStatistics[i].measurementItem.id();

		model->item(m_measurePosInDropdowns[measId], stats_col)->
			setData(QVariant(1), Qt::EditRole);
	}
	setEmitStatsChanged(true);
}

void MeasureSettings::on_button_StatisticsEn_toggled(bool checked)
{
	Q_EMIT statisticsEnabled(checked);
}

void MeasureSettings::on_button_StatisticsReset_pressed()
{
	Q_EMIT statisticsReset();
}

void MeasureSettings::setColumnData(QStandardItemModel *model, int column,
		bool en)
{
	int val = en ? 1 : 0;

	for (int i = 0; i < model->rowCount(); i++)
		model->item(i, column)->setData(QVariant(val), Qt::EditRole);
}

void MeasureSettings::setAllMeasurements(int col, bool en)
{
	QStandardItemModel *model;

	model = static_cast<QStandardItemModel *>(m_horizMeasurements->model());
	setColumnData(model, col, en);

	model = static_cast<QStandardItemModel *>(m_vertMeasurements->model());
	setColumnData(model, col, en);
}

void MeasureSettings::deleteMeasurementsOfChannel(QList<MeasurementItem>& list,
	int chnIdx)
{
	QMutableListIterator<MeasurementItem> i(list);
	while (i.hasNext()) {
		i.next();
		if (i.value().channel_id() == chnIdx)
			i.remove();
	}
}

void MeasureSettings::updateMeasurementsOnChannelDel(QList<MeasurementItem>& list,
						     int chnIdx)
{
	int listSize = list.size();
	int idx = 0;
	int currentItemIdx = 0;
	while (idx < listSize) {
		auto item = m_selectedMeasurements.at(currentItemIdx);
		if (item.channel_id() > chnIdx) {
			Q_EMIT measurementDeactivated(item.id(), item.channel_id());
			m_selectedMeasurements.push_back(MeasurementItem(item.id(),
									 item.channel_id() - 1));
			Q_EMIT measurementActivated(item.id(), item.channel_id() - 1);
			m_selectedMeasurements.removeAt(currentItemIdx);
		} else {
			currentItemIdx++;
		}
		idx++;
	}
}

void MeasureSettings::deleteStatisticsOfChannel(
	QList<struct StatisticSelection>& list, int chnIdx)
{
	QMutableListIterator<struct StatisticSelection> i(list);
	while (i.hasNext()) {
		i.next();
		if (i.value().measurementItem.channel_id() == chnIdx)
			i.remove();
	}
}

QList<MeasurementItem> MeasureSettings::statisticSelection()
{
	QList<MeasurementItem> statistics;

	for (int i = 0; i < m_selectedStatistics.size(); i++)
		statistics.push_back(m_selectedStatistics[i].measurementItem);

	return statistics;

}

void MeasureSettings::on_button_statsDeleteAll_toggled(bool checked)
{
	QPushButton *button = static_cast<QPushButton*>(QObject::sender());
	if (checked)
		button->setText("Recover");
	else
		button->setText("Delete All");

	if (!m_emitStatsDeleteAll) {
		m_emitStatsDeleteAll = true;
	} else {
		if (checked)
			deleteAllStatistics();
		else
			recoverAllStatistics();
	}
}

void MeasureSettings::on_button_GatingEnable_toggled(bool checked){
	Q_EMIT gatingEnabled(checked);
}

void MeasureSettings::disableDisplayAll()
{
	if (m_ui->button_measDisplayAll->isChecked()){
		m_ui->button_measDisplayAll->setChecked(false);
		m_enableDisplayAll = true;
	} else {
		m_enableDisplayAll = false;
	}
}

void MeasureSettings::activateDisplayAll(){
	m_ui->button_measDisplayAll->setChecked(m_enableDisplayAll);
}

void MeasureSettings::deleteAllStatistics()
{
	m_statsDeleteAllBackup = m_selectedStatistics;
	m_selectedStatistics.clear();
	Q_EMIT statisticSelectionListChanged();
	setEmitStatsChanged(false);
	setAllMeasurements(2, false); // clear all statistic checkboxes
	setEmitStatsChanged(true);
}

void MeasureSettings::recoverAllStatistics()
{
	m_selectedStatistics = m_statsDeleteAllBackup;
	m_statsDeleteAllBackup.clear();
	Q_EMIT statisticSelectionListChanged();
	loadStatisticStatesForChannel(m_selectedChannel);
}

void MeasureSettings::addStatistic(int measure_id, int ch_id)
{
	auto measurements = m_plot->measurements(ch_id);
	MeasurementData::axisType axis = measurements[measure_id]->axis();
	struct StatisticSelection selection;

	if (axis == MeasurementData::HORIZONTAL)
		selection.dropdown = m_horizMeasurements;
	else if (axis == MeasurementData::VERTICAL)
		selection.dropdown = m_vertMeasurements;
	else
		return;

	selection.measurementItem = MeasurementItem(measure_id, ch_id);
	m_selectedStatistics.push_back(selection);
}

void MeasureSettings::removeStatistic(int measure_id, int ch_id)
{
	MeasurementItem item(measure_id, ch_id);

	auto it = std::find_if(m_selectedStatistics.begin(),
			m_selectedStatistics.end(),
			[&](struct StatisticSelection s) {
				return s.measurementItem.id() == measure_id &&
					s.measurementItem.channel_id() == ch_id;
			});
		if (it != m_selectedStatistics.end()) {
			m_selectedStatistics.erase(it);
		}
}
