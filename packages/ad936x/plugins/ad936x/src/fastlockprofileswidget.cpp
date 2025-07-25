/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "fastlockprofileswidget.h"

#include <QBoxLayout>
#include <style.h>

#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace ad936x;

FastlockProfilesWidget::FastlockProfilesWidget(iio_channel *chn, QWidget *parent)
	: QWidget{parent}
{

	QVBoxLayout *layout = new QVBoxLayout(this);

	m_title = new QLabel("Fastlock Profiles", this);
	layout->addWidget(m_title);

	m_fastlockProfiles = new QComboBox(this);
	layout->addWidget(m_fastlockProfiles);

	// there are 7 fastlock possible slots
	for(int i = 0; i <= 7; i++) {
		m_fastlockProfiles->addItem(QString::number(i), i);
	}

	m_storeBtn = new QPushButton("Store", this);
	m_recallBtn = new QPushButton("Recall", this);

	connect(m_storeBtn, &QPushButton::clicked, this, [=, this]() {
		int currentProfileSetting = m_fastlockProfiles->currentData().toInt();
		iio_channel_attr_write_longlong(chn, "fastlock_store", (long long)currentProfileSetting);
	});

	connect(m_recallBtn, &QPushButton::clicked, this, [=, this]() {
		int currentProfileSetting = m_fastlockProfiles->currentData().toInt();
		iio_channel_attr_write_longlong(chn, "fastlock_recall", (long long)currentProfileSetting);
		Q_EMIT recallCalled();
	});

	Style::setStyle(m_storeBtn, style::properties::button::basicButton);
	Style::setStyle(m_recallBtn, style::properties::button::basicButton);

	QHBoxLayout *btnLayout = new QHBoxLayout();

	btnLayout->addWidget(m_storeBtn);
	btnLayout->addWidget(m_recallBtn);

	layout->addLayout(btnLayout);
}

QComboBox *FastlockProfilesWidget::fastlockProfiles() const { return m_fastlockProfiles; }

QString FastlockProfilesWidget::title() const { return m_title->text(); }

void FastlockProfilesWidget::setTitle(QString &newTitle) { m_title->setText(newTitle); }
