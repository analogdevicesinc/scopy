#include "digitalio_api.hpp"

#include "ui_digitalIoChannel.h"
#include "ui_digitalIoElement.h"
#include "ui_digitalio.h"

namespace adiscope {
void DigitalIO_API::show() { Q_EMIT dio->showTool(); }

QList<bool> DigitalIO_API::direction() const {
	QList<bool> list;
	unsigned int i;

	for (i = 0; i < 16; i++) {
		auto ch = dio->findIndividualUi(i);
		list.append(ch->second->inout->isChecked());
	}

	return list;
}

void DigitalIO_API::setDirection(const QList<bool> &list) {
	unsigned int i;

	for (i = 0; i < 16; i++) {
		auto ch = dio->findIndividualUi(i);
		ch->second->inout->setChecked(list.at(i));
		dio->setDirection(i, list.at(i));
	}
}

QList<bool> DigitalIO_API::output() const {
	QList<bool> list;
	unsigned int i;

	for (i = 0; i < 16; i++) {
		auto ch = dio->findIndividualUi(i);
		list.append(ch->second->output->isChecked());
	}

	return list;
}

QList<bool> DigitalIO_API::gpi() const {
	QList<bool> list;
	unsigned int i;
	for (i = 0; i < 16; i++) {
		auto GPI = dio->diom->getGpi() & (1 << i);
		list.append(GPI);
	}
	return list;
}

bool DigitalIO_API::running() const { return dio->ui->btnRunStop->isChecked(); }

void DigitalIO_API::run(bool en) { dio->ui->btnRunStop->setChecked(en); }

void DigitalIO_API::setOutput(const QList<bool> &list) {
	unsigned int i;

	for (i = 0; i < 16; i++) {
		auto ch = dio->findIndividualUi(i);
		ch->second->output->setChecked(list.at(i));
		dio->setOutput(i, list.at(i));
	}
}

QList<bool> DigitalIO_API::grouped() const {

	QList<bool> list;
	for (int i = 0; i < dio->groups.size(); ++i) {
		bool grouped = dio->groups[i]->ui->stackedWidget->currentIndex() == 1;
		list.push_back(grouped);
	}
	return list;
}

void DigitalIO_API::setGrouped(const QList<bool> &grouped) {
	for (int i = 0; i < grouped.size(); ++i) {
		int index = grouped[i] ? 1 : 0;
		dio->groups[i]->ui->stackedWidget->setCurrentIndex(index);
		dio->groups[i]->ui->comboBox->setCurrentIndex(index);
		dio->groups[i]->changeDirection();
	}
}

QList<bool> DigitalIO_API::locked() const {
	QList<bool> list;
	unsigned int i;
	for (i = 0; i < 16; i++) {
		auto GPI = dio->diom->isLocked(i);
		list.append(GPI);
	}
	return list;
}
} // namespace adiscope
