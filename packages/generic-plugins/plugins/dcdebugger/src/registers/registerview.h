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

#ifndef REGISTERVIEW_H
#define REGISTERVIEW_H

#include "scopy-dcdebugger_export.h"

#include "registersession.h"

#include <QList>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

namespace scopy {
class MenuCombo;
class ToolTemplate;
namespace gui {
class MenuSpinbox;
}
} // namespace scopy

namespace scopy::component {
class RegisterReader;
class RegisterWriter;
} // namespace scopy::component

namespace scopy::dcdebugger {
class BitToggleGrid;

// Registers tab of a Context or Device: target (every node under `root` that owns a
// RegisterReader / RegisterWriter), address, width, value, read / write, a bit grid to compose
// the value, a range dump, and the history of operations (exportable as CSV).
class SCOPY_DCDEBUGGER_EXPORT RegisterView : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterView(QObject *root, QWidget *parent = nullptr);

	struct Target
	{
		QObject *owner = nullptr;
		component::RegisterReader *reader = nullptr;
		component::RegisterWriter *writer = nullptr;
	};
	// Nodes under `root` (itself included) with register access, tree order.
	static QList<Target> targetsOf(QObject *root);

	RegisterSession *session() const { return m_session; }
	BitToggleGrid *bits() const { return m_bits; }

private:
	QWidget *buildRegisterSection(QWidget *parent);
	QWidget *buildBitsSection(QWidget *parent);
	QWidget *buildDumpSection(QWidget *parent);
	void selectTarget(int index);
	void applyWidth();
	void fitBitsSection();
	int width() const;
	bool address(uint32_t &out, QLineEdit *edit) const;
	void readClicked();
	void writeClicked();
	void dumpClicked();
	void onEntry(const RegisterEntry &entry);
	void onReadDone(uint32_t address, uint32_t value);
	void setValueText(uint32_t value);
	void exportHistory();
	void clearHistory();
	void setStatus(const QString &text, bool error = false);

	QList<Target> m_targets;
	RegisterSession *m_session;

	ToolTemplate *m_tool;
	QLabel *m_status;
	QPushButton *m_exportBtn;
	QPushButton *m_clearBtn;
	MenuCombo *m_target;
	QLineEdit *m_address;
	MenuCombo *m_width;
	QLineEdit *m_value;
	QPushButton *m_readBtn;
	QPushButton *m_writeBtn;
	QLineEdit *m_dumpFrom;
	gui::MenuSpinbox *m_dumpCount;
	QPushButton *m_dumpBtn;
	BitToggleGrid *m_bits;
	QWidget *m_bitsSection = nullptr;
	QTableWidget *m_history;
};

} // namespace scopy::dcdebugger

#endif // REGISTERVIEW_H
