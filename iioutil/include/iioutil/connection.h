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

#ifndef CONNECTION_H
#define CONNECTION_H
#include "scopy-iioutil_export.h"
#include "commandqueue.h"
#include <iio.h>
#include <QObject>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT Connection : public QObject
{
	Q_OBJECT
public:
	Connection(QString uri);

	const QString &uri() const;
	CommandQueue *commandQueue() const;
	struct iio_context *context() const;
	int refCount() const;

protected:
	~Connection();

	/**
	 * @brief open
	 * Initialize the connection if not previously opened.
	 * If previously opened, increase the internal refCount.
	 */
	void open();

	/**
	 * @brief close
	 * Decrement the internal refCount.
	 * Emit the aboutToBeDestroyed() signal if refCount is zero.
	 */
	void close();

	/**
	 * @brief closeAll
	 * Reset the internal refCount to zero.
	 * Force close the Connection.
	 * Emit the aboutToBeDestroyed() signal.
	 */
	void closeAll();

Q_SIGNALS:
	/**
	 * @brief aboutToBeDestroyed
	 * Connection clients should handle deinitialization
	 * of their iio_context/CommandQueue related operations
	 * in a slot connected to this signal.
	 * After the signal is emitted, the Connection object
	 * will no longer be valid.
	 */
	void aboutToBeDestroyed();

private:
	friend class ConnectionProvider;
	QString m_uri;
	CommandQueue *m_commandQueue;
	struct iio_context *m_context;
	int m_refCount = 0;
};
} // namespace scopy

#endif // CONNECTION_H
