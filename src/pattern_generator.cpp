/*
 * Copyright 2016 Analog Devices, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <vector>
#include <string.h>

#include <iio.h>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRgb>
#include <QJsonValue>
#include <QTimer>
#include <QFile>
#include <QtQml/QJSEngine>
#include <QtQml/QQmlEngine>
#include <QDirIterator>
#include <QPushButton>
#include <QFileDialog>

#include <boost/math/common_factor.hpp>

///* pulseview and sigrok */
#include "pulseview/pv/mainwindow.hpp"
#include "pulseview/pv/view/viewport.hpp"
#include "pulseview/pv/devices/binarybuffer.hpp"
#include "pulseview/pv/devicemanager.hpp"
#include "pulseview/pv/toolbars/mainbar.hpp"
#include "pulseview/pv/widgets/colourbutton.hpp"
#include "pulseview/pv/view/tracepalette.hpp"

#include <libsigrokcxx/libsigrokcxx.hpp>
#include <libsigrokdecode/libsigrokdecode.h>

#include "filter.hpp"
#include "pattern_generator.hpp"

// Generated UI
#include "ui_pattern_generator.h"
#include "ui_pg_settings.h"
#include "ui_pg_cg_settings.h"

using namespace std;
using namespace adiscope;

namespace pv {
class MainWindow;
class DeviceManager;
class Session;

namespace toolbars {
class MainBar;
}

namespace widgets {
class DeviceToolButton;
class ColourButton;
}
}

namespace sigrok {
class Context;
}

namespace adiscope {

/*QStringList PatternGenerator::digital_trigger_conditions = QStringList()
        << "edge-rising"
        << "edge-falling"
        << "edge-any"
        << "level-low"
        << "level-high";

*/

QStringList PatternGenerator::possibleSampleRates = QStringList()
                << "100000000"
                << "50000000"   << "20000000"  << "10000000"
                << "5000000"    << "2000000"   << "1000000"
                << "500000"     << "200000"    << "100000"
                << "50000"      << "20000"     << "10000"
                << "5000"       << "2000"      << "1000"
                << "500"        << "200"       << "100"
                << "50"         << "20"        << "10"
                << "5"          << "2"         << "1";

const char *PatternGenerator::channelNames[] = {
	"voltage0", "voltage1", "voltage2", "voltage3",
	"voltage4", "voltage5", "voltage6", "voltage7",
	"voltage8", "voltage9", "voltage10", "voltage11",
	"voltage12", "voltage13", "voltage14", "voltage15"
};

QStringList PatternGenerator::strStatus = QStringList() <<
                "STOPPED"<<"CONFIG"<<"RUNNING"<<"WAITING";

PatternGenerator::runState PatternGenerator::pgStatus()
{
	return _pgStatus;
}

void PatternGenerator::enableBufferUpdates(bool enabled)
{
	if (enabled) {
		connect(getCurrentPatternUI(),SIGNAL(patternParamsChanged()),bufui,
		        SLOT(updateUi()));
	} else {
		disconnect(getCurrentPatternUI(),SIGNAL(patternParamsChanged()),bufui,
		           SLOT(updateUi()));
	}
}

void PatternGenerator::setPGStatus(PatternGenerator::runState val)
{
	_pgStatus = val;
	ui->status->setText(strStatus[val]);

}

PatternGenerator::PatternGenerator(struct iio_context *ctx, Filter *filt,
                                   QPushButton *runBtn, QJSEngine *engine,
                                   DIOManager *diom, ToolLauncher *parent,
                                   bool offline_mode_) :
	Tool(ctx, runBtn, new PatternGenerator_API(this), "Pattern Generator", parent),
	settings_group(new QButtonGroup(this)),
	ui(new Ui::PatternGenerator),
	pgSettings(new Ui::PGSettings),
	cgSettings(new Ui::PGCGSettings),
	txbuf(0), buffer_created(0), currentUI(nullptr), offline_mode(offline_mode_),
	diom(diom)
{
	// IIO
	if (!offline_mode) {
		dev = filt->find_device(ctx,TOOL_PATTERN_GENERATOR);
		this->no_channels = iio_device_get_channels_count(dev);

	}

	// UI
	ui->setupUi(this);
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	this->settings_group->setExclusive(true);
	settings_group->addButton(ui->btnChSettings);
	settings_group->addButton(ui->btnPGSettings);

	PatternFactory::init();

	pgSettings->setupUi(ui->pgSettings);
	cgSettings->setupUi(ui->cgSettings);
	cgSettings->LECHLabel->setMaxLength(channelGroupLabelMaxLength);

	/*Add color buttons */
	colour_button_edge = new pv::widgets::ColourButton(
	        pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
	        cgSettings->colorEdge);
	colour_button_edge->set_palette(pv::view::TracePalette::Colours);
	colour_button_edge->setProperty("type", QVariant("edge"));

	colour_button_BG = new pv::widgets::ColourButton(
	        pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
	        cgSettings->colorBG, true);
	colour_button_BG->set_palette(pv::view::TracePalette::Colours);
	colour_button_BG->setProperty("type", QVariant("background"));

	colour_button_low = new pv::widgets::ColourButton(
	        pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
	        cgSettings->colorLow);
	colour_button_low->set_palette(pv::view::TracePalette::Colours);
	colour_button_low->setProperty("type", QVariant("low"));

	colour_button_high = new pv::widgets::ColourButton(
	        pv::view::TracePalette::Rows, pv::view::TracePalette::Cols,
	        cgSettings->colorHigh);
	colour_button_high->set_palette(pv::view::TracePalette::Colours);
	colour_button_high->setProperty("type", QVariant("high"));

	connect(pgSettings->PB_Autoset,SIGNAL(clicked(bool)),this,
	        SLOT(configureAutoSet()));
	connect(pgSettings->LE_BufferSize,SIGNAL(editingFinished()),this,
	        SLOT(updateBufferSize()));
	connect(pgSettings->LE_SampleRate,SIGNAL(editingFinished()),this,
	        SLOT(updateSampleRate()));
	connect(pgSettings->PB_Reset,SIGNAL(clicked(bool)),this,
	        SLOT(resetPGToDefault()));

	auto i=0;

	for (auto var : PatternFactory::get_ui_list()) {
		cgSettings->CBPattern->addItem(var);
		/*cgSettings->CBPattern->setItemData(i,
						   (PatternFactory::get_description_list())[i],Qt::ToolTipRole);
		i++;*/
	}

	QFontMetrics labelm(cgSettings->CBPattern->font());
	auto label_min_width = labelm.width(QString(12,'X'));
	cgSettings->CBPattern->setMinimumWidth(label_min_width);
	cgSettings->CBPattern->setMaximumWidth(label_min_width);

	connect(ui->btnChSettings, SIGNAL(pressed()), this, SLOT(toggleRightMenu()));
	connect(ui->btnPGSettings, SIGNAL(pressed()), this, SLOT(toggleRightMenu()));
	bufman = new PatternGeneratorBufferManager(&chm);

	bufui = new PatternGeneratorBufferManagerUi(ui->centralWidget,bufman,
	                ui->pgSettings,this);

	connect(bufui,SIGNAL(uiUpdated()),this,SLOT(updatePGettings()));
	connect(bufui,SIGNAL(uiUpdated()),this,SLOT(reloadBufferInDevice()));
	main_win = bufui->getPVWindow();
	chmui = new PatternGeneratorChannelManagerUI(ui->channelManagerWidget, main_win,
	                &chm, cgSettings, this);

	ui->channelManagerWidgetLayout->addWidget(chmui);
	ui->btnChSettings->setChecked(true);
	ui->rightWidget->setCurrentIndex(1);
	singleRunTimer = new QTimer();
	singleRunTimer->setSingleShot(true);
	connect(singleRunTimer,SIGNAL(timeout()),this,SLOT(singleRunStop()));

	connect(cgSettings->CBPattern,SIGNAL(activated(int)),this,
	        SLOT(patternChanged(int)));
	connect(cgSettings->CBOutput,SIGNAL(activated(int)),this,
	        SLOT(outputModeChanged(int)));
	connect(cgSettings->LECHLabel,SIGNAL(textEdited(QString)),this,
	        SLOT(changeName(QString)));
	connect(cgSettings->PBLeft,SIGNAL(pressed()),this,SLOT(pushButtonLeft()));
	connect(cgSettings->PBRight,SIGNAL(pressed()),this,SLOT(pushButtonRight()));
	connect(cgSettings->cmb_thickness, SIGNAL(currentTextChanged(QString)),
	        this, SLOT(changeChannelThickness(QString)));
	connect(cgSettings->btnCollapse, &QPushButton::clicked,
	[=](bool check) {
		cgSettings->widget_2->setVisible(!check);
	});

	connect(cgSettings->btnCollapse2, &QPushButton::clicked,
	[=](bool check) {
		cgSettings->widget_3->setVisible(!check);
	});
	connect(colour_button_edge, SIGNAL(selected(const QColor)),
	        this, SLOT(colorChanged(QColor)));
	connect(colour_button_BG, SIGNAL(selected(const QColor)),
	        this, SLOT(colorChanged(QColor)));
	connect(colour_button_low, SIGNAL(selected(const QColor)),
	        this, SLOT(colorChanged(QColor)));
	connect(colour_button_high, SIGNAL(selected(const QColor)),
	        this, SLOT(colorChanged(QColor)));

	connect(chmui,SIGNAL(channelsChanged()),bufui,SLOT(updateUi()));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)), this, SLOT(startStop(bool)));
	connect(runBtn, SIGNAL(toggled(bool)), ui->btnRunStop, SLOT(setChecked(bool)));
	connect(ui->btnRunStop, SIGNAL(toggled(bool)), runBtn, SLOT(setChecked(bool)));
	connect(ui->btnSingleRun, SIGNAL(pressed()), this, SLOT(singleRun()));

	//main_win->view_->viewport()->disableDrag();

	api->setObjectName(QString::fromStdString(Filter::tool_name(
	                           TOOL_PATTERN_GENERATOR)));
	api->load(*settings);
	api->js_register(engine);
	chm.highlightChannel(chm.get_channel_group(0));
	setPGStatus(STOPPED);
	settingsLoaded();

}

PatternGenerator::~PatternGenerator()
{
	if (!offline_mode) {
		stopPatternGeneration();
	}

	for (auto var : patterns) {
		delete var;
	}

	if (saveOnExit) {
		api->save(*settings);
	}
	delete api;

	delete ui;
	delete bufman;
	srd_exit();
}

void PatternGenerator::settingsLoaded()
{
	chmui->updateUi();
}

void PatternGenerator::generatePattern()
{
	bufman->update();
}

void PatternGenerator::toggleRightMenu(QPushButton *btn)
{
	static rightMenuState rightMenuStatus=OPENED_CG;
	static bool menuOpened = true;
	static bool prevMenuOpened = true;

	settings_group->setExclusive(!btn->isChecked());

	if (btn==ui->btnPGSettings) {
		ui->rightWidget->setCurrentIndex(0);
	} else {
		ui->rightWidget->setCurrentIndex(1);
	}

	prevMenuOpened = menuOpened;

	switch (rightMenuStatus) {
	case CLOSED:
		if (btn==ui->btnPGSettings) {
			rightMenuStatus = OPENED_PG;
		} else {
			rightMenuStatus = OPENED_CG;
		}

		break;

	case OPENED_PG:
		if (btn==ui->btnPGSettings) {
			rightMenuStatus = CLOSED;
		} else {
			rightMenuStatus = OPENED_CG;
		}

		break;

	case OPENED_CG:

		if (btn==ui->btnChSettings) {
			rightMenuStatus = CLOSED;
		} else {
			rightMenuStatus = OPENED_PG;
		}

		break;
	}

	menuOpened = !(rightMenuStatus == CLOSED);
	chmui->showHighlight(menuOpened)
	;

	if (prevMenuOpened != menuOpened) {
		ui->rightMenu->toggleMenu(menuOpened);
	}

	if (!menuOpened) {
		settings_group->setExclusive(false);
		btn->setChecked(true);
	} else {
		settings_group->setExclusive(true);
	}

}

void PatternGenerator::showColorSettings(bool check)
{
	if (check) {
		cgSettings->colorEdge->show();
		cgSettings->colorLow->show();
		cgSettings->colorHigh->show();
		cgSettings->lblEdge->show();
		cgSettings->lblLow->show();
		cgSettings->lblHigh->show();
	} else {
		cgSettings->colorEdge->hide();
		cgSettings->colorLow->hide();
		cgSettings->colorHigh->hide();
		cgSettings->lblEdge->hide();
		cgSettings->lblLow->hide();
		cgSettings->lblHigh->hide();
	}
}


PatternUI *PatternGenerator::getCurrentPatternUI()
{
	return currentUI;
}

void PatternGenerator::enableCgSettings(bool en)
{
	cgSettings->CBOutput->setEnabled(en);
	cgSettings->CBPattern->setEnabled(en);
	cgSettings->cmb_thickness->setEnabled(en);
	cgSettings->btnCollapse->setEnabled(en);
	cgSettings->btnCollapse2->setEnabled(en);
	cgSettings->LECHLabel->setEnabled(en);
	cgSettings->patternSettings->setEnabled(en);
	colour_button_BG->setEnabled(en);
	colour_button_edge->setEnabled(en);
	colour_button_high->setEnabled(en);
	colour_button_low->setEnabled(en);
}

void PatternGenerator::updateCGSettings()
{
	auto chg = chm.getHighlightedChannelGroup();
	auto ch = chm.getHighlightedChannel();

	if (chmui->findUiByChannel(ch)==nullptr
	    && chmui->findUiByChannelGroup(chg)==nullptr) {
		enableCgSettings(false);
		return;
	} else {
		enableCgSettings(true);
	}

	QString title;
	QString name;
	qreal thickness;
	QString strMode;

	bool mode;
	bool mixed=false;


	if (ch==nullptr) {
		name = QString::fromStdString(chg->get_label());
		title = name;
		thickness = chg->getCh_thickness();

		mode = chg->get_channel(0)->getOutputMode();

		if (chg->is_grouped()) {
			showColorSettings(false);

			if (cgSettings->CBOutput->findText("Mix")==-1) {
				cgSettings->CBOutput->addItem("Mix");
			}

			for (int i=0; i<chg->get_channel_count(); i++)
				if (chg->get_channel(i)->getOutputMode()!=mode) {
					mixed=true;
				}

		} else {

			if (cgSettings->CBOutput->findText("Mix")!=-1) {
				cgSettings->CBOutput->removeItem(cgSettings->CBOutput->findText("Mix"));
			}

			showColorSettings(true);
		}

		colour_button_BG->set_colour(chg->getBgcolor());
		colour_button_edge->set_colour(chg->getEdgecolor());
		colour_button_high->set_colour(chg->getHighcolor());
		colour_button_low->set_colour(chg->getLowcolor());

	} else {


		if (cgSettings->CBOutput->findText("Mix")!=-1) {
			cgSettings->CBOutput->removeItem(cgSettings->CBOutput->findText("Mix"));
		}

		name = QString::fromStdString(ch->get_label());
		auto id = QString::number(ch->get_id());
		title = "Channel " + id;
		mode = ch->getOutputMode();
		thickness = ch->getCh_thickness();
		showColorSettings(true);
		colour_button_BG->set_colour(ch->getBgcolor());
		colour_button_edge->set_colour(ch->getEdgecolor());
		colour_button_high->set_colour(ch->getHighcolor());
		colour_button_low->set_colour(ch->getLowcolor());
	}

	colour_button_high->update();
	colour_button_low->update();
	colour_button_edge->update();
	colour_button_BG->update();

	if (mixed) {
		strMode="Mix";
	} else {
		if (mode) {
			strMode = "OD";
		} else {
			strMode = "PP";
		}
	}

	auto pattern = QString::fromStdString(chg->pattern->get_name());

	cgSettings->LTitle->setText(title);
	cgSettings->CBPattern->setCurrentText(pattern);

	cgSettings->CBOutput->setCurrentText(strMode);

	cgSettings->LECHLabel->setText(name);
	cgSettings->LPattern->setText(pattern);
	cgSettings->cmb_thickness->setCurrentText(QString::number(thickness));


	deleteSettingsWidget();
	createSettingsWidget();
}

void PatternGenerator::outputModeChanged(int index)
{
	auto chg = chm.getHighlightedChannelGroup();
	auto ch = chm.getHighlightedChannel();

	if (index==2) {
		return;
	}

	if (ch) {
		ch->setOutputMode(index);
		diom->setOutputMode(ch->get_id(),index);
	} else {
		for (int i=0; i<chg->get_channel_count(); i++) {
			chg->get_channel(i)->setOutputMode(index);
			diom->setOutputMode(chg->get_channel(i)->get_id(),index);
		}
	}

}

void PatternGenerator::patternChanged(int index)
{
	auto chg = chm.getHighlightedChannelGroup();
	chg->pattern->deinit();
	delete chg->pattern;
	chg->pattern = PatternFactory::create(index);

	deleteSettingsWidget();
	createSettingsWidget();
	Q_EMIT currentUI->decoderChanged();
	bufui->updateUi();
}

void PatternGenerator::deleteSettingsWidget()
{
	if (currentUI!=nullptr) {
		currentUI->destroy_ui();
		currentUI->deleteLater();
		currentUI = nullptr;
	}
}

void PatternGenerator::createSettingsWidget()
{
	auto chg = chm.getHighlightedChannelGroup();
	cgSettings->LPattern->setText(QString::fromStdString(
	                                      chg->pattern->get_name()).toUpper());
	currentUI = PatternFactory::create_ui(chg->pattern);
	currentUI->build_ui(cgSettings->patternSettings,chg->get_channel_count());
	currentUI->get_pattern()->init();
	currentUI->post_load_ui();
	currentUI->setVisible(true);

	connect(currentUI,SIGNAL(decoderChanged()),chmui,SLOT(triggerUpdateUi()));

	if (chg->is_enabled()) {
		enableBufferUpdates(true);
	}
}

void PatternGenerator::colorChanged(QColor color)
{
	auto chg = chm.getHighlightedChannelGroup();
	auto ch = chm.getHighlightedChannel();

	auto chgui = chmui->findUiByChannelGroup(chg);
	auto chui = chmui->findUiByChannel(ch);

	QPushButton *sender = static_cast<QPushButton *>(QObject::sender());
	QString type = sender->property("type").toString();

	if (type == "background") {
		if (!chui) {
			chg->setBgcolor(color);
			chgui->updateTrace();
		} else {
			ch->setBgcolor(color);
			chui->updateTrace();
		}
	} else if (type == "edge") {
		if (!chui) {
			chg->setEdgecolor(color);
			chgui->updateTrace();
		} else {
			ch->setEdgecolor(color);
			chui->updateTrace();
		}
	} else if (type == "low") {
		if (!chui) {
			chg->setLowcolor(color);
			chgui->updateTrace();
		} else {
			ch->setLowcolor(color);
			chui->updateTrace();
		}
	} else if (type == "high") {
		if (!chui) {
			chg->setHighcolor(color);
			chgui->updateTrace();
		} else {
			ch->setHighcolor(color);
			chui->updateTrace();
		}
	}
}

void PatternGenerator::changeChannelThickness(QString text)
{
	bool ok;
	double value = text.toDouble(&ok);

	if (!ok) {
		return;
	}

	auto chg = chm.getHighlightedChannelGroup();
	auto ch = chm.getHighlightedChannel();

	auto chgui = chmui->findUiByChannelGroup(chg);
	auto chui = chmui->findUiByChannel(ch);

	if (ch==nullptr) {
		chg->setCh_thickness(value);

		if (chg->is_grouped()) {
			for (auto c : chgui->ch_ui) {
				c->updateTrace();
			}
		} else {
			chgui->updateTrace();
		}
	} else {
		ch->setCh_thickness(value);
		chui->updateTrace();
	}
}

void PatternGenerator::changeName(QString name)
{
	auto chg = chm.getHighlightedChannelGroup();
	auto ch = chm.getHighlightedChannel();

	auto chgui = chmui->findUiByChannelGroup(chg);
	auto chui = chmui->findUiByChannel(ch);

	if (ch==nullptr) {
		chg->set_label(name.toStdString());

		if (!chg->is_grouped()) {
			chg->get_channel(0)->set_label(name.toStdString());
		}

		chgui->ui->ChannelGroupLabel->setText(name);
	} else {
		ch->set_label(name.toStdString());
		chui->ui->ChannelGroupLabel->setText(name);
	}

	updateCGSettings();
}

void PatternGenerator::pushButtonLeft()
{

	auto chg = chm.getHighlightedChannelGroup();
	auto iter = std::find(chm.get_channel_groups()->begin(),
	                      chm.get_channel_groups()->end(),chg);

	if (iter!=chm.get_channel_groups()->begin()) {
		iter--;
	}

	/*if(newHighlight<chm.get_channel_group_count())
		newHighlight++;*/
	chmui->showHighlight(false);
	chmui->highlightChannel(static_cast<PatternGeneratorChannelGroup *>(*iter));
	chmui->showHighlight(true);
}

void PatternGenerator::pushButtonRight()
{
	auto chg = chm.getHighlightedChannelGroup();
	auto iter = std::find(chm.get_channel_groups()->begin(),
	                      chm.get_channel_groups()->end(),chg);

	if (iter+1!=chm.get_channel_groups()->end()) {
		iter++;
	}

	chmui->showHighlight(false);
	chmui->highlightChannel(static_cast<PatternGeneratorChannelGroup *>(*iter));
	chmui->showHighlight(true);
}

void PatternGenerator::configureAutoSet()
{
	QPushButton *autoSetToggle = static_cast<QPushButton *>(QObject::sender());
	bool autoSet = autoSetToggle->isChecked();

	if (!autoSet) {
		pgSettings->LE_BufferSize->setEnabled(true);
		pgSettings->LE_SampleRate->setEnabled(true);
	} else {
		pgSettings->LE_BufferSize->setEnabled(false);
		pgSettings->LE_SampleRate->setEnabled(false);
	}

	bufman->enableAutoSet(autoSet);
}

void PatternGenerator::updatePGettings()
{
	pgSettings->LE_BufferSize->setText(QString::number(bufman->getBufferSize()));
	pgSettings->LE_SampleRate->setText(QString::number(bufman->getSampleRate()));
}

void PatternGenerator::updateSampleRate()
{
	bool ok;
	QString str = static_cast<QLineEdit *>(QObject::sender())->text();
	uint32_t val = str.toLong(&ok);

	if (ok) {
		bufman->setSampleRate(val);
	}
}

void PatternGenerator::updateBufferSize()
{
	bool ok;
	QString str = static_cast<QLineEdit *>(QObject::sender())->text();
	uint32_t val = str.toLong(&ok);

	if (ok) {
		bufman->setBufferSize(val);
	}
}




























void PatternGenerator::reloadBufferInDevice()
{
	if (pgStatus()!=STOPPED) {
		stopPatternGeneration();
		startPatternGeneration(true);
	}
}

bool PatternGenerator::startPatternGeneration(bool cyclic)
{
	/* Enable Tx channels*/
	//char temp_buffer[12];

	if (offline_mode) {
		return true;
	}

	setPGStatus(CONFIG);

	if (!dev) {
		qDebug("Devices not found");
		return false;
	}

	qDebug("Enabling channels");

	for (int j = 0; j < no_channels; j++) {
		if (chm.get_enabled_mask() & (1<<j)) {
			qDebug()<<"enabled channel - "<<j<<"\n";
			auto ch = iio_device_find_channel(dev, channelNames[j], true);
			iio_channel_enable(ch);
		}
	}

	qDebug("Setting channel direction");
	diom->setMode(chm.get_mode_mask());
	diom->lock(chm.get_enabled_mask());

	qDebug("Setting sample rate");
	iio_device_attr_write(dev, "sampling_frequency",
	                      std::to_string(bufman->getSampleRate()).c_str());

	qDebug("Creating buffer");
	txbuf = iio_device_create_buffer(dev, bufman->getBufferSize(), cyclic);

	if (!txbuf) {
		qDebug("Could not create buffer - errno: %d - %s", errno, strerror(errno));
		return false;
	}

	buffer_created = true;
	short *p_dat;
	ptrdiff_t p_inc;

	int i = 0;

	for (p_dat = (short *)iio_buffer_start(txbuf); (p_dat < iio_buffer_end(txbuf));
	     (uint16_t *)p_dat++,i++) {
		*p_dat = bufman->buffer[i];
	}

	/* Push buffer       */
	auto number_of_bytes = iio_buffer_push(txbuf);
	qDebug("\nPushed %ld bytes to devices\r\n",number_of_bytes);
	setPGStatus(RUNNING);
	return true;
}

void PatternGenerator::stopPatternGeneration()
{
	/* Destroy buffer */
	if (!offline_mode) {

		/* Reset Tx Channls*/
		diom->unlock();

		if (buffer_created == true) {
			iio_buffer_destroy(txbuf);
			buffer_created = false;
		}

		for (int j = 0; j < no_channels; j++) {
			auto ch = iio_device_find_channel(dev, channelNames[j], true);
			iio_channel_disable(ch);
		}
	}

	setPGStatus(STOPPED);

}

void PatternGenerator::startStop(bool start)
{
//	main_win->action_view_zoom_fit()->trigger();

	if (start) {
		ui->btnRunStop->setText("Stop");
		if(singleRunTimer->isActive())
		{
			singleRunTimer->stop();
			singleRunStop();
		}
		if (startPatternGeneration(true)) {
			setPGStatus(RUNNING);
		} else {
			qDebug("Pattern generation failed");
			setPGStatus(WAITING);
		}
	} else {
		stopPatternGeneration();
		setPGStatus(STOPPED);
		ui->btnRunStop->setText("Run");
	}
}

void PatternGenerator::singleRun()
{
	main_win->action_view_zoom_fit()->trigger();
	stopPatternGeneration();
	ui->btnRunStop->setChecked(false);

	if (startPatternGeneration(false)) {
		uint32_t time_until_buffer_destroy = 500 + (uint32_t)(((
		                bufman->getBufferSize()/2)/((
		                                float)bufman->getSampleRate()))*1000.0);
		qDebug("Time until buffer destroy %d", time_until_buffer_destroy);

		singleRunTimer->setInterval(time_until_buffer_destroy);
		singleRunTimer->start();

		qDebug("Pattern generation single started");
		ui->btnSingleRun->setChecked(false);
	} else {
		qDebug("Pattern generation failed");
		ui->btnSingleRun->setChecked(true);
		setPGStatus(STOPPED);
	}
}

void PatternGenerator::singleRunStop()
{
	qDebug("Pattern Generation stopped ");
	stopPatternGeneration();
	ui->btnSingleRun->setChecked(false);
}


void PatternGenerator::toggleRightMenu()
{
	toggleRightMenu(static_cast<QPushButton *>(QObject::sender()));
}

void PatternGenerator::on_btnHideInactive_clicked()
{
	if (chmui->isDisabledShown()) {
		chmui->hideDisabled();
		ui->btnHideInactive->setText("Show All");
	} else {
		chmui->showDisabled();
		ui->btnHideInactive->setText("Hide Inactive");

	}

	chmui->updateUi();
}

void PatternGenerator::on_btnGroupWithSelected_clicked()
{
	chmui->groupSplitSelected();
	chmui->updateUi();
}

QString PatternGenerator::toString()
{
	QJsonObject obj;
	obj["chm"] = chmToJson();
	QJsonDocument doc(obj);
	QString ret(doc.toJson(QJsonDocument::Compact));
	return ret;
}



void PatternGenerator::fromString(QString val)
{
	QJsonObject obj;
	QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());

	if (!doc.isNull()) {
		if (doc.isObject()) {
			obj = doc.object();
		} else {
			qDebug() << "Document is not an object" << endl;
		}
	} else {
		qDebug() << "Invalid JSON...\n";
	}

	jsonToChm(obj);
}


QJsonValue PatternGenerator::chmToJson()
{
	QJsonObject obj;
	QJsonArray chgArray;

	for (auto i=0; i<chm.get_channel_group_count(); i++) {
		QJsonObject chgObj;
		PatternGeneratorChannelGroup *chg = chm.get_channel_group(i);

		chgObj["label"] = QString::fromStdString(chg->get_label());
		chgObj["grouped"] = chg->is_grouped();
		chgObj["enabled"] = chg->is_enabled();
		chgObj["collapsed"] = chg->isCollapsed();
		chgObj["pattern"] = Pattern_API::toJson(chg->pattern);

		auto chCount = chg->get_channel_count();
		QJsonArray chArray;

		for (auto j=0; j<chCount; j++) {
			QJsonObject chObj;
			chObj["id"] = chg->get_channel(j)->get_id();
			chObj["label"] = QString::fromStdString(chg->get_channel(j)->get_label());
			chArray.insert(j,QJsonValue(chObj));
		}

		chgObj["channels"] = chArray;
		chgArray.insert(i,chgObj);

	}

	obj["channel_groups"] = chgArray;
	QJsonValue val(obj);
	return val;
}

void PatternGenerator::jsonToChm(QJsonObject obj)
{
	chm.clearChannelGroups();
	QJsonArray chgArray = obj["chm"].toObject()["channel_groups"].toArray();

	for (auto chgRef : chgArray) {
		auto chg = chgRef.toObject();
		PatternGeneratorChannelGroup *pgchg = new PatternGeneratorChannelGroup();
		pgchg->set_label(chg["label"].toString().toStdString());
		pgchg->group(chg["grouped"].toBool());
		pgchg->enable(chg["enabled"].toBool());
		pgchg->collapse(chg["collapsed"].toBool());

		pgchg->pattern->deinit();
		delete pgchg->pattern;
		pgchg->pattern = Pattern_API::fromJson(chg["pattern"].toObject());

		QJsonArray chArray = chg["channels"].toArray();
		int i=0;

		for (auto chRef : chArray) {
			auto ch = chRef.toObject();
			int chIndex = ch["id"].toInt();
			QString chLabel = ch["label"].toString();
			pgchg->add_channel(chm.get_channel(chIndex));
			pgchg->get_channel(i)->set_label(chLabel.toStdString());
			i++;
		}

		chm.add_channel_group(pgchg);
	}

	chm.highlightChannel(chm.get_channel_group(0));
}


void PatternGenerator::resetPGToDefault()
{
	chm.clearChannels();
	chm.clearChannelGroups();

	for (int i=0; i<16; i++) {
		chm.add_channel_group(new PatternGeneratorChannelGroup(chm.get_channel(i)));
	}

	chm.highlightChannel(chm.get_channel_group(0));
	chmui->updateUi();
}

void PatternGenerator_API::refreshApi()
{
	PatternGeneratorChannelManagerUI *chmui = pg->chmui;
	PatternGeneratorChannelManager *chm = chmui->chm;

	pg_cga.clear();
	pg_cha.clear();

	for (auto i=0; i<chm->get_channel_group_count(); i++) {
		PatternGeneratorChannelGroup *chg = chm->get_channel_group(i);
		pg_cga.append(new PatternGeneratorChannelGroup_API(chg,chmui));
	}

	for (auto i=0; i<chm->get_channel_count(); i++) {
		PatternGeneratorChannel *cha = chm->get_channel(i);
		pg_cha.append(new PatternGeneratorChannel_API(cha));
	}
}


void PatternGeneratorChannel_API::setColor(QList<int> list)
{
	//	ch->set_id(val);
	ch->setBgcolor(QColor((list[0]&0xff0000)>>16,(list[0]&0xff00)>>8,list[0]&0xff));
	ch->setEdgecolor(QColor((list[1]&0xff0000)>>16,(list[1]&0xff00)>>8,list[1]&0xff));
	ch->setHighcolor(QColor((list[2]&0xff0000)>>16,(list[2]&0xff00)>>8,list[2]&0xff));
	ch->setLowcolor(QColor((list[3]&0xff0000)>>16,(list[3]&0xff00)>>8,list[3]&0xff));

}
QList<int> PatternGeneratorChannel_API::color()
{
	QList<int> list;

	list.append(ch->getBgcolor().red()<<16   | ch->getBgcolor().green()<<8   | ch->getBgcolor().blue());
	list.append(ch->getEdgecolor().red()<<16 | ch->getEdgecolor().green()<<8 | ch->getEdgecolor().blue());
	list.append(ch->getHighcolor().red()<<16 | ch->getHighcolor().green()<<8 | ch->getHighcolor().blue());
	list.append(ch->getLowcolor().red()<<16  | ch->getLowcolor().green()<<8  | ch->getLowcolor().blue());
	return list;
}



void PatternGeneratorChannelGroup_API::setColor(QList<int> list)
{
	//	ch->set_id(val);
	chg->setBgcolor(QColor((list[0]&0xff0000)>>16,(list[0]&0xff00)>>8,list[0]&0xff));
	chg->setEdgecolor(QColor((list[1]&0xff0000)>>16,(list[1]&0xff00)>>8,list[1]&0xff));
	chg->setHighcolor(QColor((list[2]&0xff0000)>>16,(list[2]&0xff00)>>8,list[2]&0xff));
	chg->setLowcolor(QColor((list[3]&0xff0000)>>16,(list[3]&0xff00)>>8,list[3]&0xff));

}
QList<int> PatternGeneratorChannelGroup_API::color()
{
	QList<int> list;

	list.append(chg->getBgcolor().red()<<16 | chg->getBgcolor().green()<<8 | chg->getBgcolor().blue());
	list.append(chg->getEdgecolor().red()<<16 | chg->getEdgecolor().green()<<8 | chg->getEdgecolor().blue());
	list.append(chg->getHighcolor().red()<<16 | chg->getHighcolor().green()<<8 | chg->getHighcolor().blue());
	list.append(chg->getLowcolor().red()<<16 | chg->getLowcolor().green()<<8 | chg->getLowcolor().blue());
	return list;
}


QList<int> PatternGeneratorChannelGroup_API::channels()
{
	QList<int> list;

	for (auto i=0; i<chg->get_channel_count(); i++) {
		list.append(chg->get_channel(i)->get_id());
	}

	return list;
}

void PatternGeneratorChannelGroup_API::setChannels(QList<int> list)
{
	while (chg->get_channel_count()) {
		chg->remove_channel(0);
	}

	for (auto val : list) {
		chg->add_channel(chm->get_channel(val));
	}
}

QString PatternGeneratorChannelGroup_API::pattern()
{
	return Pattern_API::toString(chg->pattern);
}

void PatternGeneratorChannelGroup_API::setPattern(QString str)
{
	chg->pattern->deinit();
	delete chg->pattern;
	chg->pattern = Pattern_API::fromString(str);
}

QVariantList PatternGenerator_API::getChannelGroups()
{
	QVariantList list;
	refreshApi();

	for (auto *each : pg_cga) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}

int PatternGenerator_API::channel_groups_size()
{
	refreshApi();
	return pg_cga.size();
}

void PatternGenerator_API::set_channel_groups_size(int val)
{
	PatternGeneratorChannelManager *chm = pg->chmui->chm;

	chm->clearChannelGroups();

	for (auto i=0; i<val; i++) {
		chm->add_channel_group(new PatternGeneratorChannelGroup());
	}
}

int PatternGenerator_API::channel_size()
{
	return pg_cha.size();
}

void PatternGenerator_API::set_channel_size(int val)
{
	PatternGeneratorChannelManager *chm = pg->chmui->chm;
	chm->clearChannels();
}

QVariantList PatternGenerator_API::getChannels()
{
	QVariantList list;
	refreshApi();

	for (auto *each : pg_cha) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}


bool PatternGenerator_API::running() const
{
	return pg->ui->btnRunStop->isChecked();
}
void PatternGenerator_API::run(bool en)
{
	pg->ui->btnRunStop->setChecked(en);
}

bool PatternGenerator_API::single() const
{
	return pg->ui->btnSingleRun->isChecked();
}
void PatternGenerator_API::run_single(bool en)
{
	pg->ui->btnSingleRun->setChecked(en);
}

bool PatternGenerator_API::inactiveHidden()
{
	return pg->chmui->isDisabledShown();
}

void PatternGenerator_API::setInactiveHidden(bool en)
{
	if(en)
		pg->chmui->showDisabled();
	else
		pg->chmui->hideDisabled();
}

}
