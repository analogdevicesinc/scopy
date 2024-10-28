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

#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include "scopy-dac_export.h"
#include <QWidget>
#include <QPushButton>

#include <gui/widgets/progresslineedit.h>

namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT FileBrowser : public QWidget
{
	Q_OBJECT
public:
	explicit FileBrowser(QWidget *parent = nullptr);
	~FileBrowser();
	QString getFilePath() const;
	void setDefaultDir(QString dir);
Q_SIGNALS:
	void load(QString path);
private Q_SLOTS:
	void chooseFile();
	void loadFile();

private:
	ProgressLineEdit *m_fileBufferPath;
	QPushButton *m_fileBufferBrowseBtn;
	QPushButton *m_fileBufferLoadBtn;
	QString m_filename;
	QString m_defaultDir;
};
} // namespace dac
} // namespace scopy

#endif // FILEBROWSER_H
