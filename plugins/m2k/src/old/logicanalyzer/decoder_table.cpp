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
#include "decoder_table.hpp"
#include "decoder_table_model.hpp"
#include "decoder_table_item.hpp"
#include <QFileDialog>
#include <QFuture>
#include <QFutureWatcher>
#include <QHeaderView>
#include "logic_analyzer.h"
#include "filemanager.h"
#include "qtconcurrentrun.h"

namespace adiscope::m2k {


namespace logic {


DecoderTable::DecoderTable(QWidget *parent) : QTableView(parent)
{
	setItemDelegate(new DecoderTableItemDelegate);

	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setMinimumSectionSize(500);
	horizontalHeader()->sectionResizeMode(QHeaderView::Interactive);
	horizontalScrollBar()->setEnabled(false);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	verticalHeader()->hide();
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	setAutoScroll(false);
	installEventFilter(this);
}

bool DecoderTable::eventFilter(QObject* object, QEvent* event)
{
	// prevent user from swiping through columns using touchscreen

	if (event->type() == QEvent::Paint && tableModel->getCurrentColumn() != horizontalScrollBar()->value()) {
		horizontalScrollBar()->setValue(tableModel->getCurrentColumn());
		return true;
	}
	return false;
}

void DecoderTable::setLogicAnalyzer(LogicAnalyzer *logicAnalyzer)
{
	if (logicAnalyzer != nullptr) {
		if (const auto oldDecoderModel = dynamic_cast<DecoderTableModel*>(model())) {
			if (oldDecoderModel != nullptr) {
				delete oldDecoderModel;
			}
		}
		tableModel = new DecoderTableModel(this, logicAnalyzer);
		setModel(tableModel);
	}

	m_logicAnalyzer = logicAnalyzer;
}

QVector<GenericLogicPlotCurve *> DecoderTable::getDecoderCruves()
{
	QVector<GenericLogicPlotCurve *> temp_curves = m_logicAnalyzer->getPlotCurves(true);
	temp_curves.remove(0, DIGITAL_NR_CHANNELS);
	return temp_curves;
}

DecoderTableModel* DecoderTable::decoderModel() const
{
	return dynamic_cast<DecoderTableModel*>(model());
}

void DecoderTable::exportData()
{
	QString selectedFilter;
	QStringList filter;
	QString fileType = "";

	filter += QString(tr("Comma-separated row per sample (*.csv)"));
	filter += QString(tr("Tab-delimited row per annotation (*.txt)"));
	QString fileName = QFileDialog::getSaveFileName(this,
							tr("Export"), "", filter.join(";;"),
							&selectedFilter, QFileDialog::DontUseNativeDialog);

	if (fileName.isEmpty()) {
		return;
	}

	// Check the selected file type
	if (selectedFilter != "") {
		if(selectedFilter.contains("comma", Qt::CaseInsensitive)) {
			fileType = "csv";
		}
		if(selectedFilter.contains("tab", Qt::CaseInsensitive)) {
			fileType = "txt";
		}
	}

	if (fileName.split(".").size() <= 1) {
		// file name w/o extension. Let's append it
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}

	m_logicAnalyzer->setStatusLabel("Exporting ...");

	QFuture<void> future;
	if (fileType == "csv") {
		future = QtConcurrent::run(this, &DecoderTable::exportCsv, fileName);
	} else if (fileType == "txt") {
		future = QtConcurrent::run(this, &DecoderTable::exportTxt, fileName);
	}
	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);

	connect(watcher, &QFutureWatcher<void>::finished, this, [=](){
		m_logicAnalyzer->setStatusLabel("");
	});
	connect(watcher, SIGNAL(finished()), watcher, SLOT(deleteLater()));
	watcher->setFuture(future);
}

QVector<QPair<uint64_t, uint64_t>> DecoderTable::getSearchSampleMask()
{
	// convert the table row search mask to a sample area mask

	auto curve = dynamic_cast<AnnotationCurve *>(getDecoderCruves().at(tableModel->getCurrentColumn()));
	std::map<Row, RowData> decoder(curve->getAnnotationRows());
	vector<Annotation> row;
	QVector<QPair<uint64_t, uint64_t>> sampleMask;

	for (const auto &row_map : decoder) {
		if (row_map.first.index() == tableModel->getPrimaryAnnotationIndex()) {
			row = row_map.second.get_annotations();
			break;
		}
	}

	for (auto index: tableModel->getSearchMask()) {
		int size = m_logicAnalyzer->getGroupSize();
		int off = (m_logicAnalyzer->getGroupOffset() == 0) ? size : m_logicAnalyzer->getGroupOffset();

		int start_ann = (index == 0) ? 0 : off + size * (index - 1);
		int end_ann = std::min(uint64_t(off + size * (index)) - 1, uint64_t(row.size() - 1));

		if (off == 0) {
			start_ann--;
			end_ann--;
		}

		sampleMask.append(QPair<uint64_t, uint64_t>(row[start_ann].start_sample(), row[end_ann].end_sample()));
	}

	return sampleMask;
}

bool DecoderTable::exportTxt(QString fileName)
{
	FileManager fm("Logic Analyzer");
	fm.open(fileName, FileManager::EXPORT);
	QVector<QVector<QString>> decoder_data;
	auto curve = dynamic_cast<AnnotationCurve *>(getDecoderCruves().at(tableModel->getCurrentColumn()));
	std::map<Row, RowData> decoder(curve->getAnnotationRows());
	auto sampleMask = getSearchSampleMask();

	// set decoder data
	std::map<Row, RowData>::iterator it;
	for (it = decoder.begin(); it != decoder.end(); it++) {
		auto annotations = it->second.get_annotations();
		QString title = curve->fromTitleToRowType(it->first.title());
		if (tableModel->getFiltered().value(tableModel->getCurrentColumn()).contains(title)) continue;

		for (const auto &ann : annotations) {
			QVector<QString> str = {QString::number(ann.start_sample()) + "-" + QString::number(ann.end_sample()), ann.row()->title(), ann.annotations()[0]};
			decoder_data.append(str);
		}
	}

	fm.save(QVector<QVector<double>>(), decoder_data, QStringList());

	fm.performDecoderWrite(true);

	return true;
}

bool DecoderTable::exportCsv(QString fileName)
{
	FileManager fm("Logic Analyzer");
	fm.open(fileName, FileManager::EXPORT);

	QStringList columnNames;
	std::map<Row, RowData>::iterator it;
	int col = tableModel->getCurrentColumn();
	uint64_t last_sample = 0;

	QVector<QVector<QString>> decoder_data;
	AnnotationCurve *curve = dynamic_cast<AnnotationCurve *>(getDecoderCruves().at(col));
	std::map<Row, RowData> decoder(curve->getAnnotationRows());
	QRegExp rx =  QRegExp(tableModel->getsearchString(), Qt::CaseInsensitive);
	int row_count = 0;
	int primaryCol = 0;

	auto sampleMask = getSearchSampleMask();

	// set column names
	columnNames += "Time";
	for (it = decoder.begin(); it != decoder.end(); it++) {
		last_sample = std::max(last_sample, it->second.get_max_sample());

		auto title = curve->fromTitleToRowType(it->first.title());
		if (!it->second.get_annotations().empty() && !tableModel->getFiltered()[col].contains(title)) {
			if (it->first.index() == tableModel->getPrimaryAnnotationIndex()) {
				primaryCol = columnNames.size();
			}
			columnNames += it->first.title();
		}
	}

	// initialize decoder_data
	QVector<QString> aux;
	for (uint64_t i = 0; i < last_sample; i++) {
		aux = QVector<QString>();
		for (uint64_t j = 0; j < columnNames.count(); j++) {
			aux.append("");
		}
		decoder_data.append(aux);
	}

	// set time
	for (int i = 0; i < last_sample; i++) {
		decoder_data[i][0] = QString::fromStdString(std::to_string(curve->fromSampleToTime(i))).replace(",", ".");
	}

	// populate decoder_data
	for (it = decoder.begin(); it != decoder.end(); it++) {
		auto row = it->second.get_annotations();
		auto title = curve->fromTitleToRowType(it->first.title());
		if (tableModel->getFiltered()[col].contains(title) || row.empty()) continue;
		row_count++;

		for (unsigned int ann_index = 0; ann_index < row.size(); ann_index++) {
			for (uint64_t i = row[ann_index].start_sample(); i < row[ann_index].end_sample(); i++) {
				decoder_data[i][row_count] = row[ann_index].annotations()[0];
			}
		}
	}

	// apply search
	for (uint64_t i = 0; i < last_sample; i++) {
		if (decoder_data[i][primaryCol] == "") {
			decoder_data[i] = QVector<QString>();
		}
	}
	for (auto sample_range: sampleMask) {
		for (uint64_t i = sample_range.first; i <= sample_range.second; i++) {
			if (i < last_sample)
			decoder_data[i] = QVector<QString>();
		}
	}

	fm.save(QVector<QVector<double>>(), decoder_data, columnNames);

	fm.performDecoderWrite(true);

	return true;
}

void DecoderTable::activate(bool logic)
{
	setLogicAnalyzer(m_logicAnalyzer);
	if (const auto m = decoderModel()) {
		m->reloadDecoders(logic);
		m_active = true;
	}
}

void DecoderTable::deactivate()
{
	if (const auto m = decoderModel()) {
		m->deactivate();
	}
	m_active = false;
}

void DecoderTable::showEvent(QShowEvent *event)
{
    tableModel->to_be_refreshed = true;
}

void DecoderTable::groupValuesChanged(int value)
{
	reset();
	tableModel->to_be_refreshed = true;
}
} // namespace logic
} // namespace adiscope

