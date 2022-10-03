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

#ifndef DECODER_TABLE_MODEL_H
#define DECODER_TABLE_MODEL_H

#include <bitset>
#include <QAbstractTableModel>
#include <QMap>
#include "annotationcurve.h"
#include "decoder_table.hpp"
#include "decoder_table_item.hpp"


namespace adiscope {


namespace logic {

class DecoderTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    DecoderTableModel(DecoderTable *decoderTable, LogicAnalyzer *logicAnalyzer);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setDefaultPrimaryAnnotations();

    // resize table cells and hide/show rows
    void refreshColumn(double column = -1) const;

    // activate connects signals to listen for new decoded messages
    // deactivate disconnects them.
    void activate();
    void deactivate();

    // Get index of the curve
    int indexOfCurve(const AnnotationCurve* curve) const;

    void setPrimaryAnnotation(int index);
    void selectedDecoderChanged(int index) const;

    void populateFilter(int index) const;
    QMap<int, QVector<QString>>& getFiltered();
    int getCurrentColumn() const;
    void setCurrentRow(int index);
    void setMaxRowCount();
    mutable int to_be_refreshed;
    void refreshSettings(int column = -1);
    void setSearchString(QString str) const;
    QVector<int> getSearchMask();
    QString getsearchString();
    int getPrimaryAnnotationIndex() const;

public Q_SLOTS:

    // This slot should be used when any of the decoders / annotation curves
    // have been modified (eg adding/removing decoders, etc..). This clears
    // the curves vecotr reconnects up signals to watch for annotation changes.
    void reloadDecoders(bool logic);

    void searchBoxSlot(QString text);

protected:

    // Set of curves to observe. Each should have an entry in the dataset
    std::vector<QPointer<AnnotationCurve>> m_curves;

    DecoderTable *m_decoderTable;
    LogicAnalyzer *m_logic;
    QVector<GenericLogicPlotCurve*> m_plotCurves;
    bool m_active = false;
    QMap<int, int> *m_primary_annoations;
    void populateDecoderComboBox() const;
    mutable int m_current_column;
    QMap<int, QVector<QString>> m_filteredMessages;
    mutable int max_row_count = 0;
    mutable QString searchString;
    QVector<int> searchMask;
private:
    void searchTable(QString text);
};

} // namespace logic
} // namespace adiscope

#endif // DECODER_TABLE_MODEL_H

