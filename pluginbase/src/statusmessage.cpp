#include "statusmessage.h"
#include <QLabel>
#include <QDateTime>
#include <utility>

StatusMessage::StatusMessage(QString text, int ms, QWidget *parent)
	: QWidget(parent)
	, m_ms(ms)
	, m_text(std::move(text))
	, m_widget(nullptr)
{
	m_permanent = (m_ms == -1);
	prependDateTime();
}

StatusMessage::~StatusMessage()
{
	if(!m_permanent) {
		delete m_widget;
		m_widget = nullptr;
	}
}

QString StatusMessage::getText() { return m_text; }

void StatusMessage::setText(QString text) { m_text = std::move(text); }

QWidget *StatusMessage::getWidget()
{
	if(!m_widget) {
		auto label = new QLabel(this);
		label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
		label->setText(m_text);
		label->setStyleSheet("border: none;");
		label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		label->setMaximumHeight(15);

		m_widget = label;
	}

	return m_widget;
}

void StatusMessage::setWidget(QWidget *widget) { m_widget = widget; }

int StatusMessage::getDisplayTime() { return m_ms; }

void StatusMessage::setDisplayTime(int ms) { m_ms = ms; }

void StatusMessage::prependDateTime()
{
	QDateTime dateTime = QDateTime::currentDateTime();
	QString formattedTime = dateTime.toString(TIMESTAMP_FORMAT);
	m_text.push_front(formattedTime);
}

#include "moc_statusmessage.cpp"
