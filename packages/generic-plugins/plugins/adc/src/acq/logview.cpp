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

#include "logview.h"

#include <QDateTime>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

#include <gui/style.h>

using namespace scopy;
using namespace scopy::adc;

LogView::LogView(const QString &placeholder, QWidget *parent)
	: QWidget(parent)
	, m_capacity(2000)
	, m_minSeverity(0)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(Style::getDimension(json::global::unit_0_5));

	QHBoxLayout *ctl = new QHBoxLayout();
	ctl->setContentsMargins(0, 0, 0, 0);
	ctl->setSpacing(Style::getDimension(json::global::unit_1));

	QLabel *filterLabel = new QLabel("Severity", this);
	Style::setStyle(filterLabel, style::properties::label::menuSmall);
	ctl->addWidget(filterLabel);

	m_filter = new QComboBox(this);
	// Ordered so the index *is* the minimum severity, matching both producer
	// enums — no mapping table to keep in sync.
	m_filter->addItems({"Info", "Warning", "Critical"});
	ctl->addWidget(m_filter);

	ctl->addStretch();

	QPushButton *clearBtn = new QPushButton("Clear", this);
	Style::setStyle(clearBtn, style::properties::button::basicButton);
	ctl->addWidget(clearBtn);

	lay->addLayout(ctl);

	m_view = new QTextEdit(this);
	m_view->setReadOnly(true);
	// A log is columns of aligned prefixes; a proportional font destroys that.
	m_view->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_view->setLineWrapMode(QTextEdit::NoWrap);
	m_view->setPlaceholderText(placeholder);
	// Its own fill and border, matching the tree panes in the other debug tabs — the
	// global QTextEdit rule strips the border and the QWidget rule leaves the background
	// transparent, so without this the log is an unbounded black area.
	Style::setStyle(m_view, style::properties::widget::basicComponent);
	lay->addWidget(m_view, 1);

	connect(m_filter, &QComboBox::currentIndexChanged, this, [this](int i) {
		m_minSeverity = i;
		rebuild();
	});
	connect(clearBtn, &QAbstractButton::clicked, this, &LogView::clear);
}

void LogView::setCapacity(int entries)
{
	m_capacity = entries;
	while(m_entries.size() > m_capacity) {
		m_entries.removeFirst();
	}
	rebuild();
}

QString LogView::format(const Entry &e) const
{
	const char *colorKey;
	QString tag;

	switch(e.severity) {
	case 2:
		colorKey = json::theme::content_error;
		tag = "CRIT";
		break;
	case 1:
		colorKey = json::theme::content_busy;
		tag = "WARN";
		break;
	default:
		colorKey = json::theme::content_subtle;
		tag = "INFO";
		break;
	}

	return QString("<span style=\"color:%1\">[%2] %3 | %4: %5</span>")
		.arg(Style::getColor(colorKey).name(), tag, e.timestamp, e.id.toHtmlEscaped(),
		     e.message.toHtmlEscaped());
}

void LogView::append(int severity, const QString &id, const QString &message)
{
	Entry e{severity, QDateTime::currentDateTime().toString("hh:mm:ss.zzz"), id, message};

	m_entries.append(e);
	if(m_entries.size() > m_capacity) {
		// Dropping the oldest changes what a filter widening would show, so the
		// view has to be rebuilt rather than appended to.
		m_entries.removeFirst();
		rebuild();
		return;
	}

	if(severity < m_minSeverity) {
		return;
	}

	m_view->append(format(e));
	m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->maximum());
}

void LogView::clear()
{
	m_entries.clear();
	m_view->clear();
}

void LogView::rebuild()
{
	m_view->clear();
	for(const Entry &e : m_entries) {
		if(e.severity >= m_minSeverity) {
			m_view->append(format(e));
		}
	}
	m_view->verticalScrollBar()->setValue(m_view->verticalScrollBar()->maximum());
}

#include "moc_logview.cpp"
