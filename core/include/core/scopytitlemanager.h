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

#ifndef SCOPY_SCOPYTITLEMANAGER_H
#define SCOPY_SCOPYTITLEMANAGER_H

#include <QWidget>
#include <mutex>

namespace scopy {
class ScopyTitleManager : public QObject
{
	Q_OBJECT
protected:
	ScopyTitleManager(QObject *parent = nullptr);
	~ScopyTitleManager();

	void buildTitle();

public:
	ScopyTitleManager(ScopyTitleManager &other) = delete;
	void operator=(const ScopyTitleManager &other) = delete;

	static ScopyTitleManager *GetInstance();

	static void setApplicationName(QString title);
	static void clearApplicationName();

	static void setScopyVersion(QString version);
	static void clearScopyVersion();

	static void setGitHash(QString hash);
	static void clearGitHash();

	static void setIniFileName(QString filename);
	static void clearIniFileName();

	static void clearAll();
	static QString getCurrentTitle();

	static void setMainWindow(QWidget *window);

private:
	static ScopyTitleManager *pinstance_;
	QString m_title;
	QString m_version;
	QString m_hash;
	QString m_filename;
	QString m_currentTitle;
	QWidget *m_mainWindow;
};
} // namespace scopy

#endif // SCOPY_SCOPYTITLEMANAGER_H
