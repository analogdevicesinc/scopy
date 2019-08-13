/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/* GNU Radio includes */
#include <gnuradio/blocks/float_to_complex.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/blocks/add_ff.h>
#include <gnuradio/iio/math.h>
#include <gnuradio/analog/sig_source_f.h>
#include <gnuradio/analog/fastnoise_source_f.h>

/* Qt includes */
#include <QGridLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QDebug>
#include <QFileDialog>
#include <QCheckBox>

/* Local includes */
#include "logging_categories.h"
#include "spectrum_analyzer.hpp"
#include "filter.hpp"
#include "math.hpp"
#include "fft_block.hpp"
#include "adc_sample_conv.hpp"
#include "dynamicWidget.hpp"
#include "hardware_trigger.hpp"
#include "channel_widget.hpp"
#include "db_click_buttons.hpp"
#include "filemanager.h"
#include "spectrum_analyzer_api.hpp"

/* Generated UI */
#include "ui_spectrum_analyzer.h"

#include <boost/make_shared.hpp>
#include <iio.h>
#include <iostream>

static const int MAX_REF_CHANNELS = 4;

using namespace adiscope;
using namespace std;

std::vector<std::pair<QString, FftDisplayPlot::MagnitudeType>>
SpectrumAnalyzer::mag_types = {
	{"dBFS", FftDisplayPlot::DBFS},
	{"dBV", FftDisplayPlot::DBV},
	{"dBu", FftDisplayPlot::DBU},
	{"Vpeak", FftDisplayPlot::VPEAK},
	{"Vrms", FftDisplayPlot::VRMS},
};

std::vector<std::pair<QString, FftDisplayPlot::AverageType>>
SpectrumAnalyzer::avg_types = {
	{"Sample", FftDisplayPlot::SAMPLE},
	{"Peak Hold", FftDisplayPlot::PEAK_HOLD},
	{"Peak Hold Continous", FftDisplayPlot::PEAK_HOLD_CONTINUOUS},
	{"Min Hold", FftDisplayPlot::MIN_HOLD},
	{"Min Hold Continous", FftDisplayPlot::MIN_HOLD_CONTINUOUS},
	{"Linear RMS", FftDisplayPlot::LINEAR_RMS},
	{"Linear dB", FftDisplayPlot::LINEAR_DB},
	{"Exponential RMS", FftDisplayPlot::EXPONENTIAL_RMS},
	{"Exponential dB", FftDisplayPlot::EXPONENTIAL_DB},
};

std::vector<std::pair<QString, SpectrumAnalyzer::FftWinType>>
SpectrumAnalyzer::win_types = {
	{"Flat top", FftWinType::FLAT_TOP},
	{"Rectangular", FftWinType::RECTANGULAR},
	{"Triangular (Bartlett)", FftWinType::TRIANGULAR},
	{"Hamming", FftWinType::HAMMING},
	{"Hann", FftWinType::HANN},
	{"Blackman-Harris", FftWinType::BLACKMAN_HARRIS},
	{"Kaiser", FftWinType::KAISER},
};

std::vector<QString> SpectrumAnalyzer::markerTypes = {
	"Manual",
	"Peak",
	"Delta",
	"Fixed",
};

SpectrumAnalyzer::SpectrumAnalyzer(struct iio_context *ctx, Filter *filt,
				   std::shared_ptr<GenericAdc> adc, ToolMenuItem *toolMenuItem,
                                   QJSEngine *engine, ToolLauncher *parent):
	Tool(ctx, toolMenuItem, new SpectrumAnalyzer_API(this), "Spectrum Analyzer",
	     parent),
	ui(new Ui::SpectrumAnalyzer),
	marker_selector(new DbClickButtons(this)),
	fft_plot(nullptr),
	settings_group(new QButtonGroup(this)),
	channels_group(new QButtonGroup(this)),
	adc(adc),
	adc_name(ctx ? filt->device_name(TOOL_SPECTRUM_ANALYZER) : ""),
	crt_channel_id(0),
	crt_peak(0),
	max_peak_count(10),
	fft_size(32768),
	searchVisiblePeaks(true),
	sample_rate(100e6),
	sample_rate_divider(1),
	marker_menu_opened(false),
	bin_sizes({
	256, 512, 1024, 2048, 4096, 8192, 16384, 32768
	}), nb_ref_channels(0),
	selected_ch_settings(-1),
	ref_channel_counter(0)

{

	// Get the list of names of the available channels
	QList<QString> channel_names;

	if (ctx) {
		iio = iio_manager::get_instance(ctx, adc_name);
		num_adc_channels = adc->numAdcChannels();
		adc_bits_count = adc->numAdcBits();

		auto adc_channels = adc->adcChannelList();

		for (unsigned int i = 0; i < adc_channels.size(); i++) {
			const char *id = iio_channel_get_name(adc_channels[i]);

			if (!id) {
				channel_names.push_back(
				        QString("Channel %1").arg(i + 1));
			} else {
				channel_names.push_back(QString(id));
			}
		}
	} else {
		num_adc_channels = 2;
		adc_bits_count = 12;

		for (int i = 0; i < num_adc_channels; i++)
			channel_names.push_back(
			        QString("Channel %1").arg(i + 1));
	}

	ui->setupUi(this);
	// Temporarily disable the delta marker button
	ui->pushButton_4->hide();

	// Hide Single and Preset buttons until functionality is added
	ui->btnPreset->hide();

	ui->cmb_units->blockSignals(true);
	ui->cmb_units->clear();

	for (auto it = mag_types.begin(); it != mag_types.end(); ++it) {
		ui->cmb_units->addItem(it->first);
	}

	ui->cmb_units->blockSignals(false);

	ui->comboBox_type->blockSignals(true);
	ui->comboBox_type->clear();

	for (auto it = avg_types.begin(); it != avg_types.end(); ++it) {
		ui->comboBox_type->addItem(it->first);
	}

	ui->comboBox_type->blockSignals(false);

	ui->comboBox_window->blockSignals(true);
	ui->comboBox_window->clear();

	for (auto it = win_types.begin(); it != win_types.end(); ++it) {
		ui->comboBox_window->addItem(it->first);
	}

	ui->comboBox_window->blockSignals(false);

	settings_group->addButton(ui->btnToolSettings);
	settings_group->addButton(ui->btnSweep);
	settings_group->addButton(ui->btnMarkers);
	settings_group->addButton(ui->btnAddRef);
	settings_group->setExclusive(true);

	fft_plot = new FftDisplayPlot(num_adc_channels, this);
	fft_plot->canvas()->setStyleSheet(QString("QwtPlotCanvas { "
	                                  "background-color: #141416; }"));
	fft_plot->disableLegend();
	fft_plot->setAxisScale(QwtPlot::yLeft, -200, 0, 10);
	// Disable mouse interactions with the axes until they are in a working state
	fft_plot->setXaxisMouseGesturesEnabled(false);

	for (uint i = 0; i < num_adc_channels; i++) {
		fft_plot->setYaxisMouseGesturesEnabled(i, false);
	}

	QGridLayout *gLayout = static_cast<QGridLayout *>
	                       (ui->widgetPlotContainer->layout());
	gLayout->addWidget(fft_plot, 1, 0, 1, 1);

	// Initialize spectrum channels
	for (int i = 0 ; i < num_adc_channels; i++) {
		channel_sptr channel = boost::make_shared<SpectrumChannel>(i,
		                       channel_names[i], fft_plot);
		channel->setColor(fft_plot->getLineColor(i));
		ui->channelsList->addWidget(channel->widget());
		channels.push_back(channel);

		settings_group->addButton(channel->widget()->menuButton());
		channels_group->addButton(channel->widget()->nameButton());

		connect(channel.get()->widget(), SIGNAL(menuToggled(bool)),
		        SLOT(onChannelSettingsToggled(bool)));
		connect(channel.get()->widget(), SIGNAL(selected(bool)),
		        SLOT(onChannelSelected(bool)));
		connect(channel.get()->widget(), SIGNAL(enabled(bool)),
		        SLOT(onChannelEnabled(bool)));

		ch_api.append(new SpectrumChannel_API(this,channel));
	}

	if (num_adc_channels > 0)
		channels[crt_channel_id]->widget()->nameButton()->
		setChecked(true);

	// Initialize Sweep controls

	range = new PositionSpinButton({
		{" ",1e0},
	}, "Range", 0.0, 0.0, false, false, this);
	ui->rangeLayout->addWidget(range);

	top = new PositionSpinButton({
		{" ",1e0},
	}, "Top", -100.0, 100.0, false, false, this);
	ui->topLayout->addWidget(top);

	marker_freq_pos = new PositionSpinButton({
		{"Hz",1e0},
		{"kHz",1e3},
		{"MHz",1e6}
	}, "Frequency Position", 0.0, 5e7, true, false, this);
	ui->markerFreqPosLayout->addWidget(marker_freq_pos);
	marker_freq_pos->setFineModeAvailable(false);

	startStopRange = new StartStopRangeWidget();
	connect(startStopRange, &StartStopRangeWidget::rangeChanged, [=](double start, double stop){
		fft_plot->setStartStop(start, stop);
		fft_plot->setAxisScale(QwtPlot::xBottom, start, stop);
		fft_plot->replot();

		setSampleRate(2 * stop);

		/* Re-populate the RBW list with the new available values */
		ui->cmb_rbw->blockSignals(true);
		ui->cmb_rbw->clear();
		int i = 0;

		for (; i < bin_sizes.size(); i++) {
			ui->cmb_rbw->addItem(freq_formatter.format(
						     sample_rate / bin_sizes[i], "Hz", 2));
		}

		ui->cmb_rbw->blockSignals(false);
		ui->cmb_rbw->setCurrentIndex(i - 1);

		marker_freq_pos->setMaxValue(stop);
		marker_freq_pos->setStep(2 * (stop -
						  start) / bin_sizes[ui->cmb_rbw->currentIndex()]);
	});

	connect(ui->cmb_rbw, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[=](int index){
		startStopRange->setMinimumSpanValue(10 * sample_rate / bin_sizes[index]);
	});

	// Initialize vertical axis controls
	range->setMinValue(1);
	range->setMaxValue(200);

	// Configure plot peak capabilities
	for (uint i = 0; i < num_adc_channels; i++) {
		fft_plot->setPeakCount(i, max_peak_count);
	}

	// Configure markers
	for (int i = 0; i < num_adc_channels; i++) {
		fft_plot->setMarkerCount(i, 5);

		for (int m = 0; m < 5; m++) {
			fft_plot->setMarkerEnabled(i, m, false);
			marker_api.append(new SpectrumMarker_API(this,i,m));

		}
	}

	ui->rightMenu->setMaximumWidth(0);
	ui->pushButton_3->hide();

	// Initialize Marker controls
	ui->hLayout_marker_selector->addWidget(marker_selector);

	connect(marker_selector, SIGNAL(buttonToggled(int, bool)),
	        this, SLOT(onMarkerToggled(int, bool)));
	connect(marker_selector, SIGNAL(buttonSelected(int)),
	        this, SLOT(onMarkerSelected(int)));
	marker_selector->setSelectedButton(0);

	connect(fft_plot, SIGNAL(newMarkerData()),
	        this, SLOT(onPlotNewMarkerData()));
	connect(fft_plot, SIGNAL(markerSelected(uint, uint)),
	        this, SLOT(onPlotMarkerSelected(uint, uint)));

	connect(marker_freq_pos, SIGNAL(valueChanged(double)),
	        this, SLOT(onMarkerFreqPosChanged(double)));

	connect(fft_plot, SIGNAL(sampleRateUpdated(double)),
	        this, SLOT(onPlotSampleRateUpdated(double)));
	connect(fft_plot, SIGNAL(sampleCountUpdated(uint)),
	        this, SLOT(onPlotSampleCountUpdated(uint)));

	if (ctx) {
		build_gnuradio_block_chain();
	} else {
		build_gnuradio_block_chain_no_ctx();
	}

	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		[=](bool checked){
		auto btn = dynamic_cast<CustomPushButton *>(run_button);
		btn->setChecked(checked);
	});
	connect(run_button, &QPushButton::toggled,
		ui->runSingleWidget, &RunSingleWidget::toggle);
	connect(ui->runSingleWidget, &RunSingleWidget::toggled,
		this, &SpectrumAnalyzer::runStopToggled);
	connect(this, &SpectrumAnalyzer::started,
		ui->runSingleWidget, &RunSingleWidget::toggle);


	connect(fft_plot, SIGNAL(newData()),
	        SLOT(singleCaptureDone()));

	connect(top, SIGNAL(valueChanged(double)),
	        SLOT(onTopValueChanged(double)));
	connect(range, SIGNAL(valueChanged(double)),
	        SLOT(onRangeValueChanged(double)));

	// UI default
	ui->comboBox_window->setCurrentText("Hamming");

	startStopRange->setStartValue(0);
	startStopRange->setStopValue(50e6);

	ui->gridSweepControls->addWidget(startStopRange, 0, 0, 2, 2);

	top->setValue(0);
	range->setValue(200);

	marker_freq_pos->setMinValue(1);
	marker_freq_pos->setMaxValue(startStopRange->getStopValue());
	marker_freq_pos->setStep(2 * (startStopRange->getStopValue() -
					  startStopRange->getStartValue()) / bin_sizes[ui->cmb_rbw->currentIndex()]);

	ui->lblMagUnit->setText(ui->cmb_units->currentText());
	ui->markerTable->hide();

	for (auto ch: channels) {
		ch->setFftWindow(FftWinType::HAMMING, fft_size);
	}

	connect(ui->logBtn, &QPushButton::toggled,
		fft_plot, &FftDisplayPlot::useLogFreq);

	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                           TOOL_SPECTRUM_ANALYZER)));
	api->load(*settings);
	api->js_register(engine);

	connect(ui->rightMenu, &MenuAnim::finished, this, &SpectrumAnalyzer::rightMenuFinished);
	menuOrder.push_back(ui->btnSweep);


	connect(ui->btnPrint, &QPushButton::clicked, [=](){
		fft_plot->printWithNoBackground(api->objectName(), false);
	});

	connect(ui->btnSnapshot, &QPushButton::clicked, [=](){
		QwtPlotCurve *curve = fft_plot->Curve(selected_ch_settings);
		if (selected_ch_settings < 2) {
			if (nb_ref_channels == MAX_REF_CHANNELS) {
				return;
			}

			if (!curve->data()->size()) {
				return;
			}

			QVector<double> xData;
			QVector<double> yData;
			for (size_t i = 0; i < curve->data()->size(); ++i) {
				xData.push_back(curve->data()->sample(i).x());
				yData.push_back(curve->data()->sample(i).y());
			}

			channel_sptr sc = channels.at(selected_ch_settings);

			QStringList channelDetails;

			auto iterAvg = std::find_if(avg_types.begin(), avg_types.end(),
			[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
				return p.second == sc->averageType();
			});

			if (iterAvg != avg_types.end()) {
				channelDetails.push_back((*iterAvg).first);
			}

			auto iterWin = std::find_if(win_types.begin(), win_types.end(),
			[&](const std::pair<QString, FftWinType>& p) {
				return p.second == sc->fftWindow();
			});

			if (iterWin != win_types.end()) {
				channelDetails.push_back((*iterWin).first);
			}

			channelDetails.push_back(QString::number(sc->averaging()));

			importedChannelDetails.push_back(channelDetails);

			add_ref_waveform(xData, yData);

		} else {
			// export
			auto export_dialog( new QFileDialog( this ) );
			export_dialog->setWindowModality( Qt::WindowModal );
			export_dialog->setFileMode( QFileDialog::AnyFile );
			export_dialog->setAcceptMode( QFileDialog::AcceptSave );
			export_dialog->setNameFilters({"Comma-separated values files (*.csv)",
							       "Tab-delimited values files (*.txt)"});

			if (export_dialog->exec()) {
				FileManager fm("Spectrum Analyzer");
				fm.open(export_dialog->selectedFiles().at(0), FileManager::EXPORT);

				QVector<double> freq_data, mag_data;

				for (size_t i = 0; i < curve->data()->size(); ++i) {
					freq_data.push_back(curve->sample(i).x());
					mag_data.push_back(curve->sample(i).y());
				}

				QString unit = ui->lblMagUnit->text();
				fm.save(freq_data, "Frequency(Hz)");
				fm.save(mag_data, "REF" + QString::number(selected_ch_settings - num_adc_channels + 1)
					+ "(" + unit + ")");

				QString channelDetails;

				auto iterAvg = std::find_if(avg_types.begin(), avg_types.end(),
				[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
					return p.first == importedChannelDetails[selected_ch_settings - num_adc_channels][0];
				});

				if (iterAvg != avg_types.end()) {
					channelDetails += (*iterAvg).first;
					channelDetails += ",";
				}

				auto iterWin = std::find_if(win_types.begin(), win_types.end(),
				[&](const std::pair<QString, FftWinType>& p) {
					return p.first == importedChannelDetails[selected_ch_settings - num_adc_channels][1];
				});

				if (iterWin != win_types.end()) {
					channelDetails += (*iterWin).first;
					channelDetails += ",";
				}

				channelDetails += importedChannelDetails[selected_ch_settings - num_adc_channels][2];
				channelDetails += ",";

				fm.setAdditionalInformation(channelDetails);

				fm.performWrite();
			}
		}
	});

	connect(ui->btnExport, &QPushButton::clicked,
		this, &SpectrumAnalyzer::btnExportClicked);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
	ui->runSingleWidget->toggle(false);
	setDynamicProperty(runButton(), "disabled", false);

	if (saveOnExit) {
		api->save(*settings);
	}

	delete api;
	for (auto it = ch_api.begin(); it != ch_api.end(); ++it) {
		delete *it;
	}
	for (auto it = marker_api.begin(); it != marker_api.end(); ++it) {
		delete *it;
	}

	if (iio) {
		bool started = isIioManagerStarted();

		if (started) {
			iio->lock();
		}

		for (unsigned int i = 0; i < num_adc_channels; i++) {
			iio->disconnect(fft_ids[i]);
		}

		if (started) {
			iio->unlock();
		}

		delete[] fft_ids;
	}

	delete ui;
}

void SpectrumAnalyzer::readPreferences() {
	fft_plot->setVisiblePeakSearch(prefPanel->getSpectrum_visible_peak_search());
}

void SpectrumAnalyzer::btnExportClicked()
{
	auto export_dialog( new QFileDialog( this ) );
	export_dialog->setWindowModality( Qt::WindowModal );
	export_dialog->setFileMode( QFileDialog::AnyFile );
	export_dialog->setAcceptMode( QFileDialog::AcceptSave );
	export_dialog->setNameFilters({"Comma-separated values files (*.csv)",
					       "Tab-delimited values files (*.txt)"});

	if (export_dialog->exec()) {
		FileManager fm("Spectrum Analyzer");
		fm.open(export_dialog->selectedFiles().at(0), FileManager::EXPORT);

		QVector<double> frequency_data;
		int nr_samples = fft_plot->Curve(0)->data()->size();
		for (int i = 0; i < nr_samples; ++i) {
			frequency_data.push_back(fft_plot->Curve(0)->sample(i).x());
		}

		fm.save(frequency_data, "Frequency(Hz)");

		QString channelDetails = "";

		for (int i = 0; i < channels.size(); ++i) {
			QVector<double> data;
			for (int j = 0; j < nr_samples; ++j) {
				data.push_back(fft_plot->Curve(i)->sample(j).y());
			}
			QString unit = ui->lblMagUnit->text();
			fm.save(data, "Amplitude CH" + QString::number(i + 1)
				+ "(" + unit + ")");

			/* Save information about the channels averaging type, window and
			* averaging value */
			channel_sptr sc = channels.at(i);

			auto iterAvg = std::find_if(avg_types.begin(), avg_types.end(),
			[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
				return p.second == sc->averageType();
			});

			if (iterAvg != avg_types.end()) {
				channelDetails += (*iterAvg).first;
				channelDetails += ",";
			}

			auto iterWin = std::find_if(win_types.begin(), win_types.end(),
			[&](const std::pair<QString, FftWinType>& p) {
				return p.second == sc->fftWindow();
			});

			if (iterWin != win_types.end()) {
				channelDetails += (*iterWin).first;
				channelDetails += ",";
			}

			channelDetails += QString::number(sc->averaging());
			channelDetails += ",";

		}

		fm.setAdditionalInformation(channelDetails);

		fm.performWrite();
	}

}

void SpectrumAnalyzer::triggerRightMenuToggle(CustomPushButton *btn, bool checked)
{
	if (ui->rightMenu->animInProgress()) {
		menuButtonActions.enqueue(
			QPair<CustomPushButton *, bool>(btn, checked));
	} else {
		toggleRightMenu(btn, checked);
	}
}

void SpectrumAnalyzer::toggleRightMenu(CustomPushButton *btn, bool checked)
{
	int index = 0;
	bool chSettings = false;
	int id = -1;

	if (btn != ui->btnSettings && btn != ui->btnToolSettings) {
		if (!menuOrder.contains(btn)) {
			menuOrder.push_back(btn);
		} else {
			menuOrder.removeOne(btn);
			menuOrder.push_back(btn);
		}
	}

	for (int i = 0; i < channels.size(); ++i) {
		if (channels.at(i).get()->widget()->menuButton() == btn) {
			chSettings = true;
			id = channels.at(i).get()->widget()->id();
		}
	}

	/* Also check if one of the reference channels settings btn
	 * was toggled */
	for (unsigned int i = 0; i < nb_ref_channels; ++i) {
		ChannelWidget *cw = referenceChannels.at(i);
		if (cw->menuButton() == btn) {
			chSettings = true;
			id = cw->id();
		}
	}

	if (!chSettings) {
		if (btn == ui->btnToolSettings) {
			index = 1;
		} else if (btn == ui->btnSweep) {
			index = 2;
		} else if (btn == ui->btnMarkers) {
			index = 3;
		} else if (btn == ui->btnAddRef) {
			index = 4;
		}
	}

	if (id != -1) {
		updateChannelSettingsPanel(id);
	}

	if (marker_menu_opened) {
		updateMarkerMenu(crt_channel_id);
	}

	if (!ui->btnToolSettings->isChecked()) {
		ui->btnSettings->setChecked(!!this->settings_group->checkedButton());
	}

	if (checked) {
		ui->stackedWidget->setCurrentIndex(index);
	}

	ui->rightMenu->toggleMenu(checked);
}

void SpectrumAnalyzer::rightMenuFinished(bool opened)
{
	Q_UNUSED(opened)

	while(menuButtonActions.size()) {
		auto pair = menuButtonActions.dequeue();
		toggleRightMenu(pair.first, pair.second);
	}
}

void SpectrumAnalyzer::on_btnToolSettings_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
	if (checked) {
		ui->btnSettings->setChecked(!checked);
	}
}

void SpectrumAnalyzer::on_btnSettings_clicked(bool checked)
{
	CustomPushButton *btn = nullptr;
	if (checked && !menuOrder.isEmpty()) {
		btn = menuOrder.back();
	} else {
		btn = static_cast<CustomPushButton *>(
			settings_group->checkedButton());
	}

	if (btn) {
		btn->setChecked(checked);
	}
}

void SpectrumAnalyzer::on_btnSweep_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void SpectrumAnalyzer::on_btnMarkers_toggled(bool checked)
{
	marker_menu_opened = checked;

	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void SpectrumAnalyzer::on_btnAddRef_toggled(bool checked)
{
	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(QObject::sender()), checked);
}

void SpectrumAnalyzer::on_btnBrowseFile_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this,
			   tr("Open import file"), "",
			   tr({"Comma-separated values files (*.csv);;"
			       "Tab-delimited values files (*.txt)"}));

	FileManager fm("Spectrum Analyzer");

	ui->importSettings->clear();
	import_data.clear();

	try {
		fm.open(fileName, FileManager::IMPORT);

		for (int i = 0; i < fm.getNrOfChannels(); ++i) {
			/* Amplitude CHX UNIT => mid(10, 3) strip CHX from column name */
			ui->importSettings->addChannel(i, fm.getColumnName(i).mid(10, 3));
		}

		QVector<QVector<double>> data = fm.read();
		for (int i = 0; i < data.size(); ++i) {
			import_data.push_back(data[i]);
		}

		QStringList channelDetails = fm.getAdditionalInformation();
		for (int i = 0; i < channelDetails.size() / 3; ++i) {
			QStringList currentChannelDetails;
			for (int j = 0; j < 3; ++j) {
				currentChannelDetails.push_back(channelDetails[i * 3 + j]);
			}
			importedChannelDetails.push_back(currentChannelDetails);
		}

		ui->fileLineEdit->setText(fileName);
		ui->fileLineEdit->setToolTip(fileName);

		ui->btnImport->setEnabled(true);
		ui->importSettings->setEnabled(true);


	} catch (FileManagerException &e) {
		ui->fileLineEdit->setText(e.what());
		ui->fileLineEdit->setToolTip("");
		ui->btnImport->setDisabled(true);
		ui->importSettings->setDisabled(true);
	}
}

void SpectrumAnalyzer::on_btnImport_clicked()
{
	QMap<int, bool> import_map = ui->importSettings->getExportConfig();

	for (int key : import_map.keys()) {
		if (import_map[key]) {
			add_ref_waveform(key);
		}
	}
}

void SpectrumAnalyzer::add_ref_waveform(QVector<double> xData, QVector<double> yData)
{
	if (nb_ref_channels == MAX_REF_CHANNELS) {
		return;
	}

	unsigned int curve_id = num_adc_channels + nb_ref_channels;

	QString qname = QString("REF %1").arg(++ref_channel_counter);

	fft_plot->registerReferenceWaveform(qname, xData, yData);

	auto channelWidget = new ChannelWidget(curve_id, true, false, fft_plot->getLineColor(curve_id));
	channelWidget->setShortName(qname);
	channelWidget->nameButton()->setText(channelWidget->shortName());
	channelWidget->setReferenceChannel(true);
	channelWidget->deleteButton()->setProperty(
		"curve_name", QVariant(qname));

	settings_group->addButton(channelWidget->menuButton());
	channels_group->addButton(channelWidget->nameButton());
	referenceChannels.push_back(channelWidget);

	connect(channelWidget, &ChannelWidget::menuToggled,
		this, &SpectrumAnalyzer::onChannelSettingsToggled);
	connect(channelWidget, &ChannelWidget::selected,
		this, &SpectrumAnalyzer::onChannelSelected);
	connect(channelWidget, &ChannelWidget::deleteClicked,
		this, &SpectrumAnalyzer::onReferenceChannelDeleted);
	connect(channelWidget, &ChannelWidget::enabled,
		this, &SpectrumAnalyzer::onChannelEnabled);

	ui->channelsList->addWidget(channelWidget);

	/* Increase ref channels count */
	nb_ref_channels++;

	if (!ui->btnMarkers->isEnabled()) {
		QSignalBlocker(ui->btnMarkers);
		ui->btnMarkers->setEnabled(true);
	}

	if (nb_ref_channels == MAX_REF_CHANNELS) {
		ui->btnAddRef->hide();
	}
}

void SpectrumAnalyzer::add_ref_waveform(unsigned int chIdx)
{
	QVector<double> xData;
	QVector<double> yData;

	for (int i = 0; i < import_data.size(); ++i) {
		xData.push_back(import_data[i][0]);
		yData.push_back(import_data[i][chIdx + 1]);
	}

	add_ref_waveform(xData, yData);
}

void SpectrumAnalyzer::onReferenceChannelDeleted()
{
	if (nb_ref_channels - 1 < MAX_REF_CHANNELS) {
		ui->btnAddRef->show();
	}

	ChannelWidget *channelWidget = static_cast<ChannelWidget *>(QObject::sender());
	QAbstractButton *delBtn = channelWidget->deleteButton();
	QString qname = delBtn->property("curve_name").toString();

	fft_plot->unregisterReferenceWaveform(qname);
	ui->channelsList->removeWidget(channelWidget);

	/* Check if there are other channel widgets that are enabled */
	bool channelsEnabled = false;

	referenceChannels.erase(std::find(referenceChannels.begin(),
					  referenceChannels.end(),
					  channelWidget));

	/* Update the id of the remaining channels */
	int id = num_adc_channels;
	for (auto iter = referenceChannels.begin();
	     iter != referenceChannels.end(); ++iter) {
		(*iter)->setId(id++);
	}

	nb_ref_channels--;

	if (channelWidget->id() < crt_channel_id) {
		crt_channel_id--;
	} else if (channelWidget->id() == crt_channel_id) {
		for (int i = 0; i < num_adc_channels + nb_ref_channels; ++i) {
			auto cw = getChannelWidgetAt(i);
			if (cw == channelWidget) {
				continue;
			}
			if (cw->enableButton()->isChecked()) {
				channelsEnabled = true;
				cw->nameButton()->setChecked(true);
				break;
			}
		}
	}

	if (ui->btnMarkers->isChecked() && !channelsEnabled) {
		ui->btnMarkers->setChecked(false);
		menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
					    (ui->btnMarkers, true));
		menuOrder.removeAll(ui->btnMarkers);
		ui->btnMarkers->blockSignals(true);
		ui->btnMarkers->setDisabled(true);
		ui->btnMarkers->blockSignals(false);
	}

	if (channelWidget->menuButton()->isChecked()) {
		channelWidget->menuButton()->setChecked(false);
		menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
					    (static_cast<CustomPushButton*>(channelWidget->menuButton()), true));
		menuOrder.removeAll(static_cast<CustomPushButton *>(channelWidget->menuButton()));
	}

	delete channelWidget;
}

void SpectrumAnalyzer::run()
{
	runStopToggled(true);
}

void SpectrumAnalyzer::stop()
{
	runStopToggled(false);
}

void SpectrumAnalyzer::runStopToggled(bool checked)
{
	if (checked) {
		if (iio) {
			writeAllSettingsToHardware();
		}

		fft_plot->presetSampleRate(sample_rate);
		fft_sink->set_samp_rate(sample_rate);
		start_blockchain_flow();
	} else {
		stop_blockchain_flow();
	}

	if (!checked) {
		fft_plot->resetAverageHistory();
	}
	m_running = checked;
}

void SpectrumAnalyzer::build_gnuradio_block_chain()
{
	// TO DO: don't use the 100e6 hardcoded value anymore
	fft_sink = adiscope::scope_sink_f::make(fft_size, 100e6,
	                                        "Osc Frequency", num_adc_channels,
	                                        (QObject *)fft_plot);
	fft_sink->set_trigger_mode(TRIG_MODE_TAG, 0, "buffer_start");

	bool started = isIioManagerStarted();

	if (started) {
		iio->lock();
	}

	bool canConvRawToVolts = (adc_name == "m2k-adc");

	if (canConvRawToVolts) {
		auto m2k_adc = dynamic_pointer_cast<M2kAdc>(adc);

		for (int i = 0; i < adc->numAdcChannels(); i++) {
			double corr_gain = 1.0;
			double hw_gain = 1.0;

			if (m2k_adc) {
				corr_gain = m2k_adc->chnCorrectionGain(i);
				hw_gain = m2k_adc->gainAt(
				                  m2k_adc->chnHwGainMode(i));
			}

			// Calculate the VLSB for current channel
			double vlsb = adc_sample_conv::convSampleToVolts(1,
			                corr_gain, 1, 0, hw_gain);
			fft_plot->setScaleFactor(i, vlsb);
		}
	}

	fft_ids = new iio_manager::port_id[num_adc_channels];

	for (int i = 0; i < num_adc_channels; i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, fft_size));
		auto ctm = gr::blocks::complex_to_mag_squared::make(1);

		// iio(i)->fft->ctm->fft_sink
		fft_ids[i] = iio->connect(fft, i, 0, true, fft_size);
		iio->connect(fft, 0, ctm, 0);
		iio->connect(ctm, 0, fft_sink, i);

		channels[i]->fft_block = fft;
		channels[i]->ctm_block = ctm;
	}

	if (started) {
		iio->unlock();
	}
}

void SpectrumAnalyzer::build_gnuradio_block_chain_no_ctx()
{
	// TO DO: don't use the 100e6 hardcoded value anymore
	fft_sink = adiscope::scope_sink_f::make(fft_size, 100e6,
	                                        "Osc Frequency", num_adc_channels,
	                                        (QObject *)fft_plot);

	top_block = gr::make_top_block("spectrum_analyzer");

	for (int i = 0; i < num_adc_channels; i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, fft_size));
		auto ctm = gr::blocks::complex_to_mag_squared::make(1);

		auto siggen = gr::analog::sig_source_f::make(100e6,
		                gr::analog::GR_SIN_WAVE, 5e6 + i * 5e6, 2048);
		auto noise = gr::analog::fastnoise_source_f::make(
		                     gr::analog::GR_GAUSSIAN, 1, 0, 8192);
		auto add = gr::blocks::add_ff::make();

		//siggen->|
		//        |->add->fft->ctm->fft_sink
		//noise-->|
		top_block->connect(siggen, 0, add, 0);
		top_block->connect(noise, 0, add, 1);
		top_block->connect(add, 0, fft, 0);
		top_block->connect(fft, 0, ctm, 0);
		top_block->connect(ctm, 0, fft_sink, i);

		channels[i]->fft_block = fft;
	}
}

void SpectrumAnalyzer::start_blockchain_flow()
{
	if (iio) {
		for (int i = 0; i < num_adc_channels; i++) {
			iio->start(fft_ids[i]);
		}
	} else {
		fft_sink->reset();
		top_block->start();
	}
}

void SpectrumAnalyzer::stop_blockchain_flow()
{
	if (iio) {
		for (int i = 0; i < num_adc_channels; i++) {
			iio->stop(fft_ids[i]);
		}
	} else {
		top_block->stop();
	}
}

void SpectrumAnalyzer::on_comboBox_type_currentIndexChanged(const QString& s)
{
	auto it = std::find_if(avg_types.begin(), avg_types.end(),
	[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
		return p.first == s;
	});

	if (it == avg_types.end()) {
		return;
	}

	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
		return;
	}

	auto avg_type = (*it).second;

	if (avg_type != channels[crt_channel]->averageType()) {
		channels[crt_channel]->setAverageType(avg_type);
	}
}

void SpectrumAnalyzer::on_comboBox_window_currentIndexChanged(const QString& s)
{
	auto it = std::find_if(win_types.begin(), win_types.end(),
	[&](const std::pair<QString, FftWinType>& p) {
		return p.first == s;
	});

	if (it == win_types.end()) {
		return;
	}

	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
		return;
	}

	if (!channels[crt_channel]->fft_block) {
		return;
	}

	auto win_type = (*it).second;

	if (win_type != channels[crt_channel]->fftWindow()) {
		channels[crt_channel]->setFftWindow((*it).second, fft_size);
	}
}

void SpectrumAnalyzer::on_spinBox_averaging_valueChanged(int n)
{
	int crt_channel = channelIdOfOpenedSettings();

	if (crt_channel < 0) {
		qDebug(CAT_SPECTRUM_ANALYZER) << "invalid channel ID for the opened Settings menu";
		return;
	}

	if (n != channels[crt_channel]->averaging()) {
		channels[crt_channel]->setAveraging(n);
	}
}

void SpectrumAnalyzer::onChannelSettingsToggled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	triggerRightMenuToggle(
		static_cast<CustomPushButton *>(cw->menuButton()), en);
}

void SpectrumAnalyzer::updateChannelSettingsPanel(unsigned int id)
{
	ChannelWidget *cw = getChannelWidgetAt(id);

	selected_ch_settings = id;

	if (cw->isReferenceChannel()) {
		ui->btnSnapshot->setText("Export");
	} else {
		ui->btnSnapshot->setText("Snapshot");
	}

	if (id < num_adc_channels) {
		channel_sptr sc = channels.at(id);

		QString style = QString("border: 2px solid %1").arg(cw->color().name());
		ui->lineChannelSettingsTitle->setStyleSheet(style);
		ui->channelSettingsTitle->setText(sc->name());

		/* Might be disabled */
		ui->comboBox_type->setEnabled(true);
		ui->comboBox_window->setEnabled(true);
		ui->spinBox_averaging->setEnabled(true);

		/* Migh be hidden */
		ui->comboBox_type->setVisible(true);
		ui->comboBox_window->setVisible(true);
		ui->spinBox_averaging->setVisible(true);

		ui->label_type->setVisible(true);
		ui->label_window->setVisible(true);
		ui->label_averaging->setVisible(true);

		auto it = std::find_if(avg_types.begin(), avg_types.end(),
		[&](const std::pair<QString, FftDisplayPlot::AverageType>& p) {
			return p.second == sc->averageType();
		});

		if (it != avg_types.end()) {
			ui->comboBox_type->setCurrentText((*it).first);
		}

		auto it2 = std::find_if(win_types.begin(), win_types.end(),
		[&](const std::pair<QString, FftWinType>& p) {
			return p.second == sc->fftWindow();
		});

		if (it2 != win_types.end()) {
			ui->comboBox_window->setCurrentText((*it2).first);
		}

		ui->spinBox_averaging->setValue(sc->averaging());
	} else {
		QString style = QString("border: 2px solid %1").arg(cw->color().name());
		ui->lineChannelSettingsTitle->setStyleSheet(style);
		ui->channelSettingsTitle->setText(cw->nameButton()->text());


		if ((id - num_adc_channels) < importedChannelDetails.size() &&
				importedChannelDetails[id - num_adc_channels].size() == 3) {
			ui->comboBox_type->setCurrentText(importedChannelDetails[id - num_adc_channels][0]);
			ui->comboBox_window->setCurrentText(importedChannelDetails[id - num_adc_channels][1]);
			ui->spinBox_averaging->setValue(importedChannelDetails[id - num_adc_channels][2].toInt());

			ui->comboBox_type->setDisabled(true);
			ui->comboBox_window->setDisabled(true);
			ui->spinBox_averaging->setDisabled(true);
		} else {
			ui->comboBox_type->setVisible(false);
			ui->comboBox_window->setVisible(false);
			ui->spinBox_averaging->setVisible(false);

			ui->label_type->setVisible(false);
			ui->label_window->setVisible(false);
			ui->label_averaging->setVisible(false);
		}
	}

}

ChannelWidget * SpectrumAnalyzer::getChannelWidgetAt(unsigned int id)
{
	for (int i = 0; i < channels.size(); ++i) {
		if (channels.at(i).get()->widget()->id() == id)
			return channels.at(i).get()->widget();
	}

	/* Also check if the id corresponds to one of the reference
	 * channel widgets and get an iterator pointing to that widget */
	auto cwIter = std::find_if(referenceChannels.begin(),
			       referenceChannels.end(),
			       [&id](ChannelWidget *referenceChannelWidget){
		return referenceChannelWidget->id() == id;
	});

	if (cwIter != referenceChannels.end()) {
		return *cwIter;
	}

	return nullptr;
}

void SpectrumAnalyzer::onChannelSelected(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());
	int chIdx = cw->id();

	crt_channel_id = chIdx;

	if (marker_menu_opened) {
		triggerRightMenuToggle(
			static_cast<CustomPushButton *>(ui->btnMarkers), en);
	}
}

void SpectrumAnalyzer::updateMarkerMenu(unsigned int id)
{
	ChannelWidget *cw = getChannelWidgetAt(id);

	// Is this if branch required?
	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->replot();
	}

	// Update markers settings menu based on current channel
	if (cw->nameButton()->isChecked()) {
		ui->labelMarkerSettingsTitle->setText(cw->fullName());
		QString stylesheet = QString("border: 2px solid %1"
					    ).arg(cw->color().name());
		ui->lineMarkerSettingsTitle->setStyleSheet(stylesheet);


		marker_selector->blockSignals(true);

		for (int i = 0; i < fft_plot->markerCount(id); i++) {
			marker_selector->setButtonChecked(i,
							  fft_plot->markerEnabled(id, i));
		}

		marker_selector->blockSignals(false);

		updateCrtMrkLblVisibility();
	}
}

bool SpectrumAnalyzer::isIioManagerStarted() const
{
	return iio->started() && (ui->runSingleWidget->singleButtonChecked()
						  || ui->runSingleWidget->runButtonChecked());
}

void SpectrumAnalyzer::onChannelEnabled(bool en)
{
	ChannelWidget *cw = static_cast<ChannelWidget *>(QObject::sender());

	if (en) {
		fft_plot->AttachCurve(cw->id());
		if (!ui->btnMarkers->isEnabled()) {
			ui->btnMarkers->blockSignals(true);
			ui->btnMarkers->setEnabled(true);
			ui->btnMarkers->blockSignals(false);
		}
	} else {
		bool allDisabled = true;
		for (int i = 0; i < channels.size() + nb_ref_channels; i++) {
			ChannelWidget *cw = getChannelWidgetAt(i);

			if (cw->enableButton()->isChecked()) {
				cw->nameButton()->setChecked(true);
				allDisabled = false;
				break;
			}
		}
		if (allDisabled) {
			QSignalBlocker(ui->btnMarkers);
			if (!ui->btnSweep->isChecked()) {
				QSignalBlocker(ui->btnSettings);
				ui->btnSettings->setChecked(false);
			}
			ui->btnMarkers->setChecked(false);
			ui->btnMarkers->setDisabled(true);
			menuButtonActions.removeAll(QPair<CustomPushButton*, bool>
						    (ui->btnMarkers, false));
			menuOrder.removeAll(ui->btnMarkers);
		}
		fft_plot->DetachCurve(cw->id());
	}

	for (int i = 0; i < fft_plot->markerCount(cw->id()); i++) {
		if (fft_plot->markerEnabled(cw->id(), i)) {
			fft_plot->setMarkerVisible(cw->id(), i, en);
		}
	}


	fft_plot->replot();
	updateRunButton(en);
}

void SpectrumAnalyzer::updateRunButton(bool ch_en)
{
	for (unsigned int i = 0; !ch_en && i < num_adc_channels; i++) {
		QWidget *parent = ui->channelsList->itemAt(i)->widget();
		QCheckBox *box = parent->findChild<QCheckBox *>("box");
		ch_en = box->isChecked();
	}
	ui->runSingleWidget->setEnabled(ch_en);
	runButton()->setEnabled(ch_en);
	setDynamicProperty(runButton(), "disabled", !ch_en);


	if (!ch_en) {
		runButton()->setChecked(false);
		ui->runSingleWidget->toggle(false);
	}
}

void SpectrumAnalyzer::writeAllSettingsToHardware()
{
	adc->setSampleRate(100e6);

	auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc);

	if (m2k_adc) {
		for (uint i = 0; i < adc->numAdcChannels(); i++) {
			m2k_adc->setChnHwOffset(i, 0.0);
			m2k_adc->setChnHwGainMode(i, M2kAdc::LOW_GAIN_MODE);
		}

		iio_device_attr_write_longlong(adc->iio_adc_dev(),
		                               "oversampling_ratio", sample_rate_divider);
	}

	auto trigger = adc->getTrigger();

	if (trigger) {
		for (uint i = 0; i < trigger->numChannels(); i++) {
			trigger->setTriggerMode(i, HardwareTrigger::ALWAYS);
		}
	}
}

int SpectrumAnalyzer::channelIdOfOpenedSettings() const
{
	int chId = -1;

	for (int i = 0; i < channels.size(); i++) {
		if (channels[i]->isSettingsOn()) {
			chId = channels[i]->id();
			break;
		}
	}

	return chId;
}

void SpectrumAnalyzer::on_btnLeftPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_lower_freq_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_btnRightPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_higher_freq_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_btnMaxPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_max_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_cmb_rbw_currentIndexChanged(int index)
{
	uint new_fft_size = bin_sizes[index];

	if (new_fft_size != fft_size) {
		setFftSize(new_fft_size);
	}

	marker_freq_pos->setMinValue(startStopRange->getStartValue());
	marker_freq_pos->setMaxValue(startStopRange->getStopValue());
	marker_freq_pos->setStep(2 * (startStopRange->getStopValue() -
					  startStopRange->getStartValue()) / bin_sizes[ui->cmb_rbw->currentIndex()]);
}

void SpectrumAnalyzer::setSampleRate(double sr)
{
	double max_sr = 100E6; // TO DO: make OscAdc figure out the max sr of an ADC

	sample_rate_divider = (int)(max_sr / sr);
	double new_sr = max_sr / sample_rate_divider;

	if (new_sr == sample_rate) {
		return;
	}

	if (isIioManagerStarted()) {
		stop_blockchain_flow();

		auto m2k_adc = std::dynamic_pointer_cast<M2kAdc>(adc);

		if (m2k_adc) {
			iio_device_attr_write_longlong(adc->iio_adc_dev(),
			                               "oversampling_ratio", sample_rate_divider);
		} else {
			adc->setSampleRate(sr);
		}

		fft_plot->presetSampleRate(new_sr);
		fft_plot->resetAverageHistory();
		fft_sink->set_samp_rate(new_sr);

		start_blockchain_flow();
	}

	sample_rate = new_sr;
}

void SpectrumAnalyzer::setFftSize(uint size)
{
	// TO DO: This is cumbersome. We shouldn't have to rebuild the entire
	//        block chain every time we need to change the FFT size. A
	//        spectrum_sink block similar to scope_sink_f would be better

	bool started = isIioManagerStarted();

	if (started) {
		iio->lock();
	}

	fft_size = size;
	fft_sink->set_nsamps(size);

	for (int i = 0; i < channels.size(); i++) {
		auto fft = gnuradio::get_initial_sptr(
		                   new fft_block(false, size));

		iio->disconnect(fft_ids[i]);
		fft_ids[i] = iio->connect(fft, i, 0, true, size);
		iio->connect(fft, 0, channels[i]->ctm_block, 0);
		iio->connect(channels[i]->ctm_block, 0, fft_sink, i);

		if (started) {
			iio->start(fft_ids[i]);
		}

		channels[i]->fft_block = fft;
		channels[i]->setFftWindow(channels[i]->fftWindow(), size);

		iio->set_buffer_size(fft_ids[i], size);
	}

	if (started) {
		iio->unlock();
	}
}

void SpectrumAnalyzer::on_btnDnAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_lower_mag_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::on_btnUpAmplPeak_clicked()
{
	int crt_marker = marker_selector->selectedButton();

	fft_plot->marker_to_next_higher_mag_peak(crt_channel_id, crt_marker);

	if (!ui->runSingleWidget->runButtonChecked()) {
		fft_plot->updateMarkerUi(crt_channel_id, crt_marker);
		fft_plot->replot();
	}

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::onMarkerToggled(int id, bool on)
{
	setMarkerEnabled(crt_channel_id, id, on);

	// Add/remove the marker from the marker table
	if (on) {
		int mkType = fft_plot->markerType(crt_channel_id, id);
		if(mkType < 0)
			return;

		bool mkExists = ui->markerTable->isMarker(id, crt_channel_id);
		if (!mkExists) {
			ui->markerTable->addMarker(id, crt_channel_id, QString("M%1").arg(id + 1),
						   fft_plot->markerFrequency(crt_channel_id, id),
						   fft_plot->markerMagnitude(crt_channel_id, id),
						   markerTypes[mkType]);
		} else {
			ui->markerTable->updateMarker(id, crt_channel_id,
						      fft_plot->markerFrequency(crt_channel_id, id),
						      fft_plot->markerMagnitude(crt_channel_id, id),
						      markerTypes[mkType]);
		}
	} else {
		ui->markerTable->removeMarker(id, crt_channel_id);
	}

	if (id == marker_selector->selectedButton()) {
		if (on) {
			setCurrentMarkerLabelData(crt_channel_id, id);
		}

		updateMrkFreqPosSpinBtnValue();
	}

	updateCrtMrkLblVisibility();
}

void SpectrumAnalyzer::onMarkerSelected(int id)
{
	fft_plot->selectMarker(crt_channel_id, id);
	updateWidgetsRelatedToMarker(id);
}

void SpectrumAnalyzer::setMarkerEnabled(int ch_idx, int mrk_idx, bool en)
{
	fft_plot->setMarkerEnabled(ch_idx, mrk_idx, en);

	if (en) {
		double cf = startStopRange->getCenterValue();
		fft_plot->setMarkerAtFreq(ch_idx, mrk_idx, cf);
		fft_plot->updateMarkerUi(ch_idx, mrk_idx);
	}

	fft_plot->replot();
}

void SpectrumAnalyzer::updateWidgetsRelatedToMarker(int mrk_idx)
{
	if (fft_plot->markerEnabled(crt_channel_id, mrk_idx)) {
		setCurrentMarkerLabelData(crt_channel_id, mrk_idx);
	}

	updateCrtMrkLblVisibility();
	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::setCurrentMarkerLabelData(int chIdx, int mkIdx)
{
	QString txtFreq = freq_formatter.format(
	                          fft_plot->markerFrequency(chIdx, mkIdx), "Hz", 3);
	QString txtMag = QString::number(
				 fft_plot->markerMagnitude(chIdx, mkIdx), 'f', 3) +
	                 QString(" " + ui->cmb_units->currentText());
	QString txt = QString("Marker %1 -> ").arg(mkIdx + 1) +
	              txtFreq + QString(" ") + txtMag;
	ui->lbl_crtMarkerReading->setText(txt);
}

void SpectrumAnalyzer::updateCrtMrkLblVisibility()
{
	int crt = marker_selector->selectedButton();

	if (crt >= 0) {
		bool visible = marker_selector->buttonChecked(crt);
		ui->lbl_crtMarkerReading->setVisible(visible);
	}
}

void SpectrumAnalyzer::onPlotNewMarkerData()
{
	int crt_marker = marker_selector->selectedButton();

	// Update top-right label holding the reading of the active marker
	if (fft_plot->markerEnabled(crt_channel_id, crt_marker)) {
		setCurrentMarkerLabelData(crt_channel_id, crt_marker);
		updateMrkFreqPosSpinBtnValue();
	}

	// Update the markers in the marker table
	for (int c = 0; c < num_adc_channels; c++) {
		for (int m = 0; m < fft_plot->markerCount(c); m++) {
			if (fft_plot->markerEnabled(c, m)) {
				int mkType = fft_plot->markerType(c, m);
				ui->markerTable->updateMarker(m, c,
				                              fft_plot->markerFrequency(c, m),
							      fft_plot->markerMagnitude(c, m),
				                              markerTypes[mkType]);
			}
		}
	}
}

void SpectrumAnalyzer::onPlotMarkerSelected(uint chIdx, uint mkIdx)
{
	if (crt_channel_id != chIdx) {
		channels[chIdx]->widget()->nameButton()->setChecked(true);
	}

	if (marker_selector->selectedButton() != mkIdx) {
		marker_selector->setSelectedButton(mkIdx);
		updateWidgetsRelatedToMarker(mkIdx);
	}
}

void SpectrumAnalyzer::onMarkerFreqPosChanged(double freq)
{
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

void SpectrumAnalyzer::updateMrkFreqPosSpinBtnLimits()
{
	marker_freq_pos->setMaxValue(startStopRange->getStopValue());
	marker_freq_pos->setStep(2 * startStopRange->getStopValue() /
	                             bin_sizes[ui->cmb_rbw->currentIndex()]);

	updateMrkFreqPosSpinBtnValue();
}

void SpectrumAnalyzer::updateMrkFreqPosSpinBtnValue()
{
	int crt_marker = marker_selector->selectedButton();

	if (!fft_plot->markerEnabled(crt_channel_id, crt_marker)) {
		return;
	}

//	if (!fft_plot->markerType(crt_channel_id, crt_marker) == 0) {
//		return;
//	}

	double freq = fft_plot->markerFrequency(crt_channel_id, crt_marker);

	if (freq != marker_freq_pos->value()) {
		marker_freq_pos->silentSetValue(freq);
	}
}

void SpectrumAnalyzer::onPlotSampleRateUpdated(double)
{
	updateMrkFreqPosSpinBtnLimits();
}

void SpectrumAnalyzer::onPlotSampleCountUpdated(uint)
{
	updateMrkFreqPosSpinBtnLimits();
}

void SpectrumAnalyzer::singleCaptureDone()
{
	if (ui->runSingleWidget->singleButtonChecked()) {
		Q_EMIT started(false);
	}
}

void SpectrumAnalyzer::on_cmb_units_currentIndexChanged(const QString& unit)
{
	auto it = std::find_if(mag_types.begin(), mag_types.end(),
	[&](const std::pair<QString, FftDisplayPlot::MagnitudeType>& p) {
		return p.first == unit;
	});

	if (it == mag_types.end()) {
		return;
	}

	FftDisplayPlot::MagnitudeType magType = (*it).second;

	switch (magType) {
	case FftDisplayPlot::VPEAK:
	case FftDisplayPlot::VRMS:
		fft_plot->setAxisScale(QwtPlot::yLeft, 0, 25, 10);
		top->setValue(25);
		range->setValue(25);
		break;

	default:
		fft_plot->setAxisScale(QwtPlot::yLeft, -200, 0, 10);
		top->setValue(0);
		range->setValue(200);
		break;
	}

	fft_plot->setMagnitudeType((*it).second);
	fft_plot->recalculateMagnitudes();
	fft_plot->replot();

	ui->lblMagUnit->setText(unit);
}

void SpectrumAnalyzer::on_btnMarkerTable_toggled(bool checked)
{
	ui->markerTable->setVisible(checked);

	// Set the Plot 3 times taller than the Marker Table (when visible)
	QGridLayout *layout = static_cast<QGridLayout *>(
	                              ui->widgetPlotContainer->layout());
	int row1 = getGridLayoutPosFromIndex(layout,
	                                     layout->indexOf(ui->markerTable)).first;
	int row2 = getGridLayoutPosFromIndex(layout,
	                                     layout->indexOf(fft_plot)).first;

	if (checked) {
		layout->setRowStretch(row1, 1);
		layout->setRowStretch(row2, 3);
	} else {
		layout->setRowStretch(row1, 0);
		layout->setRowStretch(row2, 0);
	}
}

QPair<int, int> SpectrumAnalyzer::getGridLayoutPosFromIndex(QGridLayout *layout,
                int index) const
{
	QPair<int, int> pos = qMakePair(-1, -1);

	if (layout && index >= 0) {
		int row, col, rowSpan, colSpan;
		layout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);
		pos.first = row;
		pos.second = col;
	}

	return pos;
}

void SpectrumAnalyzer::onTopValueChanged(double top)
{
	double rangeValue = range->value();
	fft_plot->setAxisScale(QwtPlot::yLeft, top - rangeValue, top);
	fft_plot->replot();
}

void SpectrumAnalyzer::onRangeValueChanged(double range)
{
	double topValue = top->value();
	fft_plot->setAxisScale(QwtPlot::yLeft, topValue - range, topValue);
	fft_plot->replot();
}

/*
 * class SpectrumChannel
 */
SpectrumChannel::SpectrumChannel(int id, const QString& name,
                                 FftDisplayPlot *plot):
	m_id(id),
	m_name(name),
	m_line_width(1.0),
	m_color(plot->getLineColor(id).name()),
	m_averaging(1),
	m_avg_type(FftDisplayPlot::SAMPLE),
	m_fft_win(SpectrumAnalyzer::HAMMING),
	m_plot(plot),
	m_widget(new ChannelWidget(id, false, false, m_color))
{
	m_widget->setFullName(name);
	m_widget->setShortName(QString("CH %1").arg(id + 1));
	m_widget->nameButton()->setText(m_widget->shortName());
}

bool SpectrumChannel::isSettingsOn() const
{
	return m_widget->menuButton()->isChecked();
}

void SpectrumChannel::setSettingsOn(bool on)
{
	m_widget->enableButton()->setChecked(on);
}

float SpectrumChannel::lineWidth() const
{
	return m_line_width;
}

void SpectrumChannel::setLinewidth(float width)
{
	m_line_width = width;
}

QColor SpectrumChannel::color() const
{
	return m_color;
}

void SpectrumChannel::setColor(const QColor& color)
{
	m_widget->setColor(color);
	m_color = color;
}

uint SpectrumChannel::averaging() const
{
	return m_averaging;
}

void SpectrumChannel::setAveraging(uint averaging)
{
	m_averaging = averaging;
	m_plot->setAverage(m_id, m_avg_type, averaging);
}

FftDisplayPlot::AverageType SpectrumChannel::averageType() const
{
	return m_avg_type;
}

void SpectrumChannel::setAverageType(FftDisplayPlot::AverageType avg_type)
{
	m_avg_type = avg_type;
	m_plot->setAverage(m_id, avg_type, m_averaging);
}

void SpectrumChannel::setFftWindow(SpectrumAnalyzer::FftWinType win, int taps)
{
	m_fft_win = win;

	std::vector<float> window = build_win(win, taps);
	float gain = calcCoherentPowerGain(window);
	scaletFftWindow(window, 1 / gain);
	fft_block->set_window(window);
}

SpectrumAnalyzer::FftWinType SpectrumChannel::fftWindow() const
{
	return m_fft_win;
}

std::vector<float> SpectrumChannel::build_win(SpectrumAnalyzer::FftWinType type,
                int ntaps)
{
	switch (type) {
	case SpectrumAnalyzer::FLAT_TOP:
		return gr::fft::window::flattop(ntaps);

	case SpectrumAnalyzer::RECTANGULAR:
		return gr::fft::window::rectangular(ntaps);

	case SpectrumAnalyzer::TRIANGULAR:
		return gr::fft::window::bartlett(ntaps);

	case SpectrumAnalyzer::HAMMING:
		return gr::fft::window::hamming(ntaps);

	case SpectrumAnalyzer::HANN:
		return gr::fft::window::hann(ntaps);

	case SpectrumAnalyzer::BLACKMAN_HARRIS:
		return gr::fft::window::blackman_harris(ntaps);

	case SpectrumAnalyzer::KAISER:
		return gr::fft::window::kaiser(ntaps, 0);

	default:
		std::vector<float> v(ntaps, 1.0);
		return v;
	}
}

float
SpectrumChannel::calcCoherentPowerGain(const std::vector<float>& win) const
{
	float sum = 0;

	for (int i = 0; i < win.size(); i++) {
		sum += win[i];
	}

	return sum / win.size();
}

void SpectrumChannel::scaletFftWindow(std::vector<float>& win, float gain)
{
	for (int i = 0; i < win.size(); i++) {
		win[i] *= gain;
	}
}

