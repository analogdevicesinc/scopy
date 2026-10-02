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

#pragma once

#include "scopy-core_export.h"

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

#include <core/acq_engine/datakey.h>
#include <core/decoder/idecoderbackend.h>
#include <core/decoder/idecodercatalog.h>

class QButtonGroup;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QSpinBox;

namespace scopy {
namespace acq {
class DataStore;
}

namespace decoder {

class DecoderLogger;
class DecoderManager;

// Editor for one DecoderInstance; auto-applies changes to DecoderManager.
class SCOPY_CORE_EXPORT DecoderEditor : public QWidget
{
	Q_OBJECT
public:
	DecoderEditor(const QString &uid, const DecoderInfo &info, DecoderManager *mgr, scopy::acq::DataStore *store,
		      QWidget *parent = nullptr);

	QString uid() const { return m_uid; }

	// Rebuild raw-channel combos from the DataStore key set.
	void refreshKeys(const QList<scopy::acq::DataKey> &keys);

	// Rebuild annotation-source combo from manager's active decoders.
	void refreshAnnotationSources();

Q_SIGNALS:
	void removeRequested(const QString &uid);

Q_SIGNALS:
	// "+ Stack…" click: panel opens picker, then calls appendStage().
	void stackPickerRequested(DecoderEditor *editor, const QStringList &acceptedInputIds);

public:
	// UI-side append after DecoderManager::pushStage() has run.
	void appendStage(const DecoderInfo &info);

private Q_SLOTS:
	// Collect UI state -> DecoderConfig and push to manager.
	void applyNow();

private:
	// Per-stage sub-group. Only stage 0 has channel combos; stacked
	// stages consume the previous stage's product.
	struct Stage
	{
		DecoderInfo info;
		QWidget *box{nullptr};
		QWidget *channelsBox{nullptr};
		QList<QComboBox *> channelCombos;
		QHash<QString, QWidget *> optionWidgets;
	};

	QWidget *buildStageWidget(int stageIndex, const DecoderInfo &info, Stage &out);
	QWidget *buildChannelsGroup(QWidget *parent, Stage &st);
	QWidget *buildOptionsGroup(QWidget *parent, Stage &st);
	QWidget *buildOptionEditor(const OptionInfo &o);
	QWidget *buildInputModeGroup(QWidget *parent);
	QWidget *buildAnnotationInputGroup(QWidget *parent);

	// Toggle raw-channels vs annotation-input group based on radio.
	void applyInputModeVisibility();

	void rebuildStackButtonState();

	bool isAnnotationMode() const;

	void collect(DecoderConfig &cfg, QList<scopy::acq::DataKey> &orderedRawKeys) const;

	QString m_uid;
	DecoderManager *m_mgr;
	QPointer<scopy::acq::DataStore> m_store;

	QDoubleSpinBox *m_sampleRateSpin{nullptr};

	// Per-stage sub-groups; "+ Stack…" sits below the last stage.
	QVBoxLayout *m_stagesLay{nullptr};
	QPushButton *m_stackBtn{nullptr};
	QList<Stage> m_stages;

	// Input-mode radios: Raw channels vs Annotations from another decoder.
	QGroupBox *m_inputModeBox{nullptr};
	QRadioButton *m_radioRaw{nullptr};
	QRadioButton *m_radioAnn{nullptr};
	QButtonGroup *m_inputModeGroup{nullptr};

	// Annotation-source combo: currentData() = source stage outKey string.
	QLabel *m_annSourceLabel{nullptr};
	QComboBox *m_annSourceCombo{nullptr};

	// annIn.* codec options (samplerate, bitrate, frameformat, radix).
	QWidget *m_annInBox{nullptr};
	QDoubleSpinBox *m_annInSampleRate{nullptr};
	QDoubleSpinBox *m_annInBitrate{nullptr};
	QLineEdit *m_annInFrameFormat{nullptr};
	QComboBox *m_annInRadix{nullptr};

	QPushButton *m_removeBtn{nullptr};
};

// Panel listing active decoders with an inline picker to add more.
// Uses IDecoderCatalog (non-owning; must outlive panel).
class SCOPY_CORE_EXPORT DecoderPanel : public QWidget
{
	Q_OBJECT
public:
	// Who scrolls when the panel outgrows the space it is given.
	enum Scrolling
	{
		// Its own QScrollArea, for a host that does not scroll.
		OwnScrollArea,
		// Report the full height and let the host scroll. Required when the host is itself
		// a scroll area: a QScrollArea's minimumSizeHint does not grow with its content, so
		// a nested one caps the panel's height and cuts off everything past the viewport.
		HostScrolls,
	};

	DecoderPanel(DecoderManager *mgr, scopy::acq::DataStore *store, IDecoderCatalog *catalog,
		     QWidget *parent = nullptr, Scrolling scrolling = OwnScrollArea);

	void setLogger(DecoderLogger *lg) { m_logger = lg; }

	// Called per cycle to refresh raw-channel combos.
	void refreshKeys(const QList<scopy::acq::DataKey> &keys);

	// Wired to decoderAdded/Removed; refreshes annotation-source combos.
	void refreshAnnotationSources();

private Q_SLOTS:
	void onAddClicked();
	void onEditorRemoveRequested(const QString &uid);
	void onStackPickerRequested(DecoderEditor *editor, const QStringList &acceptedInputIds);

private:
	void appendEditorFor(const QString &uid, const QString &decoderId);

	// Shared filtered picker for "+ Add…" and "+ Stack…".
	void openPicker(const QString &title, std::function<bool(const QString &)> filter,
			std::function<void(const QString &)> onAccept);

	DecoderManager *m_mgr;
	QPointer<scopy::acq::DataStore> m_store;

	// Non-owning; must outlive this widget.
	IDecoderCatalog *m_catalog{nullptr};

	QList<scopy::acq::DataKey> m_keysCache;

	QVBoxLayout *m_editorsLay{nullptr};
	QList<DecoderEditor *> m_editors;

	// Only one inline picker at a time.
	QPointer<QWidget> m_pickerWidget;

	DecoderLogger *m_logger{nullptr};
};

} // namespace decoder
} // namespace scopy
