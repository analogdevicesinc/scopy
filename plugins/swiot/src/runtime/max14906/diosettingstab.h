#ifndef DIOSETTINGSTAB_H
#define DIOSETTINGSTAB_H

#include <QWidget>
#include <gui/spinbox_a.hpp>

namespace Ui {
class DioSettingsTab;
}

namespace scopy::swiot {
class DioSettingsTab : public QWidget {
	Q_OBJECT
public:
	explicit DioSettingsTab(QWidget *parent = nullptr);

	~DioSettingsTab() override;

	double getTimeValue() const;

Q_SIGNALS:

	void timeValueChanged(double value);

private:
	Ui::DioSettingsTab *ui;
	PositionSpinButton *m_max_spin_button;
};
}
#endif // DIOSETTINGSTAB_H