#ifndef SCOPY_DIODIGITALCHANNEL_H
#define SCOPY_DIODIGITALCHANNEL_H

#include <QWidget>
#include "ui_swiotdigitalchannel.h"
#include "src/refactoring/maingui/generic_menu.hpp"
#include "core/logging_categories.h"

namespace adiscope::swiot {
class DioDigitalChannel : public adiscope::gui::GenericMenu {
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

	std::vector<std::string> m_configModes;

	QString m_selectedConfigMode;

Q_SIGNALS:
	void outputValueChanged(bool value);
};
}

#endif //SCOPY_DIODIGITALCHANNEL_H
