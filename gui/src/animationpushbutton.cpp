#include "animationpushbutton.h"
#include <QTimer>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_GUI_ANIMATION_BTN, "AnimationPushButton")

using namespace scopy;

AnimationPushButton::AnimationPushButton(QWidget *parent)
	: QPushButton(parent)
	, m_animation(nullptr)
{
	m_timer = new QTimer(this);
	connect(m_timer, &QTimer::timeout, this, &AnimationPushButton::stopAnimation);
}

AnimationPushButton::~AnimationPushButton()
{
	if(m_animation) {
		disconnect(m_animation, &QMovie::frameChanged, this, &AnimationPushButton::setBtnIcon);
	}
}

void AnimationPushButton::setAnimation(QMovie *animation, int maxRunningTimeMsec)
{
	if(animation->isValid()) {
		if(m_animation) {
			disconnect(m_animation, &QMovie::frameChanged, this, &AnimationPushButton::setBtnIcon);
		}
		m_animation = animation;
		m_timer->setInterval(maxRunningTimeMsec);
		connect(m_animation, &QMovie::frameChanged, this, &AnimationPushButton::setBtnIcon);
	} else {
		qWarning(CAT_GUI_ANIMATION_BTN) << "The animation " << animation->fileName() + " is not valid!";
	}
}

void AnimationPushButton::startAnimation()
{
	if(m_animation->isValid() && (m_animation->state() == QMovie::NotRunning)) {
		m_currentText = this->text();
		m_currentIcon = this->icon();
		m_animation->start();
		m_timer->start();
		this->setText("");
		this->setEnabled(false);
	}
}

void AnimationPushButton::stopAnimation()
{
	if(m_animation->isValid() && (m_animation->state() == QMovie::Running)) {
		m_timer->stop();
		m_animation->stop();
		this->setIcon(m_currentIcon);
		this->setText(m_currentText);
		this->setEnabled(true);
	}
}

void AnimationPushButton::setBtnIcon() { this->setIcon(m_animation->currentPixmap()); }

#include "moc_animationpushbutton.cpp"
