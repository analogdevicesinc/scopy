#include "PipelineInspector.h"

#include "DecoderManager.h"

#include <core/acq_engine/AcquisitionEngine.h>
#include <core/acq_engine/Block.h>
#include <core/acq_engine/DataStore.h>
#include <core/acq_engine/ExternalDecoderProcessor.h>
#include <core/acq_engine/ProcessorBlock.h>
#include <core/acq_engine/SourceBlock.h>

#include <gui/mapstackedwidget.h>
#include <gui/style.h>

#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QVBoxLayout>

namespace scopy {
namespace adc {

namespace {

QString joinKeys(const QList<scopy::acq::DataKey> &keys)
{
	QStringList parts;
	parts.reserve(keys.size());
	for(const scopy::acq::DataKey &k : keys)
		parts << k.key;
	return parts.join(QStringLiteral(", "));
}

} // namespace

PipelineInspector::PipelineInspector(scopy::acq::AcquisitionEngine *engine,
				     scopy::acq::DataStore *store,
				     QWidget *parent)
	: QWidget(parent)
	, m_engine(engine)
	, m_store(store)
{
	buildUi();
	connectEngine();
	rebuild();
}

void PipelineInspector::buildUi()
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(0);

	// Side by side: the panel is wide and short (it spans the bottom rail), so a
	// vertical split would leave both halves too short to read.
	auto *splitter = new QSplitter(Qt::Horizontal, this);
	// global.qss paints every QSplitter background_subtle, which is near-black and reads
	// as a gap cut through the debug panel; transparent lets the panel's own fill show in
	// the gutter. Spelled as a type selector rather than via setBackgroundColor(), whose
	// `.QWidget` selector matches exact QWidget instances only and so misses a QSplitter.
	splitter->setStyleSheet(QStringLiteral("QSplitter { background-color: transparent; }"));

	// ---- tree ----
	m_tree = new QTreeWidget(splitter);
	// The panes are the sections of this tab, so each carries its own fill and border:
	// the global QWidget rule is transparent, so without this a tree is a black hole in
	// the panel with nothing marking where it ends.
	Style::setStyle(m_tree, style::properties::widget::basicComponent);
	m_tree->setColumnCount(ColCount);
	m_tree->setHeaderLabels({"Block", "Status", "In", "Out"});
	m_tree->setAlternatingRowColors(true);
	m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tree->header()->setStretchLastSection(true);
	// Block names run long, the status column holds one short badge.
	m_tree->setColumnWidth(ColBlock, Style::getDimension(json::global::unit_5) * 2);
	m_tree->setColumnWidth(ColStatus, Style::getDimension(json::global::unit_5));

	for(int g = 0; g < GroupCount; ++g) {
		static const char *names[GroupCount] = {"Sources", "Processors", "Decoders"};
		auto *item = new QTreeWidgetItem(m_tree);
		item->setText(ColBlock, QString::fromLatin1(names[g]));
		item->setFirstColumnSpanned(false);
		item->setFlags(Qt::ItemIsEnabled); // not selectable, not checkable
		QFont f = item->font(ColBlock);
		f.setBold(true);
		item->setFont(ColBlock, f);
		item->setExpanded(true);
		m_groups[g] = item;
	}

	connect(m_tree, &QTreeWidget::itemChanged, this, &PipelineInspector::onItemChanged);
	connect(m_tree, &QTreeWidget::itemSelectionChanged, this, [this]() {
		const QList<QTreeWidgetItem *> sel = m_tree->selectedItems();
		showMenuFor(sel.isEmpty() ? nullptr : blockOf(sel.first()));
	});

	// ---- menu host ----
	m_host = new MapStackedWidget(nullptr);

	auto *placeholder = new QLabel("Select a block to see its settings.");
	placeholder->setAlignment(Qt::AlignCenter);
	placeholder->setWordWrap(true);
	Style::setStyle(placeholder, style::properties::label::subtle);
	m_placeholder = placeholder;
	m_host->add(QStringLiteral("__none__"), m_placeholder);
	m_host->show(QStringLiteral("__none__"));

	auto *hostScroll = new QScrollArea(splitter);
	hostScroll->setWidget(m_host);
	hostScroll->setWidgetResizable(true);
	hostScroll->setFrameShape(QFrame::NoFrame);
	// Same surface as the tree beside it. The fill goes on the scroll area rather than on
	// m_host: m_host is only as tall as the current page, so filling it would leave the
	// rest of the pane unpainted.
	Style::setStyle(hostScroll, style::properties::widget::basicComponent);

	splitter->addWidget(m_tree);
	splitter->addWidget(hostScroll);
	splitter->setStretchFactor(0, 3);
	splitter->setStretchFactor(1, 2);
	const int unit = Style::getDimension(json::global::unit_5);
	splitter->setSizes({unit * 5, unit * 3});

	lay->addWidget(splitter);
}

void PipelineInspector::connectEngine()
{
	if(!m_engine)
		return;

	// Structural changes. Engine signals cross from the worker thread.
	connect(m_engine, &scopy::acq::AcquisitionEngine::blocksChanged,
		this, &PipelineInspector::rebuild, Qt::QueuedConnection);

	// Run state only changes the status column, never the rows.
	connect(m_engine, &scopy::acq::AcquisitionEngine::started,
		this, &PipelineInspector::refreshStatus, Qt::QueuedConnection);
	connect(m_engine, &scopy::acq::AcquisitionEngine::stopped,
		this, &PipelineInspector::refreshStatus, Qt::QueuedConnection);
	connect(m_engine, &scopy::acq::AcquisitionEngine::forceStopped,
		this, &PipelineInspector::refreshStatus, Qt::QueuedConnection);

	connect(m_engine, &scopy::acq::AcquisitionEngine::error,
		this, &PipelineInspector::onErrorReported, Qt::QueuedConnection);

	// A fresh run supersedes the previous run's diagnostics.
	connect(m_engine, &scopy::acq::AcquisitionEngine::started, this, [this]() {
		m_diags.clear();
		refreshStatus();
	}, Qt::QueuedConnection);

	if(m_store) {
		// A new key can change what a block reports as its output — a
		// decoder's stage keys appear only once configured.
		connect(m_store, &scopy::acq::DataStore::keysChanged,
			this, &PipelineInspector::rebuild, Qt::QueuedConnection);
	}
}

void PipelineInspector::setDecoderManager(DecoderManager *mgr)
{
	m_decoderMgr = mgr;
	if(!mgr)
		return;

	// blocksChanged already covers the tree; this drops the host entry so a
	// later block reusing the name can't inherit the removed one's widget.
	connect(mgr, &DecoderManager::decoderRemoved, this, [this](const QString &name) {
		if(m_host)
			m_host->remove(name);
		rebuild();
	});
	connect(mgr, &DecoderManager::decoderAdded, this, [this](const QString &) { rebuild(); });
}

scopy::acq::Block *PipelineInspector::blockOf(QTreeWidgetItem *item)
{
	if(!item)
		return nullptr;
	auto *obj = item->data(ColBlock, Qt::UserRole).value<QObject *>();
	return qobject_cast<scopy::acq::Block *>(obj);
}

void PipelineInspector::rebuild()
{
	if(!m_engine || !m_tree)
		return;

	// Remember what the user had selected so a rebuild isn't disruptive.
	const QList<QTreeWidgetItem *> sel = m_tree->selectedItems();
	scopy::acq::Block *selected = sel.isEmpty() ? nullptr : blockOf(sel.first());

	// Structure edits fire itemChanged; suppress the enable round-trip.
	m_applying = true;

	m_rows.clear();

	int nextIndex[GroupCount] = {0, 0, 0};

	for(scopy::acq::SourceBlock *src : m_engine->sources()) {
		if(!src)
			continue;
		QTreeWidgetItem *item = syncBlockRow(m_groups[GroupSources],
						     nextIndex[GroupSources]++, src, true);
		m_rows.append({item, src, true});
	}

	for(scopy::acq::ProcessorBlock *proc : m_engine->processors()) {
		if(!proc)
			continue;
		// Decoders get their own section: they arrive and leave at runtime
		// and carry a stage stack, so mixing them into Processors buries
		// the fixed pipeline.
		auto *dec = qobject_cast<scopy::acq::ExternalDecoderProcessor *>(proc);
		const Group g = dec ? GroupDecoders : GroupProcessors;

		QTreeWidgetItem *item = syncBlockRow(m_groups[g], nextIndex[g]++, proc, false);
		m_rows.append({item, proc, false});

		if(!dec)
			continue;

		// One output key per stack stage, index 0 = root. Stages beyond the
		// root become child rows so a stacked decoder reads as a stack.
		const QList<scopy::acq::DataKey> outs = dec->outputKeys();
		while(item->childCount() > qMax(0, outs.size() - 1))
			delete item->takeChild(item->childCount() - 1);
		for(int s = 1; s < outs.size(); ++s) {
			QTreeWidgetItem *stage = (s - 1) < item->childCount()
				? item->child(s - 1)
				: new QTreeWidgetItem(item);
			stage->setFlags(Qt::ItemIsEnabled);
			stage->setText(ColBlock, QStringLiteral("stage %1").arg(s));
			stage->setText(ColStatus, QString());
			stage->setText(ColIn, outs.at(s - 1).key);
			stage->setText(ColOut, outs.at(s).key);
		}
		item->setExpanded(true);
	}

	// Drop rows for blocks that were removed from the engine.
	for(int g = 0; g < GroupCount; ++g) {
		while(m_groups[g]->childCount() > nextIndex[g])
			delete m_groups[g]->takeChild(m_groups[g]->childCount() - 1);
		m_groups[g]->setHidden(nextIndex[g] == 0);
	}

	m_applying = false;

	// Reselect by pointer; falls through to the placeholder if it's gone.
	bool restored = false;
	if(selected) {
		for(const Row &row : m_rows) {
			if(row.block == selected) {
				m_tree->setCurrentItem(row.item);
				restored = true;
				break;
			}
		}
	}
	if(!restored && selected)
		showMenuFor(nullptr);

	refreshStatus();
}

QTreeWidgetItem *PipelineInspector::syncBlockRow(QTreeWidgetItem *parent, int index,
						 scopy::acq::Block *block, bool isSource)
{
	QTreeWidgetItem *item = index < parent->childCount() ? parent->child(index)
							     : new QTreeWidgetItem(parent);

	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
	item->setText(ColBlock, block->name());
	item->setCheckState(ColBlock, block->isEnabled() ? Qt::Checked : Qt::Unchecked);
	item->setData(ColBlock, Qt::UserRole,
		      QVariant::fromValue(static_cast<QObject *>(block)));

	// watchedKeys() is the inbound edge set; sources have none by definition.
	if(auto *proc = qobject_cast<scopy::acq::ProcessorBlock *>(block))
		item->setText(ColIn, joinKeys(proc->watchedKeys()));
	else
		item->setText(ColIn, QString());

	item->setText(ColOut, joinKeys(block->outputKeys()));
	item->setToolTip(ColIn, item->text(ColIn));
	item->setToolTip(ColOut, item->text(ColOut));

	if(isSource) {
		if(auto *src = qobject_cast<scopy::acq::SourceBlock *>(block)) {
			// Channel edits change the source's key set, so the Out
			// column and the row count both need a refresh.
			connect(src, &scopy::acq::SourceBlock::channelsChanged,
				this, &PipelineInspector::rebuild, Qt::UniqueConnection);
		}
	}

	// Enable state can also be flipped from the block's own settings widget.
	connect(block, &scopy::acq::Block::enabledChanged,
		this, &PipelineInspector::refreshStatus, Qt::UniqueConnection);

	return item;
}

void PipelineInspector::refreshStatus()
{
	if(!m_engine || !m_tree)
		return;

	// Which keys the worker wrote this cycle — the basis for "live".
	QSet<scopy::acq::DataKey> produced;
	if(m_store)
		produced = m_store->cycleKeys();

	const bool running = m_engine->isRunning();

	m_applying = true;
	for(const Row &row : m_rows) {
		if(!row.item || row.block.isNull())
			continue;

		scopy::acq::Block *block = row.block;

		// Keep the checkbox honest even when the flip came from elsewhere.
		const Qt::CheckState want = block->isEnabled() ? Qt::Checked : Qt::Unchecked;
		if(row.item->checkState(ColBlock) != want)
			row.item->setCheckState(ColBlock, want);

		QString badge;
		QColor  color;

		if(!block->isEnabled()) {
			badge = QStringLiteral("off");
			color = Style::getColor(json::theme::interactive_subtle_disabled);
		} else if(!running) {
			badge = QStringLiteral("stop");
			color = Style::getColor(json::theme::content_silent);
		} else {
			bool live = false;
			for(const scopy::acq::DataKey &k : block->outputKeys()) {
				if(produced.contains(k)) {
					live = true;
					break;
				}
			}
			badge = live ? QStringLiteral("live") : QStringLiteral("idle");
			color = live ? Style::getColor(json::theme::interactive_primary_idle)
				     : Style::getColor(json::theme::content_silent);
		}

		// A diagnostic outranks the activity badge — it's the thing the user
		// needs to notice. The activity word is kept alongside the marker.
		const auto diag = m_diags.constFind(block->name());
		if(diag != m_diags.constEnd()) {
			const bool crit = diag->severity ==
				scopy::acq::AcquisitionError::Severity::Critical;
			badge = (crit ? QStringLiteral("!! ") : QStringLiteral("! ")) + badge;
			color = crit ? Style::getColor(json::theme::content_error)
				     : Style::getColor(json::theme::danger_default);
			row.item->setToolTip(ColStatus, diag->message);
		} else {
			row.item->setToolTip(ColStatus, QString());
		}

		row.item->setText(ColStatus, badge);
		row.item->setForeground(ColStatus, color);

		// Dim the whole row when the block is switched off.
		const QColor nameColor =
			Style::getColor(block->isEnabled() ? json::theme::content_default
							  : json::theme::interactive_subtle_disabled);
		row.item->setForeground(ColBlock, nameColor);
	}
	m_applying = false;
}

void PipelineInspector::onItemChanged(QTreeWidgetItem *item, int column)
{
	if(m_applying || column != ColBlock)
		return;

	scopy::acq::Block *block = blockOf(item);
	if(!block)
		return;

	const bool want = item->checkState(ColBlock) == Qt::Checked;
	if(block->isEnabled() == want)
		return;

	// enabledChanged comes back into refreshStatus(); m_applying keeps that
	// from re-entering here.
	m_applying = true;
	block->setEnabled(want);
	m_applying = false;

	refreshStatus();
}

void PipelineInspector::onErrorReported(int severity, const QString &id, const QString &message)
{
	// Block::report() emits under the block's own name(), so a report lands on
	// exactly one row without any extra plumbing. Engine-level messages use
	// "engine" and match no row — harmless, they still show in the log panel.
	const auto sev = static_cast<scopy::acq::AcquisitionError::Severity>(severity);
	if(sev == scopy::acq::AcquisitionError::Severity::Info)
		return;

	m_diags.insert(id, Diag{sev, message});
	refreshStatus();
}

void PipelineInspector::showMenuFor(scopy::acq::Block *block)
{
	if(!m_host)
		return;

	if(!block) {
		m_host->show(QStringLiteral("__none__"));
		return;
	}

	const QString key = block->name();

	if(!m_host->contains(key)) {
		// The block owns the instance, so a widget a host built itself and
		// handed over via setSettingsWidget() comes back from here too — this
		// is the only place a settings widget gets parented.
		QWidget *w = block->settingsWidget(m_host);
		if(!w) {
			m_host->show(QStringLiteral("__none__"));
			return;
		}
		m_host->add(key, w);
	}

	m_host->show(key);
}

} // namespace adc
} // namespace scopy
