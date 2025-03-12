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

#include "filebrowserwidget.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <style.h>
#include <stylehelper.h>

#include <pluginbase/preferences.h>

using namespace scopy;

FileBrowserWidget::FileBrowserWidget(BrowserDialogType type, QWidget *parent)
	: QFrame(parent)
	, m_type(type)
	, m_filter("")
	, m_baseDirectory("")
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setMargin(0);

	m_lineEdit = new QLineEdit(this);

	m_btn = new QPushButton(this);
	StyleHelper::BrowseButton(m_btn);

	lay->addWidget(m_lineEdit);
	lay->addWidget(m_btn);

	connect(m_btn, &QPushButton::pressed, this, &FileBrowserWidget::browseFile);
}

FileBrowserWidget::~FileBrowserWidget() {}

QLineEdit *FileBrowserWidget::lineEdit() const { return m_lineEdit; }

QPushButton *FileBrowserWidget::btn() const { return m_btn; }

void FileBrowserWidget::browseFile()
{
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString filePath;
	switch(m_type) {
	case FileBrowserWidget::BrowserDialogType::OPEN_FILE:
		filePath = QFileDialog::getOpenFileName(
			this, tr("Open file"), m_baseDirectory, m_filter, nullptr,
			(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
		break;
	case FileBrowserWidget::BrowserDialogType::SAVE_FILE:
		filePath = QFileDialog::getSaveFileName(
			this, tr("Save file"), m_baseDirectory, m_filter, nullptr,
			(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
		break;
	case FileBrowserWidget::BrowserDialogType::DIRECTORY:
		filePath = QFileDialog::getExistingDirectory(this, tr("Choose a directory"), m_baseDirectory,
							     (useNativeDialogs ? QFileDialog::Options()
									       : QFileDialog::ShowDirsOnly |
									      QFileDialog::DontResolveSymlinks |
									      QFileDialog::DontUseNativeDialog));
		break;
	default:
		filePath = "";
		break;
	}
	m_lineEdit->setText(filePath);
}

void FileBrowserWidget::setFilter(const QString &newFilter) { m_filter = newFilter; }

void FileBrowserWidget::setBaseDirectory(const QString &newBaseDirectory) { m_baseDirectory = newBaseDirectory; }
