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
#ifndef DIGITAL_CHANNEL_MANAGER_H
#define DIGITAL_CHANNEL_MANAGER_H


#include <QWidget>
#include <QPushButton>
#include <QJsonArray>
#include <QIntValidator>
#include <QtQml/QJSEngine>
#include <QtUiTools/QUiLoader>
#include <vector>
#include <string>

#include "filter.hpp"

//#include "pg_patterns.hpp"
#include "digitalchannel_manager.hpp"


#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/digital/m2kdigital.hpp>

using namespace libm2k::context;
using namespace libm2k::digital;

namespace adiscope {

class DIOManager : public QObject
{
	Q_OBJECT
	int direction;
	int lockMask;
	int nrOfChannels;
	int gpo;
	int gpi;
	bool outputEnabled;
	M2kDigital *digital;

public:
	void init();
	DIOManager(M2kDigital *digital, Filter *filt);
	~DIOManager();
	bool getOutputEnabled();
	void enableOutput(bool output);
	int getGpo();
	void setOutRaw(int ch, bool val);
	bool getOutRaw(int ch);
	int getGpi();
	bool getInRaw(int ch);
	void setDeviceOutRaw(int ch);

	void setOutputMode(int ch, bool mode);
	void setMode(int mask);
	void setDirection(int ch, bool output);
	bool getDirection(int ch);
	void setDeviceDirection(int ch, bool force);
	void lock(int mask);
	int getLockMask();
	bool isLocked(int ch);
	void unlock();

Q_SIGNALS:
	void locked();
	void unlocked();
};

}

#endif // PG_CHANNEL_MANAGER_H

