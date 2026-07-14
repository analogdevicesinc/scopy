#include "DecoderPanel.h"

#include "DecoderManager.h"

#include <core/acq_engine/DataStore.h>
#include <core/decoder/DecoderLogger.h>

#include <gui/style.h>
#include <gui/style_attributes.h>
#include <gui/style_properties.h>

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QSet>
#include <QSpinBox>
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
		        this, &DecoderEditor::markDirty);
	}

	// Per-stage sub-groups go here; new stages append at the bottom.
	m_stagesLay = new QVBoxLayout();
	m_stagesLay->setSpacing(6);
	lay->addLayout(m_stagesLay);

	// Root stage (index 0) — the only one with channel bindings.
	Stage root;
	QWidget *rootBox = buildStageWidget(0, info, root);
	m_stagesLay->addWidget(rootBox);
	m_stages.append(root);

	// "+ Stack…" button: disabled if the top stage produces nothing that
	// could feed another decoder.
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
		m_applyBtn  = new QPushButton("Apply", box);
		m_removeBtn = new QPushButton("Remove", box);
		scopy::Style::setStyle(m_applyBtn,  style::properties::button::borderButton);
		scopy::Style::setStyle(m_removeBtn, style::properties::button::borderButton);
		row->addWidget(m_applyBtn);
		row->addWidget(m_removeBtn);
		row->addStretch();
		m_statusDot = new QLabel(box);
		const int sz = scopy::Style::getDimension(json::global::unit_0_5);
		m_statusDot->setFixedSize(sz, sz);
		row->addWidget(m_statusDot, 0, Qt::AlignVCenter | Qt::AlignRight);
		m_statusText = new QLabel(box);
		row->addWidget(m_statusText, 0, Qt::AlignVCenter | Qt::AlignRight);
		lay->addLayout(row);
	}

	connect(m_applyBtn, &QPushButton::clicked, this, &DecoderEditor::onApplyClicked);
	connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
		Q_EMIT removeRequested(m_uid);
	});

	if(m_mgr) {
		connect(m_mgr, &DecoderManager::configApplied,
		        this, [this](const QString &uid) {
			if(uid == m_uid) setState(EditorState::Running);
		});
	}

	setState(EditorState::NotApplied);
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

	// Only the root stage binds raw channels; stacked stages consume the
	// previous stage's product.
	if(stageIndex == 0 && !info.channels.isEmpty())
		lay->addWidget(buildChannelsGroup(box, out));

	if(!info.options.isEmpty())
		lay->addWidget(buildOptionsGroup(box, out));

	// Stack removal button on stacked stages: drops this stage and any
	// above it (they'd be orphaned).
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
			// Rebuild UI: drop stage widgets from stageIndex onward.
			while(m_stages.size() > stageIndex) {
				Stage &s = m_stages.last();
				if(s.box) s.box->deleteLater();
				m_stages.removeLast();
			}
			rebuildStackButtonState();
			markDirty();
		});
	}

	return box;
}

QWidget *DecoderEditor::buildChannelsGroup(QWidget *parent, Stage &st)
{
	auto *box  = new QGroupBox("Channels", parent);
	auto *form = new QFormLayout(box);
	form->setLabelAlignment(Qt::AlignLeft);

	for(const auto &ch : st.info.channels) {
		auto *combo = new QComboBox(box);
		combo->addItem(CH_UNASSIGNED, QString());
		QString label = ch.name;
		if(ch.required) label += " *";
		if(!ch.desc.isEmpty()) combo->setToolTip(ch.desc);
		form->addRow(label + ":", combo);
		st.channelCombos.append(combo);
		connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		        this, &DecoderEditor::markDirty);
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
	markDirty();
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
		        this, &DecoderEditor::markDirty);
		return c;
	}
	case scopy::decoder::OptionType::Int: {
		// Numeric enums (e.g. cpol 0/1) are stored with choices; a combo
		// is still the correct editor because the decoder rejects
		// out-of-list values. Fall back to spinbox when we have no
		// enumerated choices.
		if(!o.choices.isEmpty()) {
			auto *c = new QComboBox(this);
			for(const QString &v : o.choices) c->addItem(v);
			if(!o.defaultValue.isEmpty()) {
				const int idx = c->findText(o.defaultValue);
				if(idx >= 0) c->setCurrentIndex(idx);
			}
			connect(c, QOverload<int>::of(&QComboBox::currentIndexChanged),
			        this, &DecoderEditor::markDirty);
			return c;
		}
		auto *s = new QSpinBox(this);
		s->setRange(-1000000000, 1000000000);
		s->setValue(o.defaultValue.toInt());
		connect(s, QOverload<int>::of(&QSpinBox::valueChanged),
		        this, &DecoderEditor::markDirty);
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
			        this, &DecoderEditor::markDirty);
			return c;
		}
		auto *s = new QDoubleSpinBox(this);
		s->setRange(-1.0e12, 1.0e12);
		s->setDecimals(3);
		s->setValue(o.defaultValue.toDouble());
		connect(s, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		        this, &DecoderEditor::markDirty);
		return s;
	}
	case scopy::decoder::OptionType::String:
	default: {
		auto *l = new QLineEdit(this);
		l->setText(o.defaultValue);
		connect(l, &QLineEdit::textChanged, this, &DecoderEditor::markDirty);
		return l;
	}
	}
}

void DecoderEditor::refreshRawKeys(const QList<scopy::acq::DataKey> &rawKeys)
{
	if(m_stages.isEmpty()) return;
	// Only the root stage binds raw channels.
	for(QComboBox *c : m_stages[0].channelCombos) {
		const QString prev = c->currentText();
		QSignalBlocker b(c);
		c->clear();
		c->addItem(CH_UNASSIGNED, QString());
		for(const auto &k : rawKeys) c->addItem(k.key, k.key);
		const int idx = c->findText(prev);
		c->setCurrentIndex(idx >= 0 ? idx : 0);
	}
}

void DecoderEditor::setApplyEnabled(bool en)
{
	if(m_applyBtn) m_applyBtn->setEnabled(en);
}

void DecoderEditor::collect(scopy::decoder::DecoderConfig &cfg,
                            QList<scopy::acq::DataKey> &orderedRawKeys) const
{
	cfg.sampleRate = m_sampleRateSpin ? m_sampleRateSpin->value() : 1.0e6;
	cfg.stack.clear();
	cfg.meta.clear();
	orderedRawKeys.clear();

	for(int si = 0; si < m_stages.size(); ++si) {
		const Stage &st = m_stages[si];
		scopy::decoder::DecoderStage stage;
		stage.decoderId = st.info.id.toStdString();

		if(si == 0) {
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
	cfg.numChannels = orderedRawKeys.size();
}

void DecoderEditor::onApplyClicked()
{
	if(!m_mgr) return;
	if(m_mgr->isEngineRunning()) {
		return;
	}
	scopy::decoder::DecoderConfig cfg;
	QList<scopy::acq::DataKey> keys;
	collect(cfg, keys);
	// State transition to Running happens via the DecoderManager::configApplied
	// signal wired in the constructor — that also covers programmatic applies.
	m_mgr->applyConfig(m_uid, cfg, keys);
}

void DecoderEditor::markDirty()
{
	// Only transition from Running → Modified. Stay in NotApplied until the
	// user actually clicks Apply once.
	if(m_state == EditorState::Running) setState(EditorState::Modified);
}

void DecoderEditor::setState(EditorState s)
{
	m_state = s;

	QString color;
	QString text;
	QString tip;
	switch(s) {
	case EditorState::NotApplied:
		color = scopy::Style::getColor(json::theme::content_silent).name();
		text  = QStringLiteral("Not applied");
		tip   = QStringLiteral("Configure channels/options and click Apply to start decoding.");
		break;
	case EditorState::Running:
		color = scopy::Style::getColor(json::global::led_success).name();
		text  = QStringLiteral("Running");
		tip   = QStringLiteral("Decoder is running with the currently displayed settings.");
		break;
	case EditorState::Modified:
		color = scopy::Style::getColor(json::global::led_error).name();
		text  = QStringLiteral("Modified");
		tip   = QStringLiteral("Settings changed since last Apply. Click Apply to update the running decoder.");
		break;
	}

	if(m_statusDot) {
		const int sz = m_statusDot->width();
		m_statusDot->setStyleSheet(QStringLiteral(
			"background-color:%1; border-radius:%2px;")
			.arg(color).arg(sz / 2));
		m_statusDot->setToolTip(tip);
	}
	if(m_statusText) {
		m_statusText->setText(text);
		m_statusText->setStyleSheet(QStringLiteral("color:%1;").arg(color));
		m_statusText->setToolTip(tip);
	}
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

	// Header with the Add button.
	auto *headerRow = new QHBoxLayout();
	auto *title     = new QLabel("<b>Protocol decoders</b>", content);
	auto *addBtn    = new QPushButton("Add decoder", content);
	scopy::Style::setStyle(addBtn, style::properties::button::basicButton);
	headerRow->addWidget(title);
	headerRow->addStretch();
	headerRow->addWidget(addBtn);
	contentLay->addLayout(headerRow);

	// Editors go below.
	m_editorsLay = new QVBoxLayout();
	m_editorsLay->setSpacing(8);
	contentLay->addLayout(m_editorsLay);
	contentLay->addStretch();

	scroll->setWidget(content);

	connect(addBtn, &QPushButton::clicked, this, &DecoderPanel::onAddClicked);

	// Show a hint if the injected catalog is missing or unavailable at
	// construction time. Concrete "why" messaging (e.g. "sigrok-cli not
	// found") belongs to the composition root — the panel stays neutral.
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
		// Prime the catalog cache off the main thread so opening the
		// stack picker (which calls catalog->info() per candidate to
		// filter by inputIds) doesn't stall on ~100 sigrok-cli spawns.
		auto *cat = m_catalog;
		(void)QtConcurrent::run([cat]() { cat->loadAll(); });
	}
}

void DecoderPanel::refreshRawKeys(const QList<scopy::acq::DataKey> &keys)
{
	// Filter to raw keys only — decoders can only consume raw digital data.
	QList<scopy::acq::DataKey> raw;
	raw.reserve(keys.size());
	for(const auto &k : keys)
		if(k.isRaw()) raw.append(k);
	m_rawKeysCache = raw;
	for(DecoderEditor *e : m_editors)
		e->refreshRawKeys(raw);
}

void DecoderPanel::setEngineRunning(bool running)
{
	m_engineRunning = running;
	for(DecoderEditor *e : m_editors)
		e->setApplyEnabled(!running);
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
	// Toggle: clicking Add while a picker is open closes it.
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
	ed->refreshRawKeys(m_rawKeysCache);
	ed->setApplyEnabled(!m_engineRunning);
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
