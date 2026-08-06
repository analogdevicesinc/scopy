#pragma once

#include <QHash>
#include <QPointer>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

#include <core/acq_engine/AcquisitionError.h>

namespace scopy {

class MapStackedWidget;

namespace acq {
class AcquisitionEngine;
class Block;
class DataStore;
} // namespace acq

namespace adc {

class DecoderManager;

// Live tree of every block registered with an AcquisitionEngine, over a host
// that shows the selected block's own settings widget.
//
// This is the single home for block settings widgets: it hosts whatever
// Block::settingsWidget() hands back, so a block has exactly one widget no
// matter how many panels want to show it. A host needing to build the widget
// itself (extra constructor arguments) gives it to the block via
// Block::setSettingsWidget() and it surfaces here.
//
// The tree is derived, never stored: the engine has no graph object, so rows
// come from sources()/processors() and the in/out columns from watchedKeys()
// and outputKeys(). Nothing here names a concrete block type — the only
// exception is grouping decoders into their own section, which is a display
// choice rather than data extraction.
//
// Main-thread only. Engine signals originate on the worker thread, so every
// connection to one is queued.
class PipelineInspector : public QWidget
{
	Q_OBJECT
public:
	PipelineInspector(scopy::acq::AcquisitionEngine *engine,
			  scopy::acq::DataStore *store,
			  QWidget *parent = nullptr);

	// Optional: lets removed decoders drop their cached menu entry.
	void setDecoderManager(scopy::adc::DecoderManager *mgr);

public Q_SLOTS:
	// Rebuild rows from the engine's current block lists. Preserves the
	// selected block and each group's expanded state.
	void rebuild();

	// Refresh the status column only; never touches tree structure. Safe to
	// call at cycle rate.
	void refreshStatus();

private:
	// Row bookkeeping for one block.
	struct Row
	{
		QTreeWidgetItem              *item{nullptr};
		QPointer<scopy::acq::Block>   block;
		bool                          isSource{false};
	};

	enum Group
	{
		GroupSources = 0,
		GroupProcessors,
		GroupDecoders,
		GroupCount,
	};

	// Column layout of the tree.
	enum Column
	{
		ColBlock = 0,
		ColStatus,
		ColIn,
		ColOut,
		ColCount,
	};

	void buildUi();
	void connectEngine();

	// Attach or refresh one block's row under `parent`.
	QTreeWidgetItem *syncBlockRow(QTreeWidgetItem *parent, int index,
				      scopy::acq::Block *block, bool isSource);

	void updateRowStatus(const Row &row);
	void showMenuFor(scopy::acq::Block *block);
	void onItemChanged(QTreeWidgetItem *item, int column);
	void onErrorReported(int severity, const QString &id, const QString &message);

	// The block a row stands for, or nullptr for a group row.
	static scopy::acq::Block *blockOf(QTreeWidgetItem *item);

	scopy::acq::AcquisitionEngine *m_engine{nullptr};
	scopy::acq::DataStore         *m_store{nullptr};
	QPointer<DecoderManager>       m_decoderMgr;

	QTreeWidget            *m_tree{nullptr};
	MapStackedWidget       *m_host{nullptr};
	QWidget                *m_placeholder{nullptr};
	QTreeWidgetItem        *m_groups[GroupCount]{};

	QList<Row> m_rows;

	// Last diagnostic per block name, shown as a badge plus tooltip.
	struct Diag
	{
		scopy::acq::AcquisitionError::Severity severity{};
		QString                                message;
	};
	QHash<QString, Diag> m_diags;

	// Guards the itemChanged -> setEnabled -> enabledChanged -> itemChanged loop.
	bool m_applying{false};
};

} // namespace adc
} // namespace scopy
