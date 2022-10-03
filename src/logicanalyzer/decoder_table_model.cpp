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
	m_current_column = 1;
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
//				count = it->second.get_annotations().size();
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
    std::map<Row, RowData> decoder;
    for (const auto &entry: m_decoderTable->getDecoderCruves()) {
	const auto &curve = dynamic_cast<AnnotationCurve *>(entry);
	decoder = curve->getAnnotationRows();
	std::map<Row, RowData>::iterator it;
	for (it = decoder.begin(); it != decoder.end(); it++) {
		count = std::max(count, it->second.get_annotations().size());
	}

    }
    return count;
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

void DecoderTableModel::populateDecoderComboBox()
{
	auto decoderComboBox = m_logic->getDecoderComboBox();

	decoderComboBox->clear();
	for (const auto &curve: m_curves) {
	    decoderComboBox->addItem(curve->getName());
	}
}

void DecoderTableModel::activate()
{
    if (m_logic->runButton()->isChecked()) {
	    m_logic->runButton()->click();
    }
    m_logic->enableRunButton(false);
    m_logic->runButton()->setEnabled(false);
    m_logic->enableSingleButton(false);

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

    m_plotCurves.remove(0, DIGITAL_NR_CHANNELS);
    populateDecoderComboBox();
//    setDefaultPrimaryAnnotations();
    refreshColumn(0);
}

void DecoderTableModel::deactivate()
{
    m_logic->enableRunButton(true);
    m_logic->runButton()->setEnabled(true);
    m_logic->enableSingleButton(true);

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
	// hide/show rows
	auto curve = dynamic_cast<AnnotationCurve *> (m_plotCurves.at(column));
	const auto verticalHeader = m_decoderTable->verticalHeader();

	if (curve->getMaxAnnotationCount() == 0) {
		verticalHeader->setDefaultSectionSize(1);
		return;
	}

	int index = curve->getMaxAnnotationCount(m_primary_annoations->value((int) column));
	if (m_decoderTable->isRowHidden(index - 1) || !m_decoderTable->isRowHidden(index)) {
		for (int row = 0; row < m_decoderTable->decoderModel()->rowCount(); row++) {
			if (row < index) {
				m_decoderTable->showRow(row);
			}
			else {
				m_decoderTable->hideRow(row);
			}
		}

	}

	// resize rows
	const int spacing = 10;
	int rowHeight = 20;

	int row_count = 0;
	std::map<Row, RowData>::iterator it;
	std::map<Row, RowData> decoder(curve->getAnnotationRows());

	for (it = decoder.begin(); it != decoder.end(); it++) {
		if (!it->second.get_annotations().empty()) {
			row_count ++;
		}
	}

	if (verticalHeader->defaultSectionSize() != rowHeight * row_count + spacing) {
		rowHeight = std::max(rowHeight, static_cast<int>(curve->getTraceHeight()));
		verticalHeader->setDefaultSectionSize(rowHeight * row_count + spacing);
	}
}

void DecoderTableModel::annotationsChanged()
{
    beginResetModel();

//    int visibleRows = 0;
//    int rowHeight = 20;
//    for (const auto &entry: m_decoderTable->getDecoderCruves()) {
//	const auto &curve = dynamic_cast<AnnotationCurve *>(entry);
//	if (curve->isVisible()) continue;
//	int n = curve->getVisibleRows();
//	if (curve->getAnnotationRows().size() == 0) {
//		return;
//	}
//        for (const auto &rowmap: curve->getAnnotationRows()) {
//            const Row &row = rowmap.first;
//            const RowData &data = rowmap.second;
//	    const auto i = row.index();

//        }

//        visibleRows = std::max(visibleRows, n);
//	rowHeight = std::max(rowHeight, static_cast<int>(curve->getTraceHeight()));
//    }
    endResetModel();
	/*

    const auto verticalHeader = m_decoderTable->verticalHeader();
    const int spacing = 10;
    verticalHeader->setDefaultSectionSize(rowHeight * visibleRows + spacing);
    verticalHeader->sectionResizeMode(QHeaderView::Fixed);*/
}

void DecoderTableModel::selectedDecoderChanged(int index)
{
	if (index >= 0) {
		m_decoderTable->scrollTo(QAbstractTableModel::index(0, index));
		m_current_column = index;
		bool changed = m_logic->setPrimaryAnntations(index, m_primary_annoations->value(index));
		if (m_decoderTable->isActive()) {
			refreshColumn(index);
		}
	}
}

QVariant DecoderTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole and orientation == Qt::Horizontal) {
        if (section < 0 or static_cast<size_t>(section) >= m_curves.size()) {
            return QVariant();
        }
	if (m_plotCurves.size() > section) {
		return m_plotCurves.at(section)->getName();
	}
	return QVariant();
   }
    m_current_column = section;
    bool changed = m_logic->setPrimaryAnntations(section, m_primary_annoations->value(section));

    return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant DecoderTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();

	const size_t col = index.column();
	if ( col >= m_curves.size() ) return QVariant();
	const auto &curve = m_curves.at(col);
	if (curve.isNull()) return QVariant();

	if (index.row() >= curve->getMaxAnnotationCount(m_primary_annoations->value(index.column()))) {
		return QVariant();
	}

	const size_t row = index.row();

	if (curve->getAnnotationRows().size() == 0) {
		return QVariant();
	}

	auto temp_curve = dynamic_cast<AnnotationCurve *>(m_plotCurves.at(index.column()));
	std::map<Row, RowData> decoder(temp_curve->getAnnotationRows());
	std::map<Row, RowData>::iterator it;
	vector<Annotation> roww;

	for (auto row_map: decoder) {
		roww = row_map.second.get_annotations();
		if (!roww.empty() && row_map.first.index() == m_primary_annoations->value(index.column())) {
			break;
		}
	}

	if (roww.empty()) {
		return QVariant();
	}

	return QVariant::fromValue(DecoderTableItem(
					   temp_curve,
					   roww[index.row()].start_sample(),
				   roww[index.row()].end_sample()
			));
}

} // namespace logic
} // namespace adiscope
