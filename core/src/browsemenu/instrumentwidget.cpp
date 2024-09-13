#include "browsemenu/instrumentwidget.h"
#include "dynamicWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <utils.h>

#include <pluginbase/toolmenuentry.h>

using namespace scopy;

InstrumentWidget::InstrumentWidget(QString uuid, QString name, QString icon, QWidget *parent)
	: QWidget(parent)
	, m_uuid(uuid)
	, m_name(name)
	, m_icon(icon)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	setFixedHeight(50);
	lay->setSpacing(0);
	lay->setContentsMargins(0, 0, 0, 0);

	QWidget *toolOption = new QWidget(this);
	toolOption->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *toolLay = new QHBoxLayout(toolOption);
	toolLay->setSpacing(0);
	toolLay->setContentsMargins(0, 0, 0, 0);
	toolBtn = new QPushButton(name);
	toolBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	toolRunBtn = new CustomPushButton(toolOption);
	toolLay->addWidget(toolBtn, Qt::AlignLeft);
	toolLay->addWidget(toolRunBtn);

	setDynamicProperty(toolRunBtn, "stopButton", true);
	toolRunBtn->setMaximumSize(32, 32);
	toolBtn->setMinimumHeight(42);

	toolBtn->setIcon(QIcon::fromTheme(m_icon));
	toolBtn->setCheckable(true);
	toolBtn->setIconSize(QSize(32, 32));

	toolRunBtn->setCheckable(true);
	toolRunBtn->setText("");

	toolBtn->setFlat(true);
	toolRunBtn->setFlat(true);

	lay->addWidget(toolOption);

	// Load stylesheets
	setStyleSheet(Util::loadStylesheetFromFile(":/gui/stylesheets/toolMenuItem.qss"));
	setAttribute(Qt::WA_StyledBackground, true);
#ifdef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
#else
	setDynamicProperty(this, "allowHover", true);
	enableDoubleClick(true);
#endif
}

InstrumentWidget::~InstrumentWidget() { qInfo() << "Delete!"; }

QPushButton *InstrumentWidget::getToolBtn() const { return toolBtn; }

QPushButton *InstrumentWidget::getToolRunBtn() const { return toolRunBtn; }

void InstrumentWidget::enableDoubleClick(bool enable)
{
	if(enable) {
		toolBtn->installEventFilter(this);
	} else {
		toolBtn->removeEventFilter(this);
		removeEventFilter(this);
	}
}

bool InstrumentWidget::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonDblClick) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if(mouseEvent->button() == Qt::LeftButton) {
			if(isEnabled()) {
				Q_EMIT doubleclick();
				return true;
			}
		}
	}

	return QObject::event(event);
}

void InstrumentWidget::setName(QString str)
{
	m_name = str;
	toolBtn->setText(m_name);
}

void InstrumentWidget::hideText(bool hidden)
{
	if(hidden) {
		toolBtn->setText("");
	} else {
		toolBtn->setText(m_name);
	}
}

void InstrumentWidget::setDisabled(bool disabled) { setDisabled(disabled); }

void InstrumentWidget::updateItem()
{
	ToolMenuEntry *tme = dynamic_cast<ToolMenuEntry *>(QObject::sender());
	Q_ASSERT(tme);
	QSignalBlocker sb(toolRunBtn);
	setVisible(tme->visible());
	setEnabled(tme->enabled());
	setName(tme->name());
	toolRunBtn->setEnabled(tme->runEnabled());
	toolRunBtn->setEnabled(tme->runBtnVisible());
	toolRunBtn->setChecked(tme->running());
	//	Util::retainWidgetSizeWhenHidden(m, tme->visible());
	//	qDebug(CAT_TOOLMANAGER) << "updating toolmenuentry for " << tme->name() << " - " << tme->uuid();
}

// void InstrumentWidget::mouseMoveEvent(QMouseEvent *event)
//{
//#ifndef __ANDROID__
//	BaseMenuItem::mouseMoveEvent(event);
//	setDynamicProperty(this, "allowHover", false);
//#endif
// }

void InstrumentWidget::enterEvent(QEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", true);
	event->accept();
#endif
}

void InstrumentWidget::leaveEvent(QEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
	event->accept();
#endif
}

QString InstrumentWidget::getId() const { return m_uuid; }
