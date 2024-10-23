#include "iiodebuglogger.h"
#include "debuggerloggingcategories.h"
#include <QVBoxLayout>
#include <QFont>
#include <style.h>

using namespace scopy::debugger;

IIODebugLogger::IIODebugLogger(QWidget *parent)
	: QFrame(parent)
	, m_textBrowser(new QTextBrowser(this))
{
	setupUi();
}

void IIODebugLogger::setupUi()
{
	setContentsMargins(0, 0, 0, 0);
	setLayout(new QVBoxLayout(this));
	layout()->addWidget(m_textBrowser);
	layout()->setContentsMargins(0, 0, 0, 0);
	m_textBrowser->setTabStopDistance(30);
	QFont mono("Monospace");
	mono.setStyleHint(QFont::Monospace);
	m_textBrowser->setFont(mono);
	m_textBrowser->setReadOnly(true);
	Style::setStyle(m_textBrowser, style::properties::widget::textBrowser);
}

void IIODebugLogger::appendLog(QString log)
{
	qDebug(CAT_DEBUGGERIIOMODEL) << "Adding to log: " << log;
	m_textBrowser->append(log);
}

#include "moc_iiodebuglogger.cpp"
