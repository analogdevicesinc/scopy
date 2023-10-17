#include "versioncheckmessage.h"

#include <pluginbase/preferences.h>
#include <QHBoxLayout>

using namespace scopy;
VersionCheckMessage::VersionCheckMessage(QWidget *parent)
	: QWidget(parent)
{
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	auto textLabel = new QLabel("Should Scopy check for online versions?    [Yes](yes)    [No](no)", this);
	connect(textLabel, &QLabel::linkActivated, this, [this](const QString &text) {
		if(text == "yes") {
			setCheckVersion(true);
		} else if(text == "no") {
			setCheckVersion(false);
		}

		delete this;
	});
	textLabel->setTextFormat(Qt::MarkdownText);
	textLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	layout()->addWidget(textLabel);

	connect(this, &VersionCheckMessage::setCheckVersion, this, &VersionCheckMessage::saveCheckVersion);
}

VersionCheckMessage::~VersionCheckMessage() {}

void VersionCheckMessage::saveCheckVersion(bool allowed) { Preferences::set("general_check_online_version", allowed); }

#include "moc_versioncheckmessage.cpp"
