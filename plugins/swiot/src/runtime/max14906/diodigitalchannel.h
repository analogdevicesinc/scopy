#ifndef SCOPY_DIODIGITALCHANNEL_H
#define SCOPY_DIODIGITALCHANNEL_H

#include <QWidget>
#include "ui_swiotdigitalchannel.h"
#include <gui/generic_menu.hpp>

namespace scopy::swiot {
class DioDigitalChannel : public scopy::gui::GenericMenu {
	Q_OBJECT
public:
	explicit DioDigitalChannel(const QString &deviceName, const QString &deviceType, QWidget *parent = nullptr);
	~DioDigitalChannel() override;

	Ui::DioDigitalChannel *getUi() const;

	void updateTimeScale(double newMin, double newMax);

	void addDataSample(double value);

	const std::vector<std::string> &getConfigModes() const;

	void setConfigModes(const std::vector<std::string> &configModes);

	const QString &getSelectedConfigMode() const;

	void setSelectedConfigMode(const QString &selectedConfigMode);

	void resetSismograph();

private:
	void connectSignalsAndSlots();

	Ui::DioDigitalChannel *ui;
	QString m_deviceName;
	QString m_deviceType;

        QPixmap m_highSnow, m_highGray, m_lowSnow, m_lowGray;
        QSize m_pixmapSize;

	std::vector<std::string> m_configModes;

	QString m_selectedConfigMode;

Q_SIGNALS:
	void outputValueChanged(bool value);
};
}

#endif //SCOPY_DIODIGITALCHANNEL_H
