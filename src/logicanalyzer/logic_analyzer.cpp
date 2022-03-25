
/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "logic_analyzer.h"

#include "ui_logic_analyzer.h"
#include "ui_cursors_settings.h"
#include "oscilloscope_plot.hpp"

#include <libsigrokdecode/libsigrokdecode.h>

#include "logicanalyzer/logicdatacurve.h"
#include "logicanalyzer/annotationcurve.h"
#include "logicanalyzer/decoder.h"

#include "gui/basemenu.h"
#include "logicgroupitem.h"
#include "logicanalyzer_api.h"

#include "gui/dynamicWidget.hpp"

#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QDateTime>

#include <QTabWidget>

#include "filter.hpp"

#include <libm2k/m2kexceptions.hpp>
#include "scopyExceptionHandler.h"

#include "gui/osc_export_settings.h"
#include "filemanager.h"
#include "config.h"
#include "state_updater.h"

using namespace adiscope;
using namespace adiscope::logic;

constexpr int MAX_BUFFER_SIZE_ONESHOT = 4 * 1024 * 1024; // 4M
constexpr int MAX_BUFFER_SIZE_STREAM = 1024 * 1024 * 1024; // 1Gb
constexpr int MAX_SR_STREAM = 5e6; // 10M
constexpr int MAX_KERNEL_BUFFERS = 64;
constexpr int DIGITAL_NR_CHANNELS = 16;

/* helper method to sort srd_decoder objects based on ids(name) */
static gint sort_pds(gconstpointer a, gconstpointer b)
{
    const struct srd_decoder *sda, *sdb;

    sda = (const struct srd_decoder *)a;
    sdb = (const struct srd_decoder *)b;
    return strcmp(sda->id, sdb->id);
}

LogicAnalyzer::LogicAnalyzer(struct iio_context *ctx, adiscope::Filter *filt,
			     adiscope::ToolMenuItem *toolMenuItem,
			     QJSEngine *engine, adiscope::ToolLauncher *parent,
			     bool offline_mode_):
	LogicTool(nullptr, toolMenuItem, new LogicAnalyzer_API(this), "Logic Analyzer", parent),
	ui(new Ui::LogicAnalyzer),
	m_plot(this, false, 16, 10, new TimePrefixFormatter, new MetricPrefixFormatter),
	m_bufferPreviewer(new DigitalBufferPreviewer(40, this)),
	m_m2k_context(m2kOpen(ctx, "")),
	m_m2kDigital(m_m2k_context->getDigital()),
	m_sampleRateButton(new ScaleSpinButton({
					{"sps", 1E0},
					{"ksps", 1E+3},
					{"Msps", 1E+6}
					}, tr("Sample Rate"), 1,
					10e7,
					true, false, this, {1, 2, 5})),
	m_bufferSizeButton(new ScaleSpinButton({
					{"samples", 1E0},
					{"k samples", 1E+3},
					{"M samples", 1E+6},
					{"G samples", 1E+9},
					}, tr("Nr of samples"), 1000,
					MAX_BUFFER_SIZE_ONESHOT,
					true, false, this, {1, 2, 5})),
	m_timePositionButton(new PositionSpinButton({
					 {"samples", 1E0},
					 }, tr("Delay"), - (1 << 13),
					 (1 << 13) - 1,
					 true, false, this)),
	m_sampleRate(1000000),
	m_bufferSize(1000),
	m_nbChannels(DIGITAL_NR_CHANNELS),
	m_horizOffset(0.0),
	m_timeTriggerOffset(0.0),
	m_resetHorizAxisOffset(true),
	m_captureThread(nullptr),
	m_stopRequested(false),
	m_acquisitionStarted(false),
	m_plotScrollBar(new QScrollBar(Qt::Vertical, this)),
	m_started(false),
	m_selectedChannel(-1),
	m_wheelEventGuard(nullptr),
	m_decoderMenu(nullptr),
	m_lastCapturedSample(0),
	m_currentGroupMenu(nullptr),
	m_autoMode(false),
	m_timer(new QTimer(this)),
	m_timerTimeout(1000),
	m_exportSettings(nullptr),
	m_saveRestoreSettings(nullptr),
	m_oscPlot(nullptr),
	m_oscChannelSelected(-1),
	m_oscDecoderMenu(nullptr),
	m_currentKernelBuffers(4),
	m_triggerUpdater(new StateUpdater(250, this))
{
	// setup ui
	setupUi();

	// setup signals slots
	connectSignalsAndSlots();

	m_plot.setLeftVertAxesCount(1);

	for (uint8_t i = 0; i < m_nbChannels; ++i) {
		QCheckBox *channelBox = new QCheckBox("DIO " + QString::number(i));

		QHBoxLayout *hBoxLayout = new QHBoxLayout();

		ui->channelEnumeratorLayout->addLayout(hBoxLayout, i % 8, i / 8);

		hBoxLayout->addWidget(channelBox);

		QComboBox *triggerBox = new QComboBox();
		triggerBox->addItem("-");

		hBoxLayout->addWidget(triggerBox);

		channelBox->setChecked(true);

		triggerBox->setStyleSheet("QComboBox { max-width: 60px; } QComboBox QAbstractItemView { min-width: 130px; }");

		triggerBox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
		for (int i = 1; i < ui->triggerComboBox->count(); ++i) {
			triggerBox->addItem(ui->triggerComboBox->itemText(i));
		}

		int condition = static_cast<int>(
					m_m2kDigital->getTrigger()->getDigitalCondition(i));
		triggerBox->setCurrentIndex((condition + 1) % 6);

		connect(triggerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			m_m2kDigital->getTrigger()->setDigitalCondition(i,
					static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>((index + 5) % 6));
		});

		// 1 for each channel
		// m_plot.addGenericPlotCurve()
		LogicDataCurve *curve = new LogicDataCurve(nullptr, i, this);
		curve->setTraceHeight(25);
		m_plot.addDigitalPlotCurve(curve, true);

		// use direct connection we want the processing
		// of the available data to be done in the capture thread
		connect(this, &LogicAnalyzer::dataAvailable, this,
			[=](uint64_t from, uint64_t to){
			if (!m_oscPlot) {
				curve->dataAvailable(from, to);
			}
		}, Qt::DirectConnection);

		m_plotCurves.push_back(curve);

		connect(channelBox, &QCheckBox::toggled, [=](bool toggled){
			m_plot.enableDigitalPlotCurve(i, toggled);
			m_plot.setOffsetWidgetVisible(i, toggled);
			m_plot.positionInGroupChanged(i, 0, 0);
			m_plot.replot();
		});
		channelBox->setChecked(false);
	}

	// Add propper zoomer
	m_plot.addZoomer(0);

	m_plot.setZoomerParams(true, 20);

	m_plot.zoomBaseUpdate();

	connect(&m_plot, &CapturePlot::timeTriggerValueChanged, [=](double value){
		double delay = (value - 1.0 / m_sampleRate * m_bufferSize / 2.0 ) / (1.0 / m_sampleRate);
		onTimeTriggerValueChanged(static_cast<int>(delay));
	});


	m_plot.enableXaxisLabels();

	initBufferScrolling();

	m_plotScrollBar->setRange(0, 140);

	// setup decoders
	setupDecoders();

	// setup trigger
	setupTriggerMenu();

	m_timePositionButton->setStep(1);

	// default: oneshot
	ui->btnStreamOneShot->setChecked(true);

	// default
	m_sampleRateButton->setValue(m_sampleRate);
	m_sampleRateButton->setIntegerDivider(m_sampleRateButton->maxValue());
	m_bufferSizeButton->setValue(m_bufferSize);

	m_triggerUpdater->setOffState(CapturePlot::Stop);
	connect(m_triggerUpdater, &StateUpdater::outputChanged,
		&m_plot, &CapturePlot::setTriggerState);

	readPreferences();

	ui->btnGeneralSettings->setChecked(true);

	api->setObjectName(QString::fromStdString(Filter::tool_name(
							  TOOL_LOGIC_ANALYZER)));
	api->load(*settings);
	api->js_register(engine);

	// Scroll wheel event filter
	m_wheelEventGuard = new MouseWheelWidgetGuard(ui->mainWidget);
	m_wheelEventGuard->installEventRecursively(ui->mainWidget);

	ui->btnHelp->setUrl("https://wiki.analog.com/university/tools/m2k/scopy/logicanalyzer");
}

LogicAnalyzer::~LogicAnalyzer()
{
	if (saveOnExit) {
		api->save(*settings);
	}

	delete api;

	disconnect(prefPanel, &Preferences::notify, this, &LogicAnalyzer::readPreferences);

	if (m_captureThread) {
		m_stopRequested = true;
		m_m2kDigital->cancelAcquisition();
		m_captureThread->join();
		delete m_captureThread;
		m_captureThread = nullptr;
	}

	for (auto &curve : m_plotCurves) {
		m_plot.removeDigitalPlotCurve(curve);
		delete curve;
	}

	if (m_buffer) {
		delete[] m_buffer;
		m_buffer = nullptr;
	}

	delete cr_ui;
	delete ui;
}

void LogicAnalyzer::setData(const uint16_t * const data, int size)
{

	qDebug() << "Set data arrived: ";

	if (m_buffer) {
		delete m_buffer;
		m_buffer = nullptr;
	}

	m_buffer = new uint16_t[size];

	memcpy(m_buffer, data, size * sizeof(uint16_t));
	Q_EMIT dataAvailable(0, size);

//	if (m_oscPlot) {
//		QMetaObject::invokeMethod(this, [=](){
//			m_oscPlot->replot();
//		}, Qt::QueuedConnection);
//	}

}

std::vector<QWidget *> LogicAnalyzer::enableMixedSignalView(CapturePlot *osc, int oscAnalogChannels)
{
	// save the state of the tool
	m_saveRestoreSettings = std::unique_ptr<SaveRestoreToolSettings>(new SaveRestoreToolSettings(this));

	if (m_started) {
		auto btn = dynamic_cast<CustomPushButton *>(run_button);
		if (btn) {
			btn->setChecked(false);
		}
	}

	// disable the menu item for the logic analyzer when mixed signal view is enabled
	toolMenuItem->setDisabled(true);

	m_oscPlot = osc;

	m_oscAnalogChannels = oscAnalogChannels;

	QTabWidget *tabWidget = new QTabWidget();
	tabWidget->setMovable(true);

	QScrollArea *generalScrollArea = new QScrollArea();
	generalScrollArea->setWidgetResizable(true);
	generalScrollArea->setMinimumSize(220, 300);
	generalScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	generalScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QWidget *channelEnumerator = new QWidget();
	QVBoxLayout *layout = new QVBoxLayout();
	QGridLayout *chEnumeratorLayout = new QGridLayout();
	channelEnumerator->setLayout(layout);
	chEnumeratorLayout->setSpacing(15);

	// add title for ch enumerator

	layout->insertLayout(0, chEnumeratorLayout);
	// move plot curves (logic + decoder) to osc plot
	for (uint8_t i = 0; i < m_nbChannels; ++i) {

		LogicDataCurve *curve = new LogicDataCurve(nullptr, i, this);
		curve->setTraceHeight(25);
		m_oscPlot->addDigitalPlotCurve(curve, false);

		m_oscPlotCurves.push_back(curve);

		auto handle = connect(this, &LogicAnalyzer::dataAvailable, this,
			[=](uint64_t from, uint64_t to){
			curve->dataAvailable(from, to);
		}, Qt::DirectConnection);

		connect(curve, &GenericLogicPlotCurve::destroyed, [=](){
			disconnect(handle);
		});

		QCheckBox *channelBox = new QCheckBox("DIO " + QString::number(i));
		curve->setName("DIO " + QString::number(i));

		QHBoxLayout *hBoxLayout = new QHBoxLayout(this);

		chEnumeratorLayout->addLayout(hBoxLayout, i % 8, i / 8);

		hBoxLayout->addWidget(channelBox);

		QComboBox *triggerBox = new QComboBox();
		triggerBox->addItem("-");

		hBoxLayout->addWidget(triggerBox);

		channelBox->setChecked(true);

		for (int i = 1; i < ui->triggerComboBox->count(); ++i) {
			triggerBox->addItem(ui->triggerComboBox->itemIcon(i),
					    ui->triggerComboBox->itemText(i));
		}

		int condition = static_cast<int>(
					m_m2kDigital->getTrigger()->getDigitalCondition(i));
		triggerBox->setCurrentIndex((condition + 1) % 6);

		connect(triggerBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			m_m2kDigital->getTrigger()->setDigitalCondition(i,
					static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>((index + 5) % 6));
		});

		connect(channelBox, &QCheckBox::toggled, [=](bool toggled){
			const int oscAnalogChannels = m_oscPlot->getAnalogChannels();
			m_oscPlot->enableDigitalPlotCurve(i, toggled);
			m_oscPlot->setOffsetWidgetVisible(i + oscAnalogChannels, toggled);
			m_oscPlot->positionInGroupChanged(i + oscAnalogChannels, 0, 0);
			m_oscPlot->replot();
		});
		channelBox->setChecked(false);
	}

	layout->insertSpacerItem(-1, new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// title
	// combobox add decoder
	QComboBox *decoderComboBox = new QComboBox();

	decoderComboBox->addItem("Select a decoder to add");


	GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
	decoderList = g_slist_sort(decoderList, sort_pds);

	for (const GSList *sl = decoderList; sl; sl = sl->next) {

	    srd_decoder *dec = (struct srd_decoder *)sl->data;

	    QString decoderInput = "";

	    GSList *dec_channels = g_slist_copy(dec->inputs);
	    for (const GSList *sl = dec_channels; sl; sl = sl->next) {
		decoderInput = QString::fromUtf8((char*)sl->data);
	    }
	    g_slist_free(dec_channels);

	    if (decoderInput == "logic") {
		decoderComboBox->addItem(QString::fromUtf8(dec->id));
	    }
	}

	g_slist_free(decoderList);

	layout->insertWidget(1, decoderComboBox);

	// decoder enumerator
	QGridLayout *decoderEnumerator = new QGridLayout();

	layout->insertLayout(2, decoderEnumerator);

	QWidget *decoderMenu = nullptr;
	QVBoxLayout *decoderSettingsLayout = new QVBoxLayout();

	QComboBox *stackDecoderComboBox = new QComboBox();
	stackDecoderComboBox->setVisible(false);
	auto updateButtonStackedDecoder = [=](){
		if (m_oscChannelSelected < m_nbChannels) {
			stackDecoderComboBox->setVisible(false);
			return;
		}

		if (m_oscChannelSelected > m_oscPlotCurves.size() - 1) {
			return;
		}

		AnnotationCurve *curve = dynamic_cast<AnnotationCurve *>(m_oscPlotCurves[m_oscChannelSelected]);

		if (!curve) {
			return;
		}

		auto stack = curve->getDecoderStack();
		auto top = stack.back();

		QSignalBlocker stackDecoderComboBoxBlocker(stackDecoderComboBox);
		stackDecoderComboBox->clear();
		stackDecoderComboBox->addItem("-");

		QString decoderOutput = "";
		GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
		decoderList = g_slist_sort(decoderList, sort_pds);
		GSList *dec_channels = g_slist_copy(top->decoder()->outputs);
		for (const GSList *sl = dec_channels; sl; sl = sl->next) {
		    decoderOutput = QString::fromUtf8((char*)sl->data);
		}
		g_slist_free(dec_channels);
		for (const GSList *sl = decoderList; sl; sl = sl->next) {

		    srd_decoder *dec = (struct srd_decoder *)sl->data;

		    QString decoderInput = "";

		    GSList *dec_channels = g_slist_copy(dec->inputs);
		    for (const GSList *sl = dec_channels; sl; sl = sl->next) {
			decoderInput = QString::fromUtf8((char*)sl->data);
		    }
		    g_slist_free(dec_channels);

		    if (decoderInput == decoderOutput) {
			qDebug() << "Added: " << QString::fromUtf8(dec->id);
			stackDecoderComboBox->addItem(QString::fromUtf8(dec->id));
		    }
		}
		g_slist_free(decoderList);

		const bool shouldBeVisible = stackDecoderComboBox->count() > 1;

		stackDecoderComboBox->setVisible(shouldBeVisible);
	};

	connect(decoderComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), [=](const QString &decoder) {
		if (!decoderComboBox->currentIndex()) {
			return;
		}

		std::shared_ptr<logic::Decoder> initialDecoder = nullptr;

		GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
		for (const GSList *sl = decoderList; sl; sl = sl->next) {
		    srd_decoder *dec = (struct srd_decoder *)sl->data;
		    if (QString::fromUtf8(dec->id) == decoder) {
			initialDecoder = std::make_shared<logic::Decoder>(dec);
		    }
		}

		AnnotationCurve *curve = new AnnotationCurve(this, initialDecoder);
		curve->setTraceHeight(25);
		m_oscPlot->addDigitalPlotCurve(curve, true);

		// In case the Osc is running the annotation curve will not
		// be displayed properly due to lack of information about
		// sr buffer size and trigger offset. We borrow this values
		// from a logic curve on the plot
		QwtPlotCurve* dummyCurve = m_oscPlot->getDigitalPlotCurve(0);
		GenericLogicPlotCurve *logicCurve = dynamic_cast<GenericLogicPlotCurve *>(dummyCurve);
		curve->setSampleRate(logicCurve->getSampleRate());
		curve->setBufferSize(logicCurve->getBufferSize());
		curve->setTimeTriggerOffset(logicCurve->getTimeTriggerOffset());

		m_oscPlotCurves.push_back(curve);

		// use direct connection we want the processing
		// of the available data to be done in the capture thread
		auto connectionHandle = connect(this, &LogicAnalyzer::dataAvailable,
			this, [=](uint64_t from, uint64_t to){
			curve->dataAvailable(from, to);
		}, Qt::DirectConnection);

		connect(curve, &QObject::destroyed, [=](){
			disconnect(connectionHandle);
		});

		g_slist_free(decoderList);

		QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QWidget *decoderMenuItem = new QWidget();
		QHBoxLayout *layout = new QHBoxLayout(this);
		decoderMenuItem->setLayout(layout);
		QCheckBox *decoderBox = new QCheckBox(decoder);
		decoderBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		layout->addWidget(decoderBox);

		QPushButton *deleteBtn = new QPushButton(this);
		deleteBtn->setFlat(true);
		deleteBtn->setIcon(QIcon(":/icons/close_hovered.svg"));
		deleteBtn->setMaximumSize(QSize(16, 16));

		layout->addWidget(deleteBtn);
		layout->insertSpacerItem(2, spacer);

		decoderBox->setVisible(true);
		deleteBtn->setVisible(true);

		connect(deleteBtn, &QPushButton::clicked, [=](){
			decoderMenuItem->deleteLater();

			int chIdx = m_oscPlotCurves.indexOf(curve);

			if (chIdx == m_oscChannelSelected) {
				m_oscPlot->channelSelected(chIdx + m_oscAnalogChannels, false);
			} else if (chIdx < m_oscChannelSelected) {
				m_oscChannelSelected--;
			}

			bool groupDeleted = false;
			m_oscPlot->removeFromGroup(chIdx,
					       m_plot.getGroupOfChannel(chIdx).indexOf(chIdx),
					       groupDeleted);

			if (groupDeleted) {
				ui->groupWidget->setVisible(false);
				m_currentGroup.clear();
				ui->groupWidgetLayout->removeWidget(m_currentGroupMenu);
				m_currentGroupMenu->deleteLater();
				m_currentGroupMenu = nullptr;
			}

			m_oscPlot->removeDigitalPlotCurve(curve);
			m_oscPlotCurves.removeOne(curve);

			disconnect(connectionHandle);

			delete curve;

			// reposition decoder menu items after deleting one
			for (int i = chIdx; i < m_oscPlotCurves.size(); ++i) {
				const int index = i - 16; // subtract logic channels count
				QLayoutItem *next = decoderEnumerator->itemAtPosition((index + 1) / 2, (index + 1) % 2);
				decoderEnumerator->removeItem(next);
				decoderEnumerator->addItem(next, index / 2, index % 2);
			}

			updateButtonStackedDecoder();
		});

		const int itemsInLayout = decoderEnumerator->count();
		decoderEnumerator->addWidget(decoderMenuItem, itemsInLayout / 2, itemsInLayout % 2);

		decoderComboBox->setCurrentIndex(0);

		connect(decoderBox, &QCheckBox::toggled, [=](bool toggled){
			const int analogChannels = m_oscPlot->getAnalogChannels();
			m_oscPlot->enableDigitalPlotCurve(analogChannels + m_nbChannels + itemsInLayout, toggled);
			m_oscPlot->setOffsetWidgetVisible(analogChannels + m_nbChannels + itemsInLayout, toggled);
			m_oscPlot->positionInGroupChanged(analogChannels + m_nbChannels + itemsInLayout, 0, 0);
			m_oscPlot->replot();
		});

		int chId = m_oscPlotCurves.size() - 1;

		connect(curve, &AnnotationCurve::decoderMenuChanged, [=](){
			if (m_oscChannelSelected != chId) {
				return;
			}

			if (m_oscDecoderMenu) {
				decoderSettingsLayout->removeWidget(m_oscDecoderMenu);
				m_oscDecoderMenu->deleteLater();
				m_oscDecoderMenu = nullptr;
			}
			m_oscDecoderMenu = curve->getCurrentDecoderStackMenu();
			decoderSettingsLayout->addWidget(m_oscDecoderMenu);

			updateButtonStackedDecoder();
		});

		decoderBox->setChecked(true);
	});

	// Current Channel Tab ! QWidget(QScrollArea(ch settings + decoder settings))
	QScrollArea *currentChannelMenuScrollArea = new QScrollArea();
	QWidget *currentChannelMenu = new QWidget();

	currentChannelMenuScrollArea->setWidgetResizable(true);

	QVBoxLayout *currentChannelMenuLayout = new QVBoxLayout();
	currentChannelMenu->setLayout(currentChannelMenuLayout);
	currentChannelMenuLayout->setContentsMargins(15, 0, 15, 0);

	QLineEdit *nameLineEdit = new QLineEdit();
	nameLineEdit->setReadOnly(true);
	nameLineEdit->setText("No channel selected!");
	nameLineEdit->setVisible(true);
	nameLineEdit->setDisabled(true);
	nameLineEdit->setAlignment(Qt::AlignCenter);
	nameLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QLineEdit *traceHeightLineEdit = new QLineEdit();
	traceHeightLineEdit->setText("");
	traceHeightLineEdit->setVisible(true);
	traceHeightLineEdit->setDisabled(true);
	traceHeightLineEdit->setAlignment(Qt::AlignCenter);
	traceHeightLineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	QComboBox *currentChannelTriggerComboBox = new QComboBox();
	currentChannelTriggerComboBox->addItem("-");
	for (int i = 1; i < ui->triggerComboBox->count(); ++i) {
		currentChannelTriggerComboBox->addItem(ui->triggerComboBox->itemIcon(i),
				    ui->triggerComboBox->itemText(i));
	}

	currentChannelMenuLayout->addWidget(nameLineEdit);
	currentChannelMenuLayout->addWidget(traceHeightLineEdit);
	currentChannelMenuLayout->addWidget(currentChannelTriggerComboBox);

	currentChannelMenuLayout->addLayout(decoderSettingsLayout);

	currentChannelMenu->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	currentChannelMenuScrollArea->setMinimumSize(200, 300);
	currentChannelMenuScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QHBoxLayout *stackDecoderLayout = new QHBoxLayout();
	currentChannelMenuLayout->addLayout(stackDecoderLayout);
	stackDecoderLayout->insertSpacerItem(0, new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
	stackDecoderLayout->insertWidget(1, stackDecoderComboBox);

	/* Setup external trigger menu */
	QWidget *externalTrigger = new QWidget(currentChannelMenu);
	QVBoxLayout *externalTriggerLayout = new QVBoxLayout(externalTrigger);
	externalTriggerLayout->setContentsMargins(0, 0, 0, 0);
	externalTrigger->setLayout(externalTriggerLayout);

	QHBoxLayout *externalSubTitle = new QHBoxLayout();
	auto label = new QLabel("EXTERNAL TRIGGER ");

	externalSubTitle->addWidget(label);
	externalSubTitle->setContentsMargins(0, 0, 0, 0);
	externalSubTitle->setSpacing(0);

	QFrame *line = new QFrame(externalTrigger);
	line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	line->setMaximumSize(QSize(16777215, 1));
	line->setStyleSheet(QString::fromUtf8("border: 1px solid rgba(255, 255, 255, 70);"));
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	externalSubTitle->addWidget(line);

	externalTriggerLayout->addLayout(externalSubTitle);

	QGridLayout *externalGridLayout = new QGridLayout();
	auto externalOnOff = new adiscope::CustomSwitch();
	externalGridLayout->addWidget(externalOnOff, 0, 0);
	auto labelCondition = new QLabel("Condition");
	externalGridLayout->addWidget(labelCondition, 1, 0);

	QFile file(":stylesheets/stylesheets/customSwitch.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QString::fromLatin1(file.readAll());
	externalOnOff->setStyleSheet(styleSheet);

	auto comboBoxCondition = new QComboBox();
	externalGridLayout->addWidget(comboBoxCondition, 1, 1);

	comboBoxCondition->setDisabled(true);

	connect(externalOnOff, &CustomSwitch::toggled, [=](bool on){
		if (on) {
			comboBoxCondition->setEnabled(true);
			m_m2kDigital->getTrigger()->setDigitalSource(SRC_TRIGGER_IN);
			const int condition = comboBoxCondition->currentIndex();
			m_m2kDigital->getTrigger()->setDigitalExternalCondition(
						static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>((condition + 5) % 6));
		} else {
			comboBoxCondition->setCurrentIndex(0);
			comboBoxCondition->setDisabled(true);
			m_m2kDigital->getTrigger()->setDigitalSource(SRC_NONE);
		}
	});
	comboBoxCondition->addItem("-");
	for (int i = 1; i < ui->triggerComboBox->count(); ++i) {
		comboBoxCondition->addItem(ui->triggerComboBox->itemIcon(i),
				    ui->triggerComboBox->itemText(i));
	}

	connect(comboBoxCondition, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
		m_m2kDigital->getTrigger()->setDigitalExternalCondition(
					static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>((index + 5) % 6));
	});

	externalTriggerLayout->addLayout(externalGridLayout);

	layout->addWidget(externalTrigger);

	currentChannelMenuLayout->insertItem(-1, new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	tabWidget->addTab(channelEnumerator, "General");
	const int channelMenuTabId = tabWidget->addTab(currentChannelMenuScrollArea, "Channel");

	m_oscChannelSelectedConnection = connect(m_oscPlot, &CapturePlot::channelSelected, [=](int chIdx, bool selected){
		chIdx -= m_oscAnalogChannels;
		if (m_oscChannelSelected != chIdx && selected) {
			m_oscChannelSelected = chIdx;
			nameLineEdit->setEnabled(true);
			nameLineEdit->setText(m_oscPlotCurves[chIdx]->getName());
			traceHeightLineEdit->setEnabled(true);
			traceHeightLineEdit->setText(QString::number(m_oscPlotCurves[chIdx]->getTraceHeight()));

			if (m_oscChannelSelected < m_nbChannels) {
				currentChannelTriggerComboBox->setEnabled(true);
				QComboBox *triggerBox = qobject_cast<QComboBox*>(chEnumeratorLayout->itemAtPosition(m_oscChannelSelected % 8,
														    m_oscChannelSelected / 8)->layout()->itemAt(1)->widget());
				currentChannelTriggerComboBox->setCurrentIndex(triggerBox->currentIndex());
			} else {
				currentChannelTriggerComboBox->setDisabled(true);
				QSignalBlocker sb(currentChannelTriggerComboBox);
				currentChannelTriggerComboBox->setCurrentIndex(0);
			}

			tabWidget->setCurrentIndex(channelMenuTabId);

			if (m_oscChannelSelected < m_nbChannels) {
				if (m_oscDecoderMenu) {
					decoderSettingsLayout->removeWidget(m_oscDecoderMenu);
					m_oscDecoderMenu->deleteLater();
					m_oscDecoderMenu = nullptr;
				}
			} else {
				if (m_oscDecoderMenu) {
					decoderSettingsLayout->removeWidget(m_oscDecoderMenu);
					m_oscDecoderMenu->deleteLater();
					m_oscDecoderMenu = nullptr;
				}

				AnnotationCurve *annCurve = dynamic_cast<AnnotationCurve *>(m_oscPlotCurves[m_oscChannelSelected]);
				m_oscDecoderMenu = annCurve->getCurrentDecoderStackMenu();
				decoderSettingsLayout->addWidget(m_oscDecoderMenu);
			}
		} else if (m_oscChannelSelected == chIdx && !selected) {
			m_oscChannelSelected = -1;
			nameLineEdit->setDisabled(true);
			nameLineEdit->setText("No channel selected!");
			traceHeightLineEdit->setDisabled(true);
			traceHeightLineEdit->setText("");
			currentChannelTriggerComboBox->setDisabled(true);
			currentChannelTriggerComboBox->setCurrentIndex(0);

			if (m_oscDecoderMenu) {
				decoderSettingsLayout->removeWidget(m_oscDecoderMenu);
				m_oscDecoderMenu->deleteLater();
				m_oscDecoderMenu = nullptr;
			}
		}

		updateButtonStackedDecoder();
	});

	connect(traceHeightLineEdit, &QLineEdit::editingFinished, [=](){
		int value = traceHeightLineEdit->text().toInt();
		m_oscPlotCurves[m_oscChannelSelected]->setTraceHeight(value);
		m_oscPlot->replot();
		m_oscPlot->positionInGroupChanged(m_oscChannelSelected, 0, 0);
	});

	connect(currentChannelTriggerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
		if (m_oscChannelSelected != -1 && m_oscChannelSelected < m_nbChannels) {
			QComboBox *triggerBox = qobject_cast<QComboBox*>(chEnumeratorLayout->itemAtPosition(m_oscChannelSelected % 8,
													    m_oscChannelSelected / 8)->layout()->itemAt(1)->widget());
			triggerBox->setCurrentIndex(index);
		}
	});

	connect(tabWidget, &QTabWidget::currentChanged, [=](int index){
		if (index == tabWidget->indexOf(currentChannelMenuScrollArea)) {
			if (m_oscChannelSelected != -1 && m_oscChannelSelected < m_nbChannels) {
				currentChannelTriggerComboBox->setEnabled(true);
				QComboBox *triggerBox = qobject_cast<QComboBox*>(chEnumeratorLayout->itemAtPosition(m_oscChannelSelected % 8,
														    m_oscChannelSelected / 8)->layout()->itemAt(1)->widget());
				currentChannelTriggerComboBox->setCurrentIndex(triggerBox->currentIndex());
			}
		}
	});

	connect(stackDecoderComboBox, &QComboBox::currentTextChanged, [=](const QString &text) {
		if (m_oscChannelSelected < m_nbChannels) {
			return;
		}

		if (m_oscChannelSelected > m_oscPlotCurves.size() - 1) {
			return;
		}

		if (!stackDecoderComboBox->currentIndex()) {
			return;
		}

		AnnotationCurve *curve = dynamic_cast<AnnotationCurve *>(m_oscPlotCurves[m_oscChannelSelected]);

		if (!curve) {
			return;
		}

		GSList *dl = g_slist_copy((GSList *)srd_decoder_list());
		for (const GSList *sl = dl; sl; sl = sl->next) {
		    srd_decoder *dec = (struct srd_decoder *)sl->data;
		    if (QString::fromUtf8(dec->id) == text) {
			curve->stackDecoder(std::make_shared<logic::Decoder>(dec));
			break;
		    }
		}

		// Update decoder menu. New decoder must be shown
		// and it might also have some options that can
		// be modified
		if (m_oscDecoderMenu) {
			decoderSettingsLayout->removeWidget(m_oscDecoderMenu);
			m_oscDecoderMenu->deleteLater();
			m_oscDecoderMenu = nullptr;
		}

		m_oscDecoderMenu = curve->getCurrentDecoderStackMenu();
		decoderSettingsLayout->addWidget(m_oscDecoderMenu);

		updateButtonStackedDecoder();
	});

	currentChannelMenuScrollArea->setWidget(currentChannelMenu);
	generalScrollArea->setWidget(channelEnumerator);

	tabWidget->addTab(generalScrollArea, "General");
	tabWidget->addTab(currentChannelMenuScrollArea, "Channel");

	return {tabWidget, generalScrollArea};
}

void LogicAnalyzer::disableMixedSignalView()
{
	// restore the menu item availability
	toolMenuItem->setEnabled(true);

	// restore the state of the tool
	m_saveRestoreSettings.reset(nullptr);

	// remove from osc plot the logic curves
	QwtPlotCurve *curve = m_oscPlot->getDigitalPlotCurve(0);
	while (curve != nullptr) {
		m_oscPlot->removeDigitalPlotCurve(curve);
		delete curve;

		curve = m_oscPlot->getDigitalPlotCurve(0);
	}
	m_oscPlotCurves.clear();
	m_oscChannelSelected = -1;

	disconnect(m_oscChannelSelectedConnection);

	m_oscPlot = nullptr;
}

void LogicAnalyzer::addCurveToPlot(QwtPlotCurve *curve)
{

}

QwtPlot *LogicAnalyzer::getCurrentPlot()
{
	return nullptr;
}

void LogicAnalyzer::connectSignalsAndSlotsForPlot(CapturePlot *plot)
{

}

void LogicAnalyzer::on_btnChannelSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);

	if (checked && m_selectedChannel != -1) {
		ui->nameLineEdit->setText(m_plot.getChannelName(m_selectedChannel));
		ui->traceHeightLineEdit->setText(QString::number(
							 m_plotCurves[m_selectedChannel]->getTraceHeight()));
		if (m_selectedChannel < m_nbChannels) {
			int condition = static_cast<int>(
						m_m2kDigital->getTrigger()->getDigitalCondition(m_selectedChannel));
			ui->triggerComboBox->setCurrentIndex((condition + 1) % 6);
		}
	}
}

void LogicAnalyzer::on_btnCursors_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void LogicAnalyzer::on_btnTrigger_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void LogicAnalyzer::on_cursorsBox_toggled(bool on)
{
	m_plot.setCursorReadoutsVisible(on);
	m_plot.setVertCursorsEnabled(on);
}

void LogicAnalyzer::on_btnSettings_clicked(bool checked)
{
	CustomPushButton *btn = nullptr;

	if (checked && !m_menuOrder.isEmpty()) {
		btn = m_menuOrder.back();
		m_menuOrder.pop_back();
	} else {
		btn = static_cast<CustomPushButton *>(
			ui->settings_group->checkedButton());
	}

	btn->setChecked(checked);
}

void LogicAnalyzer::on_btnGeneralSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
	if(checked)
		ui->btnSettings->setChecked(!checked);
}

void LogicAnalyzer::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	// At the end of each animation, check if there are other button check
	// actions that might have happened while animating and execute all
	// these queued actions
	while (m_menuButtonActions.size()) {
		auto pair = m_menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void LogicAnalyzer::onTimeTriggerValueChanged(double value)
{
	if (value > m_timePositionButton->maxValue() ||
			value < m_timePositionButton->minValue()) {
		return;
	}

	const bool wasRunning = m_started;
	if (wasRunning) {
		startStop(false);
	}

	m_plot.cancelZoom();
	m_plot.zoomBaseUpdate();

	m_timeTriggerOffset = value * (1.0 / m_sampleRate);

	m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset);
	m_plot.replot();

	if (m_resetHorizAxisOffset) {
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset;
	}

	m_m2kDigital->getTrigger()->setDigitalDelay(value);

	updateBufferPreviewer(0, m_lastCapturedSample);

	if (wasRunning) {
		startStop(true);
	}
}

void LogicAnalyzer::onSampleRateValueChanged(double value)
{
	const bool wasRunning = m_started;
	if (wasRunning) {
		startStop(false);
	}

	qDebug() << "Sample rate: " << value;
	m_sampleRate = value;

	if (ui->btnStreamOneShot->isChecked()) { // oneshot
		m_plot.cancelZoom();
//		m_timePositionButton->setValue(m_timeTriggerOffset * m_sampleRate);
		m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset);
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset;
		m_plot.replot();
		m_plot.zoomBaseUpdate();
	} else { // streaming
		m_plot.cancelZoom();
		m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0);
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0;
		m_plot.replot();
		m_plot.zoomBaseUpdate();
	}

	m_plot.setHorizUnitsPerDiv(1.0 / m_sampleRate * m_bufferSize / 16.0);

	m_timerTimeout = 1.0 / m_sampleRate * m_bufferSize * 1000.0 + 100;

	m_plot.cancelZoom();
	m_plot.zoomBaseUpdate();
	m_plot.replot();

	updateBufferPreviewer(0, m_lastCapturedSample);

	double maxT = (1 << 13) * (1.0 / m_sampleRate) - 1.0 / m_sampleRate * m_bufferSize / 2.0; // 8192 * time between samples
	double minT = -((1 << 13) - 1) * (1.0 / m_sampleRate) - 1.0 / m_sampleRate * m_bufferSize / 2.0; // (2 << 13) - 1 max hdl fifo depth
	m_plot.setTimeTriggerInterval(minT, maxT);

	if (wasRunning) {
		startStop(true);
	}
}

void LogicAnalyzer::onBufferSizeChanged(double value)
{
	const bool wasRunning = m_started;
	if (wasRunning) {
		startStop(false);
	}

	m_bufferSize = value;

	if (ui->btnStreamOneShot->isChecked()) { // oneshot
		m_plot.cancelZoom();
//		m_timePositionButton->setValue(m_timeTriggerOffset);
		m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset);
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset;
		m_plot.replot();
		m_plot.zoomBaseUpdate();
	} else { // streaming
		m_plot.cancelZoom();
		m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0);
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0;
		m_plot.replot();
		m_plot.zoomBaseUpdate();
	}

	m_plot.setHorizUnitsPerDiv(1.0 / m_sampleRate * m_bufferSize / 16.0);
	m_timerTimeout = 1.0 / m_sampleRate * m_bufferSize * 1000.0 + 100;

	m_plot.cancelZoom();
	m_plot.zoomBaseUpdate();
	m_plot.replot();

	updateBufferPreviewer(0, m_lastCapturedSample);

	double maxT = (1 << 13) * (1.0 / m_sampleRate) - 1.0 / m_sampleRate * m_bufferSize / 2.0; // 8192 * time between samples
	double minT = -((1 << 13) - 1) * (1.0 / m_sampleRate) - 1.0 / m_sampleRate * m_bufferSize / 2.0; // (2 << 13) - 1 max hdl fifo depth
	m_plot.setTimeTriggerInterval(minT, maxT);

	if (wasRunning) {
		startStop(true);
	}
}

void LogicAnalyzer::on_btnStreamOneShot_toggled(bool toggled)
{
	const bool wasRunning = m_started;
	if (wasRunning) {
		startStop(false);
	}

	m_plot.enableTimeTrigger(toggled);
	m_timePositionButton->setEnabled(toggled);

	m_m2kDigital->getTrigger()->setDigitalStreamingFlag(toggled);

	if (toggled) { // oneshot
		m_plot.cancelZoom();
		m_timePositionButton->setValue(m_timeTriggerOffset * m_sampleRate);
		m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset);
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0 + m_timeTriggerOffset;
		m_plot.replot();
		m_plot.zoomBaseUpdate();
	} else { // streaming
		m_plot.cancelZoom();
		m_plot.setHorizUnitsPerDiv(1.0 / m_sampleRate * m_bufferSize / 16.0);
		m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0);
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0;
		m_plot.replot();
		m_plot.zoomBaseUpdate();
	}

	m_bufferSizeButton->setMaxValue(toggled ? MAX_BUFFER_SIZE_ONESHOT
						: MAX_BUFFER_SIZE_STREAM);
	m_bufferPreviewer->setCursorVisible(toggled);

	if (wasRunning) {
		startStop(true);
	}
}

void LogicAnalyzer::on_btnGroupChannels_toggled(bool checked)
{
	qDebug() << checked;
	ui->btnGroupChannels->setText(checked ? "Done" : "Group");

	if (checked) {
		m_plot.beginGroupSelection();
	} else {
		if (m_plot.endGroupSelection()) {
//			channelSelectedChanged(m_selectedChannel, false);
		}
	}
}

void LogicAnalyzer::channelSelectedChanged(int chIdx, bool selected)
{
	QSignalBlocker nameLineEditBlocker(ui->nameLineEdit);
	QSignalBlocker traceHeightLineEditBlocker(ui->traceHeightLineEdit);
	QSignalBlocker triggerComboBoxBlocker(ui->triggerComboBox);
	if (m_selectedChannel != chIdx && selected) {

		if (!ui->btnChannelSettings->isChecked()) {
			ui->btnChannelSettings->setChecked(true);
		}

		qDebug() << "Selected channel: " << chIdx;

		m_selectedChannel = chIdx;

		if (m_selectedChannel < m_nbChannels) {
			ui->hardwareName->setText("DIO " + QString::number(m_selectedChannel));
		} else {
			ui->hardwareName->setText("");
		}

		ui->nameLineEdit->setEnabled(true);
		ui->nameLineEdit->setText(m_plotCurves[m_selectedChannel]->getName());
		ui->traceHeightLineEdit->setEnabled(true);
		ui->traceHeightLineEdit->setText(
					QString::number(m_plotCurves[m_selectedChannel]->getTraceHeight()));
		ui->triggerComboBox->setEnabled(true);

		qDebug() << "SIze of group for this channel is: " << m_plot.getGroupOfChannel(m_selectedChannel).size();

		updateChannelGroupWidget(true);

		if (m_selectedChannel < m_nbChannels) {
			ui->triggerComboBox->setVisible(true);
			ui->labelTrigger->setVisible(true);
			int condition = static_cast<int>(
						m_m2kDigital->getTrigger()->getDigitalCondition(m_selectedChannel));
			ui->triggerComboBox->setCurrentIndex((condition + 1) % 6);

			if (m_decoderMenu) {
				ui->decoderSettingsLayout->removeWidget(m_decoderMenu);
				m_decoderMenu->deleteLater();
				m_decoderMenu = nullptr;
			}

			updateStackDecoderButton();

		} else {
			ui->triggerComboBox->setVisible(false);
			ui->labelTrigger->setVisible(false);
			if (m_decoderMenu) {
				ui->decoderSettingsLayout->removeWidget(m_decoderMenu);
				m_decoderMenu->deleteLater();
				m_decoderMenu = nullptr;
			}
			AnnotationCurve *annCurve = dynamic_cast<AnnotationCurve *>(m_plotCurves[m_selectedChannel]);
			m_decoderMenu = annCurve->getCurrentDecoderStackMenu();
			ui->decoderSettingsLayout->addWidget(m_decoderMenu);

			updateStackDecoderButton();
		}
	} else if (m_selectedChannel == chIdx && !selected) {
		m_selectedChannel = -1;
		ui->hardwareName->setText("");
		ui->nameLineEdit->setDisabled(true);
		ui->nameLineEdit->setText("");
		ui->traceHeightLineEdit->setDisabled(true);
		ui->traceHeightLineEdit->setText(QString::number(1));
		ui->triggerComboBox->setDisabled(true);
		ui->triggerComboBox->setCurrentIndex(0);


		if (m_decoderMenu) {
			ui->decoderSettingsLayout->removeWidget(m_decoderMenu);
			m_decoderMenu->deleteLater();
			m_decoderMenu = nullptr;
		}

		updateStackDecoderButton();

		updateChannelGroupWidget(false);
	}
}

void LogicAnalyzer::setupUi()
{
	ui->setupUi(this);

	// Hide the run button
//	ui->runSingleWidget->enableRunButton(false);

	int gsettings_panel = ui->stackedWidget->indexOf(ui->generalSettings);
	ui->btnGeneralSettings->setProperty("id", QVariant(-gsettings_panel));

	/* Cursors Settings */
	ui->btnCursors->setProperty("id", QVariant(-1));

	/* Trigger Settings */
	int triggers_panel = ui->stackedWidget->indexOf(ui->triggerSettings);
	ui->btnTrigger->setProperty("id", QVariant(-triggers_panel));

	/* Channel Settings */
	int channelSettings_panel = ui->stackedWidget->indexOf(ui->channelSettings);
	ui->btnChannelSettings->setProperty("id", QVariant(-channelSettings_panel));

	// default trigger menu?
	m_menuOrder.push_back(ui->btnTrigger);

	// set default menu width to 0
	ui->rightMenu->setMaximumWidth(0);

	// Plot positioning and settings
	m_plot.disableLegend();


	// Build central widget

	QWidget* centralWidget = new QWidget(this);
	QVBoxLayout* vLayout = new QVBoxLayout(centralWidget);
	vLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->setSpacing(0);

	// add the buffer previewer
	ui->plot_and_buffPreviewer->removeWidget(ui->hLayoutBufferPreview);
	vLayout->addWidget(ui->hLayoutBufferPreview);

	// add plot elements
	QWidget* plotWidget = new QWidget(this);
	plotWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
	QGridLayout* gridLayout = new QGridLayout(plotWidget);
	gridLayout->setVerticalSpacing(0);
	gridLayout->setHorizontalSpacing(0);
	gridLayout->setContentsMargins(15, 0, 15, 0);
	plotWidget->setLayout(gridLayout);

	QSpacerItem *plotSpacer = new QSpacerItem(0, 5,
		QSizePolicy::Fixed, QSizePolicy::Fixed);

	gridLayout->addWidget(m_plot.topArea(), 0, 0, 1, 4);
	gridLayout->addWidget(m_plot.topHandlesArea(), 1, 0, 1, 4);

	gridLayout->addWidget(m_plot.leftHandlesArea(), 0, 0, 4, 1);
	gridLayout->addWidget(m_plot.rightHandlesArea(), 0, 3, 4, 1);

	gridLayout->addWidget(&m_plot, 2, 1, 1, 1);
	gridLayout->addWidget(m_plotScrollBar, 2, 5, 1, 1);

	gridLayout->addWidget(m_plot.bottomHandlesArea(), 3, 0, 1, 4);
	gridLayout->addItem(plotSpacer, 4, 0, 1, 4);

	vLayout->addWidget(plotWidget);
	centralWidget->setLayout(vLayout);

	if(prefPanel->getCurrent_docking_enabled()) {

		// main window for dock widget
		QMainWindow* mainWindow = new QMainWindow(this);
		mainWindow->setCentralWidget(0);
		mainWindow->setWindowFlags(Qt::Widget);
		ui->gridLayoutPlot->addWidget(mainWindow, 1, 0, 1, 1);

		QDockWidget* docker = DockerUtils::createDockWidget(mainWindow, centralWidget);

		mainWindow->addDockWidget(Qt::LeftDockWidgetArea, docker);

#ifdef PLOT_MENU_BAR_ENABLED
		DockerUtils::configureTopBar(docker);
#endif
	} else {
		ui->gridLayoutPlot->addWidget(centralWidget, 1, 0, 1, 1);
	}


	m_plot.setAxisVisible(QwtAxis::YLeft, false);
	m_plot.setAxisVisible(QwtAxis::XBottom, false);

	m_plot.setUsingLeftAxisScales(false);
	m_plot.enableLabels(false);

	// Buffer previewer

	m_bufferPreviewer->setVerticalSpacing(6);
	m_bufferPreviewer->setMinimumHeight(20);
	m_bufferPreviewer->setMaximumHeight(20);
	m_bufferPreviewer->setMinimumWidth(375);
//	m_bufferPreviewer->setMaximumWidth(375);
	m_bufferPreviewer->setCursorPos(0.5);

	m_bufferPreviewer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	ui->vLayoutBufferSlot->addWidget(m_bufferPreviewer);


	// Setup sweep settings menu

	ui->sweepSettingLayout->addWidget(m_sampleRateButton);
	ui->sweepSettingLayout->addWidget(m_bufferSizeButton);
	ui->sweepSettingLayout->addWidget(m_timePositionButton);

	// Setup trigger menu

	// Setup channel / decoder menu


	// Setup cursors menu

	cr_ui = new Ui::CursorsSettings;
	cr_ui->setupUi(ui->cursorsSettings);

	setDynamicProperty(cr_ui->btnLockHorizontal, "use_icon", true);

	cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);
	connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
		[=](CustomPlotPositionButton::ReadoutsPosition position){
		m_plot.moveCursorReadouts(position);
	});

	cursorsPositionButton->setPosition(CustomPlotPositionButton::ReadoutsPosition::bottomRight);

	// Disable some options we don't need for this cursor settings panel
	cr_ui->widgteBtnNormalTrack->setVisible(false);
	cr_ui->widgetVertical->setVisible(false);
	cr_ui->widgetVerticalBtns->setVisible(false);

	cr_ui->horizontalSlider->setMaximum(100);
	cr_ui->horizontalSlider->setMinimum(0);
	cr_ui->horizontalSlider->setSingleStep(1);

	ui->triggerComboBox->setDisabled(true);
	ui->nameLineEdit->setDisabled(true);
	ui->traceHeightLineEdit->setDisabled(true);

	ui->traceHeightLineEdit->setValidator(new QIntValidator(1, 100, ui->traceHeightLineEdit));
	ui->traceHeightLineEdit->setText(QString::number(1));

	ui->groupWidget->setVisible(false);
	ui->stackDecoderWidget->setVisible(false);

	// Export Settings
	m_exportSettings = new ExportSettings(this);
	m_exportSettings->enableExportButton(false);
	ui->exportLayout->addWidget(m_exportSettings);
	for (int i = 0; i < DIGITAL_NR_CHANNELS; ++i) {
		m_exportSettings->addChannel(i, "DIO" + QString::number(i));
	}
	m_exportSettings->disableUIMargins();
	connect(m_exportSettings->getExportButton(), &QPushButton::clicked,
		this, &LogicAnalyzer::exportData);
}

void LogicAnalyzer::connectSignalsAndSlots()
{
	// connect all the signals and slots here

	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		[=](bool checked){
		auto btn = dynamic_cast<CustomPushButton *>(run_button);
		btn->setChecked(checked);
//		if (!checked) {
//			m_plot.setTriggerState(CapturePlot::Stop);
//		}
	});
	connect(run_button, &QPushButton::toggled,
		ui->runSingleWidget, &RunSingleWidget::toggle);
	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		this, &LogicAnalyzer::startStop);


	connect(ui->rightMenu, &MenuAnim::finished,
		this, &LogicAnalyzer::rightMenuFinished);


	// TODO: can be moved away from here into on_cursorsBox_toggled
	connect(ui->cursorsBox, &QCheckBox::toggled, [=](bool toggled){
		if (!toggled) {
			// make sure to deselect the cursors button if
			// the cursors are disabled
			ui->btnCursors->setChecked(false);

			// we also remove the button from the history
			// so that the last menu opened button on top
			// won't open the cursors menu when it is disabled
			m_menuOrder.removeOne(ui->btnCursors);
		}
	});

	connect(&m_plot, &CapturePlot::plotSizeChanged, [=](){
		m_bufferPreviewer->setFixedWidth(m_plot.canvas()->size().width());
		m_plotScrollBar->setFixedHeight(m_plot.canvas()->size().height());
	});

	// some conenctions for the cursors menu
	connect(cr_ui->hCursorsEnable, &CustomSwitch::toggled,
		&m_plot, &CapturePlot::setVertCursorsEnabled);
	connect(cr_ui->btnLockHorizontal, &QPushButton::toggled,
		&m_plot, &CapturePlot::setHorizCursorsLocked);

	connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
		cr_ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
		m_plot.setCursorReadoutsTransparency(value);
	});
	// default: full transparency
	cr_ui->horizontalSlider->setSliderPosition(100);

	connect(m_plot.getZoomer(), &OscPlotZoomer::zoomFinished, [=](bool isZoomOut){
		updateBufferPreviewer(0, m_lastCapturedSample);
	});

	connect(m_sampleRateButton, &ScaleSpinButton::valueChanged,
		this, &LogicAnalyzer::onSampleRateValueChanged);
	connect(m_bufferSizeButton, &ScaleSpinButton::valueChanged,
		this, &LogicAnalyzer::onBufferSizeChanged);

	connect(&m_plot, &CapturePlot::timeTriggerValueChanged, [=](double value){
		double delay = (value - 1.0 / m_sampleRate * m_bufferSize / 2.0 ) / (1.0 / m_sampleRate);
		m_timePositionButton->setValue(static_cast<int>(delay));
	});

	connect(m_timePositionButton, &PositionSpinButton::valueChanged,
		this, &LogicAnalyzer::onTimeTriggerValueChanged);


	connect(m_plotScrollBar, &QScrollBar::valueChanged, [=](double value) {
		m_plot.setAllYAxis(-5 - (value * 0.05), 5 - (value * 0.05));
		m_plot.replot();
	});

	connect(&m_plot, &CapturePlot::channelSelected,
		this, &LogicAnalyzer::channelSelectedChanged);

	connect(ui->nameLineEdit, &QLineEdit::textChanged, [=](const QString &text){
		m_plot.setChannelName(text, m_selectedChannel);
		m_plotCurves[m_selectedChannel]->setName(text);
	});

	connect(ui->traceHeightLineEdit, &QLineEdit::textChanged, [=](const QString &text){
		auto validator = ui->traceHeightLineEdit->validator();
		QString toCheck = text;
		int pos;

		setDynamicProperty(ui->traceHeightLineEdit,
				   "invalid",
				   validator->validate(toCheck, pos) == QIntValidator::Intermediate);
	});

	connect(ui->traceHeightLineEdit, &QLineEdit::editingFinished, [=](){
		int value = ui->traceHeightLineEdit->text().toInt();
		m_plotCurves[m_selectedChannel]->setTraceHeight(value);
		m_plot.replot();
		m_plot.positionInGroupChanged(m_selectedChannel, 0, 0);
	});

	connect(ui->triggerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
		m_m2kDigital->getTrigger()->setDigitalCondition(m_selectedChannel,
								static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>((index + 5) % 6));
		QLayout *widgetInLayout = ui->channelEnumeratorLayout->itemAtPosition(m_selectedChannel % 8,
							    m_selectedChannel / 8)->layout();
		auto triggerBox = dynamic_cast<QComboBox *>(widgetInLayout->itemAt(1)->widget());
		QSignalBlocker triggerBlocker(triggerBox);
		triggerBox->setCurrentIndex(index);
	});

	connect(ui->stackDecoderComboBox, &QComboBox::currentTextChanged, [=](const QString &text) {
		if (m_selectedChannel < m_nbChannels) {
			return;
		}

		if (m_selectedChannel > m_plotCurves.size() - 1) {
			return;
		}

		if (!ui->stackDecoderComboBox->currentIndex()) {
			return;
		}

		AnnotationCurve *curve = dynamic_cast<AnnotationCurve *>(m_plotCurves[m_selectedChannel]);

		if (!curve) {
			return;
		}

		GSList *dl = g_slist_copy((GSList *)srd_decoder_list());
		for (const GSList *sl = dl; sl; sl = sl->next) {
		    srd_decoder *dec = (struct srd_decoder *)sl->data;
		    if (QString::fromUtf8(dec->id) == text) {
			curve->stackDecoder(std::make_shared<logic::Decoder>(dec));
			break;
		    }
		}

		// Update decoder menu. New decoder must be shown
		// and it might also have some options that can
		// be modified
		if (m_decoderMenu) {
			ui->decoderSettingsLayout->removeWidget(m_decoderMenu);
			m_decoderMenu->deleteLater();
			m_decoderMenu = nullptr;
		}

		m_decoderMenu = curve->getCurrentDecoderStackMenu();
		ui->decoderSettingsLayout->addWidget(m_decoderMenu);

		updateStackDecoderButton();
	});

	connect(ui->printBtn, &QPushButton::clicked, [=](){
		m_plot.printWithNoBackground("Logic Analyzer");
	});
}

void LogicAnalyzer::triggerRightMenuToggle(CustomPushButton *btn, bool checked)
{
	// Queue the action, if right menu animation is in progress. This way
	// the action will be remembered and performed right after the animation
	// finishes
	if (ui->rightMenu->animInProgress()) {
		m_menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void LogicAnalyzer::toggleRightMenu(CustomPushButton *btn, bool checked)
{

	qDebug() << "toggleRightMenu called!";

	int id = btn->property("id").toInt();

	if (id != -ui->stackedWidget->indexOf(ui->generalSettings)){
		if (!m_menuOrder.contains(btn)){
			m_menuOrder.push_back(btn);
		} else {
			m_menuOrder.removeOne(btn);
			m_menuOrder.push_back(btn);
		}
	}

	if (checked) {
		settingsPanelUpdate(id);
	}

	ui->rightMenu->toggleMenu(checked);
}

void LogicAnalyzer::settingsPanelUpdate(int id)
{
	if (id >= 0) {
		ui->stackedWidget->setCurrentIndex(0);
	} else {
		ui->stackedWidget->setCurrentIndex(-id);
	}

	for (int i = 0; i < ui->stackedWidget->count(); i++) {
		QSizePolicy::Policy policy = QSizePolicy::Ignored;

		if (i == ui->stackedWidget->currentIndex()) {
			policy = QSizePolicy::Expanding;
		}
		QWidget *widget = ui->stackedWidget->widget(i);
		widget->setSizePolicy(policy, policy);
	}
	ui->stackedWidget->adjustSize();
}

void LogicAnalyzer::updateBufferPreviewer(int64_t min, int64_t max)
{
	// Time interval within the plot canvas
	QwtInterval plotInterval = m_plot.axisInterval(QwtAxis::XBottom);

	// Time interval that represents the captured data
	QwtInterval dataInterval(0.0, 0.0);
	long long totalSamples = m_bufferSize;

	if (totalSamples > 0) {
		const int offset = ui->btnStreamOneShot->isChecked() ? m_timePositionButton->value()
								     : 0;
		dataInterval.setMinValue(offset / m_sampleRate);
		dataInterval.setMaxValue((offset + max) / m_sampleRate);
	}

	// Use the two intervals to determine the width and position of the
	// waveform and of the highlighted area
	QwtInterval fullInterval = plotInterval | dataInterval;
	double wPos = 1 - (fullInterval.maxValue() - dataInterval.minValue()) /
		fullInterval.width();
	double wWidth = dataInterval.width() / fullInterval.width();

	double hPos = 1 - (fullInterval.maxValue() - plotInterval.minValue()) /
		fullInterval.width();
	double hWidth = plotInterval.width() / fullInterval.width();

	// Determine the cursor position
	QwtInterval containerInterval = (totalSamples > 0) ? dataInterval :
		fullInterval;
	double containerWidth = (totalSamples > 0) ? wWidth : 1;
	double containerPos = (totalSamples > 0) ? wPos : 0;
	double cPosInContainer = 1 - (containerInterval.maxValue() - 0) /
		containerInterval.width();
	double cPos = cPosInContainer * containerWidth + containerPos;

	// Update the widget
	m_bufferPreviewer->setWaveformWidth(wWidth);
	m_bufferPreviewer->setWaveformPos(wPos);
	m_bufferPreviewer->setHighlightWidth(hWidth);
	m_bufferPreviewer->setHighlightPos(hPos);
	m_bufferPreviewer->setCursorPos(cPos);
}

void LogicAnalyzer::initBufferScrolling()
{
	connect(m_plot.getZoomer(), &OscPlotZoomer::zoomFinished, [=](bool isZoomOut){
		m_horizOffset = m_plot.HorizOffset();
	});

	connect(m_bufferPreviewer, &BufferPreviewer::bufferMovedBy, [=](int value) {
		m_resetHorizAxisOffset = false;
		double moveTo = 0.0;
		auto interval = m_plot.axisInterval(QwtAxis::XBottom);
		double min = interval.minValue();
		double max = interval.maxValue();
		int width = m_bufferPreviewer->width();
		double xAxisWidth = max - min;

		moveTo = value * xAxisWidth / width;
		m_plot.setHorizOffset(moveTo + m_horizOffset);
		m_plot.replot();
		updateBufferPreviewer(0, m_lastCapturedSample);
	});
	connect(m_bufferPreviewer, &BufferPreviewer::bufferStopDrag, [=](){
		m_horizOffset = m_plot.HorizOffset();
		m_resetHorizAxisOffset = true;
	});
	connect(m_bufferPreviewer, &BufferPreviewer::bufferResetPosition, [=](){
		m_plot.setHorizOffset(1.0 / m_sampleRate * m_bufferSize / 2.0 +
				      (ui->btnStreamOneShot ? 0 : m_timeTriggerOffset / m_sampleRate));
		m_plot.replot();
		updateBufferPreviewer(0, m_lastCapturedSample);
		m_horizOffset = 1.0 / m_sampleRate * m_bufferSize / 2.0 +
				(ui->btnStreamOneShot ? 0 : m_timeTriggerOffset / m_sampleRate);
	});
}

void LogicAnalyzer::startStop(bool start)
{
	if (m_started == start) {
			return;
	}

	m_started = start;
	m_plot.startStop(start);

	m_triggerUpdater->setEnabled(start);
	if (start) {
		if (m_captureThread) {
			m_stopRequested = true;
			m_captureThread->join();
			delete m_captureThread;
			m_captureThread = nullptr;
		}

		m_plot.setSampleRatelabelValue(m_sampleRate);
		m_plot.setBufferSizeLabelValue(m_bufferSize);
		m_plot.setTimeBaseLabelValue(m_bufferSize / m_sampleRate / m_plot.xAxisNumDiv());

		m_stopRequested = false;

		m_m2kDigital->stopAcquisition();


		const double sampleRate = m_sampleRateButton->value();
		const uint64_t bufferSize = m_bufferSizeButton->value();
		const int bufferSizeAdjusted = static_cast<int>(((bufferSize + 3 ) / 4) * 4);
		m_bufferSizeButton->setValue(bufferSizeAdjusted);

		const bool oneShotOrStream = ui->btnStreamOneShot->isChecked();
		qDebug() << "stream one shot is set to: " << oneShotOrStream;

		const double delay = oneShotOrStream ? m_timeTriggerOffset * m_sampleRate
					       : 0;

		const double setSampleRate = m_m2kDigital->setSampleRateIn((sampleRate + 1));
		m_sampleRateButton->setValue(setSampleRate);

		m_m2kDigital->getTrigger()->setDigitalStreamingFlag(!oneShotOrStream);

		for (int i = 0; i < m_plotCurves.size(); ++i) {
			QwtPlotCurve *curve = m_plot.getDigitalPlotCurve(i);
			GenericLogicPlotCurve *logic_curve = dynamic_cast<GenericLogicPlotCurve *>(curve);
			logic_curve->reset();

			logic_curve->setSampleRate(sampleRate);
			logic_curve->setBufferSize(bufferSizeAdjusted);
			logic_curve->setTimeTriggerOffset(delay);
		}

		m_lastCapturedSample = 0;

		m_captureThread = new std::thread([=](){

			if (m_buffer) {
				delete[] m_buffer;
				m_buffer = nullptr;
			}

			m_buffer = new uint16_t[bufferSizeAdjusted];
			QMetaObject::invokeMethod(this, [=](){
				m_exportSettings->enableExportButton(true);
			}, Qt::DirectConnection);

			uint64_t totalSamples = bufferSizeAdjusted;
			uint64_t chunk_size = 0;

			double oneBufferTimeout = m_timerTimeout;

			// clear any warning
			m_plot.setMaxBufferSizeErrorLabel(false);

			if (oneShotOrStream) { // oneshot
				do {
					try {
						m_m2kDigital->setKernelBuffersCountIn(1);
						break;
					} catch (libm2k::m2k_exception &e) {
						qDebug() << e.what();
					}
				} while (true);

				chunk_size = bufferSizeAdjusted;

			} else { // streaming
				const int minKernelBuffers = 4;
				const int oneBufferMaxSize = 2 * 1024 * 1024; // 2M

				m_currentKernelBuffers = minKernelBuffers;

				/* ensure we have the minimum amount of kernel buffers to fit the buffer
				 * ex: we want to capture a 40M buffer, we will use (at least) 10 kernel buffers
				 * */
				while (bufferSizeAdjusted > (m_currentKernelBuffers * oneBufferMaxSize)
						&& m_currentKernelBuffers < MAX_KERNEL_BUFFERS) {
					m_currentKernelBuffers++;
				}

				/* ensure we don't spend more than 100ms on a acquiring a buffer if we can
				 * further divide it into smaller buffers (kernel buffers still available)
				 * */
				const double maxCaptureDuration = 0.1; // 100ms
				while (((static_cast<double>(bufferSizeAdjusted) / setSampleRate)
						/ static_cast<double>(m_currentKernelBuffers)) > maxCaptureDuration
						&& m_currentKernelBuffers < MAX_KERNEL_BUFFERS) {
					m_currentKernelBuffers++;
				}

				// buffer size per kernel buffer
				chunk_size = bufferSizeAdjusted / m_currentKernelBuffers;

				uint64_t chunkSizeBeforeAdjust = chunk_size;

				// buffer size must be divisible by 4
				chunk_size = 4 * (chunk_size / 4);

				/* If the chunk_size was not disible by 4 in the first place we would get a smaller
				 * value for it. Let's check if we can increase the chunk_size to the next divisible by 4
				 * number otherwise we need to add another kernel buffer if this is possible
				 * */
				if (chunk_size + 4 <= oneBufferMaxSize && chunkSizeBeforeAdjust != chunk_size) {
					chunk_size += 4;
				}

				// If the buffer size is > 64 * 4M we need to cap the chunk_size to 4M
				if (chunk_size > oneBufferMaxSize) {
					chunk_size = oneBufferMaxSize;
				}

				if (bufferSizeAdjusted >= MAX_KERNEL_BUFFERS * oneBufferMaxSize) {
					/* in this case if the sample rate is greater than 5M samples / s
					 * warn that data might not be continuos
					 * */
					if (setSampleRate > MAX_SR_STREAM) {
						m_plot.setMaxBufferSizeErrorLabel(true, "Data might not be continuous, lower sample rate or buffer size");
					}
				}

				do {
					try {
						m_m2kDigital->setKernelBuffersCountIn(m_currentKernelBuffers);
						break;
					} catch (libm2k::m2k_exception &e) {
						qDebug() << e.what();
					}
				} while (true);

				// time for one kernel buffer + 100ms for the transfer
				oneBufferTimeout = ((oneBufferTimeout - 100) / m_currentKernelBuffers) + 100;
			}

			uint64_t absIndex = 0;

			// notify that the acquisition started one waiting for it
			// to start, in order to correctly stop it
			{
				std::unique_lock<std::mutex> lock(m_acquisitionStartedMutex);
				m_m2kDigital->startAcquisition(chunk_size);
				m_acquisitionStarted = true;
			}
			m_acquisitionStartedCv.notify_one();

			do {
				const uint64_t captureSize = std::min(chunk_size, totalSamples);

				try {
					if (m_autoMode) {
						QMetaObject::invokeMethod(this, [=](){
							m_timer->start(oneBufferTimeout);
						});
					}

					const uint16_t * const temp = m_m2kDigital->getSamplesP(chunk_size);
					memcpy(m_buffer + absIndex, temp, sizeof(uint16_t) * captureSize);

					absIndex += captureSize;
					totalSamples -= captureSize;

					if (m_autoMode) {
						QMetaObject::invokeMethod(this, [=](){
							m_timer->stop();
						});
					}

					if (m_triggerState.empty()) {
						QMetaObject::invokeMethod(this, [=](){
							m_triggerUpdater->setInput(CapturePlot::Triggered);
						}, Qt::QueuedConnection);
					} else {
						QMetaObject::invokeMethod(this, [=](){
							m_triggerUpdater->setInput(CapturePlot::Auto);
						}, Qt::QueuedConnection);
					}

				} catch (libm2k::m2k_exception &e) {
//					HANDLE_EXCEPTION(e)
					qDebug() << e.what() << " code: " << e.iioCode();
					break;
				}

				if (m_stopRequested) {
					break;
				}

				Q_EMIT dataAvailable(absIndex - captureSize, absIndex);

				QMetaObject::invokeMethod(&m_plot, // trigger replot on Main Thread
							  "replot",
							  Qt::QueuedConnection);
				m_lastCapturedSample = absIndex;
				updateBufferPreviewer(0, m_lastCapturedSample);

				QMetaObject::invokeMethod(this,
							  "restoreTriggerState",
							  Qt::QueuedConnection);

				if (!totalSamples && ui->runSingleWidget->runButtonChecked()) {
					m_m2kDigital->stopAcquisition();
					{
						std::unique_lock<std::mutex> lock(m_acquisitionStartedMutex);
						m_acquisitionStarted = false;
					}
					m_m2kDigital->getTrigger()->setDigitalStreamingFlag(!oneShotOrStream);

					// notify that the acquisition started one waiting for it
					// to start, in order to correctly stop it
					{
						std::unique_lock<std::mutex> lock(m_acquisitionStartedMutex);
						m_m2kDigital->startAcquisition(chunk_size);
						m_acquisitionStarted = true;
					}
					m_acquisitionStartedCv.notify_one();

					totalSamples = bufferSizeAdjusted;
					absIndex = 0;

					int ms = (int)(1000.0 / getScopyPreferences()->getTarget_fps());
					std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				}
			} while (totalSamples && !m_stopRequested);

			m_started = false;

			if (ui->runSingleWidget->singleButtonChecked()) {
				QMetaObject::invokeMethod(ui->runSingleWidget,
							  "toggle",
							  Qt::QueuedConnection,
							  Q_ARG(bool, false));
				m_triggerUpdater->setEnabled(false);
			}
			m_m2kDigital->stopAcquisition();

			QMetaObject::invokeMethod(&m_plot,
						  "replot");

			QMetaObject::invokeMethod(this, [=](){
				m_exportSettings->enableExportButton(true);
			}, Qt::DirectConnection);

		});

	} else {
		if (m_captureThread) {
			// a stop request might be triggered before getting to getSamplesP,
			// thus cancelAcquisition in this case will be a no-op. In order to avoid
			// this issue we wait for acquisition to be started
			{
				std::unique_lock<std::mutex> lock(m_acquisitionStartedMutex);
				m_acquisitionStartedCv.wait(lock, [=]{ return m_acquisitionStarted; });
			}

			m_stopRequested = true;
			try {
				m_m2kDigital->cancelAcquisition(); // cancelBufferIn
			} catch (...) {
				qDebug() << "Error";
			}

			m_captureThread->join();
			m_acquisitionStarted = false;

			delete m_captureThread;
			m_captureThread = nullptr;
			restoreTriggerState();
		}
	}
}

void LogicAnalyzer::setupDecoders()
{
	if (srd_decoder_load_all() != SRD_OK) {
		qDebug() << "Error: srd_decoder_load_all failed!";
	}

	ui->addDecoderComboBox->addItem(tr("Select a decoder to add"));


	GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
	decoderList = g_slist_sort(decoderList, sort_pds);

	for (const GSList *sl = decoderList; sl; sl = sl->next) {

	    srd_decoder *dec = (struct srd_decoder *)sl->data;

	    QString decoderInput = "";

	    GSList *dec_channels = g_slist_copy(dec->inputs);
	    for (const GSList *sl = dec_channels; sl; sl = sl->next) {
		decoderInput = QString::fromUtf8((char*)sl->data);
	    }
	    g_slist_free(dec_channels);

	    if (decoderInput == "logic") {
		ui->addDecoderComboBox->addItem(QString::fromUtf8(dec->id));
	    }
	}

	g_slist_free(decoderList);

	connect(ui->addDecoderComboBox, QOverload<const QString &>::of(&QComboBox::currentIndexChanged), [=](const QString &decoder) {
		if (!ui->addDecoderComboBox->currentIndex()) {
			return;
		}

		std::shared_ptr<logic::Decoder> initialDecoder = nullptr;

		GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
		for (const GSList *sl = decoderList; sl; sl = sl->next) {
		    srd_decoder *dec = (struct srd_decoder *)sl->data;
		    if (QString::fromUtf8(dec->id) == decoder) {
			initialDecoder = std::make_shared<logic::Decoder>(dec);
		    }
		}

		g_slist_free(decoderList);

		AnnotationCurve *curve = new AnnotationCurve(this, initialDecoder);
		curve->setTraceHeight(25);
		m_plot.addDigitalPlotCurve(curve, true);

		// use direct connection we want the processing
		// of the available data to be done in the capture thread
		auto connectionHandle = connect(this, &LogicAnalyzer::dataAvailable,
			this, [=](uint64_t from, uint64_t to){
			if (!m_oscPlot) {
				curve->dataAvailable(from, to);
			}
		}, Qt::DirectConnection);

		curve->setSampleRate(m_sampleRate);
		curve->setBufferSize(m_bufferSize);

		const double delay = ui->btnStreamOneShot ? m_timeTriggerOffset * m_sampleRate
					       : 0;
		curve->setTimeTriggerOffset(delay);

		curve->dataAvailable(0, m_lastCapturedSample);

		m_plotCurves.push_back(curve);
		int chId = m_plotCurves.size() - 1;

		QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QWidget *decoderMenuItem = new QWidget();
		QHBoxLayout *layout = new QHBoxLayout(this);
		decoderMenuItem->setLayout(layout);
		QCheckBox *decoderBox = new QCheckBox(decoder);
		decoderBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		layout->addWidget(decoderBox);

		QPushButton *deleteBtn = new QPushButton(this);
		deleteBtn->setFlat(true);
		deleteBtn->setIcon(QIcon(":/icons/close_hovered.svg"));
		deleteBtn->setMaximumSize(QSize(16, 16));

		layout->addWidget(deleteBtn);
		layout->insertSpacerItem(2, spacer);

		decoderBox->setVisible(true);
		deleteBtn->setVisible(true);

		connect(deleteBtn, &QPushButton::clicked, [=](){
			decoderMenuItem->deleteLater();

			int chIdx = m_plotCurves.indexOf(curve);

			if (chIdx == m_selectedChannel) {
				channelSelectedChanged(chIdx, false);
			} else if (chIdx < m_selectedChannel) {
				m_selectedChannel--;
			}

			bool groupDeleted = false;
			m_plot.removeFromGroup(chIdx,
					       m_plot.getGroupOfChannel(chIdx).indexOf(chIdx),
					       groupDeleted);

			if (groupDeleted) {
				ui->groupWidget->setVisible(false);
				m_currentGroup.clear();
				ui->groupWidgetLayout->removeWidget(m_currentGroupMenu);
				m_currentGroupMenu->deleteLater();
				m_currentGroupMenu = nullptr;
			}

			m_plot.removeDigitalPlotCurve(curve);
			m_plotCurves.removeOne(curve);

			disconnect(connectionHandle);

			delete curve;

			// reposition decoder menu items after deleting one
			for (int i = chIdx; i < m_plotCurves.size(); ++i) {
				const int index = i - 16; // subtract logic channels count
				QLayoutItem *next = ui->decoderEnumeratorLayout->itemAtPosition((index + 1) / 2, (index + 1) % 2);
				ui->decoderEnumeratorLayout->removeItem(next);
				ui->decoderEnumeratorLayout->addItem(next, index / 2, index % 2);
			}
		});

		const int itemsInLayout = ui->decoderEnumeratorLayout->count();
		ui->decoderEnumeratorLayout->addWidget(decoderMenuItem, itemsInLayout / 2,
							itemsInLayout % 2);

		ui->addDecoderComboBox->setCurrentIndex(0);

		connect(decoderBox, &QCheckBox::toggled, [=](bool toggled){
			m_plot.enableDigitalPlotCurve(m_nbChannels + itemsInLayout, toggled);
			m_plot.setOffsetWidgetVisible(m_nbChannels + itemsInLayout, toggled);
			m_plot.positionInGroupChanged(m_nbChannels + itemsInLayout, 0, 0);
			m_plot.replot();
		});

		decoderBox->setChecked(true);

		connect(curve, &AnnotationCurve::decoderMenuChanged, [=](){
			if (m_selectedChannel != chId) {
				return;
			}

			if (m_decoderMenu) {
				ui->decoderSettingsLayout->removeWidget(m_decoderMenu);
				m_decoderMenu->deleteLater();
				m_decoderMenu = nullptr;
			}
			m_decoderMenu = curve->getCurrentDecoderStackMenu();
			ui->decoderSettingsLayout->addWidget(m_decoderMenu);

			updateStackDecoderButton();
		});
	});

}

void LogicAnalyzer::updateStackDecoderButton()
{
	qDebug() << "updateStackDecoderButton called!";

	if (m_selectedChannel < m_nbChannels) {
		ui->stackDecoderWidget->setVisible(false);
		return;
	}

	if (m_selectedChannel > m_plotCurves.size() - 1) {
		return;
	}

	AnnotationCurve *curve = dynamic_cast<AnnotationCurve *>(m_plotCurves[m_selectedChannel]);

	if (!curve) {
		return;
	}

	auto stack = curve->getDecoderStack();
	auto top = stack.back();

	QSignalBlocker stackDecoderComboBoxBlocker(ui->stackDecoderComboBox);
	ui->stackDecoderComboBox->clear();
	ui->stackDecoderComboBox->addItem("-");

	QString decoderOutput = "";
	GSList *decoderList = g_slist_copy((GSList *)srd_decoder_list());
	decoderList = g_slist_sort(decoderList, sort_pds);
	GSList *dec_channels = g_slist_copy(top->decoder()->outputs);
	for (const GSList *sl = dec_channels; sl; sl = sl->next) {
	    decoderOutput = QString::fromUtf8((char*)sl->data);
	}
	g_slist_free(dec_channels);
	for (const GSList *sl = decoderList; sl; sl = sl->next) {

	    srd_decoder *dec = (struct srd_decoder *)sl->data;

	    QString decoderInput = "";

	    GSList *dec_channels = g_slist_copy(dec->inputs);
	    for (const GSList *sl = dec_channels; sl; sl = sl->next) {
		decoderInput = QString::fromUtf8((char*)sl->data);
	    }
	    g_slist_free(dec_channels);

	    if (decoderInput == decoderOutput) {
		qDebug() << "Added: " << QString::fromUtf8(dec->id);
		ui->stackDecoderComboBox->addItem(QString::fromUtf8(dec->id));
	    }
	}
	g_slist_free(decoderList);

	const bool shouldBeVisible = ui->stackDecoderComboBox->count() > 1;

	ui->stackDecoderWidget->setVisible(shouldBeVisible);
}

void LogicAnalyzer::updateChannelGroupWidget(bool visible)
{

	QVector<int> channelsInGroup = m_plot.getGroupOfChannel(m_selectedChannel);

	const bool shouldBeVisible = visible & (channelsInGroup.size() > 0);

	ui->groupWidget->setVisible(shouldBeVisible);

	qDebug() << "channel group widget should be visible: " << shouldBeVisible
		 << " visible: " << visible << " channelsInGroup: " << channelsInGroup.size();

	if (!shouldBeVisible) {
		return;
	}

	if (channelsInGroup == m_currentGroup) {
		return;
	}

	m_currentGroup = channelsInGroup;

	if (m_currentGroupMenu) {
		ui->groupWidgetLayout->removeWidget(m_currentGroupMenu);
		m_currentGroupMenu->deleteLater();
		m_currentGroupMenu = nullptr;
	}

	m_currentGroupMenu = new BaseMenu(ui->groupWidget);
	ui->groupWidgetLayout->addWidget(m_currentGroupMenu);

	connect(m_currentGroupMenu, &BaseMenu::itemMovedFromTo, [=](short from, short to){
		m_plot.positionInGroupChanged(m_selectedChannel, from, to);
	});

	for (int i = 0; i < channelsInGroup.size(); ++i) {
		QString name = m_plotCurves[channelsInGroup[i]]->getName();
		LogicGroupItem *item = new LogicGroupItem(name, m_currentGroupMenu);
		connect(m_plotCurves[channelsInGroup[i]], &GenericLogicPlotCurve::nameChanged,
				item, &LogicGroupItem::setName);
		connect(item, &LogicGroupItem::deleteBtnClicked, [=](){
			bool groupDeleted = false;
			m_plot.removeFromGroup(m_selectedChannel, item->position(), groupDeleted);

			qDebug() << "m_selectedChannel: " << m_selectedChannel << " deleted: " << m_currentGroup[item->position()];
			if (m_selectedChannel == m_currentGroup[item->position()] && !groupDeleted) {
				ui->groupWidget->setVisible(false);
			}

			m_currentGroup.removeAt(item->position());
			if (groupDeleted) {
				ui->groupWidget->setVisible(false);
				m_currentGroup.clear();
				ui->groupWidgetLayout->removeWidget(m_currentGroupMenu);
				m_currentGroupMenu->deleteLater();
				m_currentGroupMenu = nullptr;
			}
		});
		m_currentGroupMenu->insertMenuItem(item);
	}

	// TODO: fix hardcoded value
	m_currentGroupMenu->setMaximumHeight(channelsInGroup.size() * 27);
}

void LogicAnalyzer::setupTriggerMenu()
{
	connect(ui->btnTriggerMode, &CustomSwitch::toggled, [=](bool toggled){
		m_autoMode = toggled;
		if (m_autoMode && m_started) {

			double oneBufferTimeOut = m_timerTimeout;

			if (!ui->btnStreamOneShot->isChecked()) {
				oneBufferTimeOut /= m_currentKernelBuffers;
			}

			m_timer->start(oneBufferTimeOut);

			qDebug() << "auto mode: " << m_autoMode << " with timeout: "
				 << oneBufferTimeOut << " when logic is started: " << m_started;
		}

		if (toggled) {
			m_triggerUpdater->setIdleState(CapturePlot::Auto);
			m_triggerUpdater->setInput(CapturePlot::Auto);
		} else {
			m_triggerUpdater->setIdleState(CapturePlot::Waiting);
			m_triggerUpdater->setInput(CapturePlot::Waiting);
		}

		if (!m_autoMode) {
			m_timer->stop();
		}
	});

	ui->triggerLogicComboBox->addItem("OR");
	ui->triggerLogicComboBox->addItem("AND");

	connect(ui->triggerLogicComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
		m_m2kDigital->getTrigger()->setDigitalMode(static_cast<libm2k::digital::DIO_TRIGGER_MODE>(index));
	});

	if (m_m2kDigital->getTrigger()->hasCrossInstrumentTrigger()) {
		ui->externalWidget->setEnabled(true);
		ui->lblWarningFw->setVisible(false);

		ui->externalTriggerSourceComboBox->addItem(tr("External Trigger In"));
		ui->externalTriggerSourceComboBox->addItem(tr("Oscilloscope"));

		connect(ui->externalTriggerSourceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
			m_m2kDigital->getTrigger()->setDigitalSource(static_cast<M2K_TRIGGER_SOURCE_DIGITAL>(index));
			if (index) { // oscilloscope
			/* set external trigger condition to none if the source is
			* set to oscilloscope (trigger in)
			* */
				ui->externalTriggerConditionComboBox->setCurrentIndex(0); // None
			}

		/*
		 * Disable the condition combo box if oscilloscope (trigger in) is selected
		 * and enable it if external trigger in is selected (trigger logic)
		 * */
			ui->externalTriggerConditionComboBox->setDisabled(index);
		});

		connect(ui->externalTriggerConditionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
			m_m2kDigital->getTrigger()->setDigitalExternalCondition(
						static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>((index + 5) % 6));
		});


		connect(ui->btnEnableExternalTrigger, &CustomSwitch::toggled, [=](bool on){
			if (on) {
				const int source = ui->externalTriggerSourceComboBox->currentIndex();
				const int condition = ui->externalTriggerConditionComboBox->currentIndex();
				m_m2kDigital->getTrigger()->setDigitalSource(static_cast<M2K_TRIGGER_SOURCE_DIGITAL>(source));
				m_m2kDigital->getTrigger()->setDigitalExternalCondition(
							static_cast<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>((condition + 5) % 6));
				ui->externalTriggerConditionComboBox->setDisabled(source);
			} else {
				m_m2kDigital->getTrigger()->setDigitalSource(M2K_TRIGGER_SOURCE_DIGITAL::SRC_NONE);
			}
		});

		QSignalBlocker blockerExternalTriggerConditionComboBox(ui->externalTriggerConditionComboBox);
		const int condition = static_cast<int>(
					m_m2kDigital->getTrigger()->getDigitalExternalCondition());
		ui->externalTriggerConditionComboBox->setCurrentIndex((condition + 1) % 6);

		QSignalBlocker blockerExternalTriggerSourceComboBox(ui->externalTriggerSourceComboBox);
		ui->externalTriggerSourceComboBox->setCurrentIndex(0);
		m_m2kDigital->getTrigger()->setDigitalSource(M2K_TRIGGER_SOURCE_DIGITAL::SRC_NONE);
	} else {
		ui->externalWidget->setEnabled(false);
		ui->lblWarningFw->setVisible(true);
	}

	connect(m_timer, &QTimer::timeout,
		this, &LogicAnalyzer::saveTriggerState);
}

void LogicAnalyzer::saveTriggerState()
{
	// save trigger state and set to no trigger each channel
	if (m_started && !m_triggerState.size()) {
		const bool streaming = !ui->btnStreamOneShot->isChecked();
		if (streaming) {
			m_m2kDigital->getTrigger()->setDigitalStreamingFlag(false);
		}

		for (int i = 0; i < m_nbChannels; ++i) {
			m_triggerState.push_back(m_m2kDigital->getTrigger()->getDigitalCondition(i));
			m_m2kDigital->getTrigger()->setDigitalCondition(i, M2K_TRIGGER_CONDITION_DIGITAL::NO_TRIGGER_DIGITAL);
		}

		auto externalTriggerCondition = m_m2kDigital->getTrigger()->getDigitalExternalCondition();
		m_triggerState.push_back(externalTriggerCondition);
		m_m2kDigital->getTrigger()->setDigitalExternalCondition(M2K_TRIGGER_CONDITION_DIGITAL::NO_TRIGGER_DIGITAL);

		if (streaming) {
			m_m2kDigital->getTrigger()->setDigitalStreamingFlag(true);
		}
	}
}

void LogicAnalyzer::restoreTriggerState()
{
	// restored saved trigger state
	if (m_triggerState.size()) {
		const bool streaming = !ui->btnStreamOneShot->isChecked();
		if (streaming) {
			m_m2kDigital->getTrigger()->setDigitalStreamingFlag(false);
		}

		for (int i = 0; i < m_nbChannels; ++i) {
			m_triggerState.push_back(m_m2kDigital->getTrigger()->getDigitalCondition(i));
			m_m2kDigital->getTrigger()->setDigitalCondition(i, m_triggerState[i]);
		}

		m_m2kDigital->getTrigger()->setDigitalExternalCondition(m_triggerState.back());

		if (streaming) {
			m_m2kDigital->getTrigger()->setDigitalStreamingFlag(true);
		}

		m_triggerState.clear();
	}
}

void LogicAnalyzer::readPreferences()
{
	bool showFps = prefPanel->getShow_plot_fps();
	m_plot.setVisibleFpsLabel(showFps);

	for (GenericLogicPlotCurve *curve : qAsConst(m_plotCurves)) {
		if (curve->getType() == LogicPlotCurveType::Data) {
			LogicDataCurve *ldc = dynamic_cast<LogicDataCurve*>(curve);
			if (!ldc) {
				continue;
			}

			ldc->setDisplaySampling(prefPanel->getDisplaySamplingPoints());
		}
	}
	ui->instrumentNotes->setVisible(prefPanel->getInstrumentNotesActive());

	m_plot.replot();
}

void LogicAnalyzer::exportData()
{
	QString separator = "";
	QString startRow = ";";
	QString endRow = "\n";
	QString selectedFilter;
	bool done = false;
	bool noChannelEnabled = true;

	m_exportConfig = m_exportSettings->getExportConfig();
	auto keys = m_exportConfig.keys();
	for (auto x : qAsConst(keys)) {
		if(m_exportConfig[x]) {
			noChannelEnabled =  false;
			break;
		}
	}

	if (noChannelEnabled)
		return;

	QStringList filter;
	filter += QString(tr("Comma-separated values files (*.csv)"));
	filter += QString(tr("Tab-delimited values files (*.txt)"));
	filter += QString(tr("Value Change Dump(*.vcd)"));
	filter += QString(tr("All Files(*)"));

	QString fileName = QFileDialog::getSaveFileName(this,
	tr("Export"), "", filter.join(";;"),
	    &selectedFilter, (m_useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if (fileName.isEmpty()) {
		return;
	}

	// Check the selected file type
	if (selectedFilter != "") {
		if(selectedFilter.contains("comma", Qt::CaseInsensitive)) {
			separator = ",";
		}
		if(selectedFilter.contains("tab", Qt::CaseInsensitive)) {
			separator = "\t";
		}
		if(selectedFilter.contains("Change Dump", Qt::CaseInsensitive)) {
			endRow = " $end\n";
			startRow = "$";
		}
	}

	if (fileName.split(".").size() <= 1) {
		// file name w/o extension. Let's append it
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}



	if (separator != "") {
		done = exportTabCsv(separator, fileName);
	} else {
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly)) {
			return;
		}

		QTextStream out(&file);

		/* Write the general information */
		out << startRow << "date " << QDateTime::currentDateTime().toString() << endRow;
		out << startRow << "version Scopy - " << QString(SCOPY_VERSION_GIT) << endRow;
		out << startRow << "comment " << QString::number(m_bufferSize) <<
		       " samples acquired at " << QString::number(m_sampleRate) <<
		       " Hz " << endRow;


		file.close();

		done = exportVcd(fileName, startRow, endRow);
	}
}

bool LogicAnalyzer::exportTabCsv(const QString &separator, const QString &fileName)
{
	FileManager fm("Logic Analyzer");
	fm.open(fileName, FileManager::EXPORT);

	QStringList chNames;
	for (unsigned int ch = 0; ch < DIGITAL_NR_CHANNELS; ch++) {
		if (m_exportConfig[ch]) {
			chNames.push_back("Channel " + QString::number(ch));
		}
	}

	QVector<QVector<double>> data;

	if (!m_buffer) {
		return false;
	} else {
		for (unsigned int i = 0; i < m_lastCapturedSample; ++i) {
			uint64_t sample = m_buffer[i];
			QVector<double> line;
			for (unsigned int ch = 0; ch < DIGITAL_NR_CHANNELS; ++ch) {
				int bit = (sample >> ch) & 1;
				if(m_exportConfig[ch]) {
					line.push_back(bit);
				}
			}
			data.push_back(line);
		}
	}

	fm.setSampleRate(m_sampleRate);

	fm.save(data, chNames);

	fm.performWrite();

	return true;
}

bool LogicAnalyzer::exportVcd(const QString &fileName, const QString &startSep, const QString &endSep)
{
	uint64_t current_sample, prev_sample;
	int current_bit, prev_bit, p;
	QString timescaleFormat;
	double timescale;
	bool timestamp_written = false;

	if (m_sampleRate == 0) {
		return false;
	}


	QFile file(fileName);
	if (!file.open(QIODevice::Append)) {
		return false;
	}

	QTextStream out(&file);

	/* Write the specific header */
	timescale = 1 / m_sampleRate;
	if (timescale < 1e-6) {
		timescaleFormat = "ns";
		timescale *= 1e9;
	} else if (timescale < 1e-3) {
		timescaleFormat = "us";
		timescale *= 1e6;
	} else if (timescale < 1) {
		timescaleFormat = "ms";
		timescale *= 1e3;
	} else {
		timescaleFormat = "s";
	}

	out << startSep << "timescale " << QString::number(timescale) << " " << timescaleFormat << endSep;
	out << startSep << "scope module Scopy" << endSep;
	int counter = 0;
	for (unsigned int ch = 0; ch < DIGITAL_NR_CHANNELS; ++ch) {
		if (m_exportConfig[ch]) {
			char c = '!' + counter;
			out << startSep << "var wire 1 " << c << " DIO" <<
			       QString::number(ch) << endSep;
			counter++;
		}
	}
	out << startSep << "upscope" << endSep;
	out << startSep << "enddefinitions" << endSep;

	/* Write the values */
	if (m_buffer) {
		for (uint64_t i = 0; i < m_lastCapturedSample; i++) {
			current_sample = m_buffer[i];
			if (i == 0) {
				prev_sample = current_sample;
			} else {
				prev_sample = m_buffer[i - 1];
			}
			timestamp_written = false;
			p = 0;
			for (unsigned int ch = 0; ch < DIGITAL_NR_CHANNELS; ch++) {

				current_bit = (current_sample >> ch) & 1;
				prev_bit = (prev_sample >> ch) & 1;

				if ((current_bit == prev_bit) && (i != 0)) {
					p++;
					continue;
				}
				if (!timestamp_written)
					out << "#" << QString::number(i);
				if (m_exportConfig[ch]) {
					char c = '0' + current_bit;
					char c2 = '!' + p;
					out << ' ' << c << c2;
					p++;
					timestamp_written = true;
				}
			}
			if (timestamp_written) {
				out << "\n";
			}
		}
	} else {
		file.close();
		return false;
	}

	file.close();
	return true;
}
