#include "logging_categories.h"
#include "marker_rightmenuplugin.hpp"

#include <QComboBox>

using namespace adiscope;
using namespace gui;

MarkerRightMenuPlugin::MarkerRightMenuPlugin(QWidget *parent, ToolView* toolView, bool dockable, ChannelManager *chManager, FftDisplayPlot *fft_plot) :
	IRightMenuPlugin(parent, toolView, dockable),
	fft_plot(fft_plot),
	marker_selector(new DbClickButtons(parent)),
	chManager(chManager),
	markerTypes({
	    tr("Manual"),
	    tr("Peak"),
	    tr("Delta"),
	    tr("Fixed")
	}),
	lbl_crtMarkerReading(new QLabel(parent))
{
	init();
}

MarkerRightMenuPlugin::~MarkerRightMenuPlugin()
{
	delete m_ui;
}

void MarkerRightMenuPlugin::init()
{
	qDebug(CAT_MARKER_RIGHTMENU_PLUGIN) << "init()";

	menu = new GenericMenu(new QWidget());
	menu->initInteractiveMenu();

	menuBtn = toolView->buildNewInstrumentMenu(menu, dockable, "Markers", false, false);

	initMarkers();
	connectSignals();

	menu->setMenuWidget(m_ui->scrollArea_2);
	toolView->addPlotInfoWidget(lbl_crtMarkerReading);

	if (dockable) {
		markerTableDocker = toolView->addDockableTabbedWidget(markerTable, "Markers Table");
	}
	else {
		toolView->addFixedCentralWidget(markerTable);
	}
	Q_EMIT m_ui->btnMarkerTable->toggled(false);
}

void MarkerRightMenuPlugin::connectSignals()
{
	connect(menu, &GenericMenu::menuVisibilityChanged, this, &MarkerRightMenuPlugin::menuToggled);

	connect(m_ui->btnLeftPeak, &QPushButton::clicked, this, &MarkerRightMenuPlugin::on_btnLeftPeak_clicked);
	connect(m_ui->btnRightPeak, &QPushButton::clicked, this, &MarkerRightMenuPlugin::on_btnRightPeak_clicked);
	connect(m_ui->btnDnAmplPeak, &QPushButton::clicked, this, &MarkerRightMenuPlugin::on_btnDnAmplPeak_clicked);
	connect(m_ui->btnMaxPeak, &QPushButton::clicked, this, &MarkerRightMenuPlugin::on_btnMaxPeak_clicked);
	connect(m_ui->btnUpAmplPeak, &QPushButton::clicked, this, &MarkerRightMenuPlugin::on_btnUpAmplPeak_clicked);
	connect(m_ui->btnMarkerTable, &QPushButton::toggled, this, &MarkerRightMenuPlugin::enableMarkerTable);
	connect(chManager, &ChannelManager::selectedChannel, this, &MarkerRightMenuPlugin::updateMarkerMenu);
}

void MarkerRightMenuPlugin::initMenu()
{
	m_ui = new Ui::MarkerSettings;
	m_ui->setupUi(menu);

	// Configure markers
	for (size_t i = 0; i < chManager->getChannelsCount(); i++) {
		fft_plot->setMarkerCount(i, 5);

		for (int m = 0; m < 5; m++) {
			fft_plot->setMarkerEnabled(i, m, false);
//				marker_api.append(new SpectrumMarker_API(this,i,m));

		}
	}
	marker_freq_pos = new PositionSpinButton({
	{tr("Hz"),1e0},
	{tr("kHz"),1e3},
	{tr("MHz"),1e6}
	}, tr("Frequency Position"), 0.0, 5e7, true, false, parent);
	m_ui->markerFreqPosLayout->addWidget(marker_freq_pos);
	marker_freq_pos->setFineModeAvailable(false);

	// Initialize Marker controls
	m_ui->hLayout_marker_selector->addWidget(marker_selector);

	connect(marker_selector, &DbClickButtons::buttonToggled,
		this, &MarkerRightMenuPlugin::onMarkerToggled);
	connect(marker_selector, &DbClickButtons::buttonSelected,
		this, &MarkerRightMenuPlugin::onMarkerSelected);
	marker_selector->setSelectedButton(0);

	connect(fft_plot, &FftDisplayPlot::newMarkerData,
		this, &MarkerRightMenuPlugin::onPlotNewMarkerData);
	connect(fft_plot, &FftDisplayPlot::markerSelected,
		this, &MarkerRightMenuPlugin::onPlotMarkerSelected);

	connect(marker_freq_pos, &PositionSpinButton::valueChanged,
		this, &MarkerRightMenuPlugin::onMarkerFreqPosChanged);

	marker_freq_pos->setMinValue(1);
	marker_freq_pos->setMaxValue(fft_plot->getStopFrequency());
	marker_freq_pos->setStep(2 * (fft_plot->getStartFrequency() -
					  fft_plot->getStartFrequency()) / fft_plot->getResolutionBW());

	qDebug(CAT_MARKER_RIGHTMENU_PLUGIN()) << fft_plot->getStopFrequency() << 2 * (fft_plot->getStartFrequency() -
										      fft_plot->getStartFrequency()) / fft_plot->getResolutionBW();
}

void MarkerRightMenuPlugin::initTable()
{
	markerTable = new adiscope::MarkerTable(parent);
	markerTable->setObjectName(QString::fromUtf8("markerTable"));
	QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Maximum);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(markerTable->sizePolicy().hasHeightForWidth());
	markerTable->setSizePolicy(sizePolicy1);
	markerTable->setMaximumSize(QSize(16777215, 120));
}

void MarkerRightMenuPlugin::initMarkers()
{
	initMenu();
	initTable();

	m_ui->pushButton_4->hide();
	m_ui->pushButton_3->hide();
}

void MarkerRightMenuPlugin::enableMarkerTable(bool en)
{
	if (dockable) {
		markerTableDocker->setVisible(en);
	} else {
		markerTable->setVisible(en);
	}
}

void MarkerRightMenuPlugin::onMarkerFreqPosChanged(double freq)
{
	int crt_channel_id = chManager->getSelectedChannel();
	int crt_marker = marker_selector->selectedButton();

	if (!fft_plot->markerEnabled(crt_channel_id, crt_marker)) {
		return;
	}

	fft_plot->setMarkerAtFreq(crt_channel_id, crt_marker, freq);
	fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
	double actual_freq = fft_plot->markerFrequency(crt_channel_id,
			     crt_marker);

	marker_freq_pos->silentSetValue(actual_freq);

	fft_plot->replot();
}

void MarkerRightMenuPlugin::updateMarkerMenu(unsigned int id)
{
	const ChannelWidget *cw = chManager->getChannelAt(id);

	// Is this if branch required?
	if (!toolView->getRunBtn()->isChecked()) {
		fft_plot->replot();
	}

	// Update markers settings menu based on current channel
	if (cw->nameButton()->isChecked()) {
		m_ui->labelMarkerSettingsTitle->setText(cw->fullName());
		QString stylesheet = QString("border: 2px solid %1"
					    ).arg(cw->color().name());
		m_ui->lineMarkerSettingsTitle->setStyleSheet(stylesheet);


		marker_selector->blockSignals(true);

		for (size_t i = 0; i < fft_plot->markerCount(id); i++) {
			marker_selector->setButtonChecked(i,
							  fft_plot->markerEnabled(id, i));
		}

		marker_selector->blockSignals(false);

		updateCrtMrkLblVisibility();
	}
}

void MarkerRightMenuPlugin::updateCrtMrkLblVisibility()
{
	int crt = marker_selector->selectedButton();

	if (crt >= 0) {
		bool visible = marker_selector->buttonChecked(crt);
			lbl_crtMarkerReading->setVisible(visible);
	}
}

void MarkerRightMenuPlugin::updateMrkFreqPosSpinBtnValue()
{
	int crt_marker = marker_selector->selectedButton();

	if (!fft_plot->markerEnabled(chManager->getSelectedChannel(), crt_marker)) {
		return;
	}

	double freq = fft_plot->markerFrequency(chManager->getSelectedChannel(), crt_marker);

	if (freq != marker_freq_pos->value()) {
		marker_freq_pos->silentSetValue(freq);
	}
}

void MarkerRightMenuPlugin::updateWidgetsRelatedToMarker(int mrk_id)
{
	if (fft_plot->markerEnabled(chManager->getSelectedChannel(), mrk_id)) {
		setCurrentMarkerLabelData(chManager->getSelectedChannel(), mrk_id);
	}

	updateCrtMrkLblVisibility();
	updateMrkFreqPosSpinBtnValue();
}

void MarkerRightMenuPlugin::setMarkerEnabled(int ch_idx, int mrk_idx, bool en)
{
	fft_plot->setMarkerEnabled(ch_idx, mrk_idx, en);

	if (en) {
		double cf = fft_plot->getStartFrequency() + (fft_plot->getStopFrequency() - fft_plot->getStartFrequency()) / 2;
		fft_plot->setMarkerAtFreq(ch_idx, mrk_idx, cf);
		fft_plot->updateMarkerUi(ch_idx, mrk_idx);
	}

	fft_plot->replot();
}

void MarkerRightMenuPlugin::onPlotMarkerSelected(uint chIdx, uint mkIdx)
{
	if (chManager->getSelectedChannel() != chIdx) {
		chManager->getChannelAt(chIdx)->nameButton()->setChecked(true);
	}

	if (marker_selector->selectedButton() != mkIdx) {
		marker_selector->setSelectedButton(mkIdx);
			updateWidgetsRelatedToMarker(mkIdx);
	}
}

void MarkerRightMenuPlugin::setCurrentMarkerLabelData(int chIdx, int mkIdx)
{
	// TODO: units should not be fixed to dBFS

	QString txtFreq = freq_formatter.format(
				  fft_plot->markerFrequency(chIdx, mkIdx), "Hz", 3);
	QString txtMag = QString::number(
				 fft_plot->markerMagnitude(chIdx, mkIdx), 'f', 3) +
			 QString(" dBFS");
	QString txt = QString("Marker %1 -> ").arg(mkIdx + 1) +
		      txtFreq + QString(" ") + txtMag;
	lbl_crtMarkerReading->setText(txt);
}

void MarkerRightMenuPlugin::onPlotNewMarkerData()
{
	int crt_marker = marker_selector->selectedButton();

	// Update top-right label holding the reading of the active marker
	if (fft_plot->markerEnabled(chManager->getSelectedChannel(), crt_marker)) {
		setCurrentMarkerLabelData(chManager->getSelectedChannel(), crt_marker);
		updateMrkFreqPosSpinBtnValue();
	}

	// Update the markers in the marker table
	for (size_t c = 0; c < chManager->getChannelsCount(); c++) {
		for (size_t m = 0; m < fft_plot->markerCount(c); m++) {
			if (fft_plot->markerEnabled(c, m)) {
				int mkType = fft_plot->markerType(c, m);
				markerTable->updateMarker(m, c,
							      fft_plot->markerFrequency(c, m),
							      fft_plot->markerMagnitude(c, m),
							      markerTypes[mkType]);
			}
		}
	}
}

void MarkerRightMenuPlugin::onMarkerSelected(int id)
{
	fft_plot->selectMarker(chManager->getSelectedChannel(), id);
		updateWidgetsRelatedToMarker(id);
}

void MarkerRightMenuPlugin::onMarkerToggled(int id, bool on)
{
	int crt_channel_id = chManager->getSelectedChannel();

	setMarkerEnabled(crt_channel_id, id, on);

	// Add/remove the marker from the marker table
	if (on) {
		int mkType = fft_plot->markerType(crt_channel_id, id);
		if(mkType < 0)
			return;

		bool mkExists = markerTable->isMarker(id, crt_channel_id);
		if (!mkExists) {
			markerTable->addMarker(id, crt_channel_id, QString("M%1").arg(id + 1),
						   fft_plot->markerFrequency(crt_channel_id, id),
						   fft_plot->markerMagnitude(crt_channel_id, id),
						   markerTypes[mkType]);
		} else {
			markerTable->updateMarker(id, crt_channel_id,
						      fft_plot->markerFrequency(crt_channel_id, id),
						      fft_plot->markerMagnitude(crt_channel_id, id),
						      markerTypes[mkType]);
		}
	} else {
		markerTable->removeMarker(id, crt_channel_id);
	}

	if (id == marker_selector->selectedButton()) {
		if (on) {
				setCurrentMarkerLabelData(crt_channel_id, id);
		}

		updateMrkFreqPosSpinBtnValue();
	}

	updateCrtMrkLblVisibility();
}

void MarkerRightMenuPlugin::menuToggled(bool toggled)
{
	if (toggled) {
		int id = chManager->getSelectedChannel();
		updateMarkerMenu(id);
	}
}

void MarkerRightMenuPlugin::on_btnLeftPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();
	fft_plot->marker_to_next_lower_freq_peak(chManager->getSelectedChannel(), crt_marker);

	if (!toolView->getRunBtn()->isChecked()) {
		fft_plot->updateMarkerUi(chManager->getSelectedChannel(), crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void MarkerRightMenuPlugin::on_btnRightPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();
	fft_plot->marker_to_next_higher_freq_peak(chManager->getSelectedChannel(), crt_marker);

	if (!toolView->getRunBtn()->isChecked()) {
		fft_plot->updateMarkerUi(chManager->getSelectedChannel(), crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void MarkerRightMenuPlugin::on_btnMaxPeak_clicked()
{
	if (fft_plot->getCurveSize(chManager->getSelectedChannel()) <= 0) {
		return;
	}

	int crt_marker = marker_selector->selectedButton();
	fft_plot->marker_to_max_peak(chManager->getSelectedChannel(), crt_marker);

	if (!toolView->getRunBtn()->isChecked()) {
		fft_plot->updateMarkerUi(chManager->getSelectedChannel(), crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}


void MarkerRightMenuPlugin::on_btnDnAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();
	fft_plot->marker_to_next_lower_mag_peak(chManager->getSelectedChannel(), crt_marker);

	if (!toolView->getRunBtn()->isChecked()) {
		fft_plot->updateMarkerUi(chManager->getSelectedChannel(), crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void MarkerRightMenuPlugin::on_btnUpAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();
	fft_plot->marker_to_next_higher_mag_peak(chManager->getSelectedChannel(), crt_marker);

	if (!toolView->getRunBtn()->isChecked()) {
		fft_plot->updateMarkerUi(chManager->getSelectedChannel(), crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}
