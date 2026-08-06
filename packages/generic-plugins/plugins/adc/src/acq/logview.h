/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QComboBox>
#include <QString>
#include <QTextEdit>
#include <QVector>
#include <QWidget>

namespace scopy {
namespace adc {

// A severity-filtered message log, for the Debug popup's "Logs" and
// "Decoder logs" tabs.
//
// Entries are kept as data rather than as formatted text, so changing the
// filter re-renders what has already arrived instead of only affecting what
// comes next — a log you can't retroactively widen is a log you have to
// reproduce the fault in.
//
// Severity is an int on the API because the two producers use different enums
// with the same ordering: AcquisitionError::Severity and decoder::LogLevel are
// both Info=0, Warning=1, Critical=2.
class LogView : public QWidget
{
	Q_OBJECT
public:
	explicit LogView(const QString &placeholder, QWidget *parent = nullptr);

	// Newest-first cap on retained entries. A run left going overnight would
	// otherwise grow this without bound.
	void setCapacity(int entries);

public Q_SLOTS:
	void append(int severity, const QString &id, const QString &message);
	void clear();

private:
	struct Entry
	{
		int     severity;
		QString timestamp;
		QString id;
		QString message;
	};

	// One entry as an HTML line, coloured by severity.
	QString format(const Entry &e) const;

	// Re-renders every retained entry that passes the filter. Only on filter
	// change or clear — append() is incremental.
	void rebuild();

	QComboBox *m_filter;
	QTextEdit *m_view;

	QVector<Entry> m_entries;
	int            m_capacity;
	int            m_minSeverity;
};

} // namespace adc
} // namespace scopy

#endif // LOGVIEW_H
