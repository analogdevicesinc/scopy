/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QSettings>
#include <QtGlobal>

#include "config.h"
#include "tool_launcher.hpp"

using namespace adiscope;

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	auto pythonpath = qgetenv("SCOPY_PYTHONPATH");
	if (!pythonpath.isNull())
		qputenv("PYTHONPATH", pythonpath);

	QCoreApplication::setOrganizationName("ADI");
	QCoreApplication::setOrganizationDomain("analog.com");
	QCoreApplication::setApplicationName("Scopy");
	QCoreApplication::setApplicationVersion(SCOPY_VERSION_GIT);
	QSettings::setDefaultFormat(QSettings::IniFormat);

	QCommandLineParser parser;

	parser.addHelpOption();
	parser.addVersionOption();

	parser.process(app);

	ToolLauncher launcher;
	launcher.show();

	return app.exec();
}
