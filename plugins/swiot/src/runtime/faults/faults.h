#ifndef FAULTS_H
#define FAULTS_H

#include "pluginbase/toolmenuentry.h"
#include "ui_swiotfaults.h"

#include "faultspage.h"
#include <gui/tool_view.hpp>
#include <gui/generic_menu.hpp>
#include <gui/channel_manager.hpp>

#include <iio.h>

namespace scopy::swiot {
class Faults : public QWidget {
	Q_OBJECT
public:
	explicit Faults(struct iio_context *ctx, ToolMenuEntry *tme, QWidget *parent = nullptr);
	~Faults();

	void pollFaults();

Q_SIGNALS:
        void backBtnPressed();

public Q_SLOTS:
	void runButtonClicked();
	void singleButtonClicked();

	void externalPowerSupply(bool ps);

private:
        void connectSignalsAndSlots();
        void setupDynamicUi(QWidget *parent);
        static QPushButton* createBackButton();

	struct iio_context* ctx;

	Ui::Faults *ui;
        QPushButton *m_backButton;
	QLabel* m_statusLabel;
	QWidget* m_statusContainer;

	QTimer *timer;
	QThread *thread;

	uint32_t ad74413r_numeric;
	uint32_t max14906_numeric;

	scopy::gui::ToolView *m_toolView{};
	FaultsPage *m_faultsPage;

	ToolMenuEntry *m_tme;
};
}
#endif // FAULTS_H
