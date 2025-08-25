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

#ifndef SETTINGSMENU_H
#define SETTINGSMENU_H

#include <QWidget>
#include <analysismenu.h>
#include <buffermenu.h>

namespace scopy::qiqplugin {

class SettingsMenu : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsMenu(QWidget *parent = nullptr);

	void setAvailableChannels(const QMap<QString, QList<ChannelInfo>> &channels);
	void setAnalysisTypes(const QStringList &types);
	void setAnalysisParams(const QString &type, const QVariantMap &params);
	void validateAnalysisParams(const QString &type, const QVariantMap &config);
Q_SIGNALS:
	void analysisChanged(const QString &type);
	void analysisConfig(const QString &type, const QVariantMap &inConfig);
	void bufferParamsChanged(const BufferParams &params);

private Q_SLOTS:
	void onAnalysisApply();

private:
	BufferMenu *m_bufferMenu;
	AnalysisMenu *m_analysisMenu;
	MenuCombo *m_analysisCb;

	void setupUI();
};
} // namespace scopy::qiqplugin
#endif // SETTINGSMENU_H
