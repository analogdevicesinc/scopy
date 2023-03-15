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
#ifndef POWER_CONTROLLER_API_HPP
#define POWER_CONTROLLER_API_HPP

#include "power_controller.hpp"

using namespace adiscope;
namespace adiscope::m2k {

class PowerController_API : public adiscope::ApiObject
{
	Q_OBJECT

	Q_PROPERTY(bool sync READ syncEnabled WRITE enableSync);
	Q_PROPERTY(int tracking_percent
			READ getTrackingPercent
			WRITE setTrackingPercent);
	Q_PROPERTY(double dac1_value READ valueDac1 WRITE setValueDac1);
	Q_PROPERTY(double dac2_value READ valueDac2 WRITE setValueDac2);
	Q_PROPERTY(bool dac1_enabled READ Dac1Enabled WRITE setDac1Enabled);
	Q_PROPERTY(bool dac2_enabled READ Dac2Enabled WRITE setDac2Enabled);
	Q_PROPERTY(QString notes READ getNotes WRITE setNotes)

public:
	explicit PowerController_API(PowerController *pw) :
		ApiObject(), pw(pw) {}
	~PowerController_API() {}

	bool syncEnabled() const;
	void enableSync(bool en);

	int getTrackingPercent() const;
	void setTrackingPercent(int percent);

	double valueDac1() const;
	void setValueDac1(double value);

	double valueDac2() const;
	void setValueDac2(double value);

	bool Dac1Enabled() const;
	void setDac1Enabled(bool enable);

	bool Dac2Enabled() const;
	void setDac2Enabled(bool enable);

	QString getNotes();
	void setNotes(QString str);

	Q_INVOKABLE void show();

private:
	PowerController *pw;
};
}

#endif // POWER_CONTROLLER_API_HPP
