#include "fastlockprofileswidget.h"

#include <QBoxLayout>
#include <style.h>

#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

FastlockProfilesWidget::FastlockProfilesWidget(iio_channel *chn, QWidget *parent)
	: QWidget{parent}
{

	QVBoxLayout *layout = new QVBoxLayout(this);
	this->setLayout(layout);

	m_title = new QLabel("Fastlock Profiles", this);
	layout->addWidget(m_title);

	m_fastlockProfiles = new QComboBox(this);
	layout->addWidget(m_fastlockProfiles);

	// there are 7 fastlock posibile slots
	for(int i = 0; i <= 7; i++) {
		m_fastlockProfiles->addItem(QString::number(i), i);
	}

	m_storeBtn = new QPushButton("Store", this);
	m_recallBtn = new QPushButton("Recall", this);

	connect(m_storeBtn, &QPushButton::clicked, this, [=, this]() {
		int currentProfileSetting = m_fastlockProfiles->currentData().toInt();
		iio_channel_attr_write_longlong(chn, "fastlock_store", (long long)currentProfileSetting);
	});

	connect(m_recallBtn, &QPushButton::clicked, this, [=, this]() {
		int currentProfileSetting = m_fastlockProfiles->currentData().toInt();
		iio_channel_attr_write_longlong(chn, "fastlock_recall", (long long)currentProfileSetting);
		Q_EMIT recallCalled();
	});

	Style::setStyle(m_storeBtn, style::properties::button::basicButton);
	Style::setStyle(m_recallBtn, style::properties::button::basicButton);

	QHBoxLayout *btnLayout = new QHBoxLayout();

	btnLayout->addWidget(m_storeBtn);
	btnLayout->addWidget(m_recallBtn);

	layout->addLayout(btnLayout);
}

QComboBox *FastlockProfilesWidget::fastlockProfiles() const { return m_fastlockProfiles; }

QString FastlockProfilesWidget::title() const { return m_title->text(); }

void FastlockProfilesWidget::setTitle(QString *newTitle) { m_title->setText(*newTitle); }
