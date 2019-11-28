/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PATTERN_GENERATOR_API_HPP
#define PATTERN_GENERATOR_API_HPP

#include "pattern_generator.hpp"

namespace adiscope {
class PatternGeneratorChannel_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel);
	Q_PROPERTY(int id READ id WRITE setId);
	Q_PROPERTY(int mask READ mask);
	Q_PROPERTY(bool outputMode READ outputMode WRITE setOutputMode)
	Q_PROPERTY(QList<int> color READ color WRITE setColor);
	Q_PROPERTY(double thickness READ thickness WRITE setThickness);
	PatternGeneratorChannel *ch;
	PatternGenerator *pg;
	PatternGeneratorChannelManagerUI *chmui;
	PatternGeneratorChannelManager *chm;


public:
	PatternGeneratorChannel_API(PatternGeneratorChannel *_ch,
				    PatternGeneratorChannelManagerUI *_chmui)
		: ch(_ch),chmui(_chmui),chm(_chmui->chm),pg(_chmui->pg) {}

	QList<int> color();
	void setColor(QList<int>);

	QString label()
	{
		return QString::fromStdString(ch->get_label());
	}
	void setLabel(QString _str)
	{
		ch->set_label(_str.toStdString());
	}
	int id()
	{
		return ch->get_id();
	}
	void setId(int val)
	{
		//	ch->set_id(val);
	}
	int mask()
	{
		return ch->get_mask();
	}
	bool outputMode()
	{
		return ch->getOutputMode();
	}
	void setOutputMode(bool val)
	{
		ch->setOutputMode(val);
		pg->diom->setOutputMode(id(), val);
	}

	void setThickness(double val)
	{
		ch->setCh_thickness(val);
	}
	double thickness()
	{
		return ch->getCh_thickness();
	}
};

class PatternGeneratorChannelGroup_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QString label READ label WRITE setLabel);
	Q_PROPERTY(bool grouped READ grouped WRITE setGrouped);
	Q_PROPERTY(bool enabled READ enabled WRITE setEnabled);
	Q_PROPERTY(bool collapsed READ collapsed WRITE setCollapsed);
	Q_PROPERTY(QString pattern READ pattern WRITE setPattern);

	Q_PROPERTY(QList<int> channel_list READ channels WRITE setChannels);
	Q_PROPERTY(double thickness READ thickness WRITE setThickness);
	Q_PROPERTY(QList<int> color READ color WRITE setColor);

	PatternGeneratorChannelGroup *chg;
	QList<int> channel_list_;

	PatternGeneratorChannelManagerUI *chmui;
	PatternGeneratorChannelManager *chm;

public:
	PatternGeneratorChannelGroup_API(PatternGeneratorChannelGroup *_chg,
	                                 PatternGeneratorChannelManagerUI *chmui) : chg(_chg),chmui(chmui),
		chm(chmui->chm) {}

	QString label()
	{
		return QString::fromStdString(chg->get_label());
	}
	void setLabel(QString _str)
	{
		chg->set_label(_str.toStdString());
	}

	QList<int> channels();
	void setChannels(QList<int>);

	QList<int> color();
	void setColor(QList<int>);


	bool grouped()
	{
		return chg->is_grouped();
	}
	void setGrouped(bool _val)
	{
		chg->group(_val);
	}

	QString pattern();
	void setPattern(QString);

	bool enabled()
	{
		return chg->is_enabled();
	}
	void setEnabled(bool _val)
	{
		return chg->enable(_val);
	}

	bool collapsed()
	{
		return chg->isCollapsed();
	}
	void setCollapsed(bool _val)
	{
		chg->collapse(_val);
	}
	void setThickness(double val)
	{
		chg->setCh_thickness(val,false);
	}
	double thickness()
	{
		return chg->getCh_thickness();
	}

};

class PatternGenerator_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(int channel_size READ channel_size WRITE set_channel_size SCRIPTABLE
	           false);
	Q_PROPERTY(QVariantList channel_list READ getChannels);

	Q_PROPERTY(int channel_groups_size READ channel_groups_size WRITE
	           set_channel_groups_size SCRIPTABLE false);
	Q_PROPERTY(QVariantList channel_groups_list READ getChannelGroups);

	Q_PROPERTY(bool running READ running WRITE run STORED false);
	Q_PROPERTY(bool single READ single WRITE run_single STORED false);
	Q_PROPERTY(bool inactive_hidden READ inactiveHidden WRITE setInactiveHidden);

public:
	explicit PatternGenerator_API(PatternGenerator *pg) :
		ApiObject(), pg(pg) {}
	~PatternGenerator_API();

	void save(QSettings& settings);


	int channel_groups_size();
	void set_channel_groups_size(int val);
	QVariantList getChannelGroups();

	int channel_size();
	void set_channel_size(int val);
	QVariantList getChannels();

	bool running() const;
	void run(bool en);
	bool single() const;
	void run_single(bool en);
	bool inactiveHidden();
	void setInactiveHidden(bool);

	Q_INVOKABLE void show();

private:
	void refreshApi();
	PatternGenerator *pg;
	QList<PatternGeneratorChannelGroup_API *> pg_cga;
	QList<PatternGeneratorChannel_API *> pg_cha;
};
}

#endif // PATTERN_GENERATOR_API_HPP
