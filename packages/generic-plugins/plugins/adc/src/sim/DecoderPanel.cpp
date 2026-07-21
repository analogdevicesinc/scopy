#include "DecoderPanel.h"

#include "DecoderManager.h"

#include <core/acq_engine/DataStore.h>
#include <core/decoder/DecoderLogger.h>

#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/style_properties.h>

#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>
#include <QScrollArea>
#include <QSet>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtConcurrent/QtConcurrentRun>

namespace scopy {
namespace adc {

static constexpr const char *kPanelId = "decoder-panel";

static const QString CH_UNASSIGNED = QStringLiteral("<unassigned>");

// -----------------------------------------------------------------------------
// DecoderEditor
// -----------------------------------------------------------------------------

DecoderEditor::DecoderEditor(const QString &uid,
                             const scopy::decoder::DecoderInfo &info,
                             DecoderManager *mgr,
                             scopy::acq::DataStore *store,
                             QWidget *parent)
	: QWidget(parent)
	, m_uid(uid)
	, m_mgr(mgr)
	, m_store(store)
{
	auto *box = new QGroupBox(QString("%1 [%2]").arg(info.name, uid), this);
	auto *outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->addWidget(box);

	auto *lay = new QVBoxLayout(box);
	lay->setSpacing(6);

	{
		auto *row = new QHBoxLayout();
		row->addWidget(new QLabel("Sample rate (Hz):", box));
		m_sampleRateSpin = new QDoubleSpinBox(box);
		m_sampleRateSpin->setRange(1.0, 1.0e12);
		m_sampleRateSpin->setDecimals(0);
		m_sampleRateSpin->setSingleStep(1.0e5);
		m_sampleRateSpin->setValue(1.0e6);
		row->addWidget(m_sampleRateSpin, 1);
		lay->addLayout(row);
		connect(m_sampleRateSpin,
		        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		        this, &DecoderEditor::applyNow);
	}

	// Input-mode selector must precede the stages layout.
	m_inputModeBox = qobject_cast<QGroupBox *>(buildInputModeGroup(box));
	lay->addWidget(m_inputModeBox);

	m_stagesLay = new QVBoxLayout();
	m_stagesLay->setSpacing(6);
	lay->addLayout(m_stagesLay);

	// Root stage (index 0): only stage with channel bindings.
	Stage root;
	QWidget *rootBox = buildStageWidget(0, info, root);
	m_stagesLay->addWidget(rootBox);
	m_stages.append(root);

	// annIn.* codec sub-group; visibility driven by input-mode radio.
	m_annInBox = buildAnnotationInputGroup(box);
	lay->addWidget(m_annInBox);

	applyInputModeVisibility();

	// Disabled when top stage has no chainable output.
	m_stackBtn = new QPushButton("Add stacked decoder", box);
	scopy::Style::setStyle(m_stackBtn, style::properties::button::subtleButton);
	lay->addWidget(m_stackBtn);
	connect(m_stackBtn, &QPushButton::clicked, this, [this]() {
		if(m_stages.isEmpty()) return;
		Q_EMIT stackPickerRequested(this,
		                            m_stages.last().info.outputIds);
	});
	rebuildStackButtonState();

	{
		auto *row = new QHBoxLayout();
		m_removeBtn = new QPushButton("Remove", box);
		scopy::Style::setStyle(m_removeBtn, style::properties::button::borderButton);
		row->addStretch();
		row->addWidget(m_removeBtn);
		lay->addLayout(row);
	}

	connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
		Q_EMIT removeRequested(m_uid);
	});
}

QWidget *DecoderEditor::buildStageWidget(int stageIndex,
                                         const scopy::decoder::DecoderInfo &info,
                                         Stage &out)
{
	out.info = info;

	const QString title = stageIndex == 0
		? QStringLiteral("Root: %1").arg(info.name)
		: QStringLiteral("Stack %1: %2").arg(stageIndex).arg(info.name);
	auto *box  = new QGroupBox(title, this);
	auto *lay  = new QVBoxLayout(box);
	lay->setSpacing(4);
	out.box = box;

	if(!info.description.isEmpty() || !info.documentation.isEmpty()) {
		auto *row = new QHBoxLayout();
		row->setSpacing(4);

		auto *desc = new QLabel(info.description, box);
		desc->setWordWrap(true);
		desc->setStyleSheet("color:#888");
		row->addWidget(desc, 1);

		if(!info.documentation.isEmpty()) {
			auto *infoBtn = new QToolButton(box);
			infoBtn->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
			infoBtn->setAutoRaise(true);
			QString html = info.documentation.toHtmlEscaped();
			html.replace(QStringLiteral("\n\n"), QStringLiteral("</p><p>"));
			html.replace(QChar('\n'), QChar(' '));
			infoBtn->setToolTip(QStringLiteral("<p>%1</p>").arg(html));
			row->addWidget(infoBtn, 0, Qt::AlignTop);
		}

		lay->addLayout(row);
	}

	// Only root stage binds raw channels.
	if(stageIndex == 0 && !info.channels.isEmpty())
		lay->addWidget(buildChannelsGroup(box, out));

	if(!info.options.isEmpty())
		lay->addWidget(buildOptionsGroup(box, out));

	// Stacked stages: drop this and everything above it.
	if(stageIndex > 0) {
		auto *row = new QHBoxLayout();
		auto *rm  = new QPushButton(QStringLiteral("Unstack decoder"), box);
		scopy::Style::setStyle(rm, style::properties::button::subtleButton);
		row->addStretch();
		row->addWidget(rm);
		lay->addLayout(row);
		connect(rm, &QPushButton::clicked, this, [this, stageIndex]() {
			if(!m_mgr) return;
			if(m_mgr->isEngineRunning()) return;
			m_mgr->popStagesFrom(m_uid, stageIndex);
			while(m_stages.size() > stageIndex) {
				Stage &s = m_stages.last();
				if(s.box) s.box->deleteLater();
				m_stages.removeLast();
			}
			rebuildStackButtonState();
			applyNow();
		});
	}

	return box;
}

QWidget *DecoderEditor::buildChannelsGroup(QWidget *parent, Stage &st)
{
	auto *box  = new QGroupBox("Channels", parent);
	auto *form = new QFormLayout(box);
	form->setLabelAlignment(Qt::AlignLeft);
	st.channelsBox = box;

	for(const auto &ch : st.info.channels) {
		auto *combo = new QComboBox(box);
		combo->addItem(CH_UNASSIGNED, QString());
		QString label = ch.name;
		if(ch.required) label += " *";
		if(!ch.desc.isEmpty()) combo->setToolTip(ch.desc);
		form->addRow(label + ":", combo);
		st.channelCombos.append(combo);
		connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		        this, &DecoderEditor::applyNow);
	}
	return box;
}

QWidget *DecoderEditor::buildOptionsGroup(QWidget *parent, Stage &st)
{
	auto *box  = new QGroupBox("Options", parent);
	auto *form = new QFormLayout(box);
	form->setLabelAlignment(Qt::AlignLeft);

	for(const auto &o : st.info.options) {
		QWidget *w = buildOptionEditor(o);
		if(!w) continue;
		st.optionWidgets.insert(o.id, w);
		form->addRow(o.name + ":", w);
	}
	return box;
}

void DecoderEditor::rebuildStackButtonState()
{
	if(!m_stackBtn) return;
	const bool topProduces = !m_stages.isEmpty()
	                      && !m_stages.last().info.outputIds.isEmpty();
	m_stackBtn->setEnabled(topProduces);
	m_stackBtn->setToolTip(topProduces
		? QStringLiteral("Stack a decoder on top of %1")
			.arg(m_stages.last().info.name)
		: QStringLiteral("This decoder does not expose an output that can be consumed by another decoder."));
}

void DecoderEditor::appendStage(const scopy::decoder::DecoderInfo &info)
{
	const int stageIndex = m_stages.size();
	Stage s;
	QWidget *w = buildStageWidget(stageIndex, info, s);
	m_stagesLay->addWidget(w);
	m_stages.append(s);
	rebuildStackButtonState();
	applyNow();
}

QWidget *DecoderEditor::buildOptionEditor(const scopy::decoder::OptionInfo &o)
{
	switch(o.type) {
	case scopy::decoder::OptionType::Enum: {
		auto *c = new QComboBox(this);
		for(const QString &v : o.choices) c->addItem(v);
		if(!o.defaultValue.isEmpty()) {
			const int idx = c->findText(o.defaultValue);
			if(idx >= 0) c->setCurrentIndex(idx);
		}
		connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged),
		        this, &DecoderEditor::applyNow);
		return c;
	}
	case scopy::decoder::OptionType::Int: {
		// Numeric enums (e.g. cpol 0/1): use combo; else spinbox.
		if(!o.choices.isEmpty()) {
			auto *c = new QComboBox(this);
			for(const QString &v : o.choices) c->addItem(v);
			if(!o.defaultValue.isEmpty()) {
				const int idx = c->findText(o.defaultValue);
				if(idx >= 0) c->setCurrentIndex(idx);
			}
			connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged),
			        this, &DecoderEditor::applyNow);
			return c;
		}
		auto *s = new QSpinBox(this);
		s->setRange(-1000000000, 1000000000);
		s->setValue(o.defaultValue.toInt());
		connect(s, QOverload<int>::of(&QSpinBox::valueChanged),
		        this, &DecoderEditor::applyNow);
		return s;
	}
	case scopy::decoder::OptionType::Double: {
		if(!o.choices.isEmpty()) {
			auto *c = new QComboBox(this);
			for(const QString &v : o.choices) c->addItem(v);
			if(!o.defaultValue.isEmpty()) {
				const int idx = c->findText(o.defaultValue);
				if(idx >= 0) c->setCurrentIndex(idx);
			}
			connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged),
			        this, &DecoderEditor::applyNow);
			return c;
		}
		auto *s = new QDoubleSpinBox(this);
		s->setRange(-1.0e12, 1.0e12);
		s->setDecimals(3);
		s->setValue(o.defaultValue.toDouble());
		connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		        this, &DecoderEditor::applyNow);
		return s;
	}
	case scopy::decoder::OptionType::String:
	default: {
		auto *l = new QLineEdit(this);
		l->setText(o.defaultValue);
		connect(l, &QLineEdit::textChanged, this, &DecoderEditor::applyNow);
		return l;
	}
	}
}

QWidget *DecoderEditor::buildInputModeGroup(QWidget *parent)
{
	auto *box  = new QGroupBox(QStringLiteral("Input"), parent);
	auto *lay  = new QVBoxLayout(box);
	lay->setSpacing(4);

	m_radioRaw = new QRadioButton(QStringLiteral("Raw digital channels"), box);
	m_radioAnn = new QRadioButton(QStringLiteral("Annotations from another decoder"), box);
	m_radioRaw->setChecked(true);

	m_inputModeGroup = new QButtonGroup(box);
	m_inputModeGroup->setExclusive(true);
	m_inputModeGroup->addButton(m_radioRaw, 0);
	m_inputModeGroup->addButton(m_radioAnn, 1);

	lay->addWidget(m_radioRaw);
	lay->addWidget(m_radioAnn);

	// Populated from the manager's topology, not DataStore keys.
	auto *row = new QHBoxLayout();
	m_annSourceLabel = new QLabel(QStringLiteral("Source annotation stream:"), box);
	m_annSourceCombo = new QComboBox(box);
	m_annSourceCombo->addItem(CH_UNASSIGNED, QString());
	row->addWidget(m_annSourceLabel);
	row->addWidget(m_annSourceCombo, 1);
	lay->addLayout(row);

	connect(m_radioRaw, &QRadioButton::toggled, this, [this](bool) {
		applyInputModeVisibility();
		applyNow();
	});
	connect(m_radioAnn, &QRadioButton::toggled, this, [this](bool) {
		applyInputModeVisibility();
		applyNow();
	});
	connect(m_annSourceCombo,
	        QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &DecoderEditor::applyNow);

	// No channel roles: lock to annotation-input mode.
	if(!m_stages.isEmpty() && m_stages[0].info.channels.isEmpty()) {
		m_radioAnn->setChecked(true);
		m_radioRaw->setEnabled(false);
	}

	return box;
}

bool DecoderEditor::isAnnotationMode() const
{
	return m_radioAnn && m_radioAnn->isChecked();
}

void DecoderEditor::applyInputModeVisibility()
{
	const bool ann = isAnnotationMode();
	if(!m_stages.isEmpty() && m_stages[0].channelsBox)
		m_stages[0].channelsBox->setVisible(!ann);
	if(m_annSourceLabel) m_annSourceLabel->setVisible(ann);
	if(m_annSourceCombo) m_annSourceCombo->setVisible(ann);
	if(m_annInBox)       m_annInBox->setVisible(ann);
}

QWidget *DecoderEditor::buildAnnotationInputGroup(QWidget *parent)
{
	auto *box = new QGroupBox(QStringLiteral("Annotation input"), parent);
	auto *form = new QFormLayout(box);
	form->setLabelAlignment(Qt::AlignLeft);

	m_annInSampleRate = new QDoubleSpinBox(box);
	m_annInSampleRate->setRange(1.0, 1.0e12);
	m_annInSampleRate->setDecimals(0);
	m_annInSampleRate->setSingleStep(1.0e5);
	m_annInSampleRate->setValue(1.0e7);
	m_annInSampleRate->setToolTip(QStringLiteral(
		"Synthetic samplerate used to regenerate the upstream "
		"waveform for sigrok-cli's -I protocoldata."));
	form->addRow(QStringLiteral("Samplerate (Hz):"), m_annInSampleRate);

	m_annInBitrate = new QDoubleSpinBox(box);
	m_annInBitrate->setRange(1.0, 1.0e9);
	m_annInBitrate->setDecimals(0);
	m_annInBitrate->setSingleStep(1200);
	m_annInBitrate->setValue(115200);
	m_annInBitrate->setToolTip(QStringLiteral(
		"UART baud / SPI clock rate for the regenerated waveform."));
	form->addRow(QStringLiteral("Bitrate:"), m_annInBitrate);

	m_annInFrameFormat = new QLineEdit(box);
	m_annInFrameFormat->setText(QStringLiteral("8n1"));
	m_annInFrameFormat->setToolTip(QStringLiteral(
		"UART frame format (data bits + parity + stop bits), "
		"e.g. \"8n1\", \"7e1\"."));
	form->addRow(QStringLiteral("Frame format:"), m_annInFrameFormat);

	m_annInRadix = new QComboBox(box);
	m_annInRadix->addItem(QStringLiteral("hex"));
	m_annInRadix->addItem(QStringLiteral("dec"));
	m_annInRadix->addItem(QStringLiteral("bin"));
	m_annInRadix->addItem(QStringLiteral("ascii"));
	m_annInRadix->setToolTip(QStringLiteral(
		"How to parse the upstream annotation text into bytes. "
		"Match the upstream decoder's 'format' option."));
	form->addRow(QStringLiteral("Radix:"), m_annInRadix);

	connect(m_annInSampleRate,
	        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &DecoderEditor::applyNow);
	connect(m_annInBitrate,
	        QOverload<double>::of(&QDoubleSpinBox::valueChanged),
	        this, &DecoderEditor::applyNow);
	connect(m_annInFrameFormat, &QLineEdit::textChanged,
	        this, &DecoderEditor::applyNow);
	connect(m_annInRadix, QOverload<int>::of(&QComboBox::currentIndexChanged),
	        this, &DecoderEditor::applyNow);

	return box;
}

void DecoderEditor::refreshKeys(const QList<scopy::acq::DataKey> &keys)
{
	if(m_stages.isEmpty()) return;

	// Raw digital keys only; annotation sources come from a separate combo.
	QList<scopy::acq::DataKey> raw;
	raw.reserve(keys.size());
	for(const auto &k : keys)
		if(k.isRaw()) raw.append(k);

	for(QComboBox *c : m_stages[0].channelCombos) {
		const QString prev = c->currentData().toString();
		QSignalBlocker b(c);
		c->clear();
		c->addItem(CH_UNASSIGNED, QString());
		for(const auto &k : raw) c->addItem(k.key, k.key);

		int idx = 0;
		if(!prev.isEmpty()) {
			const int found = c->findData(prev);
			if(found >= 0) idx = found;
		}
		c->setCurrentIndex(idx);
	}
}

void DecoderEditor::refreshAnnotationSources()
{
	if(!m_annSourceCombo || !m_mgr) return;

	const QString prev = m_annSourceCombo->currentData().toString();
	QSignalBlocker b(m_annSourceCombo);
	m_annSourceCombo->clear();
	m_annSourceCombo->addItem(CH_UNASSIGNED, QString());

	// Every stage of every other decoder is a potential source.
	for(const DecoderInstance &d : m_mgr->decoders()) {
		if(d.uid == m_uid) continue;
		for(int stage = 0; stage < d.outKeys.size(); ++stage) {
			const scopy::acq::DataKey &k = d.outKeys[stage];
			const QString upstreamId = (stage < d.stageIds.size())
				? d.stageIds[stage] : QString();
			const QString label = QStringLiteral("%1 · stage %2 (%3)")
				.arg(d.uid).arg(stage).arg(upstreamId);
			m_annSourceCombo->addItem(label, k.key);
		}
	}

	int idx = 0;
	if(!prev.isEmpty()) {
		const int found = m_annSourceCombo->findData(prev);
		if(found >= 0) idx = found;
	}
	m_annSourceCombo->setCurrentIndex(idx);
}

void DecoderEditor::collect(scopy::decoder::DecoderConfig &cfg,
                            QList<scopy::acq::DataKey> &orderedRawKeys) const
{
	cfg.sampleRate = m_sampleRateSpin ? m_sampleRateSpin->value() : 1.0e6;
	cfg.stack.clear();
	cfg.meta.clear();
	orderedRawKeys.clear();
	cfg.rootInput = scopy::decoder::RootInput::DigitalSamples;
	cfg.annotationInput = scopy::decoder::AnnotationInputRef{};

	const bool annMode = isAnnotationMode();
	QString annKey;
	if(annMode && m_annSourceCombo)
		annKey = m_annSourceCombo->currentData().toString();

	if(annMode) {
		cfg.rootInput = scopy::decoder::RootInput::Annotations;
		// channelId() == "<uid>/<stage>"
		scopy::acq::DataKey dk(annKey);
		const QString cid = dk.channelId();
		const int slash = cid.lastIndexOf(QChar('/'));
		QString srcUid;
		int srcStage = 0;
		if(slash > 0) {
			srcUid   = cid.left(slash);
			srcStage = cid.mid(slash + 1).toInt();
		} else {
			srcUid = cid;
		}
		cfg.annotationInput.sourceUid        = srcUid.toStdString();
		cfg.annotationInput.sourceStageIndex = srcStage;

		// Fill annIn.* meta; upstreamId comes from the manager.
		if(m_mgr) {
			DecoderInstance *src = m_mgr->find(srcUid);
			if(src && srcStage >= 0 && srcStage < src->stageIds.size()) {
				cfg.meta["annIn.upstreamId"] =
					src->stageIds[srcStage].toStdString();
			}
		}
		if(m_annInSampleRate)
			cfg.meta["annIn.samplerate"] =
				QString::number(m_annInSampleRate->value(), 'f', 0).toStdString();
		if(m_annInBitrate)
			cfg.meta["annIn.bitrate"] =
				QString::number(m_annInBitrate->value(), 'f', 0).toStdString();
		if(m_annInFrameFormat && !m_annInFrameFormat->text().isEmpty())
			cfg.meta["annIn.frameformat"] =
				m_annInFrameFormat->text().toStdString();
		if(m_annInRadix)
			cfg.meta["annIn.radix"] =
				m_annInRadix->currentText().toStdString();
	}

	for(int si = 0; si < m_stages.size(); ++si) {
		const Stage &st = m_stages[si];
		scopy::decoder::DecoderStage stage;
		stage.decoderId = st.info.id.toStdString();

		if(si == 0 && !annMode) {
			for(int i = 0; i < st.info.channels.size()
			            && i < st.channelCombos.size(); ++i) {
				QComboBox *c = st.channelCombos[i];
				const QString key = c->currentData().toString();
				if(key.isEmpty()) continue;
				scopy::decoder::ChannelMap m;
				m.role     = st.info.channels[i].id.toStdString();
				m.bitIndex = orderedRawKeys.size();
				stage.channels.push_back(m);
				orderedRawKeys.append(scopy::acq::DataKey(key));
			}
		}

		for(const auto &o : st.info.options) {
			QWidget *w = st.optionWidgets.value(o.id, nullptr);
			if(!w) continue;
			QString value;
			if(auto *c = qobject_cast<QComboBox *>(w))          value = c->currentText();
			else if(auto *s = qobject_cast<QSpinBox *>(w))      value = QString::number(s->value());
			else if(auto *d = qobject_cast<QDoubleSpinBox *>(w))value = QString::number(d->value());
			else if(auto *l = qobject_cast<QLineEdit *>(w))     value = l->text();
			if(value.isEmpty()) continue;
			stage.options[o.id.toStdString()] = value.toStdString();
		}

		cfg.stack.push_back(std::move(stage));
	}
	cfg.numChannels = annMode ? 0 : orderedRawKeys.size();
}

void DecoderEditor::applyNow()
{
	if(!m_mgr) return;
	// applyConfig requires a stopped engine; edits while running are dropped.
	if(m_mgr->isEngineRunning()) return;

	scopy::decoder::DecoderConfig cfg;
	QList<scopy::acq::DataKey> keys;
	collect(cfg, keys);
	m_mgr->applyConfig(m_uid, cfg, keys);
}

// -----------------------------------------------------------------------------
// DecoderPanel
// -----------------------------------------------------------------------------

DecoderPanel::DecoderPanel(DecoderManager *mgr,
                           scopy::acq::DataStore *store,
                           scopy::decoder::IDecoderCatalog *catalog,
                           QWidget *parent)
	: QWidget(parent)
	, m_mgr(mgr)
	, m_store(store)
	, m_catalog(catalog)
{
	auto *outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->setSpacing(0);

	auto *scroll = new QScrollArea(this);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	outer->addWidget(scroll);

	auto *content = new QWidget(scroll);
	auto *contentLay = new QVBoxLayout(content);
	contentLay->setContentsMargins(8, 8, 8, 8);
	contentLay->setSpacing(8);

	auto *headerRow = new QHBoxLayout();
	auto *title     = new QLabel("<b>Protocol decoders</b>", content);
	auto *addBtn    = new QPushButton("New decoder", content);
	scopy::Style::setStyle(addBtn, style::properties::button::basicButton);
	headerRow->addWidget(title);
	headerRow->addStretch();
	headerRow->addWidget(addBtn);
	contentLay->addLayout(headerRow);

	m_editorsLay = new QVBoxLayout();
	m_editorsLay->setSpacing(8);
	contentLay->addLayout(m_editorsLay);
	contentLay->addStretch();

	scroll->setWidget(content);

	connect(addBtn, &QPushButton::clicked, this, &DecoderPanel::onAddClicked);

	if(m_mgr) {
		connect(m_mgr, &DecoderManager::decoderAdded,
		        this, [this](const QString &) { refreshAnnotationSources(); });
		connect(m_mgr, &DecoderManager::decoderRemoved,
		        this, [this](const QString &) { refreshAnnotationSources(); });
	}

	if(!m_catalog || !m_catalog->isAvailable()) {
		auto *hint = new QLabel(
			"No decoder backend is available. Check that the "
			"required tool/library is installed and configured.",
			content);
		hint->setWordWrap(true);
		hint->setStyleSheet("color:#c88");
		contentLay->insertWidget(1, hint);
		addBtn->setEnabled(false);
	} else {
		// Prime the catalog cache off the main thread.
		auto *cat = m_catalog;
		(void)QtConcurrent::run([cat]() { cat->loadAll(); });
	}
}

void DecoderPanel::refreshKeys(const QList<scopy::acq::DataKey> &keys)
{
	m_keysCache = keys;
	for(DecoderEditor *e : m_editors)
		e->refreshKeys(keys);
}

void DecoderPanel::refreshAnnotationSources()
{
	for(DecoderEditor *e : m_editors)
		e->refreshAnnotationSources();
}

void DecoderPanel::openPicker(const QString &title,
                              std::function<bool(const QString &)> filter,
                              std::function<void(const QString &)> onAccept)
{
	if(!m_catalog) {
		if(m_logger)
			m_logger->critical(kPanelId, QStringLiteral("openPicker: no catalog injected"));
		return;
	}
	const QList<QString> ids = m_catalog->decoders();
	if(ids.isEmpty()) {
		if(m_logger)
			m_logger->warning(kPanelId, QStringLiteral("no decoders returned by catalog"));
		return;
	}

	if(m_pickerWidget) {
		m_pickerWidget->deleteLater();
		m_pickerWidget = nullptr;
	}

	auto *box  = new QGroupBox(title, this);
	auto *lay  = new QVBoxLayout(box);
	lay->setSpacing(6);

	auto *filterEdit = new QLineEdit(box);
	filterEdit->setPlaceholderText("Filter…");
	lay->addWidget(filterEdit);

	auto *list = new QListWidget(box);
	list->setSortingEnabled(false);
	list->setMinimumHeight(200);
	int shown = 0;
	for(const QString &id : ids) {
		if(filter && !filter(id)) continue;
		auto *it = new QListWidgetItem(
			QString("%1  —  %2").arg(id, m_catalog->shortDescription(id)),
			list);
		it->setData(Qt::UserRole, id);
		++shown;
	}
	if(shown == 0) {
		auto *placeholder = new QLabel(
			QStringLiteral("No compatible decoder available."), box);
		placeholder->setStyleSheet("color:#c88");
		lay->addWidget(placeholder);
	}
	lay->addWidget(list, 1);

	auto *btnRow    = new QHBoxLayout();
	auto *addBtn    = new QPushButton("Add", box);
	auto *cancelBtn = new QPushButton("Cancel", box);
	scopy::Style::setStyle(addBtn,    style::properties::button::basicButton);
	scopy::Style::setStyle(cancelBtn, style::properties::button::basicButton);
	btnRow->addStretch();
	btnRow->addWidget(addBtn);
	btnRow->addWidget(cancelBtn);
	lay->addLayout(btnRow);

	connect(filterEdit, &QLineEdit::textChanged, list, [list](const QString &q) {
		for(int i = 0; i < list->count(); ++i) {
			auto *it = list->item(i);
			it->setHidden(!q.isEmpty()
				      && !it->text().contains(q, Qt::CaseInsensitive));
		}
	});

	auto accept = [this, list, onAccept]() {
		auto *sel = list->currentItem();
		if(!sel) return;
		const QString id = sel->data(Qt::UserRole).toString();
		if(id.isEmpty()) return;
		if(m_pickerWidget) {
			m_pickerWidget->deleteLater();
			m_pickerWidget = nullptr;
		}
		if(onAccept) onAccept(id);
	};

	connect(addBtn, &QPushButton::clicked, this, accept);
	connect(list,  &QListWidget::itemDoubleClicked, this,
		[accept](QListWidgetItem *) { accept(); });
	connect(cancelBtn, &QPushButton::clicked, this, [this]() {
		if(m_pickerWidget) {
			m_pickerWidget->deleteLater();
			m_pickerWidget = nullptr;
		}
	});

	m_pickerWidget = box;
	m_editorsLay->addWidget(box);
	filterEdit->setFocus();
}

void DecoderPanel::onAddClicked()
{
	// Toggle: clicking Add while picker is open closes it.
	if(m_pickerWidget) {
		m_pickerWidget->deleteLater();
		m_pickerWidget = nullptr;
		return;
	}
	openPicker(QStringLiteral("Add protocol decoder"),
	           /*filter*/ nullptr,
	           [this](const QString &id) {
		const QString uid = m_mgr->addDecoder(id);
		if(uid.isEmpty()) {
			if(m_logger)
				m_logger->critical(kPanelId,
					QStringLiteral("DecoderManager::addDecoder failed"));
			return;
		}
		appendEditorFor(uid, id);
	});
}

void DecoderPanel::onStackPickerRequested(DecoderEditor *editor,
                                          const QStringList &acceptedInputIds)
{
	if(!editor || !m_catalog) return;
	if(!m_mgr) return;
	if(m_mgr->isEngineRunning()) {
		if(m_logger)
			m_logger->warning(kPanelId,
				QStringLiteral("cannot stack while engine is running"));
		return;
	}
	if(m_pickerWidget) {
		m_pickerWidget->deleteLater();
		m_pickerWidget = nullptr;
		return;
	}

	const QSet<QString> accepted(acceptedInputIds.begin(),
	                             acceptedInputIds.end());
	auto *cat = m_catalog;
	auto filter = [cat, accepted](const QString &id) {
		if(accepted.isEmpty()) return false;
		const auto info = cat->info(id);
		for(const QString &in : info.inputIds)
			if(accepted.contains(in)) return true;
		return false;
	};

	QPointer<DecoderEditor> edRef(editor);
	openPicker(QStringLiteral("Stack decoder on %1")
			.arg(editor->uid()),
	           filter,
	           [this, edRef](const QString &id) {
		if(!edRef) return;
		const int stageIndex = m_mgr->pushStage(edRef->uid(), id);
		if(stageIndex < 0) {
			if(m_logger)
				m_logger->warning(kPanelId,
					QStringLiteral("pushStage failed"));
			return;
		}
		if(!m_catalog) return;
		edRef->appendStage(m_catalog->info(id));
	});
}

void DecoderPanel::appendEditorFor(const QString &uid, const QString &decoderId)
{
	if(!m_catalog) return;
	const auto info = m_catalog->info(decoderId);
	auto *ed = new DecoderEditor(uid, info, m_mgr, m_store.data(), this);
	ed->refreshKeys(m_keysCache);
	ed->refreshAnnotationSources();
	m_editorsLay->addWidget(ed);
	m_editors.append(ed);

	connect(ed, &DecoderEditor::removeRequested,
	        this, &DecoderPanel::onEditorRemoveRequested);
	connect(ed, &DecoderEditor::stackPickerRequested,
	        this, &DecoderPanel::onStackPickerRequested);
}

void DecoderPanel::onEditorRemoveRequested(const QString &uid)
{
	for(int i = 0; i < m_editors.size(); ++i) {
		if(m_editors[i]->uid() != uid) continue;
		DecoderEditor *ed = m_editors.takeAt(i);
		m_editorsLay->removeWidget(ed);
		ed->deleteLater();
		break;
	}
	if(m_mgr) m_mgr->removeDecoder(uid);
}

} // namespace adc
} // namespace scopy
