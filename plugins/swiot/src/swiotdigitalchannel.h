#ifndef SCOPY_SWIOTDIGITALCHANNEL_H
#define SCOPY_SWIOTDIGITALCHANNEL_H

#include <QWidget>
#include "ui_swiotdigitalchannel.h"
#include "src/refactoring/maingui/generic_menu.hpp"
#include <core/logging_categories.h>


namespace adiscope {
class DigitalChannel : public gui::GenericMenu {
	Q_OBJECT
public:
	explicit DigitalChannel(const QString &deviceName, const QString &deviceType, QWidget *parent = nullptr);
	~DigitalChannel() override;

	Ui::DigitalChannel *getUi() const;

	void updateTimeScale(double newMin, double newMax);

	void addDataSample(double value);

	const std::vector<std::string> &getConfigModes() const;

	void setConfigModes(const std::vector<std::string> &configModes);

	const QString &getSelectedConfigMode() const;

	void setSelectedConfigMode(const QString &selectedConfigMode);

private:
	void connectSignalsAndSlots();

	Ui::DigitalChannel *ui;
	QString m_deviceName;
	QString m_deviceType;

	std::vector<std::string> m_configModes;

	QString m_selectedConfigMode;
};
}


#endif //SCOPY_SWIOTDIGITALCHANNEL_H
