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

#include "filesourcewidget.h"

#include <gui/style.h>
#include <gui/widgets/filebrowserwidget.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QTimer>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

namespace {

// Matches the other block panels' layout so they line up.
QWidget *labelledRow(const QString &text, QWidget *field, QWidget *parent)
{
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);

	auto *label = new QLabel(text, w);
	Style::setStyle(label, style::properties::label::subtle);

	lay->addWidget(label);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(field);
	return w;
}

// The same 10 Hz SnapshotSourceWidget polls at, for the same reason: the playhead advances once
// per acquisition cycle, far faster than a label is worth redrawing, and a skipped position is
// correct for a readout.
constexpr int kPollIntervalMs = 100;

} // namespace

FileSourceWidget::FileSourceWidget(FileSourceBlock *src, QWidget *parent)
	: QWidget(parent)
	, m_src(src)
{
	setupUI();

	if(m_src) {
		connect(m_src, &FileSourceBlock::slotsChanged, this, &FileSourceWidget::rebuildRows);
		connect(m_src, &FileSourceBlock::slotChanged, this, &FileSourceWidget::syncRow);
		// This widget is owned by the menu page's layout while the block is parented to the
		// engine, so a teardown destroys the block first — and m_poll would tick into it.
		connect(m_src, &QObject::destroyed, this, [this]() {
			m_src = nullptr;
			m_poll->stop();
		});
	}

	rebuildRows();
}

void FileSourceWidget::setupUI()
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	auto *section = new MenuSectionWidget(this);
	Style::setStyle(section, style::properties::widget::border);
	section->contentLayout()->setSpacing(8);

	m_emptyHint = new QLabel(tr("No files. Add one, then pick a file to read."), section);
	m_emptyHint->setWordWrap(true);
	Style::setStyle(m_emptyHint, style::properties::label::subtle);
	section->contentLayout()->addWidget(m_emptyHint);

	m_rowsLay = new QVBoxLayout();
	m_rowsLay->setContentsMargins(0, 0, 0, 0);
	m_rowsLay->setSpacing(6);
	section->contentLayout()->addLayout(m_rowsLay);

	auto *addBtn = new QPushButton(tr("+ Add file"), section);
	Style::setStyle(addBtn, style::properties::button::basicButton);
	connect(addBtn, &QPushButton::clicked, this, [this]() {
		if(m_src) {
			m_src->addSlot();
		}
	});
	section->contentLayout()->addWidget(addBtn);

	mainLayout->addWidget(section);

	m_poll = new QTimer(this);
	m_poll->setInterval(kPollIntervalMs);
	connect(m_poll, &QTimer::timeout, this, [this]() {
		for(int i = 0; i < m_rows.size(); ++i) {
			syncPlayback(i);
		}
	});
	m_poll->start();
}

void FileSourceWidget::rebuildRows()
{
	m_building = true;

	QLayoutItem *item;
	while((item = m_rowsLay->takeAt(0)) != nullptr) {
		if(QWidget *w = item->widget()) {
			w->deleteLater();
		}
		delete item;
	}
	m_rows.clear();

	const QList<FileSourceBlock::SlotInfo> list = m_src ? m_src->slotInfos() : QList<FileSourceBlock::SlotInfo>();
	for(int i = 0; i < list.size(); ++i) {
		m_rowsLay->addWidget(buildRow(i, list.at(i)));
	}
	m_emptyHint->setVisible(list.isEmpty());

	m_building = false;

	// After the guard clears, so the labels read the block rather than the widgets that were
	// just being populated.
	for(int i = 0; i < m_rows.size(); ++i) {
		syncRow(i);
	}
}

QWidget *FileSourceWidget::buildRow(int index, const FileSourceBlock::SlotInfo &info)
{
	auto *frame = new QFrame(this);
	frame->setFrameShape(QFrame::StyledPanel);
	auto *lay = new QVBoxLayout(frame);
	lay->setContentsMargins(6, 6, 6, 6);
	lay->setSpacing(4);

	Row row;

	auto *header = new QWidget(frame);
	auto *headerLay = new QHBoxLayout(header);
	headerLay->setContentsMargins(0, 0, 0, 0);
	row.browser = new FileBrowserWidget(FileBrowserWidget::OPEN_FILE, header);
	row.browser->setFilter(fileReaderFilter());
	headerLay->addWidget(row.browser);
	auto *removeBtn = new QPushButton(tr("Remove"), header);
	Style::setStyle(removeBtn, style::properties::button::borderButton);
	connect(removeBtn, &QPushButton::clicked, this, [this, index]() {
		if(m_src) {
			m_src->removeSlot(index);
		}
	});
	headerLay->addWidget(removeBtn);
	lay->addWidget(header);

	row.firstColumn = new QComboBox(frame);
	row.firstColumn->addItem(tr("Auto"), int(CsvFileReader::FirstColumnMode::Auto));
	row.firstColumn->addItem(tr("Time / X"), int(CsvFileReader::FirstColumnMode::Time));
	row.firstColumn->addItem(tr("Data"), int(CsvFileReader::FirstColumnMode::Data));
	row.firstColumnRow = labelledRow(tr("First column:"), row.firstColumn, frame);
	lay->addWidget(row.firstColumnRow);

	row.rate = new gui::MenuSpinbox(tr("Sample rate"), 0, QStringLiteral("Hz"), 0, 1e12, true, false, true, frame);
	lay->addWidget(row.rate);

	row.cyclic = new QCheckBox(tr("Cyclic playback"), frame);
	lay->addWidget(row.cyclic);

	row.readback = new QLabel(frame);
	row.readback->setWordWrap(true);
	Style::setStyle(row.readback, style::properties::label::subtle);
	lay->addWidget(row.readback);

	row.playback = new QLabel(frame);
	Style::setStyle(row.playback, style::properties::label::subtle);
	lay->addWidget(row.playback);

	// textChanged rather than editingFinished, because the browse dialog sets the text
	// programmatically and would not emit the latter. The isFile() check is what keeps a
	// half-typed path from being parsed on every keystroke.
	connect(row.browser->lineEdit(), &QLineEdit::textChanged, this, [this, index](const QString &path) {
		if(m_building || !m_src) {
			return;
		}
		const QList<FileSourceBlock::SlotInfo> list = m_src->slotInfos();
		if(index < list.size() && path != list.at(index).path && QFileInfo(path).isFile()) {
			m_src->setSlotFile(index, path);
		}
	});
	connect(row.firstColumn, &QComboBox::currentIndexChanged, this, [this, index, box = row.firstColumn](int idx) {
		if(!m_building && m_src && idx >= 0) {
			m_src->setSlotFirstColumnMode(index,
						      CsvFileReader::FirstColumnMode(box->itemData(idx).toInt()));
		}
	});
	connect(row.rate, &gui::MenuSpinbox::valueChanged, this, [this, index](double v) {
		if(!m_building && m_src) {
			m_src->setSlotSampleRate(index, v);
		}
	});
	connect(row.cyclic, &QCheckBox::toggled, this, [this, index](bool on) {
		if(!m_building && m_src) {
			m_src->setSlotCyclic(index, on);
		}
	});
	m_rows.append(row);
	Q_UNUSED(info)
	return frame;
}

void FileSourceWidget::syncRow(int index)
{
	if(index < 0 || index >= m_rows.size() || !m_src) {
		return;
	}
	const QList<FileSourceBlock::SlotInfo> list = m_src->slotInfos();
	if(index >= list.size()) {
		return;
	}
	const FileSourceBlock::SlotInfo &info = list.at(index);
	const Row &row = m_rows.at(index);

	{
		QSignalBlocker b(row.browser->lineEdit());
		row.browser->lineEdit()->setText(info.path);
	}

	row.firstColumnRow->setVisible(info.firstColumnMode.has_value());
	if(info.firstColumnMode) {
		QSignalBlocker b(row.firstColumn);
		row.firstColumn->setCurrentIndex(row.firstColumn->findData(int(*info.firstColumnMode)));
	}

	// Only editable when the file declared no rate; overriding one the file carries would show a
	// timebase the data does not have.
	row.rate->setEnabled(!info.declaresRate);
	row.rate->setValueSilent(info.sampleRate);

	{
		QSignalBlocker b(row.cyclic);
		row.cyclic->setChecked(info.cyclic);
	}
	if(info.path.isEmpty()) {
		row.readback->setText(tr("No file loaded"));
	} else {
		row.readback->setText(tr("%1: %2 channels, %3 samples, %4")
					      .arg(info.title)
					      .arg(info.channelCount)
					      .arg(QLocale::system().toString(info.sampleCount))
					      .arg(info.sampleRate > 0.0 ? tr("%1 Hz").arg(info.sampleRate, 0, 'g', 8)
									 : tr("rate unknown")));
	}

	syncPlayback(index);
}

void FileSourceWidget::syncPlayback(int index)
{
	if(index < 0 || index >= m_rows.size() || !m_src) {
		return;
	}
	const QList<FileSourceBlock::SlotInfo> list = m_src->slotInfos();
	if(index >= list.size()) {
		return;
	}
	const FileSourceBlock::SlotInfo &info = list.at(index);
	QLabel *playback = m_rows.at(index).playback;

	if(!info.cyclic || info.sampleCount <= 0) {
		playback->clear();
		return;
	}
	playback->setText(tr("playing: sample %1 / %2")
				  .arg(QLocale::system().toString(info.playhead))
				  .arg(QLocale::system().toString(info.sampleCount)));
}

#include "moc_filesourcewidget.cpp"
