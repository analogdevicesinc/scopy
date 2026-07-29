#include "TriggerProcessorWidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace scopy {
namespace acq {

TriggerProcessorWidget::TriggerProcessorWidget(TriggerProcessor *proc, QWidget *parent)
	: QWidget(parent)
	, m_proc(proc)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(8, 8, 8, 8);
	lay->setSpacing(6);

	m_statusLabel = new QLabel(this);
	m_statusLabel->setAlignment(Qt::AlignCenter);
	lay->addWidget(m_statusLabel);
	if(m_proc->isEnabled())
		setStatusWaiting();
	else
		setStatusIdle();
	connect(m_proc, &ProcessorBlock::enabledChanged, this, [this](bool en) {
		// Enable transition resets the run-scoped fire flag: the next
		// stop event decides whether we ever saw a fire in this arm.
		m_firedSinceArm = false;
		if(en)
			setStatusWaiting();
		else
			setStatusIdle();
	});
	// fired() only records that we saw a fire during this run — the label
	// is not touched. setAcquisitionRunning(false) is what promotes to
	// "triggered" on stop.
	connect(m_proc, &TriggerProcessor::fired, this,
		[this](quint32){ m_firedSinceArm = true; }, Qt::QueuedConnection);
	// skipped() carries no label meaning under the new rules.

	auto *ruleRow = new QHBoxLayout();
	ruleRow->addWidget(new QLabel("Combine:", this));
	m_opCombo = new QComboBox(this);
	m_opCombo->addItem("AND", static_cast<int>(CombineRule::Op::AND));
	m_opCombo->addItem("OR",  static_cast<int>(CombineRule::Op::OR));
	m_opCombo->setCurrentIndex(m_proc->combineRule().op == CombineRule::Op::AND ? 0 : 1);
	ruleRow->addWidget(m_opCombo);

	ruleRow->addWidget(new QLabel("Window:", this));
	m_windowSpin = new QSpinBox(this);
	m_windowSpin->setRange(1, 1'000'000);
	m_windowSpin->setValue(static_cast<int>(m_proc->combineRule().coincidenceWindow));
	ruleRow->addWidget(m_windowSpin);
	ruleRow->addStretch();
	lay->addLayout(ruleRow);

	auto pushRule = [this]() {
		CombineRule r;
		r.op = static_cast<CombineRule::Op>(m_opCombo->currentData().toInt());
		r.coincidenceWindow = static_cast<quint32>(m_windowSpin->value());
		m_proc->setCombineRule(r);
	};
	connect(m_opCombo,    QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, [pushRule](int){ pushRule(); });
	connect(m_windowSpin, QOverload<int>::of(&QSpinBox::valueChanged),
		this, [pushRule](int){ pushRule(); });

	auto *specRow = new QHBoxLayout();
	m_sampleSpecificCb = new QCheckBox("Sample-specific", this);
	m_sampleSpecificCb->setChecked(m_proc->sampleSpecific());
	specRow->addWidget(m_sampleSpecificCb);
	specRow->addWidget(new QLabel("Sample:", this));
	m_sampleSpin = new QSpinBox(this);
	m_sampleSpin->setRange(0, 1'000'000);
	m_sampleSpin->setValue(static_cast<int>(m_proc->targetSample()));
	m_sampleSpin->setEnabled(m_proc->sampleSpecific());
	m_windowSpin->setEnabled(!m_proc->sampleSpecific());
	specRow->addWidget(m_sampleSpin);
	specRow->addWidget(new QLabel("± samples:", this));
	m_toleranceSpin = new QSpinBox(this);
	m_toleranceSpin->setRange(0, 1'000'000);
	m_toleranceSpin->setValue(static_cast<int>(m_proc->sampleTolerance()));
	m_toleranceSpin->setEnabled(m_proc->sampleSpecific());
	specRow->addWidget(m_toleranceSpin);
	specRow->addStretch();
	lay->addLayout(specRow);

	connect(m_sampleSpecificCb, &QCheckBox::toggled, this, [this](bool on) {
		m_proc->setSampleSpecific(on);
		m_sampleSpin->setEnabled(on);
		if(m_toleranceSpin) m_toleranceSpin->setEnabled(on);
		m_windowSpin->setEnabled(!on);
	});
	connect(m_sampleSpin, QOverload<int>::of(&QSpinBox::valueChanged),
		this, [this](int v) { m_proc->setTargetSample(static_cast<quint32>(v)); });
	connect(m_toleranceSpin, QOverload<int>::of(&QSpinBox::valueChanged),
		this, [this](int v) { m_proc->setSampleTolerance(static_cast<quint32>(v)); });
	connect(m_proc, &TriggerProcessor::targetSampleChanged, this, [this](quint32 s) {
		if(!m_sampleSpin) return;
		QSignalBlocker b(m_sampleSpin);
		m_sampleSpin->setValue(static_cast<int>(s));
	});
	connect(m_proc, &TriggerProcessor::sampleToleranceChanged, this, [this](quint32 t) {
		if(!m_toleranceSpin) return;
		QSignalBlocker b(m_toleranceSpin);
		m_toleranceSpin->setValue(static_cast<int>(t));
	});
	connect(m_proc, &TriggerProcessor::sampleSpecificChanged, this, [this](bool on) {
		if(m_sampleSpecificCb) {
			QSignalBlocker b(m_sampleSpecificCb);
			m_sampleSpecificCb->setChecked(on);
		}
		if(m_sampleSpin)     m_sampleSpin->setEnabled(on);
		if(m_toleranceSpin)  m_toleranceSpin->setEnabled(on);
		if(m_windowSpin)     m_windowSpin->setEnabled(!on);
	});

	auto *addBtn = new QPushButton("+ Add condition", this);
	lay->addWidget(addBtn);
	connect(addBtn, &QPushButton::clicked, this, [this]() {
		TriggerCondition c;
		c.label = QString("cond %1").arg(m_proc->conditions().size() + 1);
		m_proc->addCondition(c);
	});

	auto *rowsHost = new QWidget(this);
	m_rowsLay = new QVBoxLayout(rowsHost);
	m_rowsLay->setContentsMargins(0, 0, 0, 0);
	m_rowsLay->setSpacing(4);
	lay->addWidget(rowsHost);

	lay->addStretch();

	connect(m_proc, &TriggerProcessor::conditionsChanged,
		this, &TriggerProcessorWidget::rebuildRows);
	rebuildRows();
}

void TriggerProcessorWidget::rebuildRows()
{
	// Wipe existing rows.
	QLayoutItem *item;
	while((item = m_rowsLay->takeAt(0)) != nullptr) {
		if(QWidget *w = item->widget())
			w->deleteLater();
		delete item;
	}
	m_keyCombos.clear();

	const QList<TriggerCondition> conds = m_proc->conditions();
	for(int i = 0; i < conds.size(); ++i) {
		const TriggerCondition &c = conds[i];
		auto *frame = new QFrame(this);
		frame->setFrameShape(QFrame::StyledPanel);
		auto *form = new QFormLayout(frame);
		form->setContentsMargins(4, 4, 4, 4);
		form->setSpacing(2);

		// Row header: enable + remove.
		auto *headerRow = new QWidget(frame);
		auto *headerLay = new QHBoxLayout(headerRow);
		headerLay->setContentsMargins(0, 0, 0, 0);
		auto *enCb = new QCheckBox(c.label.isEmpty() ? QString("#%1").arg(i) : c.label, headerRow);
		enCb->setChecked(c.enabled);
		headerLay->addWidget(enCb);
		headerLay->addStretch();
		auto *rmBtn = new QPushButton("Remove", headerRow);
		headerLay->addWidget(rmBtn);
		form->addRow(headerRow);

		auto *keyCombo = new QComboBox(frame);
		keyCombo->setEditable(false);
		// Populate from the current key list. If the condition's seed key
		// isn't in the list, prepend it as a stale entry so the selection
		// round-trips without silently mutating.
		for(const QString &k : m_availableKeys)
			keyCombo->addItem(k);
		if(!c.key.key.isEmpty()) {
			if(keyCombo->findText(c.key.key) < 0)
				keyCombo->insertItem(0, c.key.key);
			keyCombo->setCurrentIndex(keyCombo->findText(c.key.key));
		}
		m_keyCombos.append(keyCombo);
		form->addRow("Key:", keyCombo);

		auto *edgeCombo = new QComboBox(frame);
		edgeCombo->addItem("Rising",  static_cast<int>(TriggerCondition::Edge::Rising));
		edgeCombo->addItem("Falling", static_cast<int>(TriggerCondition::Edge::Falling));
		edgeCombo->addItem("Either",  static_cast<int>(TriggerCondition::Edge::Either));
		edgeCombo->setCurrentIndex(static_cast<int>(c.edge));
		form->addRow("Edge:", edgeCombo);

		auto push = [this, i, keyCombo, edgeCombo, enCb]() {
			TriggerCondition nc;
			nc.key     = DataKey(keyCombo->currentText().trimmed());
			nc.edge    = static_cast<TriggerCondition::Edge>(edgeCombo->currentData().toInt());
			nc.enabled = enCb->isChecked();
			nc.label   = enCb->text();
			m_proc->setCondition(i, nc);
		};

		connect(edgeCombo,  QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, [push](int){ push(); });
		connect(keyCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, [push](int){ push(); });
		connect(enCb,       &QCheckBox::toggled, this, [this, i](bool en) {
			m_proc->setConditionEnabled(i, en);
		});
		connect(rmBtn, &QPushButton::clicked, this, [this, i]() {
			m_proc->removeCondition(i);
		});

		m_rowsLay->addWidget(frame);
	}
}

void TriggerProcessorWidget::setAvailableKeys(const QStringList &keys)
{
	m_availableKeys = keys;
	// Refresh live combos in place — preserving current selection — instead
	// of rebuilding rows (which would drop focus / break editing UX).
	const QList<TriggerCondition> conds = m_proc->conditions();
	for(int i = 0; i < m_keyCombos.size() && i < conds.size(); ++i) {
		QComboBox *cb = m_keyCombos[i];
		if(!cb)
			continue;
		const QString prev = cb->currentText();
		QSignalBlocker b(cb);
		cb->clear();
		for(const QString &k : m_availableKeys)
			cb->addItem(k);
		if(!prev.isEmpty()) {
			if(cb->findText(prev) < 0)
				cb->insertItem(0, prev);
			cb->setCurrentIndex(cb->findText(prev));
		}
	}
}

void TriggerProcessorWidget::setMaxTargetSample(int maxIdx)
{
	if(!m_sampleSpin || maxIdx < 0)
		return;
	// Preserve current value; QSpinBox already clamps on setRange().
	m_sampleSpin->setRange(0, maxIdx);
}

void TriggerProcessorWidget::setAcquisitionRunning(bool running)
{
	m_acqRunning = running;
	if(running) {
		// Arm: fresh run, no fires yet. Label reflects enable state.
		m_firedSinceArm = false;
		if(m_proc && m_proc->isEnabled())
			setStatusWaiting();
		else
			setStatusIdle();
	} else {
		// Stop: promote to "triggered" iff a fire happened during this
		// run and the processor is still enabled. Otherwise revert to
		// the appropriate idle/waiting baseline.
		if(m_proc && m_proc->isEnabled()) {
			if(m_firedSinceArm)
				setStatusTriggered();
			else
				setStatusWaiting();
		} else {
			setStatusIdle();
		}
	}
}

void TriggerProcessorWidget::setStatusIdle()
{
	m_statusLabel->setText(QStringLiteral("idle"));
	m_statusLabel->setStyleSheet(QStringLiteral(
		"QLabel { background:#333; color:#aaa; padding:2px; border-radius:2px; }"));
}

void TriggerProcessorWidget::setStatusWaiting()
{
	m_statusLabel->setText(QStringLiteral("waiting"));
	m_statusLabel->setStyleSheet(QStringLiteral(
		"QLabel { background:#5a4a10; color:#ffd94a; padding:2px; border-radius:2px; }"));
}

void TriggerProcessorWidget::setStatusTriggered()
{
	m_statusLabel->setText(QStringLiteral("triggered"));
	m_statusLabel->setStyleSheet(QStringLiteral(
		"QLabel { background:#1e5a1e; color:#a8e6a1; padding:2px; border-radius:2px; }"));
}

} // namespace acq
} // namespace scopy
