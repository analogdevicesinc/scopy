#include "DecoderPanel.h"

#include "DecoderManager.h"

#include <core/acq_engine/DataStore.h>
#include <core/decoder/DecoderLogger.h>

#include <gui/style.h>
#include <gui/style_attributes.h>

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QScrollArea>
#include <QSpinBox>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

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
	, m_info(info)
	, m_mgr(mgr)
	, m_store(store)
{
	auto *box = new QGroupBox(QString("%1 [%2]").arg(info.name, uid), this);
	auto *outer = new QVBoxLayout(this);
	outer->setContentsMargins(0, 0, 0, 0);
	outer->addWidget(box);

	auto *lay = new QVBoxLayout(box);
	lay->setSpacing(6);

	// Status row: right-aligned dot + text indicating whether the current
	// widget values are running, not yet applied, or modified since Apply.
	{
		auto *row = new QHBoxLayout();
		row->addStretch();
		m_statusDot = new QLabel(box);
		const int sz = scopy::Style::getDimension(json::global::unit_0_5);
		m_statusDot->setFixedSize(sz, sz);
		row->addWidget(m_statusDot, 0, Qt::AlignVCenter);
		m_statusText = new QLabel(box);
		row->addWidget(m_statusText, 0, Qt::AlignVCenter);
		lay->addLayout(row);
	}

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
			// QToolTip wraps at ~80 columns when the string contains no
			// HTML tags; wrap the docs in <p> and pre-escape so newlines
			// become paragraph breaks rather than a single blob.
			QString html = info.documentation.toHtmlEscaped();
			html.replace(QStringLiteral("\n\n"), QStringLiteral("</p><p>"));
			html.replace(QChar('\n'), QChar(' '));
			infoBtn->setToolTip(QStringLiteral("<p>%1</p>").arg(html));
			row->addWidget(infoBtn, 0, Qt::AlignTop);
		}

		lay->addLayout(row);
	}

	// Sample rate
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

	// Channels
	if(!m_info.channels.isEmpty())
		lay->addWidget(buildChannelsGroup(box));

	// Options
	if(!m_info.options.isEmpty())
		lay->addWidget(buildOptionsGroup(box));

	// Apply / Remove buttons
	{
		auto *row = new QHBoxLayout();
		m_applyBtn  = new QPushButton("Apply", box);
		m_removeBtn = new QPushButton("Remove", box);
		row->addWidget(m_applyBtn);
		row->addWidget(m_removeBtn);
		row->addStretch();
		lay->addLayout(row);
	}

	connect(m_applyBtn, &QPushButton::clicked, this, &DecoderEditor::onApplyClicked);
	connect(m_removeBtn, &QPushButton::clicked, this, [this]() {
		Q_EMIT removeRequested(m_uid);
	});

	// Authoritative apply signal: also fires for programmatic applies.
	if(m_mgr) {
		connect(m_mgr, &DecoderManager::configApplied,
		        this, [this](const QString &uid) {
			if(uid == m_uid) setState(EditorState::Running);
		});
	}

	// Initial visual state.
	setState(EditorState::NotApplied);
}

QWidget *DecoderEditor::buildChannelsGroup(QWidget *parent)
{
	auto *box  = new QGroupBox("Channels", parent);
	auto *form = new QFormLayout(box);
	form->setLabelAlignment(Qt::AlignLeft);

	for(const auto &ch : m_info.channels) {
		auto *combo = new QComboBox(box);
		combo->addItem(CH_UNASSIGNED, QString());
		// Raw keys populated later by refreshRawKeys().
		QString label = ch.name;
		if(ch.required) label += " *";
		if(!ch.desc.isEmpty()) combo->setToolTip(ch.desc);
		form->addRow(label + ":", combo);
		m_channelCombos.append(combo);
		connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		        this, &DecoderEditor::markDirty);
	}
	return box;
}

QWidget *DecoderEditor::buildOptionsGroup(QWidget *parent)
{
	auto *box  = new QGroupBox("Options", parent);
	auto *form = new QFormLayout(box);
	form->setLabelAlignment(Qt::AlignLeft);

	for(const auto &o : m_info.options) {
		QWidget *w = buildOptionEditor(o);
		if(!w) continue;
		m_optionWidgets.insert(o.id, w);
		form->addRow(o.name + ":", w);
	}
	return box;
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
	for(QComboBox *c : m_channelCombos) {
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
	cfg.decoderId  = m_info.id.toStdString();
	cfg.sampleRate = m_sampleRateSpin ? m_sampleRateSpin->value() : 1.0e6;
	cfg.channels.clear();
	cfg.options.clear();
	cfg.meta.clear();
	orderedRawKeys.clear();

	// Channels: append role→bitIndex for every assigned combo; bitIndex is
	// the position in orderedRawKeys.
	for(int i = 0; i < m_info.channels.size() && i < m_channelCombos.size(); ++i) {
		QComboBox *c = m_channelCombos[i];
		const QString key = c->currentData().toString();
		if(key.isEmpty()) continue; // unassigned
		scopy::decoder::ChannelMap m;
		m.role     = m_info.channels[i].id.toStdString();
		m.bitIndex = orderedRawKeys.size();
		cfg.channels.push_back(m);
		orderedRawKeys.append(scopy::acq::DataKey(key));
	}
	cfg.numChannels = orderedRawKeys.size();

	// Options
	for(const auto &o : m_info.options) {
		QWidget *w = m_optionWidgets.value(o.id, nullptr);
		if(!w) continue;
		QString value;
		if(auto *c = qobject_cast<QComboBox *>(w))          value = c->currentText();
		else if(auto *s = qobject_cast<QSpinBox *>(w))      value = QString::number(s->value());
		else if(auto *d = qobject_cast<QDoubleSpinBox *>(w))value = QString::number(d->value());
		else if(auto *l = qobject_cast<QLineEdit *>(w))     value = l->text();
		if(value.isEmpty()) continue;
		cfg.options[o.id.toStdString()] = value.toStdString();
	}
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
	auto *addBtn    = new QPushButton("+ Add decoder…", content);
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

void DecoderPanel::onAddClicked()
{
	if(!m_catalog) {
		if(m_logger)
			m_logger->critical(kPanelId, QStringLiteral("onAddClicked: no catalog injected"));
		return;
	}
	const QList<QString> ids = m_catalog->decoders();
	if(ids.isEmpty()) {
		if(m_logger)
			m_logger->warning(kPanelId, QStringLiteral("no decoders returned by catalog"));
		return;
	}

	// Toggle: clicking Add while a picker is open closes it.
	if(m_pickerWidget) {
		m_pickerWidget->deleteLater();
		m_pickerWidget = nullptr;
		return;
	}

	auto *box  = new QGroupBox("Add protocol decoder", this);
	auto *lay  = new QVBoxLayout(box);
	lay->setSpacing(6);

	auto *filter = new QLineEdit(box);
	filter->setPlaceholderText("Filter…");
	lay->addWidget(filter);

	auto *list = new QListWidget(box);
	list->setSortingEnabled(false);
	list->setMinimumHeight(200);
	for(const QString &id : ids) {
		auto *it = new QListWidgetItem(
			QString("%1  —  %2").arg(id, m_catalog->shortDescription(id)),
			list);
		it->setData(Qt::UserRole, id);
	}
	lay->addWidget(list, 1);

	auto *btnRow    = new QHBoxLayout();
	auto *addBtn    = new QPushButton("Add", box);
	auto *cancelBtn = new QPushButton("Cancel", box);
	btnRow->addStretch();
	btnRow->addWidget(addBtn);
	btnRow->addWidget(cancelBtn);
	lay->addLayout(btnRow);

	connect(filter, &QLineEdit::textChanged, list, [list](const QString &q) {
		for(int i = 0; i < list->count(); ++i) {
			auto *it = list->item(i);
			it->setHidden(!q.isEmpty()
				      && !it->text().contains(q, Qt::CaseInsensitive));
		}
	});

	auto accept = [this, list]() {
		auto *sel = list->currentItem();
		if(!sel) return;
		const QString id = sel->data(Qt::UserRole).toString();
		if(id.isEmpty()) return;
		if(m_pickerWidget) {
			m_pickerWidget->deleteLater();
			m_pickerWidget = nullptr;
		}
		const QString uid = m_mgr->addDecoder(id);
		if(uid.isEmpty()) {
			if(m_logger)
				m_logger->critical(kPanelId, QStringLiteral("DecoderManager::addDecoder failed"));
			return;
		}
		appendEditorFor(uid, id);
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
	filter->setFocus();
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
