#ifndef SWIOTFAULTS_HPP
#define SWIOTFAULTS_HPP

#include "ui_swiotfaults.h"

#include "faultspage.hpp"
#include "src/refactoring/tool/tool_view.hpp"
#include "src/refactoring/maingui/generic_menu.hpp"
#include "src/refactoring/maingui/channel_manager.hpp"

#include <iio.h>

namespace adiscope::swiot {
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

	adiscope::gui::ToolView *m_toolView{};
	adiscope::gui::ChannelManager *m_monitorChannelManager{};
	FaultsPage *m_faultsPage{};
};
}
#endif // SWIOTFAULTS_HPP
