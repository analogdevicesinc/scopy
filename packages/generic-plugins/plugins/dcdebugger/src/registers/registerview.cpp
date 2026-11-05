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

#include "registerview.h"

#include "bittogglegrid.h"
#include "componentnode.h"
#include "nodepanel.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>

#include <component/registerreader.h>
#include <component/registerwriter.h>
#include <dcdebugger/style_properties.h>
#include <filemanager.h>
#include <menucombo.h>
#include <menulineedit.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <style.h>
#include <tooltemplate.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;

namespace {
constexpr int MAX_DUMP = 4096;

QString targetLabel(QObject *owner)
{
	const NodeKind kind = ComponentNode::kindOf(owner);
	const QString name = ComponentNode::name(owner, kind);
	const QString value = ComponentNode::value(owner, kind);
	return value.isEmpty() || value == name ? name : QStringLiteral("%1 (%2)").arg(name, value);
}

QLineEdit *hexEdit(const QString &title, const QString &placeholder, QWidget *parent, QWidget **wrapper)
{
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	auto *label = new QLabel(title, w);
	Style::setStyle(label, style::properties::label::subtle);
	auto *edit = new MenuLineEdit(w);
	edit->edit()->setPlaceholderText(placeholder);
	// The global sheet paints plain widgets in the window colour; inside a grey section every
	// container and label has to take the section background explicitly.
	for(QWidget *part : std::initializer_list<QWidget *>{w, label, edit}) {
		Style::setStyle(part, style::properties::widget::basicBackground);
	}
	lay->addWidget(label);
	lay->addWidget(edit, 1);
	*wrapper = w;
	return edit->edit();
}

QLabel *sectionTitle(const QString &text, QWidget *parent)
{
	auto *label = new QLabel(text, parent);
	Style::setStyle(label, style::properties::label::menuSmall);
	Style::setStyle(label, style::properties::widget::basicBackground);
	return label;
}
} // namespace

QList<RegisterView::Target> RegisterView::targetsOf(QObject *root)
{
	QList<Target> out;
	if(!root) {
		return out;
	}
	QList<QObject *> owners;
	for(RegisterReader *r : root->findChildren<RegisterReader *>()) {
		if(!owners.contains(r->parent())) {
			owners.append(r->parent());
		}
	}
	for(RegisterWriter *w : root->findChildren<RegisterWriter *>()) {
		if(!owners.contains(w->parent())) {
			owners.append(w->parent());
		}
	}
	for(QObject *o : owners) {
		if(!o) {
			continue;
		}
		Target t;
		t.owner = o;
		t.reader = o->findChild<RegisterReader *>(QString(), Qt::FindDirectChildrenOnly);
		t.writer = o->findChild<RegisterWriter *>(QString(), Qt::FindDirectChildrenOnly);
		out.append(t);
	}
	return out;
}

RegisterView::RegisterView(QObject *root, QWidget *parent)
	: QWidget(parent)
	, m_targets(targetsOf(root))
	, m_session(new RegisterSession(this))
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	lay->addWidget(m_tool);

	// Top: the status next to Export / Clear.
	m_status = new QLabel("Idle", this);
	Style::setStyle(m_status, style::properties::label::subtle);
	m_tool->addWidgetToTopContainerHelper(m_status, TTA_LEFT);
	m_exportBtn = new QPushButton("Export...", this);
	m_exportBtn->setToolTip("Save the history as CSV");
	Style::setStyle(m_exportBtn, style::properties::dcdebugger::exportButton);
	m_exportBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_clearBtn = new QPushButton("Clear", this);
	m_clearBtn->setToolTip("Forget the history");
	Style::setStyle(m_clearBtn, style::properties::dcdebugger::exportButton);
	m_clearBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_tool->addWidgetToTopContainerHelper(m_exportBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_clearBtn, TTA_RIGHT);
	connect(m_exportBtn, &QPushButton::clicked, this, &RegisterView::exportHistory);
	connect(m_clearBtn, &QPushButton::clicked, this, &RegisterView::clearHistory);

	// Central: Register | Bits, then Dump range, then the history.
	auto *central = new QWidget(this);
	auto *clay = new QVBoxLayout(central);
	clay->setContentsMargins(0, 0, 0, 0);
	clay->setSpacing(10);
	auto *controls = new QWidget(central);
	auto *ctlLay = new QVBoxLayout(controls);
	ctlLay->setContentsMargins(0, 0, 0, 0);
	ctlLay->setSpacing(10);
	auto *row = new QWidget(controls);
	auto *rlay = new QHBoxLayout(row);
	rlay->setContentsMargins(0, 0, 0, 0);
	rlay->setSpacing(10);
	// Register takes the spare width; Bits is as wide as its bit count needs (16 per row at most).
	rlay->addWidget(buildRegisterSection(row), 1);
	rlay->addWidget(buildBitsSection(row), 0);
	ctlLay->addWidget(row);
	ctlLay->addWidget(buildDumpSection(controls));
	m_history = makeTable({"Time", "Op", "Address", "Value"}, central);
	m_history->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	clay->addWidget(controls);
	clay->addWidget(m_history, 1);
	m_tool->addWidgetToCentralContainerHelper(central);

	connect(m_bits, &BitToggleGrid::valueEdited, this, &RegisterView::setValueText);
	connect(m_session, &RegisterSession::entryAdded, this, &RegisterView::onEntry);
	connect(m_session, &RegisterSession::readDone, this, &RegisterView::onReadDone);
	connect(m_session, &RegisterSession::busyChanged, this, [this](bool busy) {
		m_readBtn->setEnabled(!busy && m_session->canRead());
		m_writeBtn->setEnabled(!busy && m_session->canWrite());
		m_dumpBtn->setEnabled(!busy && m_session->canRead());
		m_target->combo()->setEnabled(!busy);
	});
	connect(m_session, &RegisterSession::dumpFinished, this, [this](int ok, int total) {
		setStatus(QStringLiteral("Dumped %1 of %2 registers").arg(ok).arg(total), ok != total);
	});

	selectTarget(0);
}

QWidget *RegisterView::buildRegisterSection(QWidget *parent)
{
	auto *section = new MenuSectionWidget(parent);
	section->contentLayout()->setSpacing(8);
	section->contentLayout()->addWidget(sectionTitle("Register", section));
	m_target = new MenuCombo("Device", section);
	for(const Target &t : std::as_const(m_targets)) {
		m_target->combo()->addItem(targetLabel(t.owner));
	}
	m_target->setVisible(m_targets.size() > 1);
	QWidget *addrRow = nullptr;
	m_address = hexEdit("Address (hex)", "0x0", section, &addrRow);
	m_address->setObjectName("registerAddress");
	m_width = new MenuCombo("Width (bits)", section);
	m_width->combo()->addItems({"8", "16", "32"});
	QWidget *valueRow = nullptr;
	m_value = hexEdit("Value (hex)", "0x0", section, &valueRow);
	m_value->setObjectName("registerValue");
	auto *btnRow = new QWidget(section);
	Style::setStyle(btnRow, style::properties::widget::basicBackground);
	auto *blay = new QHBoxLayout(btnRow);
	blay->setContentsMargins(0, 0, 0, 0);
	m_readBtn = new QPushButton("Read", btnRow);
	m_writeBtn = new QPushButton("Write", btnRow);
	Style::setStyle(m_readBtn, style::properties::button::basicButton);
	Style::setStyle(m_writeBtn, style::properties::button::basicButton);
	blay->addWidget(m_readBtn);
	blay->addWidget(m_writeBtn);
	for(QWidget *w : std::initializer_list<QWidget *>{m_target, addrRow, m_width, valueRow, btnRow}) {
		section->contentLayout()->addWidget(w);
	}
	section->contentLayout()->addStretch();
	section->setMinimumWidth(320);
	section->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	connect(m_target->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, &RegisterView::selectTarget);
	connect(m_width->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, &RegisterView::applyWidth);
	connect(m_readBtn, &QPushButton::clicked, this, &RegisterView::readClicked);
	connect(m_writeBtn, &QPushButton::clicked, this, &RegisterView::writeClicked);
	connect(m_address, &QLineEdit::returnPressed, this, &RegisterView::readClicked);
	connect(m_value, &QLineEdit::returnPressed, this, &RegisterView::writeClicked);
	connect(m_value, &QLineEdit::editingFinished, this, [this]() {
		uint32_t v = 0;
		if(RegisterSession::parseHex(m_value->text(), width(), v)) {
			m_bits->setValue(v);
		}
	});
	return section;
}

QWidget *RegisterView::buildBitsSection(QWidget *parent)
{
	auto *section = new MenuSectionWidget(parent);
	section->contentLayout()->setSpacing(8);
	section->contentLayout()->addWidget(sectionTitle("Bits (click to toggle, then Write)", section));
	m_bits = new BitToggleGrid(section);
	section->contentLayout()->addWidget(m_bits);
	section->contentLayout()->addStretch();
	section->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_bitsSection = section;
	fitBitsSection();
	return section;
}

QWidget *RegisterView::buildDumpSection(QWidget *parent)
{
	auto *section = new MenuSectionWidget(parent);
	section->contentLayout()->setSpacing(8);
	section->contentLayout()->addWidget(sectionTitle("Dump range", section));
	auto *row = new QWidget(section);
	Style::setStyle(row, style::properties::widget::basicBackground);
	auto *rlay = new QHBoxLayout(row);
	rlay->setContentsMargins(0, 0, 0, 0);
	rlay->setSpacing(10);
	QWidget *fromRow = nullptr;
	m_dumpFrom = hexEdit("From (hex)", "0x0", row, &fromRow);
	m_dumpFrom->setObjectName("dumpFrom");
	m_dumpCount = new gui::MenuSpinbox("Count", 16, "", 1, MAX_DUMP, false, false, false, row);
	m_dumpCount->setIncrementMode(gui::MenuSpinbox::IS_POW2);
	m_dumpCount->setScalingEnabled(false);
	Style::setStyle(m_dumpCount, style::properties::widget::basicBackground);
	m_dumpBtn = new QPushButton("Dump", row);
	Style::setStyle(m_dumpBtn, style::properties::button::basicButton);
	m_dumpBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	rlay->addWidget(fromRow, 1);
	rlay->addWidget(m_dumpCount, 0);
	rlay->addWidget(m_dumpBtn, 0, Qt::AlignBottom);
	section->contentLayout()->addWidget(row);
	connect(m_dumpBtn, &QPushButton::clicked, this, &RegisterView::dumpClicked);
	return section;
}

void RegisterView::selectTarget(int index)
{
	if(index < 0 || index >= m_targets.size()) {
		m_session->setTarget(nullptr, nullptr);
		m_readBtn->setEnabled(false);
		m_writeBtn->setEnabled(false);
		m_dumpBtn->setEnabled(false);
		setStatus("No register access on this node", true);
		return;
	}
	const Target &t = m_targets[index];
	m_session->setTarget(t.reader, t.writer);
	// Default width: what the leaf reports (data_width * 8 on a bus, 32 on libiio).
	const int bits = std::clamp(m_session->valueBits(), 1, 32);
	const int idx = bits <= 8 ? 0 : (bits <= 16 ? 1 : 2);
	{
		QSignalBlocker block(m_width->combo());
		m_width->combo()->setCurrentIndex(idx);
	}
	applyWidth();
	m_readBtn->setEnabled(m_session->canRead());
	m_writeBtn->setEnabled(m_session->canWrite());
	m_dumpBtn->setEnabled(m_session->canRead());
	setStatus(QStringLiteral("%1: address %2 bit, value %3 bit%4")
			  .arg(targetLabel(t.owner))
			  .arg(m_session->addressBits())
			  .arg(m_session->valueBits())
			  .arg(m_session->canWrite() ? "" : " (read-only)"));
}

int RegisterView::width() const { return m_width->combo()->currentText().toInt(); }

void RegisterView::fitBitsSection()
{
	// One column per bit up to a full 16-bit row, at the grid's button pitch plus the margins.
	const int cols = std::min(m_bits->width(), 16);
	m_bitsSection->setFixedWidth(cols * 32 + 40);
}

void RegisterView::applyWidth()
{
	m_bits->setWidth(width());
	fitBitsSection();
	uint32_t v = 0;
	if(RegisterSession::parseHex(m_value->text(), 32, v)) {
		setValueText(v & RegisterSession::mask(width()));
	}
}

bool RegisterView::address(uint32_t &out, QLineEdit *edit) const
{
	if(!RegisterSession::parseHex(edit->text(), m_session->addressBits(), out)) {
		return false;
	}
	return true;
}

void RegisterView::readClicked()
{
	uint32_t addr = 0;
	if(!address(addr, m_address)) {
		setStatus(QStringLiteral("Address must be hex within %1 bits").arg(m_session->addressBits()), true);
		return;
	}
	setStatus(QStringLiteral("Reading %1...").arg(RegisterSession::hex(addr, m_session->addressBits())));
	m_session->read(addr);
}

void RegisterView::writeClicked()
{
	uint32_t addr = 0;
	uint32_t value = 0;
	if(!address(addr, m_address)) {
		setStatus(QStringLiteral("Address must be hex within %1 bits").arg(m_session->addressBits()), true);
		return;
	}
	if(!RegisterSession::parseHex(m_value->text(), width(), value)) {
		setStatus(QStringLiteral("Value must be hex within %1 bits").arg(width()), true);
		return;
	}
	m_bits->setValue(value);
	setStatus(QStringLiteral("Writing %1 <- %2...")
			  .arg(RegisterSession::hex(addr, m_session->addressBits()),
			       RegisterSession::hex(value, width())));
	m_session->write(addr, value);
}

void RegisterView::dumpClicked()
{
	uint32_t from = 0;
	if(!address(from, m_dumpFrom)) {
		setStatus(QStringLiteral("From must be hex within %1 bits").arg(m_session->addressBits()), true);
		return;
	}
	const int count = int(m_dumpCount->value());
	setStatus(QStringLiteral("Dumping %1 registers from %2...")
			  .arg(count)
			  .arg(RegisterSession::hex(from, m_session->addressBits())));
	m_session->dump(from, count);
}

void RegisterView::onEntry(const RegisterEntry &entry)
{
	const int row = m_history->rowCount();
	m_history->insertRow(row);
	m_history->setItem(row, 0, new QTableWidgetItem(entry.time.toString("hh:mm:ss")));
	m_history->setItem(row, 1, new QTableWidgetItem(entry.op));
	m_history->setItem(row, 2, new QTableWidgetItem(RegisterSession::hex(entry.address, m_session->addressBits())));
	auto *value = new QTableWidgetItem(entry.ok ? RegisterSession::hex(entry.value, width())
						    : QStringLiteral("failed: %1").arg(entry.error));
	if(!entry.ok) {
		value->setForeground(QColor(Style::getAttribute(json::theme::content_error)));
	}
	m_history->setItem(row, 3, value);
	m_history->scrollToBottom();
	if(!entry.ok) {
		setStatus(QStringLiteral("%1 %2 failed: %3")
				  .arg(entry.op, RegisterSession::hex(entry.address, m_session->addressBits()),
				       entry.error),
			  true);
	} else if(entry.op == "write") {
		setStatus(QStringLiteral("%1 <- %2")
				  .arg(RegisterSession::hex(entry.address, m_session->addressBits()),
				       RegisterSession::hex(entry.value, width())));
	}
}

void RegisterView::onReadDone(uint32_t address, uint32_t value)
{
	setValueText(value & RegisterSession::mask(width()));
	m_bits->setValue(value);
	setStatus(QStringLiteral("%1 = %2").arg(RegisterSession::hex(address, m_session->addressBits()),
						RegisterSession::hex(value, width())));
}

void RegisterView::setValueText(uint32_t value) { m_value->setText(RegisterSession::hex(value, width())); }

void RegisterView::exportHistory()
{
	const QList<RegisterEntry> &h = m_session->history();
	if(h.isEmpty()) {
		setStatus("Nothing to export: the history is empty", true);
		return;
	}
	QString csv = "time,op,address,value,error\n";
	for(const RegisterEntry &e : h) {
		csv += e.time.toString(Qt::ISODateWithMs) + "," + e.op + "," +
			RegisterSession::hex(e.address, m_session->addressBits()) + "," +
			(e.ok ? RegisterSession::hex(e.value, width()) : QString()) + "," +
			(e.ok ? QString() : "\"" + QString(e.error).replace('"', "'") + "\"") + "\n";
	}
	if(FileManagerHelper::saveDataToFile(this, csv, "Export register history")) {
		setStatus(QStringLiteral("Exported %1 rows").arg(h.size()));
	}
}

void RegisterView::clearHistory()
{
	m_session->clearHistory();
	m_history->setRowCount(0);
	setStatus("History cleared");
}

void RegisterView::setStatus(const QString &text, bool error)
{
	m_status->setText(text);
	Style::setStyle(m_status, style::properties::label::warning, error);
	Style::setStyle(m_status, style::properties::label::subtle, !error);
}

#include "moc_registerview.cpp"
