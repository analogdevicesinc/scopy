#include "deviceinfopage.h"
#include <QVBoxLayout>

using namespace scopy;

DeviceInfoPage::DeviceInfoPage(Connection *conn, QWidget *parent)
	: QWidget(parent)
	, m_conn(conn)
	, m_infoPage(new InfoPage(this))
	, m_title(new QLabel("Device Info", this))
{
	setupUi();
	setupInfoPage();
}

void DeviceInfoPage::setupUi()
{
	m_infoPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(new QVBoxLayout(this));
	m_title->setStyleSheet("font-weight: bold;");
	layout()->addWidget(m_title);
	layout()->addWidget(m_infoPage);
}

void DeviceInfoPage::setupInfoPage()
{
	if(!m_conn) {
		qWarning() << "Error, invalid connection, cannot create info page.";
		return;
	}

	const char *name;
	const char *value;
	for(int i = 0; i < iio_context_get_attrs_count(m_conn->context()); ++i) {
		int ret = iio_context_get_attr(m_conn->context(), i, &name, &value);
		if(ret != 0)
			continue;

		m_infoPage->update(name, value);
	}
}

#include "moc_deviceinfopage.cpp"
