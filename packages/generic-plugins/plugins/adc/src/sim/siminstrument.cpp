#include "siminstrument.h"

#include <core/acq_engine/SourceBlock.h>

#include <gui/style.h>

#include <QDateTime>
#include <QFontDatabase>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHeaderView>

#include <type_traits>
#include <variant>

namespace scopy {
namespace adc {

static const QString SAMPLE_INDEX_ENTRY = "Sample Index";

SimInstrument::SimInstrument(QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(0);

	m_tool = new ToolTemplate(this);
	lay->addWidget(m_tool);

	setupUi();
}

SimInstrument::~SimInstrument() {}

void SimInstrument::setupUi()
{
	m_tool->topContainer()->setVisible(true);

	// ---- top-bar: run / single (left) ----
	m_runBtn = new RunBtn(this);
	m_runBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_LEFT);

	m_singleBtn = new SingleShotBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_LEFT);

	// ---- top-bar: panel toggle buttons (right) ----
	m_settingsBtn = new GearBtn(this);
	m_settingsBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_settingsBtn, TTA_RIGHT);

	m_cursorBtn = new QPushButton("Cursors", this);
	m_cursorBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_cursorBtn, TTA_RIGHT);

	m_logBtn = new QPushButton("Logs", this);
	m_logBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_logBtn, TTA_RIGHT);

	m_datastoreBtn = new QPushButton("DataStore", this);
	m_datastoreBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_datastoreBtn, TTA_RIGHT);

	m_decoderBtn = new QPushButton("Decoders", this);
	m_decoderBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_decoderBtn, TTA_RIGHT);

	m_decoderLogBtn = new QPushButton("Decoder Logs", this);
	m_decoderLogBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_decoderLogBtn, TTA_RIGHT);

	m_pipelineBtn = new QPushButton("Pipeline", this);
	m_pipelineBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_pipelineBtn, TTA_RIGHT);

	// ---- central: oscilloscope + waterfall in a vertical splitter ----
	m_plot = new PlotWidget(this);
	m_waterfall = new WaterfallPlotWidget(this);

	auto *splitter = new QSplitter(Qt::Vertical, this);
	splitter->addWidget(m_plot);
	splitter->addWidget(m_waterfall);
	splitter->setStretchFactor(0, 2);
	splitter->setStretchFactor(1, 1);
	splitter->setSizes({400, 200});
	m_tool->addWidgetToCentralContainerHelper(splitter);

	// ---- right panel: acquisition log ----
	m_tool->rightContainer()->setVisible(true);
	m_tool->setRightContainerWidth(500);

	m_logView = new QTextEdit(this);
	m_logView->setReadOnly(true);
	m_logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_logView->setLineWrapMode(QTextEdit::NoWrap);
	m_logView->setPlaceholderText("No errors or warnings.");
	m_tool->rightStack()->add("log-view", m_logView);

	// ---- right panel: decoder log ----
	m_decoderLogView = new QTextEdit(this);
	m_decoderLogView->setReadOnly(true);
	m_decoderLogView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_decoderLogView->setLineWrapMode(QTextEdit::NoWrap);
	m_decoderLogView->setPlaceholderText("No decoder messages.");
	m_tool->rightStack()->add("decoder-log-view", m_decoderLogView);

	// ---- right panel: DataStore inspector ----
	// One top-level row per key; expanding it lists that stream's newest
	// samples. The tree's own expander is the "drop down" — no extra widget per
	// row, and the collapsed state is what keeps the per-cycle refresh cheap
	// (child rows are only built for rows the user actually opened).
	auto *dsPage = new QWidget(this);
	auto *dsLay  = new QVBoxLayout(dsPage);
	dsLay->setContentsMargins(0, 0, 0, 0);
	dsLay->setSpacing(4);

	auto *dsCtl = new QHBoxLayout();
	dsCtl->setContentsMargins(4, 4, 4, 0);
	dsCtl->addWidget(new QLabel("Show samples:", dsPage));
	m_datastoreSampleCount = new QSpinBox(dsPage);
	m_datastoreSampleCount->setRange(0, 4096);
	m_datastoreSampleCount->setValue(32);
	m_datastoreSampleCount->setToolTip(
		"Newest samples listed under an expanded key. 0 lists none.\n"
		"Rows refresh every acquisition cycle, so keep this small.\n"
		"Samples are indexed from 0, oldest-first within the shown window.\n"
		"Annotation streams list their sample range instead.");
	dsCtl->addWidget(m_datastoreSampleCount);
	m_datastoreHex = new QCheckBox("hex", dsPage);
	m_datastoreHex->setToolTip("Show integer samples in hex. Floats are unaffected.");
	dsCtl->addWidget(m_datastoreHex);
	dsCtl->addStretch();
	dsLay->addLayout(dsCtl);

	m_datastoreTable = new QTreeWidget(dsPage);
	m_datastoreTable->setColumnCount(4);
	// Child rows reuse the first two columns for index and value, hence the
	// double labels.
	m_datastoreTable->setHeaderLabels(
		{"Key / Sample", "Type / Value", "Samples", "History (used/cap)"});
	m_datastoreTable->setAlternatingRowColors(true);
	m_datastoreTable->header()->setStretchLastSection(true);
	m_datastoreTable->setUniformRowHeights(true);
	// Sample values are columns of numbers; a proportional font makes them
	// impossible to scan.
	m_datastoreTable->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	dsLay->addWidget(m_datastoreTable);
	m_tool->rightStack()->add("datastore-view", dsPage);

	// ---- signal wiring: run / single ----
	connect(m_runBtn, &QPushButton::toggled, this, [this](bool checked) {
		if(checked)
			Q_EMIT requestRun();
		else
			Q_EMIT requestStop();
	});

	connect(m_singleBtn, &QPushButton::toggled, this, [this](bool checked) {
		if(checked)
			Q_EMIT requestSingle();
	});

	// ---- signal wiring: DataStore inspector ----
	// Fill on expand rather than waiting for the next cycle: while the engine
	// is stopped there is no next cycle, and a row that opens empty reads as a
	// bug in the store.
	connect(m_datastoreTable, &QTreeWidget::itemExpanded, this,
		[this](QTreeWidgetItem *item) {
			if(!item || item->parent() || !m_datastoreRef)
				return;
			fillDatastoreSamples(item, m_datastoreRef,
					     scopy::acq::DataKey(item->text(0)));
		});
	// Collapsing drops the child rows so they cost nothing until reopened.
	connect(m_datastoreTable, &QTreeWidget::itemCollapsed, this,
		[](QTreeWidgetItem *item) {
			if(item && !item->parent())
				qDeleteAll(item->takeChildren());
		});

	auto refillExpanded = [this]() {
		if(!m_datastoreRef || !m_datastoreTable)
			return;
		for(int i = 0; i < m_datastoreTable->topLevelItemCount(); ++i) {
			QTreeWidgetItem *item = m_datastoreTable->topLevelItem(i);
			if(item->isExpanded())
				fillDatastoreSamples(item, m_datastoreRef,
						     scopy::acq::DataKey(item->text(0)));
		}
	};
	connect(m_datastoreSampleCount, &QSpinBox::valueChanged, this, refillExpanded);
	connect(m_datastoreHex, &QCheckBox::toggled, this, refillExpanded);
}

void SimInstrument::wirePanelButton(QPushButton *btn, const QString &menuId)
{
	connect(btn, &QPushButton::toggled, this, [this, btn, menuId](bool on) {
		m_tool->openRightContainerHelper(on);
		if(on) {
			for(QPushButton *other : std::as_const(m_panelBtns)) {
				if(other != btn && other) {
					QSignalBlocker b(other);
					other->setChecked(false);
				}
			}
			m_tool->requestMenu(menuId);
		}
	});
}

void SimInstrument::buildControlPanel(scopy::acq::AcquisitionEngine *engine,
				      const QList<CurveDescriptor> &curves)
{
	// Collect all panel buttons for mutual exclusion wiring. Stored as a
	// member so registerDecoderPanel() can append the Decoders button
	// after buildControlPanel() runs.
	m_panelBtns = {m_settingsBtn, m_cursorBtn, m_logBtn, m_decoderLogBtn, m_datastoreBtn};

	// ---- Settings panel (scrollable, single right-side panel) ----
	auto *settingsInner = new QWidget();
	auto *settingsLay   = new QVBoxLayout(settingsInner);
	settingsLay->setContentsMargins(8, 8, 8, 8);
	settingsLay->setSpacing(8);
	m_settingsInner    = settingsInner;
	m_settingsInnerLay = settingsLay;

	// -- Engine group --
	auto *engineGroup = new QGroupBox("Engine", settingsInner);
	auto *engineLay   = new QVBoxLayout(engineGroup);
	engineLay->setSpacing(4);

	// Acquisition mode
	engineLay->addWidget(new QLabel("Acq Mode:"));
	m_modeCombo = new QComboBox(engineGroup);
	m_modeCombo->addItem("Continuous");
	m_modeCombo->addItem("Triggered");
	m_modeCombo->setCurrentIndex(1); // default: Triggered
	engineLay->addWidget(m_modeCombo);

	// Max FPS
	engineLay->addWidget(new QLabel("Max FPS:"));
	auto *fpsSpinBox = new QSpinBox(engineGroup);
	fpsSpinBox->setRange(0, 144);
	fpsSpinBox->setValue(static_cast<int>(engine->maxFPS()));
	engineLay->addWidget(fpsSpinBox);

	// Buffer size
	engineLay->addWidget(new QLabel("Buffer size:"));
	auto *bufSpinBox = new QSpinBox(engineGroup);
	bufSpinBox->setRange(1, 1000000);
	bufSpinBox->setValue(static_cast<int>(engine->bufferSize()));
	m_sampleSizeWidget = bufSpinBox;
	engineLay->addWidget(bufSpinBox);

	// Plot size
	engineLay->addWidget(new QLabel("Plot size:"));
	auto *plotSpinBox = new QSpinBox(engineGroup);
	plotSpinBox->setRange(1, 10000000);
	plotSpinBox->setValue(static_cast<int>(engine->bufferSize()));
	m_plotSizeWidget = plotSpinBox;
	engineLay->addWidget(plotSpinBox);

	settingsLay->addWidget(engineGroup);

	// Wire engine spinboxes
	connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &SimInstrument::maxFpsChanged);
	connect(bufSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &SimInstrument::sampleSizeChanged);
	connect(plotSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &SimInstrument::plotSizeChanged);
	connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &SimInstrument::acqModeChanged);

	// Per-block settings (sources, processors, trigger, decoders) are not
	// duplicated here — the Pipeline panel hosts each block's single widget,
	// reached by selecting its row. This panel keeps what isn't block-owned:
	// engine parameters and the per-curve axis bindings.

	// -- Per-curve groups --
	m_curveSelectors.clear();
	for(const CurveDescriptor &desc : curves) {
		auto *curveGroup = new QGroupBox(desc.name, settingsInner);
		auto *curveLay   = new QVBoxLayout(curveGroup);
		curveLay->setSpacing(4);

		// Color indicator
		auto *colorLabel = new QLabel(curveGroup);
		colorLabel->setFixedSize(16, 16);
		colorLabel->setStyleSheet(
			QString("background-color: %1; border: 1px solid #888;").arg(desc.color.name()));

		auto *headerRow    = new QWidget(curveGroup);
		auto *headerLayout = new QHBoxLayout(headerRow);
		headerLayout->setContentsMargins(0, 0, 0, 0);
		headerLayout->addWidget(colorLabel);
		headerLayout->addStretch();
		curveLay->addWidget(headerRow);

		// X key selector
		curveLay->addWidget(new QLabel("X Axis:"));
		auto *xCombo = new QComboBox(curveGroup);
		xCombo->addItem(SAMPLE_INDEX_ENTRY);
		curveLay->addWidget(xCombo);

		// Y key selector
		curveLay->addWidget(new QLabel("Y Axis:"));
		auto *yCombo = new QComboBox(curveGroup);
		yCombo->addItem(SAMPLE_INDEX_ENTRY);
		curveLay->addWidget(yCombo);

		// Waterfall-specific: history rows spinbox
		if(desc.name == "Waterfall") {
			curveLay->addWidget(new QLabel("History rows:"));
			auto *wfRowsSpin = new QSpinBox(curveGroup);
			wfRowsSpin->setRange(10, 2000);
			wfRowsSpin->setSingleStep(10);
			wfRowsSpin->setValue(200);
			curveLay->addWidget(wfRowsSpin);
			connect(wfRowsSpin, QOverload<int>::of(&QSpinBox::valueChanged),
				this, &SimInstrument::waterfallRowsChanged);
		}

		// The processors feeding this curve are listed by name only; their
		// controls live on their own row in the Pipeline panel.
		if(!desc.processors.isEmpty()) {
			QStringList names;
			names.reserve(desc.processors.size());
			for(scopy::acq::ProcessorBlock *proc : desc.processors)
				names << proc->name();
			auto *chain = new QLabel(QString("Processors: %1").arg(names.join(" → ")),
						 curveGroup);
			chain->setWordWrap(true);
			Style::setStyle(chain, style::properties::label::subtle);
			curveLay->addWidget(chain);
		}

		settingsLay->addWidget(curveGroup);

		CurveSelectors cs;
		cs.xKey = xCombo;
		cs.yKey = yCombo;
		m_curveSelectors.append(cs);
	}

	settingsLay->addStretch();

	// Wrap in scroll area
	auto *settingsScroll = new QScrollArea(this);
	settingsScroll->setWidget(settingsInner);
	settingsScroll->setWidgetResizable(true);
	settingsScroll->setFrameShape(QFrame::NoFrame);
	m_tool->rightStack()->add("settings-panel", settingsScroll);

	// ---- Wire panel toggle buttons (mutual exclusion) ----
	wirePanelButton(m_settingsBtn,    "settings-panel");
	wirePanelButton(m_cursorBtn,      "cursor-config");
	wirePanelButton(m_logBtn,         "log-view");
	wirePanelButton(m_decoderLogBtn,  "decoder-log-view");
	wirePanelButton(m_datastoreBtn,   "datastore-view");
}

void SimInstrument::registerDecoderPanel(QWidget *panel)
{
	if(!panel) return;
	m_tool->rightStack()->add("decoder-panel", panel);
	m_panelBtns.append(m_decoderBtn);
	wirePanelButton(m_decoderBtn, "decoder-panel");
}

void SimInstrument::registerPipelinePanel(QWidget *panel)
{
	if(!panel) return;
	m_tool->rightStack()->add("pipeline-panel", panel);
	m_panelBtns.append(m_pipelineBtn);
	wirePanelButton(m_pipelineBtn, "pipeline-panel");
}

QString SimInstrument::curveXKey(int i) const
{
	if(i < 0 || i >= m_curveSelectors.size())
		return {};
	const QString t = m_curveSelectors[i].xKey->currentText();
	return (t == SAMPLE_INDEX_ENTRY) ? QString{} : t;
}

QString SimInstrument::curveYKey(int i) const
{
	if(i < 0 || i >= m_curveSelectors.size())
		return {};
	const QString t = m_curveSelectors[i].yKey->currentText();
	return (t == SAMPLE_INDEX_ENTRY) ? QString{} : t;
}

void SimInstrument::updateCurveKeyCombos(const QList<scopy::acq::DataKey> &keys)
{
	for(int i = 0; i < m_curveSelectors.size(); ++i) {
		CurveSelectors &cs = m_curveSelectors[i];

		// X combo
		{
			const QString prev = cs.xKey->currentText();
			QSignalBlocker b(cs.xKey);
			cs.xKey->clear();
			cs.xKey->addItem(SAMPLE_INDEX_ENTRY);
			for(const scopy::acq::DataKey &k : keys)
				cs.xKey->addItem(k.key);
			const int idx = cs.xKey->findText(prev);
			cs.xKey->setCurrentIndex(idx >= 0 ? idx : 0);
		}

		// Y combo — default first curve to first key, second curve to second key
		{
			const QString prev = cs.yKey->currentText();
			QSignalBlocker b(cs.yKey);
			cs.yKey->clear();
			cs.yKey->addItem(SAMPLE_INDEX_ENTRY);
			for(const scopy::acq::DataKey &k : keys)
				cs.yKey->addItem(k.key);
			const int idx      = cs.yKey->findText(prev);
			const int fallback = (i < cs.yKey->count() - 1) ? (i + 1) : 0;
			cs.yKey->setCurrentIndex(idx >= 0 ? idx : fallback);
		}
	}
}

void SimInstrument::onStarted()
{
	QSignalBlocker blocker(m_runBtn);
	m_runBtn->setChecked(true);
	if(m_modeCombo)
		m_modeCombo->setEnabled(false);
	if(m_sampleSizeWidget)
		m_sampleSizeWidget->setEnabled(false);
	if(m_plotSizeWidget)
		m_plotSizeWidget->setEnabled(false);
	m_singleBtn->setEnabled(false);
}

void SimInstrument::onForceStopped()
{
	onStopped();
}

void SimInstrument::onStopped()
{
	QSignalBlocker blocker(m_runBtn);
	m_runBtn->setChecked(false);
	if(m_modeCombo)
		m_modeCombo->setEnabled(true);
	if(m_sampleSizeWidget)
		m_sampleSizeWidget->setEnabled(true);
	if(m_plotSizeWidget)
		m_plotSizeWidget->setEnabled(true);
	QSignalBlocker singleBlocker(m_singleBtn);
	m_singleBtn->setChecked(false);
	m_singleBtn->setEnabled(true);
}

void SimInstrument::appendLog(int severity, const QString &id, const QString &message)
{
	const QString ts  = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
	const auto    sev = static_cast<scopy::acq::AcquisitionError::Severity>(severity);

	QString color, tag;
	if(sev == scopy::acq::AcquisitionError::Severity::Critical) {
		color = "#ff4444";
		tag   = "CRIT";
	} else if(sev == scopy::acq::AcquisitionError::Severity::Warning) {
		color = "#ffaa00";
		tag   = "WARN";
	} else {
		color = "#aaaaaa";
		tag   = "INFO";
	}

	const QString line = QString("<span style=\"color:%1\">[%2] %3 | %4: %5</span>")
				     .arg(color, tag, ts,
					  id.toHtmlEscaped(),
					  message.toHtmlEscaped());

	m_logView->append(line);
	m_logView->verticalScrollBar()->setValue(
		m_logView->verticalScrollBar()->maximum());
}

void SimInstrument::appendDecoderLog(int level, const QString &id, const QString &message)
{
	const QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");

	// Mirrors scopy::decoder::LogLevel: Info=0, Warning=1, Critical=2.
	QString color, tag;
	if(level >= 2) {
		color = "#ff4444";
		tag   = "CRIT";
	} else if(level == 1) {
		color = "#ffaa00";
		tag   = "WARN";
	} else {
		color = "#aaaaaa";
		tag   = "INFO";
	}

	const QString line = QString("<span style=\"color:%1\">[%2] %3 | %4: %5</span>")
				     .arg(color, tag, ts,
					  id.toHtmlEscaped(),
					  message.toHtmlEscaped());

	m_decoderLogView->append(line);
	m_decoderLogView->verticalScrollBar()->setValue(
		m_decoderLogView->verticalScrollBar()->maximum());
}

void SimInstrument::refreshDatastoreView(scopy::acq::DataStore *store)
{
	if(!store || !m_datastoreTable)
		return;

	// Remembered so itemExpanded can fill a row on the spot; while the engine
	// is stopped this function is not called again.
	m_datastoreRef = store;

	const QList<scopy::acq::DataKey> keys = store->keys();

	// Remove rows whose key no longer exists in the store
	for(int i = m_datastoreTable->topLevelItemCount() - 1; i >= 0; --i) {
		const QString rowKey = m_datastoreTable->topLevelItem(i)->text(0);
		bool found = false;
		for(const scopy::acq::DataKey &k : keys) {
			if(k.key == rowKey) { found = true; break; }
		}
		if(!found)
			delete m_datastoreTable->takeTopLevelItem(i);
	}

	// Update or insert a row for each key
	for(const scopy::acq::DataKey &k : keys) {
		const scopy::acq::SampleBuffer buf = store->snapshot(k);
		const auto type = buf.type();

		QTreeWidgetItem *item = nullptr;
		for(int i = 0; i < m_datastoreTable->topLevelItemCount(); ++i) {
			if(m_datastoreTable->topLevelItem(i)->text(0) == k.key) {
				item = m_datastoreTable->topLevelItem(i);
				break;
			}
		}
		if(!item) {
			item = new QTreeWidgetItem(m_datastoreTable);
			item->setText(0, k.key);
		}

		item->setText(1, type ? scopy::acq::sampleTypeName(*type) : QStringLiteral("-"));
		item->setText(2, QString::number(buf.size()));
		item->setText(3, QString("%1/%2").arg(buf.depth()).arg(buf.capacity()));

		// Show the expander before the row has ever been opened, so an
		// unexpanded stream still advertises that it can be drilled into.
		// Hidden when there is nothing to show, so the arrow never opens onto
		// an empty list.
		const bool drillable = !buf.empty() && m_datastoreSampleCount &&
				       m_datastoreSampleCount->value() > 0;
		item->setChildIndicatorPolicy(drillable
			? QTreeWidgetItem::ShowIndicator
			: QTreeWidgetItem::DontShowIndicator);

		if(item->isExpanded())
			fillDatastoreSamples(item, store, k);
	}
}

namespace {

// One sample as text. Integers honour `hex`; floats never do (a hex float is
// not what anyone reading a scope trace wants).
template<class T>
QString formatSample(T v, bool hex)
{
	if constexpr(std::is_floating_point_v<T>) {
		return QString::number(static_cast<double>(v), 'g', 6);
	} else {
		// Cast through the unsigned width so a negative qint8 prints as its
		// two's-complement byte instead of a sign-extended "0xffffff80".
		if(hex)
			return QStringLiteral("0x%1").arg(
				static_cast<quint64>(static_cast<std::make_unsigned_t<T>>(v)),
				sizeof(T) * 2, 16, QLatin1Char('0'));
		return QString::number(v);
	}
}

} // namespace

// Rebuilds `item`'s children from the newest samples of `key`. Reuses existing
// child rows: this runs every cycle for every expanded row, and recreating the
// items instead of retexting them makes the tree flicker and lose selection.
void SimInstrument::fillDatastoreSamples(QTreeWidgetItem *item,
					 scopy::acq::DataStore *store,
					 const scopy::acq::DataKey &key)
{
	if(!item || !store || !m_datastoreSampleCount)
		return;

	const int  want = m_datastoreSampleCount->value();
	const bool hex  = m_datastoreHex && m_datastoreHex->isChecked();
	if(want <= 0) {
		qDeleteAll(item->takeChildren());
		return;
	}

	// window() spans chunks and is right-anchored, so this is the newest `want`
	// samples regardless of how the acquisition happened to be chunked.
	const scopy::acq::SampleVariant v = store->window(key, want);

	// Rows to display: (col0, col1). Numeric streams show an index and a value;
	// annotation streams show a sample range and "klass: text" — the same
	// records, but an index into them means nothing to a reader.
	QList<QPair<QString, QString>> rows;

	std::visit([&](const auto &vec) {
		using Vec = std::decay_t<decltype(vec)>;
		if constexpr(std::is_same_v<Vec, QVector<scopy::acq::Annotation>>) {
			const int first = std::max<int>(0, vec.size() - want);
			for(int i = first; i < vec.size(); ++i) {
				const scopy::acq::Annotation &a = vec.at(i);
				rows.append({QString("%1-%2").arg(a.startSample).arg(a.endSample),
					     a.klass.isEmpty() ? a.text
							       : a.klass + ": " + a.text});
			}
		} else {
			// window() already trimmed to `want` and returns oldest-first,
			// so this is a plain 0-based index into the displayed window.
			for(int i = 0; i < vec.size(); ++i)
				rows.append({QString::number(i), formatSample(vec.at(i), hex)});
		}
	}, v);

	while(item->childCount() > rows.size())
		delete item->takeChild(item->childCount() - 1);
	while(item->childCount() < rows.size())
		new QTreeWidgetItem(item);

	for(int i = 0; i < rows.size(); ++i) {
		QTreeWidgetItem *child = item->child(i);
		child->setText(0, rows.at(i).first);
		child->setText(1, rows.at(i).second);
	}
}

} // namespace adc
} // namespace scopy
