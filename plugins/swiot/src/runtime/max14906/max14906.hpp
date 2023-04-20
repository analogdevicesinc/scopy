#ifndef MAX14906TOOL_HPP
#define MAX14906TOOL_HPP

#include "ui_swiotmax14906.h"
#include "diocontroller.hpp"
#include "diodigitalchannelcontroller.hpp"
#include "src/runtime/readerthread.h"
#include "diosettingstab.h"
#include <gui/tool_view.hpp>
#include <gui/flexgridlayout.hpp>

namespace scopy::swiot {
class DioDigitalChannel;
class DioController;

class Max14906 : public QWidget {
	Q_OBJECT
public:
	explicit Max14906(struct iio_context *ctx, QWidget *parent = nullptr);
	~Max14906() override;

Q_SIGNALS:
        void backBtnPressed();

private Q_SLOTS:
	void runButtonToggled();
	void singleButtonToggled();

	void timerChanged(double value);

private:
        static QPushButton* createBackButton();
	void initChannels();
	void setupDynamicUi(QWidget *parent);
	void initMonitorToolView();
	scopy::gui::GenericMenu* createGeneralSettings(const QString& title, QColor* color);
	void connectSignalsAndSlots();

        QPushButton *m_backButton;
	DioController *max14906ToolController;
	Ui::Max14906 *ui;
	DioSettingsTab *m_max14906SettingsTab;
	scopy::gui::ToolView *m_toolView;
	FlexGridLayout* m_customColGrid;
	scopy::gui::GenericMenu* m_generalSettingsMenu;
	scopy::gui::ChannelManager* m_monitorChannelManager;
	scopy::gui::SubsectionSeparator *settingsWidgetSeparator;
	QTimer *m_qTimer;

	ReaderThread *m_readerThread;
	QMap<int, DioDigitalChannelController*> m_channelControls;
};
}
#endif // MAX14906TOOL_HPP
