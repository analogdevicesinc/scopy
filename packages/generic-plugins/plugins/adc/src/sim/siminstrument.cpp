#include "siminstrument.h"

#include "SourceBlock.h"

#include <QCheckBox>
#include <QDateTime>
#include <QFontDatabase>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QVBoxLayout>

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

	m_cursorBtn = new QPushButton("Cur", this);
	m_cursorBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_cursorBtn, TTA_RIGHT);

	m_logBtn = new QPushButton("Log", this);
	m_logBtn->setCheckable(true);
	m_tool->addWidgetToTopContainerHelper(m_logBtn, TTA_RIGHT);

	// ---- central: plot ----
	m_plot = new PlotWidget(this);
	m_tool->addWidgetToCentralContainerHelper(m_plot);

	// ---- right panel: acquisition log ----
	m_tool->rightContainer()->setVisible(true);
	m_tool->setRightContainerWidth(240);

	m_logView = new QTextEdit(this);
	m_logView->setReadOnly(true);
	m_logView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	m_logView->setLineWrapMode(QTextEdit::NoWrap);
	m_logView->setPlaceholderText("No errors or warnings.");
	m_tool->rightStack()->add("log-view", m_logView);

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
}

void SimInstrument::wirePanelButton(QPushButton *btn, const QString &menuId,
				    const QList<QPushButton *> &allBtns)
{
	connect(btn, &QPushButton::toggled, this, [this, btn, menuId, allBtns](bool on) {
		m_tool->openRightContainerHelper(on);
		if(on) {
			for(QPushButton *other : allBtns) {
				if(other != btn) {
					QSignalBlocker b(other);
					other->setChecked(false);
				}
			}
			m_tool->requestMenu(menuId);
		}
	});
}

void SimInstrument::buildControlPanel(sim::AcquisitionEngine *engine,
				      const QList<CurveDescriptor> &curves)
{
	// Collect all panel buttons for mutual exclusion wiring
	QList<QPushButton *> panelBtns = {m_settingsBtn, m_cursorBtn, m_logBtn};

	// ---- Settings panel (scrollable, single right-side panel) ----
	auto *settingsInner = new QWidget();
	auto *settingsLay   = new QVBoxLayout(settingsInner);
	settingsLay->setContentsMargins(8, 8, 8, 8);
	settingsLay->setSpacing(8);

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

	settingsLay->addWidget(engineGroup);

	// Wire engine spinboxes
	connect(fpsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &SimInstrument::maxFpsChanged);
	connect(bufSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
		this, &SimInstrument::sampleSizeChanged);
	connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &SimInstrument::acqModeChanged);

	// -- Source groups --
	for(sim::SourceBlock *src : engine->sources()) {
		const bool hasChannels = !src->channelIds().isEmpty();
		QWidget   *srcWidget   = src->createSettingsWidget(nullptr);
		if(!hasChannels && !srcWidget)
			continue;

		auto *srcGroup = new QGroupBox(QString("Source: %1").arg(src->id()), settingsInner);
		auto *srcLay   = new QVBoxLayout(srcGroup);
		srcLay->setSpacing(4);

		for(const QString &chId : src->channelIds()) {
			auto *cb = new QCheckBox(chId, srcGroup);
			cb->setChecked(src->isChannelEnabled(chId));
			connect(cb, &QCheckBox::toggled, this, [src, chId](bool en) {
				src->enableChannel(chId, en);
			});
			srcLay->addWidget(cb);
		}

		if(srcWidget) {
			srcWidget->setParent(srcGroup);
			srcLay->addWidget(srcWidget);
		}

		settingsLay->addWidget(srcGroup);
	}

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

		// Processor settings — one sub-group per processor
		for(sim::ProcessorBlock *proc : desc.processors) {
			QWidget *procWidget = proc->createSettingsWidget(curveGroup);
			if(!procWidget)
				continue;

			auto *procGroup = new QGroupBox(proc->name(), curveGroup);
			auto *procLay   = new QVBoxLayout(procGroup);
			procLay->setContentsMargins(4, 4, 4, 4);
			procWidget->setParent(procGroup);
			procLay->addWidget(procWidget);
			curveLay->addWidget(procGroup);
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
	wirePanelButton(m_settingsBtn, "settings-panel", panelBtns);
	wirePanelButton(m_cursorBtn,   "cursor-config",  panelBtns);
	wirePanelButton(m_logBtn,      "log-view",       panelBtns);
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

void SimInstrument::updateCurveKeyCombos(const QList<sim::DataKey> &keys)
{
	for(int i = 0; i < m_curveSelectors.size(); ++i) {
		CurveSelectors &cs = m_curveSelectors[i];

		// X combo
		{
			const QString prev = cs.xKey->currentText();
			QSignalBlocker b(cs.xKey);
			cs.xKey->clear();
			cs.xKey->addItem(SAMPLE_INDEX_ENTRY);
			for(const sim::DataKey &k : keys)
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
			for(const sim::DataKey &k : keys)
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
	QSignalBlocker singleBlocker(m_singleBtn);
	m_singleBtn->setChecked(false);
	m_singleBtn->setEnabled(true);
}

void SimInstrument::appendLog(int severity, const QString &id, const QString &message)
{
	const QString ts  = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
	const auto    sev = static_cast<sim::AcquisitionError::Severity>(severity);

	QString color, tag;
	if(sev == sim::AcquisitionError::Severity::Critical) {
		color = "#ff4444";
		tag   = "CRIT";
	} else if(sev == sim::AcquisitionError::Severity::Warning) {
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

} // namespace adc
} // namespace scopy
