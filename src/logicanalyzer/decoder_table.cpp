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
#include <QHeaderView>
#include "logic_analyzer.h"

namespace adiscope {


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

