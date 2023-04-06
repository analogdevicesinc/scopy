#ifndef MAX14906TOOL_HPP
#define MAX14906TOOL_HPP

#include "ui_swiotmax14906.h"
#include "customcolqgridlayout.hpp"
#include "swiotmax14906controller.hpp"
#include <core/logging_categories.h>
#include "swiotdigitalchannelcontroller.h"
#include "swiotmax14906readerthread.h"
#include "swiotmax14906settingstab.h"
#include "src/tool/tool_view.hpp"

namespace adiscope {
//#define POLLING_INTERVAL_MAX 1000
//class DigitalChannel;
//class Max14906ToolController;

class Max14906Tool : public QWidget {
	Q_OBJECT
public:
	Max14906Tool(struct iio_context *ctx, QWidget *parent = nullptr);
	~Max14906Tool() override;

private Q_SLOTS:
	void runButtonToggled();
	void singleButtonToggled();

	void timerChanged(double value);

private:
	void initChannels();
        void setupDynamicUi(QWidget *parent);
	void initMonitorToolView();
        adiscope::gui::GenericMenu* createGeneralSettings(const QString& title, QColor* color);
	void connectSignalsAndSlots();

	Max14906ToolController *max14906ToolController;
	Ui::Max14906Tool *ui;
        Max14906SettingsTab *m_max14906SettingsTab;
        adiscope::gui::ToolView *m_toolView;
	CustomColQGridLayout* m_customColGrid;
        adiscope::gui::GenericMenu* m_generalSettingsMenu;
        adiscope::gui::ChannelManager* m_monitorChannelManager;
        adiscope::gui::SubsectionSeparator *settingsWidgetSeparator;
	QTimer *m_qTimer;

	MaxReaderThread *m_readerThread;
	QMap<int, DigitalChannelController*> m_channelControls;
};
}
#endif // MAX14906TOOL_HPP
