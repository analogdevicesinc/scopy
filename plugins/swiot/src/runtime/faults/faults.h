#ifndef FAULTS_H
#define FAULTS_H

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
	explicit Faults(struct iio_context *ctx, QWidget *parent = nullptr);
	~Faults() override;

	void getAd74413rFaultsNumeric();
	void getMax14906FaultsNumeric();

	void pollFaults();

Q_SIGNALS:
        void backBtnPressed();

protected Q_SLOTS:
	void runButtonClicked();
	void singleButtonClicked();

private:
        void connectSignalsAndSlots();
        void setupDynamicUi(QWidget *parent);
        static QPushButton* createBackButton();

	struct iio_context* ctx;

	Ui::Faults *ui;
        QPushButton *m_backButton;

	QTimer *timer;
	QThread *thread;

	uint32_t ad74413r_numeric;
	uint32_t max14906_numeric;

	scopy::gui::ToolView *m_toolView{};
	scopy::gui::ChannelManager *m_monitorChannelManager{};
	FaultsPage *m_faultsPage{};
};
}
#endif // FAULTS_H
