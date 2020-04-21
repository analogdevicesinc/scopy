/*
 * Copyright 2018 Analog Devices, Inc.
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

#include "external_script_api.hpp"

#include <QApplication>
#include <QElapsedTimer>
#include <QJSEngine>
#include <QMetaProperty>
#include <QThread>

using namespace adiscope;

ExternalScript_API::ExternalScript_API(QObject *parent)
	: QObject(parent)
	, m_external_process(nullptr)
	, m_working_dir(QCoreApplication::applicationDirPath())
	, m_process_timeout(0) {}

QString ExternalScript_API::start(const QString &cmd) {
	QString ret = "";
	if (m_external_process != nullptr) {
		m_external_process->deleteLater();
		m_external_process = nullptr;
	}
	m_external_process = new QProcess(this);
	m_external_process->setWorkingDirectory(m_working_dir);

	QStringList arguments;
	arguments << "-c" << cmd;
	m_external_process->start("/bin/sh", arguments);

	if (m_process_timeout > 0) {
		m_external_process->waitForFinished(m_process_timeout);
		ret = m_external_process->readAll();
		m_external_process->deleteLater();
		m_external_process = nullptr;
	} else {
		m_external_process->waitForFinished(-1);
		ret = m_external_process->readAll();
	}
	return ret;
}

void ExternalScript_API::setWorkingDir(const QString &root) { m_working_dir = root; }

QString ExternalScript_API::getWorkingDir() { return m_working_dir; }

void ExternalScript_API::setProcessTimeout(int timeout_ms) { m_process_timeout = timeout_ms; }

int ExternalScript_API::getProcessTimeout() { return m_process_timeout; }
