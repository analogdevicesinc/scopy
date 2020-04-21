#ifndef PG_CHANNEL_MANAGER_H
#define PG_CHANNEL_MANAGER_H

#include "digitalchannel_manager.hpp"
#include "libsigrokdecode/libsigrokdecode.h"
#include "pg_patterns.hpp"

#include "ui_pg_cg_settings.h"
#include "ui_pg_channel.h"
#include "ui_pg_channel_group.h"
#include "ui_pg_channel_header.h"
#include "ui_pg_channel_manager.h"

#include <QBitmap>
#include <QDebug>
#include <QDrag>
#include <QGroupBox>
#include <QMimeData>
#include <QMimeType>
#include <QScrollArea>
#include <QScrollBar>

namespace pv {
namespace view {
class View;
class LogicSignal;
class DecodeTrace;
} // namespace view
} // namespace pv
namespace Ui {
class PGChannelGroup;
class PGChannel;
class PGChannelManager;
class PGChannelManagerHeader;
} // namespace Ui

namespace adiscope {
class PatternGenerator;
class PatternGeneratorChannelGroup;
class PatternGeneratorChannelGroupUI;
class PatternGeneratorChannelManagerUI;

class PatternGeneratorChannel : public Channel {
	bool outputMode;

public:
	PatternGeneratorChannel(uint16_t id_, std::string &label_);
	qreal getCh_thickness() const;
	void setCh_thickness(const qreal value);
	bool getOutputMode() const;
	void setOutputMode(bool value);

private:
	qreal ch_thickness;
};

class PatternGeneratorChannelUI : public ChannelUI {
	Q_OBJECT
	PatternGeneratorChannelManagerUI *managerUi;
	PatternGeneratorChannelGroupUI *chgui;
	PatternGeneratorChannelGroup *chg;
	PatternGeneratorChannel *ch;
	QPoint dragStartPosition;

	QRect topDragbox;
	QRect centerDragbox;
	QRect botDragbox;

	int traceOffset;
	int traceHeight;
	std::shared_ptr<pv::view::LogicSignal> trace;

public:
	Ui::PGChannelGroup *ui;
	PatternGeneratorChannelUI(PatternGeneratorChannel *ch,
				  PatternGeneratorChannelGroup *chg,
				  PatternGeneratorChannelGroupUI *chgui,
				  PatternGeneratorChannelManagerUI *managerUi,
				  QWidget *parent = 0);
	~PatternGeneratorChannelUI();
	PatternGeneratorChannelManagerUI *getManagerUi() const;
	PatternGeneratorChannel *getChannel();
	PatternGeneratorChannelGroup *getChannelGroup();
	void enableControls(bool val);

	void setTrace(std::shared_ptr<pv::view::LogicSignal> &item);
	std::shared_ptr<pv::view::LogicSignal> getTrace();
	void highlight(bool val);

	QFrame *topSep, *botSep;

	void updateTrace();
	void highlightTopSeparator();
	void highlightBotSeparator();
	void resetSeparatorHighlight(bool force = false);
	bool eventFilter(QObject *watched, QEvent *event);
Q_SIGNALS:
	void requestUpdateUi();
private Q_SLOTS:
	void split();
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
};

class PatternGeneratorChannelGroup : public ChannelGroup {
	bool collapsed;

public:
	PatternGeneratorChannelGroup(PatternGeneratorChannel *ch = nullptr,
				     bool en = false);
	~PatternGeneratorChannelGroup();
	Pattern *pattern;
	PatternGeneratorChannel *get_channel(int index);
	bool isCollapsed();
	void collapse(bool val);
	void append(PatternGeneratorChannelGroup *tojoin);
	qreal getCh_thickness() const;
	void setCh_thickness(const qreal value, bool setChannels = true);

private:
	qreal ch_thickness;
};

class PatternGeneratorChannelGroupUI : public ChannelGroupUI {
	Q_OBJECT
	Q_PROPERTY(int checked READ isChecked WRITE check)
	int checked;
	PatternGeneratorChannelManagerUI *managerUi;
	int isChecked();
	void check(int val);
	QPoint dragStartPosition;
	QRect centerDragbox;
	QRect topDragbox;
	QRect botDragbox;
	int traceOffset;
	int traceHeight;

public:
	std::shared_ptr<pv::view::TraceTreeItem> trace;
	std::shared_ptr<pv::view::LogicSignal> logicTrace;
	std::shared_ptr<pv::view::DecodeTrace> decodeTrace;
	Ui::PGChannelGroup *ui;
	std::vector<PatternGeneratorChannelUI *> ch_ui;
	PatternGeneratorChannelGroupUI(
		PatternGeneratorChannelGroup *chg,
		PatternGeneratorChannelManagerUI *managerUi,
		QWidget *parent = 0);
	~PatternGeneratorChannelGroupUI();
	PatternGeneratorChannelGroup *getChannelGroup();
	PatternGeneratorChannelManagerUI *getManagerUi() const;
	void setTrace(std::shared_ptr<pv::view::LogicSignal> item);
	void setTrace(std::shared_ptr<pv::view::DecodeTrace> item);
	std::shared_ptr<pv::view::TraceTreeItem> getTrace();
	void enableControls(bool enabled);
	int getTraceOffset();

	QFrame *topSep, *botSep, *chUiSep;

	void updateTrace();

	std::map<const srd_channel *, std::shared_ptr<pv::view::TraceTreeItem>>
	setupDecoder(const char *decoder, std::vector<int> ids);

	void setupUARTDecoder();
	void setupSPIDecoder();
	void setupI2CDecoder();
	void setupParallelDecoder();
	void highlight(bool val);
	void highlightTopSeparator();
	void highlightBotSeparator();
	void hideSeparatorHighlight(bool force = false);
	bool eventFilter(QObject *watched, QEvent *event);

Q_SIGNALS:
	void channel_selected();
	void channel_enabled();
	void requestUpdateUi();

private Q_SLOTS:

	void select(bool selected);
	void enable(bool enabled);
	void split();
	void collapse();

private:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);
	void mousePressEvent(QMouseEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void dragEnterEvent(QDragEnterEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);
	void dragMoveEvent(QDragMoveEvent *event);
	void dropEvent(QDropEvent *event);
};

class PatternGeneratorChannelManager : public ChannelManager {
	PatternGeneratorChannelGroup *highlightedChannelGroup;
	PatternGeneratorChannel *highlightedChannel;
	const uint32_t maxBufferSize = 1000000;

public:
	void highlightChannel(PatternGeneratorChannelGroup *chg,
			      PatternGeneratorChannel *ch = nullptr);
	PatternGeneratorChannelGroup *getHighlightedChannelGroup();
	PatternGeneratorChannel *getHighlightedChannel();
	PatternGeneratorChannelManager();
	~PatternGeneratorChannelManager();
	PatternGeneratorChannelGroup *get_channel_group(int index);

	uint16_t get_mode_mask();
	void join(std::vector<int> index);
	void split(int index);
	void move(int from, int to, bool after = true);
	void moveChannel(int fromChgIndex, int from, int to, bool after = true);
	void splitChannel(int chgIndex, int chIndex);
	void preGenerate();
	void generatePatterns(short *mainbuffer, uint32_t sampleRate,
			      uint32_t bufferSize);
	void commitBuffer(PatternGeneratorChannelGroup *chg, short *mainBuffer,
			  uint32_t bufferSize);
	short remap_buffer(uint8_t *mapping, uint32_t val);

	uint32_t computeSuggestedSampleRate();
	uint32_t computeSuggestedBufferSize(uint32_t sample_rate);
	void add_channel_group(PatternGeneratorChannelGroup *chg);
	PatternGeneratorChannel *get_channel(int);
	void clearChannels();
	void clearChannelGroups();
};

class PatternGeneratorChannelManagerUI : public QWidget {
	Q_OBJECT
	QWidget *settingsWidget; // pointer to settingspage in stacked widget in
				 // main pg ui
	QWidget *channelManagerHeaderWiget;
	Ui::PGChannelManagerHeader *chmHeader;

	bool disabledShown;
	Ui::PGCGSettings *cgSettings;

public:
	const bool pixmapEnable = true;
	const bool pixmapGrab = true;
	const bool pixmapRetainSize = true;
	const int pixmapScale = 1;

public:
	pv::MainWindow *main_win;
	std::vector<PatternGeneratorChannelGroupUI *> chg_ui;
	std::vector<QFrame *> separators;
	PatternGeneratorChannelManagerUI(QWidget *parent,
					 pv::MainWindow *main_win_,
					 PatternGeneratorChannelManager *chm,
					 Ui::PGCGSettings *cgSettings,
					 PatternGenerator *pg);
	~PatternGeneratorChannelManagerUI();

	PatternGeneratorChannelGroupUI *
	findUiByChannelGroup(PatternGeneratorChannelGroup *toFind);
	PatternGeneratorChannelUI *
	findUiByChannel(PatternGeneratorChannel *toFind);
	PatternGeneratorChannelManager *chm;
	PatternGenerator *pg;
	Ui::PGChannelManager *ui;

	QWidget *hoverWidget;

	void highlightChannel(PatternGeneratorChannelGroup *chg,
			      PatternGeneratorChannel *ch = nullptr);

	void updateUi();
	void selectChannelGroup(PatternGeneratorChannelGroupUI *selected);
	QWidget *getSettingsWidget() const;

	bool isDisabledShown();
	void showDisabled();
	void hideDisabled();

	void showHighlight(bool val);
	void setHoverWidget(QWidget *hover);
	void clearHoverWidget();

	void groupSplitSelected();
	bool eventFilter(QObject *object, QEvent *event);
	void updatePlot();

	std::vector<PatternGeneratorChannelGroupUI *> getEnabledChannelGroups();

	bool getUseDecoders() const;
	void setUseDecoders(bool use_decoders);

private:
	QFrame *addSeparator(QVBoxLayout *lay, int pos);
	bool m_use_decoders;

Q_SIGNALS:
	void channelsChanged();

private Q_SLOTS:
	void chmScrollChanged(int val);
	void chmRangeChanged(int min, int max);
public Q_SLOTS:
	void triggerUpdateUi();
	void triggerUpdateUiNoSettings();
};

} // namespace adiscope
#endif // PG_CHANNEL_MANAGER_H
