#pragma once

#include "scopy-core_export.h"

#include "SnapshotSource.h"

#include <QList>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTimer;
class QVBoxLayout;

namespace scopy {
namespace acq {

class DataStore;
class AcquisitionEngine;

// Settings panel for a SnapshotSource: one row per slot, each with its name, source picker,
// cyclic-mode toggle and capture / clear buttons.
//
// Built by the host rather than the block, because the key pickers need the DataStore and the
// AcquisitionEngine and a block can reach neither; handed over with Block::setSettingsWidget().
//
// Rows rebuild wholesale on slotsChanged(), but a capture refreshes only the affected row, so
// re-capturing does not drop the focus or a combo the reader is halfway through.
class SCOPY_CORE_EXPORT SnapshotSourceWidget : public QWidget
{
	Q_OBJECT
public:
	// `store` and `engine` may be null — the pickers just come up empty.
	explicit SnapshotSourceWidget(SnapshotSource *src, DataStore *store, AcquisitionEngine *engine,
				      QWidget *parent = nullptr);

public Q_SLOTS:
	// Comes off the worker thread — connect queued.
	void refreshKeys();

	// Refresh one row's status label and its controls' enabled state from the block.
	void syncRowStatus(int index);

private:
	struct Row
	{
		QLineEdit *title{nullptr};
		QComboBox *source{nullptr};
		QCheckBox *cyclicBox{nullptr};
		QLabel *status{nullptr};
		QLabel *keyHint{nullptr}; // the key name the title produces
		QPushButton *captureBtn{nullptr};
		QPushButton *clearBtn{nullptr};
	};

	void setupUI();
	void rebuildRows();
	QWidget *buildRow(int index, const SnapshotSource::Slot &s);

	void pushRow(int index);     // the row's source selection into the block
	void commitTitle(int index); // reverting the field when the block rejects it
	void syncKeyHint(int index); // the key the title would produce, or why it produces none

	// Borrowed. Nulled by QObject::destroyed connects rather than held as QPointer, because
	// this widget can outlive all three: it lives in the menu page's layout while the block
	// is parented to the engine.
	SnapshotSource *m_src{nullptr};
	DataStore *m_store{nullptr};
	AcquisitionEngine *m_engine{nullptr};

	QVBoxLayout *m_rowsLay{nullptr};
	QLabel *m_emptyHint{nullptr};
	QList<Row> m_rows;

	// Drives the playback counter. Polled rather than signalled: the position advances once
	// per acquisition cycle, far faster than a label is worth redrawing.
	QTimer *m_poll{nullptr};

	// True while rows are being populated, so the widgets' own change signals do not write
	// their half-built state back into the block.
	bool m_building{false};
};

} // namespace acq
} // namespace scopy
