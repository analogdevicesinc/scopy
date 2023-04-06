#ifndef MAX14906TOOL_HPP
#define MAX14906TOOL_HPP

#include "ui_swiotmax14906.h"
#include "src/refactoring/maincore/customcolqgridlayout.hpp"
#include "diocontroller.hpp"
#include "core/logging_categories.h"
#include "diodigitalchannelcontroller.hpp"
#include "src/runtime/readerthread.h"
#include "diosettingstab.h"
#include "src/refactoring/tool/tool_view.hpp"

namespace adiscope::swiot {
//#define POLLING_INTERVAL_MAX 1000
class DioDigitalChannel;
class DioController;

class Max14906 : public QWidget {
	Q_OBJECT
public:
	explicit Max14906(struct iio_context *ctx, QWidget *parent = nullptr);
	~Max14906() override;

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

	DioController *max14906ToolController;
	Ui::Max14906 *ui;
	DioSettingsTab *m_max14906SettingsTab;
	adiscope::gui::ToolView *m_toolView;
	CustomColQGridLayout* m_customColGrid;
	adiscope::gui::GenericMenu* m_generalSettingsMenu;
	adiscope::gui::ChannelManager* m_monitorChannelManager;
	adiscope::gui::SubsectionSeparator *settingsWidgetSeparator;
	QTimer *m_qTimer;

	ReaderThread *m_readerThread;
	QMap<int, DioDigitalChannelController*> m_channelControls;
};
}
#endif // MAX14906TOOL_HPP
