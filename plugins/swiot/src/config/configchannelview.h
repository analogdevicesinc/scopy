#ifndef CONFIGCHANNELVIEW_H
#define CONFIGCHANNELVIEW_H

#include "qwidget.h"

#include <QObject>
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>

namespace scopy::swiot {
class ConfigChannelView: public QWidget
{
	Q_OBJECT
public:
	explicit ConfigChannelView(int channelIndex, QWidget *parent = nullptr);
	~ConfigChannelView();

	bool isChannelEnabled() const;
	void setChannelEnabled(bool channelEnabled);

	const QString &getSelectedDevice() const;
	void setSelectedDevice(const QString &selectedDevice);

	const QStringList &getDeviceAvailable() const;
	void setDeviceAvailable(const QStringList &deviceAvailable);

	const QString &getSelectedFunction() const;
	void setSelectedFunction(const QString &selectedFunction);

	const QStringList &getFunctionAvailable() const;
	void setFunctionAvailable(const QStringList &functionAvailable);

	void connectSignalsAndSlots();

	QLabel *getChannelLabel() const;
	QComboBox *getDeviceOptions() const;
	QComboBox *getFunctionOptions() const;
	QCheckBox *getEnabledCheckBox() const;

Q_SIGNALS:
	void enabledChanged(int channelIndex, bool enabled);
	void deviceChanged(int channelIndex, const QString& device);
	void functionChanged(int channelIndex, const QString& function);

private:
	int m_channelIndex;
	bool m_channelEnabled;
	QString m_selectedDevice;
	QStringList m_deviceAvailable;

	QString m_selectedFunction;
	QStringList m_functionAvailable;

	// UI elements
	QLabel* channelLabel;

	QComboBox* deviceOptions;
	QComboBox* functionOptions;
	QCheckBox* enabledCheckBox;
//	Ui::ConfigChannelView* m_ui;
};
}

#endif // CONFIGCHANNELVIEW_H
