#ifndef SCOPY_DIODIGITALCHANNEL_H
#define SCOPY_DIODIGITALCHANNEL_H

#include <QWidget>
#include "ui_diodigitalchannel.h"
#include <gui/generic_menu.hpp>
#include <QDockWidget>

namespace scopy::swiot {
class DioDigitalChannel : public QWidget {
	Q_OBJECT
public:
	explicit DioDigitalChannel(const QString &deviceName, const QString &deviceType, QWidget *parent = nullptr);
	~DioDigitalChannel() override;

	void updateTimeScale(double newMax);
	void addDataSample(double value);
	const std::vector<std::string> *getConfigModes() const;
	void setConfigModes(std::vector<std::string> *configModes);
	const QString &getSelectedConfigMode() const;
	void setSelectedConfigMode(const QString &selectedConfigMode);
	void resetSismograph();

private:
	void connectSignalsAndSlots();

	Ui::DioDigitalChannel *ui;
	QString m_deviceName;
	QString m_deviceType;

	std::vector<std::string> *m_configModes;
	QString m_selectedConfigMode;

	friend class DioDigitalChannelController;

Q_SIGNALS:
	void outputValueChanged(bool value);
};
}

#endif //SCOPY_DIODIGITALCHANNEL_H
