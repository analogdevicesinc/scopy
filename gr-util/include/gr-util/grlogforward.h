/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef GRLOGFORWARD_H
#define GRLOGFORWARD_H

#include "scopy-gr-util_export.h"

#include <QObject>
#include <QString>
#include <spdlog/sinks/base_sink.h>
#include <mutex>

namespace scopy::grutil {

class SCOPY_GR_UTIL_EXPORT GRLogForward : public QObject, public spdlog::sinks::base_sink<std::mutex>
{
	Q_OBJECT

protected:
	GRLogForward(QObject *parent = nullptr);
	~GRLogForward();

	void sink_it_(const spdlog::details::log_msg &msg) override;
	void flush_() override;

public:
	GRLogForward(GRLogForward &other) = delete;
	void operator=(const GRLogForward &) = delete;
	static GRLogForward *GetInstance();

Q_SIGNALS:
	void newLogMessage(QString message);

private:
	static GRLogForward *pinstance_;
};

} // namespace scopy::grutil

#endif // GRLOGFORWARD_H
