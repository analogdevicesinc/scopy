#include "versioncheckmessage.h"

#include <pluginbase/preferences.h>
#include <QHBoxLayout>

using namespace scopy;
VersionCheckMessage::VersionCheckMessage(QWidget *parent)
	: QWidget(parent)
{
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	auto textLabel =
		new QLabel("<p>Should Scopy check for online versions?&nbsp;&nbsp;&nbsp;&nbsp;<a style='color: white; "
			   "font-weight: bold;' href='yes'>Yes</a>&nbsp;&nbsp;&nbsp;&nbsp;<a style='color: white; "
			   "font-weight: bold;' href='no'>No</a></p>",
			   this);
	connect(textLabel, &QLabel::linkActivated, this, [this](const QString &text) {
		if(text == "yes") {
			setCheckVersion(true);
		} else if(text == "no") {
			setCheckVersion(false);
		}

		delete this;
	});
	textLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	layout()->addWidget(textLabel);

	connect(this, &VersionCheckMessage::setCheckVersion, this, &VersionCheckMessage::saveCheckVersion);
}

VersionCheckMessage::~VersionCheckMessage() {}

void VersionCheckMessage::saveCheckVersion(bool allowed) { Preferences::set("general_check_online_version", allowed); }

#include "moc_versioncheckmessage.cpp"
