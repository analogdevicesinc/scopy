/*
 * Copyright (c) 2020 Analog Devices Inc.
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
#include "decoder_table_model.hpp"
#include "logic_analyzer.h"
#include "qcombobox.h"
#include <QDebug>
#include <QHeaderView>
#include <QRegExp>

namespace adiscope {


namespace logic {

DecoderTableModel::DecoderTableModel(DecoderTable *decoderTable, LogicAnalyzer *logicAnalyzer):
	QAbstractTableModel(decoderTable),
	m_decoderTable(decoderTable),
	m_logic(logicAnalyzer)
{
	m_plotCurves = m_logic->getPlotCurves(true);
	m_decoderTable->verticalHeader()->setMinimumSectionSize(1);
	m_primary_annoations = new QMap<int, int>();
	m_current_column = 0;
	to_be_refreshed = false;
}

void DecoderTableModel::setDefaultPrimaryAnnotations()
{
	for (int i=DIGITAL_NR_CHANNELS; i<m_plotCurves.size(); i++) {
		int count = 0;
		std::map<Row, RowData>::iterator it;
		AnnotationCurve *curve = dynamic_cast<AnnotationCurve *>(m_plotCurves[i]);
		std::map<Row, RowData> decoder(curve->getAnnotationRows());

		for (it = decoder.begin(); it != decoder.end(); it++) {
			if (!it->second.get_annotations().empty()) {
				count = it->first.index();
				break;
			}
		}
		m_primary_annoations->insert(i, count);
	}
}

int DecoderTableModel::rowCount(const QModelIndex &parent) const
{
	// Return max of all packets
	size_t count = 0;
	if (!m_active) {
		return 0;
	}

	return max_row_count;
}

int DecoderTableModel::columnCount(const QModelIndex &parent) const
{
	// One column per curve
	if (!m_active) {
		return 0;
	}
	return m_decoderTable->getDecoderCruves().size();
}

int DecoderTableModel::indexOfCurve(const AnnotationCurve *curve) const
{
	for (size_t i=0; i < m_curves.size(); i++) {
		if (m_curves[i] == curve) return i;
	}
	return -1;
}

void DecoderTableModel::setPrimaryAnnotation(int index)
{
	auto prev_index = m_primary_annoations->value(m_current_column);
	m_primary_annoations->remove(m_current_column);
	m_primary_annoations->insert(m_current_column, index);
	m_decoderTable->reset();

	if(m_decoderTable->isActive() && m_curves.at(m_current_column)->getMaxAnnotationCount(index) != m_curves.at(m_current_column)->getMaxAnnotationCount(prev_index)) {
		refreshColumn(m_current_column);
	}
}

void DecoderTableModel::populateDecoderComboBox() const
{
	auto decoderComboBox = m_logic->getDecoderComboBox();

	decoderComboBox->clear();
	for (const auto &curve: m_curves) {
		decoderComboBox->addItem(curve->getName());
	}
}


int DecoderTableModel::getCurrentColumn()
{
	return m_current_column;
}


QMap<int, QVector<QString>>& DecoderTableModel::getFiltered()
{
	return m_filteredMessages;
}

void DecoderTableModel::setCurrentRow(int index)
{
	auto curve = dynamic_cast<AnnotationCurve *> (m_plotCurves.at(m_current_column));
	if (index >= curve->getMaxAnnotationCount(m_primary_annoations->value((int) m_current_column))) return;

	m_decoderTable->scrollTo(this->index(index, m_current_column));
	m_decoderTable->selectRow(index);
}

void DecoderTableModel::setMaxRowCount()
{
	uint64_t count = 0;
	std::map<Row, RowData> decoder;
	for (const auto &entry: m_decoderTable->getDecoderCruves()) {
		const auto &curve = dynamic_cast<AnnotationCurve *>(entry);
		count = std::max(count, curve->getMaxAnnotationCount());
	}
	max_row_count = count;
}

void DecoderTableModel::refreshSettings(int column) const
{
	if (column == -1) {
		column = m_current_column;
	}

	populateDecoderComboBox();
	populateFilter(column);
}

void DecoderTableModel::setSearchString(QString str)
{
	searchString = str;
}

void DecoderTableModel::activate()
{
	if (m_logic->runButton()->isChecked()) {
		m_logic->runButton()->click();
	}
	m_logic->enableRunButton(false);
	m_logic->runButton()->setEnabled(false);
	//    m_logic->enableSingleButton(false);

	for (const auto &curve: m_curves) {
		if (curve.isNull()) continue;
		QObject::connect(
					curve,
					&AnnotationCurve::annotationsChanged,
					this,
					&DecoderTableModel::annotationsChanged
					);
	}
	m_active = true;

	m_filteredMessages.clear();
	for (int i=0; i<m_curves.size(); i++) {
		m_filteredMessages.insert(i, QVector<QString>());
	}

	m_plotCurves.remove(0, DIGITAL_NR_CHANNELS);
	populateDecoderComboBox();
	populateFilter(0);
	//    setDefaultPrimaryAnnotations();
	setMaxRowCount();
	to_be_refreshed = true;
}

void DecoderTableModel::deactivate()
{
	m_logic->enableRunButton(true);
	m_logic->runButton()->setEnabled(true);
	//    m_logic->enableSingleButton(true);

	// Disconnect signals
	for (const auto &curve: m_curves) {
		if (curve.isNull()) continue;
		QObject::disconnect(
					curve,
					&AnnotationCurve::annotationsChanged,
					this,
					&DecoderTableModel::annotationsChanged
					);
	}
	m_active = false;
}

void DecoderTableModel::reloadDecoders(bool logic)
{
	// Disconnect signals
	deactivate();

	m_curves.clear();

	// Reconnect signals for all the annotation curves
	for (const auto &curve: m_plotCurves) {
		if (const auto annCurve = dynamic_cast<AnnotationCurve *>(curve)) {
			m_curves.emplace_back(annCurve);
		}
	}

	// Reconnect signals
	activate();

	// Recompute samples
	annotationsChanged();
}

void DecoderTableModel::refreshColumn(double column) const
{
	if (m_plotCurves.empty()) return;

	if (column == -1) {
		column = m_current_column;
	}

	auto curve = dynamic_cast<AnnotationCurve *> (m_plotCurves.at(column));
	const auto verticalHeader = m_decoderTable->verticalHeader();
	int index = curve->getMaxAnnotationCount(m_primary_annoations->value((int) column));

	// resize rows
	int spacing;
	if (m_logic->getTableInfo()) {
		spacing = 10 + curve->m_infoHeight;
	} else {
		spacing = 10;
	}
	int rowHeight = 25;

	int row_count = 0;
	std::map<Row, RowData>::iterator it;
	std::map<Row, RowData> decoder(curve->getAnnotationRows());

	for (it = decoder.begin(); it != decoder.end(); it++) {
		if (!it->second.get_annotations().empty()) {
			row_count ++;
		}
	}

	auto new_height = rowHeight * (row_count - m_filteredMessages.value(column).size()) + spacing;
	if (verticalHeader->minimumSectionSize() != new_height || verticalHeader->maximumSectionSize() != new_height) {
		verticalHeader->setMinimumSectionSize(new_height);
		verticalHeader->setDefaultSectionSize(new_height);
		verticalHeader->setMaximumSectionSize(new_height);
	}

	// hide/show rows
	bool no_rows = true;
	for (int row = 0; row < m_decoderTable->decoderModel()->rowCount(); row++) {
		if (row < index && !searchMask.contains(row)) {
			m_decoderTable->showRow(row);
			no_rows = false;
		} else {
			m_decoderTable->hideRow(row);
		}
	}
	if (no_rows) {
		m_decoderTable->showRow(0);
		verticalHeader->setMinimumSectionSize(1);
		verticalHeader->setDefaultSectionSize(1);
		verticalHeader->setMaximumSectionSize(1);
	}
}

void DecoderTableModel::annotationsChanged()
{
	beginResetModel();
	endResetModel();
}

void DecoderTableModel::selectedDecoderChanged(int index) const
{
	if (index >= 0) {
		m_decoderTable->scrollTo(QAbstractTableModel::index(0, index));
		m_current_column = index;
		m_logic->setPrimaryAnntations(index, m_primary_annoations->value(index));
		if (m_decoderTable->isActive()) {
			refreshColumn(index);
			populateFilter(index);
		}
	}
}

void DecoderTableModel::populateFilter(int index) const
{
	if (index < 0 || m_plotCurves.empty()) return;
	auto temp_curve = dynamic_cast<AnnotationCurve *>(m_plotCurves.at(index));
	std::map<Row, RowData> decoder(temp_curve->getAnnotationRows());
	m_logic->clearFilter();

	int count = 0;
	for (auto row_map: decoder) {
		if (!row_map.second.get_annotations().empty()) {
			count ++;
			auto title = row_map.first.title();
			if (title.indexOf(':')) {
				m_logic->addFilterRow(QIcon(), title.mid(title.indexOf(':') + 1));
			}
			else {
				m_logic->addFilterRow(QIcon(), title);
			}
		}
	}
}

QVariant DecoderTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || m_decoderTable->signalsBlocked() || m_plotCurves.empty()) return QVariant();

	const size_t col = index.column();
	if ( col >= m_curves.size() ) return QVariant();
	const auto &curve = m_curves.at(col);
	if (curve.isNull()) return QVariant();

	if (curve->getAnnotationRows().size() == 0) {
		return QVariant();
	}

	if (index.row() < m_decoderTable->rowAt(m_decoderTable->rect().top()) ||
			(m_decoderTable->rowAt(m_decoderTable->rect().bottom()) < index.row() &&
			 m_decoderTable->rowAt(m_decoderTable->rect().bottom()) != -1)) return QVariant();

	auto temp_curve = dynamic_cast<AnnotationCurve *>(m_plotCurves.at(index.column()));
	std::map<Row, RowData> decoder(temp_curve->getAnnotationRows());
	vector<Annotation> row;

	for (auto row_map: decoder) {
		row = row_map.second.get_annotations();
		if (!row.empty() && row_map.first.index() == m_primary_annoations->value(index.column())) {
			break;
		}
	}

	if (row.empty()) {
		return QVariant();
	}

	if (to_be_refreshed) {
		refreshColumn(col);
		to_be_refreshed = false;
		if (to_be_refreshed == 2) selectedDecoderChanged(col);
	}

	if (searchMask.count() == curve->getMaxAnnotationCount(m_primary_annoations->value(index.column()))) {
		return QVariant();
	}

	return QVariant::fromValue(DecoderTableItem(
					   temp_curve,
					   row[index.row()].start_sample(),
				   row[index.row()].end_sample(),
			m_filteredMessages.value(index.column()),
			m_logic->getTableInfo()
			));
}

void DecoderTableModel::searchBoxSignal(QString text)
{
	setSearchString(text);
	searchMask.clear();

	if (!searchString.isEmpty()) {
		QRegExp rx =  QRegExp(searchString, Qt::CaseInsensitive);

		auto temp_curve = dynamic_cast<AnnotationCurve *>(m_plotCurves.at(m_current_column));
		std::map<Row, RowData> decoder(temp_curve->getAnnotationRows());
		vector<Annotation> row;
		int row_index = -1;
		QString primary_title;

		// get primary annotation
		for (auto row_map: decoder) {
			row = row_map.second.get_annotations();
			primary_title = (row_map.first.title().indexOf(':')) ? row_map.first.title().mid(row_map.first.title().indexOf(':') + 1)
									     : row_map.first.title();
			if (!row.empty() && row_map.first.index() == m_primary_annoations->value(m_current_column)) {
				break;
			}
		}

		for (auto sample_area: row) {
			auto start_sample = sample_area.start_sample();
			auto end_sample = sample_area.end_sample();
			row_index++;

			for (auto row_map: decoder) {
				QString title = (row_map.first.title().indexOf(':')) ? row_map.first.title().mid(row_map.first.title().indexOf(':') + 1)
										     : row_map.first.title();
				if (m_filteredMessages.value(m_current_column).contains(title)) continue;

				for (auto ann: row_map.second.get_annotations()) {

					if ((title != primary_title &&
					     ((unsigned)(ann.end_sample()-start_sample-1) <= (end_sample-start_sample-1) ||
					      (unsigned)(ann.start_sample()-start_sample) < (end_sample-start_sample)) ||
					     ann.start_sample() <= start_sample && ann.end_sample() >= end_sample) ||
							(start_sample <= ann.start_sample() && ann.end_sample() <= end_sample)) {

						for (auto value: ann.annotations()) {
							if (rx.indexIn(value) != -1) {
								goto skip_loop;
							}
						}
					}
				}
			}
			// add to mask if not found
			searchMask.append(row_index);
skip_loop:
			continue;
		}
	}
	m_decoderTable->reset();
	to_be_refreshed = true;
}

} // namespace logic
} // namespace adiscope
