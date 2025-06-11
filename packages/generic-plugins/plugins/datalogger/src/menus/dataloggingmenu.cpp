/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "menus/dataloggingmenu.hpp"

#include <QFileDialog>
#include <datamonitorstylehelper.hpp>
#include <menucollapsesection.h>
#include <menusectionwidget.h>
#include <style.h>
#include <timemanager.hpp>
#include <pluginbase/preferences.h>

using namespace scopy;
using namespace datamonitor;

DataLoggingMenu::DataLoggingMenu(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	MenuSectionWidget *logDataContainer = new MenuSectionWidget(this);
	MenuCollapseSection *logDataSection = new MenuCollapseSection(
		"DATA LOGGING", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, logDataContainer);
	logDataSection->contentLayout()->setSpacing(10);

	QWidget *progressFileBrowser = new QWidget(logDataSection);
	QVBoxLayout *progressFileBrowserLay = new QVBoxLayout(progressFileBrowser);
	progressFileBrowserLay->setMargin(0);
	progressFileBrowserLay->setSpacing(1);

	fileBrowser = new FileBrowserWidget(FileBrowserWidget::SAVE_FILE, progressFileBrowser);
	fileBrowser->setFilter(tr("Comma-separated values files (*.csv);;All Files(*)"));
	connect(fileBrowser->btn(), &QPushButton::pressed, this,
		[this]() { liveDataLoggingButton->onOffswitch()->setChecked(false); });

	progressBar = new SmallProgressBar(progressFileBrowser);
	progressFileBrowserLay->addWidget(fileBrowser);
	progressFileBrowserLay->addWidget(progressBar);

	QLineEdit *fileBrowserEdit = fileBrowser->lineEdit();
	fileBrowserEdit->setReadOnly(true);

	liveDataLoggingButton = new MenuOnOffSwitch("Live data logging", logDataSection);

	dataLoggingBtn = new QPushButton("Save data", logDataSection);

	dataLoadingBtn = new QPushButton("Import data", logDataSection);

	toggleButtonsEnabled(false);

	///// time manager timeout used for requesting continuous data logging
	auto &&timeTracker = TimeManager::GetInstance();
	connect(timeTracker, &TimeManager::timeout, this, [=, this]() {
		if(liveDataLoggingButton->onOffswitch()->isChecked()) {
			Q_EMIT requestLiveDataLogging(fileBrowserEdit->text());
		}
	});

	connect(dataLoggingBtn, &QPushButton::clicked, this, [=, this]() {
		updateDataLoggingStatus(ProgressBarState::BUSY);
		Q_EMIT requestDataLogging(fileBrowserEdit->text());
	});

	connect(liveDataLoggingButton->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		m_liveDataLogging = toggled;
		dataLoggingBtn->setEnabled(!toggled);
		dataLoadingBtn->setEnabled(!toggled);
	});

	connect(dataLoadingBtn, &QPushButton::clicked, this, [=, this]() {
		updateDataLoggingStatus(ProgressBarState::BUSY);
		Q_EMIT requestDataLoading(fileBrowserEdit->text());
	});

	connect(fileBrowserEdit, &QLineEdit::textChanged, this, [=, this](QString path) {
		filename = path;
		if(filename.isEmpty() && fileBrowserEdit->isEnabled()) {
			fileBrowserEdit->setText(tr("No file selected"));
			fileBrowserEdit->setStyleSheet("color:red");
			toggleButtonsEnabled(false);

		} else {
			fileBrowserEdit->setStyleSheet("color:white");
			toggleButtonsEnabled(true);
			Q_EMIT pathChanged(path);
		}
	});

	logDataSection->contentLayout()->addWidget(new QLabel("Choose file"));
	logDataSection->contentLayout()->addWidget(progressFileBrowser);
	logDataSection->contentLayout()->addWidget(liveDataLoggingButton);
	logDataSection->contentLayout()->addWidget(dataLoggingBtn);
	logDataSection->contentLayout()->addWidget(dataLoadingBtn);

	logDataContainer->contentLayout()->addWidget(logDataSection);

	mainLayout->addWidget(logDataContainer);

	DataMonitorStyleHelper::DataLoggingMenuStyle(this);
}

void DataLoggingMenu::updateDataLoggingStatus(ProgressBarState status)
{
	if(status == ProgressBarState::SUCCESS) {
		progressBar->setBarColor(Style::getAttribute(json::theme::content_success));
	}
	if(status == ProgressBarState::ERROR) {
		progressBar->setBarColor(Style::getAttribute(json::theme::content_error));
	}
	if(status == ProgressBarState::BUSY) {
		progressBar->startProgress();
		progressBar->setBarColor(Style::getAttribute(json::theme::content_busy));
	}
}

bool DataLoggingMenu::liveDataLogging() const { return m_liveDataLogging; }

void DataLoggingMenu::toggleButtonsEnabled(bool en)
{
	dataLoggingBtn->setEnabled(en);
	dataLoadingBtn->setEnabled(en);
	liveDataLoggingButton->setEnabled(en);
}

#include "moc_dataloggingmenu.cpp"
