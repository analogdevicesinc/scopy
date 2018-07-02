#include "pattern_generator_api.hpp"
#include "ui_pattern_generator.h"

namespace adiscope {
void PatternGenerator_API::show()
{
	Q_EMIT pg->showTool();
}

void PatternGenerator_API::refreshApi()
{
	PatternGeneratorChannelManagerUI *chmui = pg->chmui;
	PatternGeneratorChannelManager *chm = chmui->chm;

	pg_cga.clear();
	pg_cha.clear();

	for (auto i=0; i<chm->get_channel_group_count(); i++) {
		PatternGeneratorChannelGroup *chg = chm->get_channel_group(i);
		pg_cga.append(new PatternGeneratorChannelGroup_API(chg,chmui));
	}

	for (auto i=0; i<chm->get_channel_count(); i++) {
		PatternGeneratorChannel *cha = chm->get_channel(i);
		pg_cha.append(new PatternGeneratorChannel_API(cha,chmui));
	}
}


void PatternGeneratorChannel_API::setColor(QList<int> list)
{
	//	ch->set_id(val);
	ch->setBgcolor(QColor((list[0]&0xff0000)>>16,(list[0]&0xff00)>>8,list[0]&0xff));
	ch->setEdgecolor(QColor((list[1]&0xff0000)>>16,(list[1]&0xff00)>>8,list[1]&0xff));
	ch->setHighcolor(QColor((list[2]&0xff0000)>>16,(list[2]&0xff00)>>8,list[2]&0xff));
	ch->setLowcolor(QColor((list[3]&0xff0000)>>16,(list[3]&0xff00)>>8,list[3]&0xff));

}
QList<int> PatternGeneratorChannel_API::color()
{
	QList<int> list;

	list.append(ch->getBgcolor().red()<<16   | ch->getBgcolor().green()<<8   | ch->getBgcolor().blue());
	list.append(ch->getEdgecolor().red()<<16 | ch->getEdgecolor().green()<<8 | ch->getEdgecolor().blue());
	list.append(ch->getHighcolor().red()<<16 | ch->getHighcolor().green()<<8 | ch->getHighcolor().blue());
	list.append(ch->getLowcolor().red()<<16  | ch->getLowcolor().green()<<8  | ch->getLowcolor().blue());
	return list;
}



void PatternGeneratorChannelGroup_API::setColor(QList<int> list)
{
	//	ch->set_id(val);
	chg->setBgcolor(QColor((list[0]&0xff0000)>>16,(list[0]&0xff00)>>8,list[0]&0xff));
	chg->setEdgecolor(QColor((list[1]&0xff0000)>>16,(list[1]&0xff00)>>8,list[1]&0xff));
	chg->setHighcolor(QColor((list[2]&0xff0000)>>16,(list[2]&0xff00)>>8,list[2]&0xff));
	chg->setLowcolor(QColor((list[3]&0xff0000)>>16,(list[3]&0xff00)>>8,list[3]&0xff));

}
QList<int> PatternGeneratorChannelGroup_API::color()
{
	QList<int> list;

	list.append(chg->getBgcolor().red()<<16 | chg->getBgcolor().green()<<8 | chg->getBgcolor().blue());
	list.append(chg->getEdgecolor().red()<<16 | chg->getEdgecolor().green()<<8 | chg->getEdgecolor().blue());
	list.append(chg->getHighcolor().red()<<16 | chg->getHighcolor().green()<<8 | chg->getHighcolor().blue());
	list.append(chg->getLowcolor().red()<<16 | chg->getLowcolor().green()<<8 | chg->getLowcolor().blue());
	return list;
}


QList<int> PatternGeneratorChannelGroup_API::channels()
{
	QList<int> list;

	for (auto i=0; i<chg->get_channel_count(); i++) {
		list.append(chg->get_channel(i)->get_id());
	}

	return list;
}

void PatternGeneratorChannelGroup_API::setChannels(QList<int> list)
{
	while (chg->get_channel_count()) {
		chg->remove_channel(0);
	}

	for (auto val : list) {
		chg->add_channel(chm->get_channel(val));
	}
}

QString PatternGeneratorChannelGroup_API::pattern()
{
	return Pattern_API::toString(chg->pattern);
}

void PatternGeneratorChannelGroup_API::setPattern(QString str)
{
	chg->pattern->deinit();
	delete chg->pattern;
	chg->pattern = Pattern_API::fromString(str);
}

QVariantList PatternGenerator_API::getChannelGroups()
{
	QVariantList list;
	refreshApi();

	for (auto *each : pg_cga) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}

int PatternGenerator_API::channel_groups_size()
{
	refreshApi();
	return pg_cga.size();
}

void PatternGenerator_API::set_channel_groups_size(int val)
{
	PatternGeneratorChannelManager *chm = pg->chmui->chm;

	chm->clearChannelGroups();

	for (auto i=0; i<val; i++) {
		chm->add_channel_group(new PatternGeneratorChannelGroup());
	}
}

int PatternGenerator_API::channel_size()
{
	return pg_cha.size();
}

void PatternGenerator_API::set_channel_size(int val)
{
	PatternGeneratorChannelManager *chm = pg->chmui->chm;
	chm->clearChannels();
}

QVariantList PatternGenerator_API::getChannels()
{
	QVariantList list;
	refreshApi();

	for (auto *each : pg_cha) {
		list.append(QVariant::fromValue(each));
	}

	return list;
}


bool PatternGenerator_API::running() const
{
	return pg->ui->btnRunStop->isChecked();
}
void PatternGenerator_API::run(bool en)
{
	if (pg->ui->btnRunStop->isEnabled()) {
		pg->ui->btnRunStop->setChecked(en);
	}
}

bool PatternGenerator_API::single() const
{
	return pg->ui->btnSingleRun->isChecked();
}

void PatternGenerator_API::run_single(bool en)
{
	if(pg->ui->btnSingleRun->isEnabled()) {
		pg->ui->btnSingleRun->click();
	}
}

bool PatternGenerator_API::inactiveHidden()
{
	return pg->chmui->isDisabledShown();
}

void PatternGenerator_API::setInactiveHidden(bool en)
{
	if(en)
		pg->chmui->showDisabled();
	else
		pg->chmui->hideDisabled();
}
}
