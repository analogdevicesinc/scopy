#include "ui_logic_analyzer_general_menu.h"

#include <QCheckBox>

#include <scopy/gui/logic_analyzer_general_menu.hpp>

using namespace scopy::gui;

constexpr int MAX_BUFFER_SIZE_ONESHOT = 4 * 1024 * 1024; // 4M
constexpr int DIGITAL_NR_CHANNELS = 16;

LogicAnalyzerGeneralMenu::LogicAnalyzerGeneralMenu(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::LogicAnalyzerGeneralMenu)
	, m_exportSettings(new ExportSettings)
	, m_nbChannels(DIGITAL_NR_CHANNELS)
{
	m_ui->setupUi(this);

	initUi();
}

LogicAnalyzerGeneralMenu::~LogicAnalyzerGeneralMenu()
{
	delete m_timePositionButton;
	delete m_sampleRateButton;
	delete m_bufferSizeButton;
	delete m_exportSettings;
	delete m_ui;
}

void LogicAnalyzerGeneralMenu::initUi()
{
	m_ui->widgetMenuHeader->setLabel("General Settings");
	m_ui->widgetMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetDecodersSubsSep->setLabel("DECODERS");
	m_ui->widgetDecodersSubsSep->setButtonVisible(false);

	m_ui->widgetAvailableChannelsSubsSep->setLabel("AVAILABLE CHANNELS");
	m_ui->widgetAvailableChannelsSubsSep->setButtonVisible(false);

	m_sampleRateButton = new ScaleSpinButton({{"sps", 1E0}, {"ksps", 1E+3}, {"Msps", 1E+6}}, tr("Sample Rate"), 1,
						 10e7, true, false, this, {1, 2, 5});
	m_bufferSizeButton =
		new ScaleSpinButton({{"samples", 1E0}, {"k samples", 1E+3}, {"M samples", 1E+6}, {"G samples", 1E+9}},
				    tr("Nr of samples"), 1000, MAX_BUFFER_SIZE_ONESHOT, true, false, this, {1, 2, 5});

	m_timePositionButton =
		new PositionSpinButton({{"samples", 1E0}}, tr("Delay"), -(1 << 13), (1 << 13) - 1, true, false, this);

	m_ui->vLayoutSweepSetting->addWidget(m_sampleRateButton);
	m_ui->vLayoutSweepSetting->addWidget(m_bufferSizeButton);
	m_ui->vLayoutSweepSetting->addWidget(m_timePositionButton);

	for (uint8_t i = 0; i < m_nbChannels; ++i) {
		QCheckBox* channelBox = new QCheckBox("DIO " + QString::number(i));

		QHBoxLayout* hBoxLayout = new QHBoxLayout();

		m_ui->gridLayoutChannelEnumerator->addLayout(hBoxLayout, i % 8, i / 8);

		hBoxLayout->addWidget(channelBox);

		QComboBox* triggerBox = new QComboBox();
		triggerBox->addItem("-");

		hBoxLayout->addWidget(triggerBox);

		channelBox->setChecked(true);

		triggerBox->setStyleSheet("QComboBox QAbstractItemView { min-width: 130px; }");

		triggerBox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
		// TODO: find a way to comunicate between menus
		//		for (int i = 1; i < m_ui->cmbBoxTrigger->count(); ++i) {
		//			triggerBox->addItem(m_ui->cmbBoxTrigger->itemText(i));
		//		}

		channelBox->setChecked(false);
	}

	// Export Settings
	m_exportSettings->enableExportButton(false);
	m_ui->exportLayout->addWidget(m_exportSettings);
	for (int i = 0; i < DIGITAL_NR_CHANNELS; ++i) {
		m_exportSettings->addChannel(i, "DIO" + QString::number(i));
	}
	m_exportSettings->disableUIMargins();
}
