/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef CONFIGCHANNELVIEW_H
#define CONFIGCHANNELVIEW_H

#include <QWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QObject>
#include <QString>

#define DEVICE_COMBO_WIDTH 105
#define FUNCTION_COMBO_WIDTH 175

namespace scopy::swiotrefactor {
class ConfigChannelView : public QWidget
{
	Q_OBJECT
public:
	explicit ConfigChannelView(int channelIndex, QWidget *parent = nullptr);
	~ConfigChannelView();

	bool isChannelEnabled() const;
	void setChannelEnabled(bool channelEnabled);

	const QString &getSelectedDevice() const;
	void setSelectedDevice(const QString &selectedDevice);

protected:
	bool eventFilter(QObject *object, QEvent *event) override;

public:
	const QStringList &getDeviceAvailable() const;
	void setDeviceAvailable(const QStringList &deviceAvailable);

	const QString &getSelectedFunction() const;
	void setSelectedFunction(const QString &selectedFunction);

	const QStringList &getFunctionAvailable() const;
	void setFunctionAvailable(const QStringList &functionAvailable);

	void connectSignalsAndSlots();

Q_SIGNALS:
	void enabledChanged(int channelIndex, bool enabled);
	void deviceChanged(int channelIndex, const QString &device);
	void functionChanged(int channelIndex, const QString &function);

	void showPath(int channelIndex, const QString &device);
	void hidePaths();

private:
	int m_channelIndex;
	bool m_channelEnabled;
	QString m_selectedDevice;
	QStringList m_deviceAvailable;

	QString m_selectedFunction;
	QStringList m_functionAvailable;

	QLabel *m_chnlLabel;
	QCheckBox *m_chnlCheck;
	QComboBox *m_deviceOptions;
	QComboBox *m_functionOptions;

	void setHighlightPalette();
	void initTutorialProperties();
};
} // namespace scopy::swiotrefactor

#endif // CONFIGCHANNELVIEW_H
