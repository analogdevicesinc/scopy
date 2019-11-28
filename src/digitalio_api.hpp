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
#ifndef DIGITALIO_API_HPP
#define DIGITALIO_API_HPP

#include "digitalio.hpp"

namespace adiscope {
class DigitalIO_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(QList<bool> group READ grouped WRITE setGrouped SCRIPTABLE false);
	Q_PROPERTY(QList<bool> dir READ direction WRITE setDirection SCRIPTABLE true);
	Q_PROPERTY(QList<bool> out READ output    WRITE setOutput SCRIPTABLE true);

	Q_PROPERTY(QList<bool> gpi READ gpi STORED false);
	Q_PROPERTY(QList<bool> locked READ locked STORED false);
	Q_PROPERTY(bool running READ running WRITE run STORED false);

public:
	explicit DigitalIO_API(DigitalIO *dio) : ApiObject(), dio(dio) {}
	~DigitalIO_API() {}

	QList<bool> direction() const;
	void setDirection(const QList<bool>& list);
	QList<bool> output() const;
	void setOutput(const QList<bool>& list);
	void setOutput(int ch, int direction);
	QList<bool> grouped() const;
	void setGrouped(const QList<bool>& grouped);

	QList<bool> gpi() const;
	QList<bool> locked() const;
	bool running() const;
	void run(bool en);

	Q_INVOKABLE void show();

private:
	DigitalIO *dio;
};
}

#endif // DIGITALIO_API_HPP
