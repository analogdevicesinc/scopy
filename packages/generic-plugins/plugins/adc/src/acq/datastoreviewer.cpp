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

#include "datastoreviewer.h"

#include <QApplication>
#include <QClipboard>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSplitter>
#include <QTimer>
#include <QVBoxLayout>

#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/SampleBuffer.h>
#include <gui/style.h>

using namespace scopy;
using namespace scopy::adc;

DataStoreViewer::DataStoreViewer(scopy::acq::DataStore *store, QWidget *parent)
	: QWidget(parent)
	, m_store(store)
{
	buildUi();
	refresh();
}

void DataStoreViewer::buildUi()
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(Style::getDimension(json::global::unit_0_5));

	// Side by side: the panel is wide and short (it spans the bottom rail), so
	// stacking the key list above the samples would leave both too short to read.
	QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
	// global.qss paints every QSplitter background_subtle, which is near-black and reads
	// as a gap cut through the debug panel; transparent lets the panel's own fill show in
	// the gutter. Spelled as a type selector rather than via setBackgroundColor(), whose
	// `.QWidget` selector matches exact QWidget instances only and so misses a QSplitter.
	splitter->setStyleSheet(QStringLiteral("QSplitter { background-color: transparent; }"));

	// ---- left: the key list ----
	m_tree = new QTreeWidget(splitter);
	// The panes are the sections of this tab, so each carries its own fill and border:
	// the global QWidget rule is transparent, so without this a tree is a black hole in
	// the panel with nothing marking where it ends.
	Style::setStyle(m_tree, style::properties::widget::basicComponent);
	m_tree->setColumnCount(ColCount);
	m_tree->setHeaderLabels({"Key", "Type", "Samples", "History (used/cap)"});
	m_tree->setAlternatingRowColors(true);
	m_tree->setRootIsDecorated(false);
	m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tree->header()->setStretchLastSection(true);
	m_tree->setUniformRowHeights(true);
	m_tree->setColumnWidth(ColKey, Style::getDimension(json::global::unit_5) * 3);
	splitter->addWidget(m_tree);

	// ---- right: the selected stream's samples ----
	QWidget *right = new QWidget(splitter);
	// ToolTemplate paints itself with a widget-level `.QWidget { background_subtle }`, and a
	// widget stylesheet cascades to every descendant and outranks the app-wide one. `.QWidget`
	// matches exact QWidget instances only, so subclasses (the trees below) escape it and a
	// plain container like this one does not — it comes out near-black over the panel's fill.
	// Overriding here rather than reaching for a Style property: a property only works through
	// the app stylesheet, which is the rule that loses.
	Style::setBackgroundColor(right, QStringLiteral("transparent"));
	QVBoxLayout *rl = new QVBoxLayout(right);
	rl->setContentsMargins(0, 0, 0, 0);
	rl->setSpacing(Style::getDimension(json::global::unit_0_5));

	QHBoxLayout *ctl = new QHBoxLayout();
	ctl->setContentsMargins(0, 0, 0, 0);
	ctl->setSpacing(Style::getDimension(json::global::unit_1));

	m_samplesTitle = new QLabel("Select a key.", right);
	Style::setStyle(m_samplesTitle, style::properties::label::menuSmall);
	ctl->addWidget(m_samplesTitle);
	ctl->addStretch();

	m_sampleCount = new QSpinBox(right);
	m_sampleCount->setRange(0, 4096);
	m_sampleCount->setValue(32);
	m_sampleCount->setToolTip("Newest samples listed for the selected key. 0 lists none.\n"
				  "Rows refresh every acquisition cycle, so keep this small.\n"
				  "Samples are indexed from 0, oldest-first within the shown window.\n"
				  "Annotation streams list their sample range instead.");
	ctl->addWidget(m_sampleCount);

	m_hex = new QCheckBox("hex", right);
	m_hex->setToolTip("Show integer samples in hex. Floats are unaffected.");
	ctl->addWidget(m_hex);

	m_copyBtn = new QPushButton("Copy to clipboard", right);
	m_copyBtn->setToolTip("Copy every sample of the selected key, one per line.\n"
			      "Not just the rows listed — the count above only limits the display.");
	m_copyBtn->setEnabled(false);
	Style::setStyle(m_copyBtn, style::properties::button::basicButton);
	ctl->addWidget(m_copyBtn);

	rl->addLayout(ctl);

	m_samples = new QTreeWidget(right);
	m_samples->setColumnCount(SampleColCount);
	m_samples->setHeaderLabels({"#", "Value"});
	m_samples->setAlternatingRowColors(true);
	m_samples->setRootIsDecorated(false);
	m_samples->setUniformRowHeights(true);
	m_samples->header()->setStretchLastSection(true);
	m_samples->setColumnWidth(ColIndex, Style::getDimension(json::global::unit_5));
	// Sample values are columns of numbers; a proportional font makes them
	// impossible to scan.
	m_samples->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	Style::setStyle(m_samples, style::properties::widget::basicComponent);
	rl->addWidget(m_samples, 1);

	splitter->addWidget(right);
	splitter->setStretchFactor(0, 3);
	splitter->setStretchFactor(1, 2);
	const int unit = Style::getDimension(json::global::unit_5);
	splitter->setSizes({unit * 6, unit * 4});

	lay->addWidget(splitter, 1);

	// Fill on select rather than waiting for the next cycle: while the engine is
	// stopped there is no next cycle, and a pane that stays empty on click reads as
	// a bug in the store.
	connect(m_tree, &QTreeWidget::itemSelectionChanged, this, &DataStoreViewer::fillSamples);
	connect(m_sampleCount, &QSpinBox::valueChanged, this, &DataStoreViewer::fillSamples);
	connect(m_hex, &QCheckBox::toggled, this, &DataStoreViewer::fillSamples);
	connect(m_copyBtn, &QAbstractButton::clicked, this, &DataStoreViewer::copySamples);
}

scopy::acq::DataKey DataStoreViewer::selectedKey() const
{
	if(!m_tree) {
		return scopy::acq::DataKey();
	}
	const QList<QTreeWidgetItem *> sel = m_tree->selectedItems();
	if(sel.isEmpty()) {
		return scopy::acq::DataKey();
	}
	return scopy::acq::DataKey(sel.first()->text(ColKey));
}

void DataStoreViewer::refresh()
{
	if(!m_store || !m_tree) {
		return;
	}

	const QList<scopy::acq::DataKey> keys = m_store->keys();

	// Drop rows whose key no longer exists.
	for(int i = m_tree->topLevelItemCount() - 1; i >= 0; --i) {
		const QString rowKey = m_tree->topLevelItem(i)->text(ColKey);
		bool found = false;
		for(const scopy::acq::DataKey &k : keys) {
			if(k.key == rowKey) {
				found = true;
				break;
			}
		}
		if(!found) {
			delete m_tree->takeTopLevelItem(i);
		}
	}

	for(const scopy::acq::DataKey &k : keys) {
		const scopy::acq::SampleBuffer buf = m_store->snapshot(k);
		const auto type = buf.type();

		QTreeWidgetItem *item = nullptr;
		for(int i = 0; i < m_tree->topLevelItemCount(); ++i) {
			if(m_tree->topLevelItem(i)->text(ColKey) == k.key) {
				item = m_tree->topLevelItem(i);
				break;
			}
		}
		if(!item) {
			item = new QTreeWidgetItem(m_tree);
			item->setText(ColKey, k.key);
		}

		item->setText(ColType, type ? scopy::acq::sampleTypeName(*type) : QStringLiteral("-"));
		item->setText(ColSamples, QString::number(buf.size()));
		item->setText(ColHistory, QString("%1/%2").arg(buf.depth()).arg(buf.capacity()));
	}

	// Exactly one stream is listed, so this is a fixed cost per cycle rather than
	// one that grows with the key count.
	fillSamples();
}

namespace {

// One sample as text. Integers honour `hex`; floats never do (a hex float is not
// what anyone reading a scope trace wants).
template <class T>
QString formatSample(T v, bool hex)
{
	if constexpr(std::is_floating_point_v<T>) {
		return QString::number(static_cast<double>(v), 'g', 6);
	} else {
		// Cast through the unsigned width so a negative qint8 prints as its
		// two's-complement byte instead of a sign-extended "0xffffff80".
		if(hex) {
			return QStringLiteral("0x%1").arg(
				static_cast<quint64>(static_cast<std::make_unsigned_t<T>>(v)), sizeof(T) * 2, 16,
				QLatin1Char('0'));
		}
		return QString::number(v);
	}
}

} // namespace

void DataStoreViewer::fillSamples()
{
	if(!m_samples || !m_store) {
		return;
	}

	const scopy::acq::DataKey key = selectedKey();
	if(key.key.isEmpty()) {
		m_samples->clear();
		m_samplesTitle->setText("Select a key.");
		m_copyBtn->setEnabled(false);
		return;
	}
	m_samplesTitle->setText(key.key);

	// Copy goes to the store, not to these rows, so it only needs a selected key with
	// something written under it — a display window of 0 rows still has samples to copy.
	m_copyBtn->setEnabled(!m_store->snapshot(key).empty());

	const int want = m_sampleCount->value();
	const bool hex = m_hex->isChecked();
	if(want <= 0) {
		m_samples->clear();
		return;
	}

	// window() spans chunks and is right-anchored, so this is the newest `want`
	// samples regardless of how the acquisition happened to be chunked.
	const scopy::acq::SampleVariant v = m_store->window(key, want);

	// Rows to display: (col0, col1). Numeric streams show an index and a value;
	// annotation streams show a sample range and "klass: text" — the same records,
	// but an index into them means nothing to a reader.
	QList<QPair<QString, QString>> rows;

	std::visit(
		[&](const auto &vec) {
			using Vec = std::decay_t<decltype(vec)>;
			if constexpr(std::is_same_v<Vec, QVector<scopy::acq::Annotation>>) {
				const int first = std::max<int>(0, vec.size() - want);
				for(int i = first; i < vec.size(); ++i) {
					const scopy::acq::Annotation &a = vec.at(i);
					rows.append({QString("%1-%2").arg(a.startSample).arg(a.endSample),
						     a.klass.isEmpty() ? a.text : a.klass + ": " + a.text});
				}
			} else {
				// window() already trimmed to `want` and returns oldest-first,
				// so this is a plain 0-based index into the displayed window.
				for(int i = 0; i < vec.size(); ++i) {
					rows.append({QString::number(i), formatSample(vec.at(i), hex)});
				}
			}
		},
		v);

	while(m_samples->topLevelItemCount() > rows.size()) {
		delete m_samples->takeTopLevelItem(m_samples->topLevelItemCount() - 1);
	}
	while(m_samples->topLevelItemCount() < rows.size()) {
		new QTreeWidgetItem(m_samples);
	}

	for(int i = 0; i < rows.size(); ++i) {
		QTreeWidgetItem *row = m_samples->topLevelItem(i);
		row->setText(ColIndex, rows.at(i).first);
		row->setText(ColValue, rows.at(i).second);
	}
}

void DataStoreViewer::copySamples()
{
	if(!m_store) {
		return;
	}

	const scopy::acq::DataKey key = selectedKey();
	if(key.key.isEmpty()) {
		return;
	}

	// Everything the store holds for the key, oldest chunk first. The sample count
	// spinbox only bounds what the pane lists — a copy is a one-off, so there is no
	// reason to make the reader widen the display to get the whole stream out.
	const scopy::acq::SampleBuffer buf = m_store->snapshot(key);
	const bool hex = m_hex->isChecked();

	// Values only, one per line: the index column is just the row number, and leaving
	// it out means the text pastes straight into a plot or a spreadsheet column.
	QStringList lines;
	for(std::size_t c = buf.depth(); c-- > 0;) {
		std::visit(
			[&](const auto &vec) {
				using Vec = std::decay_t<decltype(vec)>;
				for(const auto &s : vec) {
					if constexpr(std::is_same_v<Vec, QVector<scopy::acq::Annotation>>) {
						lines << (s.klass.isEmpty() ? s.text : s.klass + ": " + s.text);
					} else {
						lines << formatSample(s, hex);
					}
				}
			},
			buf.sample(c));
	}

	QGuiApplication::clipboard()->setText(lines.join(QLatin1Char('\n')));
}

#include "moc_datastoreviewer.cpp"
