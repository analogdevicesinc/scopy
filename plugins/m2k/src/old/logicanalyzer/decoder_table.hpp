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

#ifndef DECODER_TABLE_H
#define DECODER_TABLE_H

#include <QAbstractTableModel>
#include <QTableView>
#include "decoder.h"
#include "decoder_table_item.hpp"
#include "logic_analyzer.h"

namespace adiscope::m2k {


namespace logic {

class DecoderTableModel;

class DecoderTable : public QTableView {
    Q_OBJECT

public:
    DecoderTable(QWidget *parent = nullptr);

    void setLogicAnalyzer(LogicAnalyzer* logicAnalyzer);
    void activate(bool logic);
    void deactivate();
    QVector<GenericLogicPlotCurve *> getDecoderCruves();
    inline bool isActive() const { return m_active; }

    // Shortcut to get a reference to the model.
    DecoderTableModel* decoderModel() const;
    bool eventFilter(QObject *object, QEvent *event);

public Q_SLOTS:
    void groupValuesChanged(int value);
    void exportData();

private:
    bool m_active;
    DecoderTableModel* tableModel;
    LogicAnalyzer* m_logicAnalyzer;
    QVector<GenericLogicPlotCurve *> *temp_curves;
    bool exportCsv(QString fileName);
    bool exportTxt(QString fileName);
    QVector<QPair<uint64_t, uint64_t> > getSearchSampleMask();

protected:
    void showEvent(QShowEvent *event);
};
} // namespace logic
} // namespace adiscope

#endif // DECODER_TABLE_H
