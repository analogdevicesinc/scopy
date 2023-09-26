#include <widgets/cursorsettings.h>
#include <QVBoxLayout>
#include <stylehelper.h>

using namespace scopy;

CursorSettings::CursorSettings(QWidget *parent)
	: QWidget(parent)
{
	initUI();
	connectSignals();
}

CursorSettings::~CursorSettings() {}

void CursorSettings::initUI()
{
	StyleHelper::TransparentWidget(this, "cursorSettings");
	setFixedWidth(200);
	layout = new QVBoxLayout(this);
	layout->setMargin(0);
	setLayout(layout);

	// horizontal controls
	horizControls = new MenuSectionWidget(this);
	layout->addWidget(horizControls);
	horizControls->contentLayout()->setSpacing(10);

	horizEn = new MenuOnOffSwitch(tr("Horizontal"), horizControls, false);
	horizEn->onOffswitch()->setChecked(true);
	horizControls->contentLayout()->addWidget(horizEn);
	horizLock = new MenuOnOffSwitch(tr("Lock"), horizControls, false);
	horizControls->contentLayout()->addWidget(horizLock);
	horizTrack = new MenuOnOffSwitch(tr("Track"), horizControls, false);
	horizControls->contentLayout()->addWidget(horizTrack);

	// vertical controls
	vertControls = new MenuSectionWidget(this);
	layout->addWidget(vertControls);
	vertControls->contentLayout()->setSpacing(10);

	vertEn = new MenuOnOffSwitch(tr("Vertical"), vertControls, false);
	vertEn->onOffswitch()->setChecked(true);
	vertControls->contentLayout()->addWidget(vertEn);
	vertLock = new MenuOnOffSwitch(tr("Lock"), vertControls, false);
	vertControls->contentLayout()->addWidget(vertLock);

	// readouts controls
	readoutsControls = new MenuSectionWidget(this);
	layout->addWidget(readoutsControls);

	readoutsDrag = new MenuOnOffSwitch(tr("Move readouts"), readoutsControls, false);
	readoutsControls->contentLayout()->addWidget(readoutsDrag);

	resize(sizeHint());
}

void CursorSettings::connectSignals()
{
	connect(horizEn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool toggled) {
		horizTrack->setEnabled(toggled);
		horizLock->setEnabled(toggled);
	});
	connect(vertEn->onOffswitch(), &QAbstractButton::toggled, this, [=](bool toggled) {
		vertLock->setEnabled(toggled);
		if(!toggled)
			horizTrack->onOffswitch()->setChecked(false);
	});
	connect(horizTrack->onOffswitch(), &QAbstractButton::toggled, this, [=](bool toggled) {
		if(toggled && !vertEn->onOffswitch()->isChecked())
			vertEn->onOffswitch()->setChecked(true);

		vertLock->setEnabled(vertEn->onOffswitch()->isChecked() && !toggled);
		vertLock->onOffswitch()->setChecked(false);
	});
}

QAbstractButton *CursorSettings::getHorizEn()
{
	return horizEn->onOffswitch();
}

QAbstractButton *CursorSettings::getHorizLock()
{
	return horizLock->onOffswitch();
}

QAbstractButton *CursorSettings::getHorizTrack()
{
	return horizTrack->onOffswitch();
}

QAbstractButton *CursorSettings::getVertEn()
{
	return vertEn->onOffswitch();
}

QAbstractButton *CursorSettings::getVertLock()
{
	return vertLock->onOffswitch();
}

QAbstractButton *CursorSettings::getReadoutsDrag()
{
	return readoutsDrag->onOffswitch();
}
