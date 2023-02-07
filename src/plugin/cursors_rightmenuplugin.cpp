#include "gui/dynamicWidget.hpp"
#include "plugin/irightmenuplugin.h"
#include <FftDisplayPlot.h>
#include <newinstrument.hpp>
#include <ui_cursors_settings.h>

using namespace adiscope;
using namespace gui;
class CursorsRightMenuPlugin : public IRightMenuPlugin
{
private:
	DisplayPlot *plot;
	Ui::CursorsSettings *cr_ui;
	CustomMenuButton* menuBtn;
	ChannelManager* chManager;

	bool hasHorizontal;
	bool hasVertical;

public:
	CursorsRightMenuPlugin(QWidget *parent, ToolView* toolView, bool enableHorizontal = true, bool enableVertical = true, ChannelManager *channelManager = nullptr, DisplayPlot *plot = nullptr) : IRightMenuPlugin(parent, toolView),
		plot(plot),
		chManager(channelManager),
		hasHorizontal(enableHorizontal),
		hasVertical(enableVertical)
	{
	}

	void init() override
	{
		menu = new GenericMenu(new QWidget());
		menu->initInteractiveMenu();

		menuBtn = toolView->buildNewInstrumentMenu(menu, false, "Cursors", true, false);

		initCursors();

		menu->setMenuWidget(cr_ui->scrollArea);
	}

	void initCursors()
	{
		cr_ui = new Ui::CursorsSettings;
		cr_ui->setupUi(menu);
		setDynamicProperty(cr_ui->btnLockHorizontal, "use_icon", true);
		setDynamicProperty(cr_ui->btnLockVertical, "use_icon", true);

		connect(cr_ui->btnLockHorizontal, &QPushButton::toggled,
			plot, &FftDisplayPlot::setHorizCursorsLocked);
		connect(cr_ui->btnLockVertical, &QPushButton::toggled,
			plot, &FftDisplayPlot::setVertCursorsLocked);

		auto cursorsPositionButton = new CustomPlotPositionButton(cr_ui->posSelect);

		connect(cr_ui->hCursorsEnable, SIGNAL(toggled(bool)), plot, SLOT(setVertCursorsEnabled(bool)));
		connect(cr_ui->vCursorsEnable, SIGNAL(toggled(bool)), plot, SLOT(setHorizCursorsEnabled(bool)));
		connect(cr_ui->btnNormalTrack, &QPushButton::toggled,
			this, &CursorsRightMenuPlugin::toggleCursorsMode);

		cr_ui->horizontalSlider->setMaximum(100);
		cr_ui->horizontalSlider->setMinimum(0);
		cr_ui->horizontalSlider->setSingleStep(1);

		connect(cr_ui->horizontalSlider, &QSlider::valueChanged, [=](int value){
			cr_ui->transLabel->setText(tr("Transparency ") + QString::number(value) + "%");
			plot->setCursorReadoutsTransparency(value);
		});
		cr_ui->horizontalSlider->setSliderPosition(0);

		connect(cursorsPositionButton, &CustomPlotPositionButton::positionChanged,
			[=](CustomPlotPositionButton::ReadoutsPosition position){
			plot->moveCursorReadouts(position);
		});

		connect(menuBtn->getCheckBox(), &QCheckBox::stateChanged, this, &CursorsRightMenuPlugin::menuCheckboxClicked);

		// hide widgets if not required
		cr_ui->widgetHorizontal->setVisible(hasHorizontal);
		cr_ui->widgetHorizontalBtns->setVisible(hasHorizontal);

		cr_ui->widgetVertical->setVisible(hasVertical);
		cr_ui->widgetVerticalBtns->setVisible(hasVertical);
	}

	void toggleCursorsMode(bool toggled)
	{
		cr_ui->hCursorsEnable->setEnabled(toggled && hasHorizontal);
		cr_ui->vCursorsEnable->setEnabled(toggled && hasVertical);

		if (toggled) {
			plot->setVertCursorsEnabled(cr_ui->hCursorsEnable->isChecked() && hasHorizontal);
			plot->setHorizCursorsEnabled(cr_ui->vCursorsEnable->isChecked() && hasVertical);
		} else {
			plot->setVertCursorsEnabled(hasHorizontal);
			plot->setHorizCursorsEnabled(hasVertical);
		}

		cr_ui->btnLockVertical->setEnabled(toggled);
		plot->trackModeEnabled(toggled);
	}

	void menuCheckboxClicked(int on)
	{
		plot->setHorizCursorsEnabled(on ? cr_ui->vCursorsEnable->isChecked() : false);
		plot->setVertCursorsEnabled(on ? cr_ui->hCursorsEnable->isChecked() : false);

		plot->trackModeEnabled(on ? cr_ui->btnNormalTrack->isChecked() : true);

		if (on) {
			plot->setCursorReadoutsVisible(true);
		}

		if (!hasHorizontal) {
			Q_EMIT cr_ui->hCursorsEnable->toggled(false);
			cr_ui->hCursorsEnable->setChecked(false);
			cr_ui->hCursorsEnable->blockSignals(true);
		}
		if (!hasVertical) {
			Q_EMIT cr_ui->vCursorsEnable->toggled(false);
			cr_ui->vCursorsEnable->setChecked(false);
			cr_ui->vCursorsEnable->blockSignals(true);
		}
	}
};
