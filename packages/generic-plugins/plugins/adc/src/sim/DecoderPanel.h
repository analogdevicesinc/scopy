#pragma once

#include <QComboBox>
#include <QHash>
#include <QList>
#include <QPointer>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

#include <functional>

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

Q_SIGNALS:
	// Emitted when the "+ Stack…" button is clicked. The panel opens
	// a filtered picker and, on success, calls appendStage(id) here.
	void stackPickerRequested(DecoderEditor *editor,
	                          const QStringList &acceptedInputIds);

public:
	// Append a stage to this editor after DecoderManager::pushStage has
	// already grown the runtime side. Rebuilds the stack UI and does not
	// re-apply the config (caller decides).
	void appendStage(const scopy::decoder::DecoderInfo &info);

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
	// One per-stage sub-group inside the editor. Only stage 0 has a
	// populated channel-combos list (stacked stages consume the previous
	// stage's product, so they don't bind raw channels).
	struct Stage {
		scopy::decoder::DecoderInfo info;
		QWidget                    *box{nullptr};
		QList<QComboBox *>          channelCombos;
		QHash<QString, QWidget *>   optionWidgets;
	};

	QWidget *buildStageWidget(int stageIndex,
	                          const scopy::decoder::DecoderInfo &info,
	                          Stage &out);
	QWidget *buildChannelsGroup(QWidget *parent, Stage &st);
	QWidget *buildOptionsGroup(QWidget *parent, Stage &st);
	QWidget *buildOptionEditor(const scopy::decoder::OptionInfo &o);

	void rebuildStackButtonState();

	// Read the current UI values into cfg + orderedRawKeys.
	void collect(scopy::decoder::DecoderConfig &cfg,
	             QList<scopy::acq::DataKey> &orderedRawKeys) const;

	// Push a new visual state to the status dot + text.
	void setState(EditorState s);

	QString                        m_uid;
	DecoderManager                *m_mgr;
	QPointer<scopy::acq::DataStore> m_store;

	QDoubleSpinBox                *m_sampleRateSpin{nullptr};

	// Container for the per-stage sub-groups; the "+ Stack…" button lives
	// right below the last stage's widget.
	QVBoxLayout                   *m_stagesLay{nullptr};
	QPushButton                   *m_stackBtn{nullptr};
	QList<Stage>                   m_stages;

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
	void onStackPickerRequested(DecoderEditor *editor,
	                            const QStringList &acceptedInputIds);

private:
	void appendEditorFor(const QString &uid, const QString &decoderId);

	// Shared filtered picker used by both "+ Add decoder…" and
	// "+ Stack decoder…". The filter is applied to every decoder id
	// returned by the catalog; false hides that row entirely. On accept,
	// the callback receives the selected decoder id.
	void openPicker(const QString &title,
	                std::function<bool(const QString &)> filter,
	                std::function<void(const QString &)> onAccept);

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
