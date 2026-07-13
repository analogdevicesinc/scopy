#pragma once

#include <QComboBox>
#include <QHash>
#include <QList>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

#include <core/acq_engine/DataKey.h>
#include <core/decoder/IDecoderBackend.h>
#include <core/decoder/IDecoderCatalog.h>

class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QSpinBox;

namespace scopy {
namespace acq { class DataStore; }
namespace decoder { class DecoderLogger; }

namespace adc {

class DecoderManager;

// Editor widget for one active DecoderInstance. Builds itself from a
// DecoderInfo (sigrok metadata) and pushes changes to DecoderManager
// only when the Apply button is clicked.
class DecoderEditor : public QWidget
{
	Q_OBJECT
public:
	DecoderEditor(const QString &uid,
	              const scopy::decoder::DecoderInfo &info,
	              DecoderManager *mgr,
	              scopy::acq::DataStore *store,
	              QWidget *parent = nullptr);

	QString uid() const { return m_uid; }

	// Rebuild channel-key combo entries. Preserves current selections
	// where possible.
	void refreshRawKeys(const QList<scopy::acq::DataKey> &rawKeys);

	// Enable/disable Apply while the engine is running.
	void setApplyEnabled(bool en);

Q_SIGNALS:
	void removeRequested(const QString &uid);

private Q_SLOTS:
	void onApplyClicked();
	void markDirty();

public:
	// Three-state indicator of the editor vs. running processor:
	//   NotApplied — decoder added but Apply never clicked yet
	//   Running    — last-applied config matches what the processor uses
	//   Modified   — user edited a value since the last Apply
	enum class EditorState { NotApplied, Running, Modified };

private:
	QWidget *buildChannelsGroup(QWidget *parent);
	QWidget *buildOptionsGroup(QWidget *parent);
	QWidget *buildOptionEditor(const scopy::decoder::OptionInfo &o);

	// Read the current UI values into cfg + orderedRawKeys.
	void collect(scopy::decoder::DecoderConfig &cfg,
	             QList<scopy::acq::DataKey> &orderedRawKeys) const;

	// Push a new visual state to the status dot + text.
	void setState(EditorState s);

	QString                        m_uid;
	scopy::decoder::DecoderInfo    m_info;
	DecoderManager                *m_mgr;
	QPointer<scopy::acq::DataStore> m_store;

	QDoubleSpinBox                *m_sampleRateSpin{nullptr};

	// One channel row per DecoderInfo::channels entry (same order).
	QList<QComboBox *>             m_channelCombos;

	// Option id → widget (heterogeneous). We introspect at collect() time.
	QHash<QString, QWidget *>      m_optionWidgets;

	QPushButton                   *m_applyBtn{nullptr};
	QPushButton                   *m_removeBtn{nullptr};

	QLabel                        *m_statusDot{nullptr};
	QLabel                        *m_statusText{nullptr};
	EditorState                    m_state{EditorState::NotApplied};
};

// Right-stack panel that lists all active decoders and lets the user add
// new ones via an inline picker. Backend-agnostic: it uses IDecoderCatalog
// for enumeration/introspection so any concrete catalog implementation
// (sigrok-cli, libsigrok, a custom CLI, …) plugs in via the composition
// root. The catalog is non-owning and must outlive the panel.
class DecoderPanel : public QWidget
{
	Q_OBJECT
public:
	DecoderPanel(DecoderManager *mgr,
	             scopy::acq::DataStore *store,
	             scopy::decoder::IDecoderCatalog *catalog,
	             QWidget *parent = nullptr);

	void setLogger(scopy::decoder::DecoderLogger *lg) { m_logger = lg; }

	// Called by the controller each cycle so the channel combos reflect
	// the current DataStore key set.
	void refreshRawKeys(const QList<scopy::acq::DataKey> &keys);

	// Called by the controller on engine started/stopped to gate Apply.
	void setEngineRunning(bool running);

private Q_SLOTS:
	void onAddClicked();
	void onEditorRemoveRequested(const QString &uid);

private:
	void appendEditorFor(const QString &uid, const QString &decoderId);

	DecoderManager                     *m_mgr;
	QPointer<scopy::acq::DataStore>     m_store;

	// Non-owning: injected at construction, driven by whichever backend
	// the composition root wired up. Must outlive this widget.
	scopy::decoder::IDecoderCatalog    *m_catalog{nullptr};

	// Cached raw-key list forwarded to each new editor at creation time.
	QList<scopy::acq::DataKey>          m_rawKeysCache;
	bool                                m_engineRunning{false};

	QVBoxLayout                        *m_editorsLay{nullptr};
	QList<DecoderEditor *>              m_editors;

	// Inline picker section shown by "+ Add decoder…". Only one at a
	// time; clicking Add again closes any existing picker first.
	QPointer<QWidget>                   m_pickerWidget;

	scopy::decoder::DecoderLogger      *m_logger{nullptr};
};

} // namespace adc
} // namespace scopy
