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

#include "grlogforward.h"
#include "grlog.h"

#include <gnuradio/logger.h>

#include <QApplication>
#include <QMetaObject>

namespace scopy::grutil {

GRLogForward *GRLogForward::pinstance_{nullptr};

GRLogForward::GRLogForward(QObject *parent)
	: QObject(parent)
{}

GRLogForward::~GRLogForward() {}

void GRLogForward::sink_it_(const spdlog::details::log_msg &msg)
{
	spdlog::memory_buf_t formatted;
	formatter_->format(msg, formatted);
	QString qmsg = "[gnuradio] " + QString::fromStdString(fmt::to_string(formatted)).trimmed();

	switch(msg.level) {
	case spdlog::level::debug:
		qDebug(SCOPY_GR_UTIL) << qmsg;
		break;
	case spdlog::level::info:
		qInfo(SCOPY_GR_UTIL) << qmsg;
		break;
	case spdlog::level::warn:
		qWarning(SCOPY_GR_UTIL) << qmsg;
		break;
	case spdlog::level::err:
		qCritical(SCOPY_GR_UTIL) << qmsg;
		break;
	case spdlog::level::critical:
		qCritical(SCOPY_GR_UTIL) << qmsg;
		break;
	default:
		qInfo(SCOPY_GR_UTIL) << qmsg;
		break;
	}

	Q_EMIT newLogMessage(qmsg);
}

void GRLogForward::flush_() {}

GRLogForward *GRLogForward::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new GRLogForward(QApplication::instance());
		pinstance_->set_pattern("%v");
		gr::logging::singleton().add_default_sink(
			std::shared_ptr<GRLogForward>(pinstance_, [](GRLogForward *) {}));
		qInfo(SCOPY_GR_UTIL) << "GNU Radio log forwarding initialized";
	}
	return pinstance_;
}

} // namespace scopy::grutil

#include "moc_grlogforward.cpp"
