#include "statusmessage.h"
#include <QLabel>
#include <QDateTime>

using namespace scopy;

// ---------------------------------- STATUS_MESSAGE_TEXT ----------------------------------

StatusMessageText::StatusMessageText(QString text, int ms, QWidget *parent)
	: m_ms(ms)
	, m_text(text)
{
	setParent(parent);
	prependDateTime();

	auto *label = new QLabel(m_text, this);
	label->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	label->setStyleSheet("border: none;");
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	label->setMaximumHeight(15);
	m_widget = label;
}

StatusMessageText::~StatusMessageText() { delete m_widget; }

QString StatusMessageText::getText() { return m_text; }

QWidget *StatusMessageText::getWidget() { return m_widget; }

int StatusMessageText::getDisplayTime() { return m_ms; }

void StatusMessageText::prependDateTime()
{
	QDateTime dateTime = QDateTime::currentDateTime();
	QString formattedTime = dateTime.toString(TIMESTAMP_FORMAT);
	m_text.push_front(formattedTime);
}

// ---------------------------------- STATUS_MESSAGE_WIDGET ----------------------------------

StatusMessageWidget::StatusMessageWidget(QWidget *widget, QString description, int ms, QWidget *parent)
	: m_ms(ms)
	, m_text(description)
	, m_widget(widget)
{
	setParent(parent);
}

StatusMessageWidget::~StatusMessageWidget()
{
	// the widget is not permanently displayed, so the responsibility of deleting it belongs to this class
	if(m_ms != -1) {
		delete m_widget;
		m_widget = nullptr;
	}
}

QString StatusMessageWidget::getText() { return m_text; }

QWidget *StatusMessageWidget::getWidget() { return m_widget; }

int StatusMessageWidget::getDisplayTime() { return m_ms; }

#include "moc_statusmessage.cpp"
