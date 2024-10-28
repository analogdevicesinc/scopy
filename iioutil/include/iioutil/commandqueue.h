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

#ifndef IIOCOMMANDQUEUE_H
#define IIOCOMMANDQUEUE_H

#include "command.h"

#include <QThread>
#include <QThreadPool>
#include <QTime>

#include <deque>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CommandQueue : public QObject
{
	Q_OBJECT
public:
	/**
	 * @brief IioCommandQueue::IioCommandQueue
	 * @param numberOfThreads
	 * @param parent
	 */
	explicit CommandQueue(QObject *parent = nullptr);
	~CommandQueue();
	void enqueue(Command *newCmd);
	void start();
	void wait();
	void requestStop();
	void runCmd();
	QTime lastCmdTime() const;

private Q_SLOTS:
	void resolveNext(scopy::Command *cmd);

private:
	std::deque<Command *> m_commandQueue;
	std::mutex m_commandMutex;
	std::mutex m_enqueueMutex;
	std::atomic<bool> m_running;
	QThreadPool m_commandExecThreadPool;
	QTime m_lastCmdTime;
};
} // namespace scopy
#endif // IIOCOMMANDQUEUE_H
