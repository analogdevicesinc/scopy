#include "connection.h"

using namespace scopy;

Connection::Connection(QString uri)
{
	this->m_uri = uri;
	this->m_context = nullptr;
	this->m_commandQueue = nullptr;
	this->m_iioTreeScan = nullptr;
	this->m_refCount = 0;
}

Connection::~Connection()
{
	if(this->m_commandQueue) {
		delete this->m_commandQueue;
		this->m_commandQueue = nullptr;
	}
	if(this->m_iioTreeScan) {
		delete this->m_iioTreeScan;
		this->m_iioTreeScan = nullptr;
	}
	if(this->m_context) {
		iio_context_destroy(this->m_context);
		this->m_context = nullptr;
	}
}

const QString &Connection::uri() const { return m_uri; }

CommandQueue *Connection::commandQueue() const { return m_commandQueue; }

IIOTreeScan *Connection::iioTreeScan() const { return m_iioTreeScan; }

iio_context *Connection::context() const { return m_context; }

int Connection::refCount() const { return m_refCount; }

void Connection::open()
{
	if(!this->m_context) {
		this->m_context = iio_create_context_from_uri(this->m_uri.toStdString().c_str());
		if(this->m_context) {
			this->m_commandQueue = new CommandQueue();
			this->m_iioTreeScan = new IIOTreeScan(this->m_context);
			this->m_refCount++;
		}
	} else {
		this->m_refCount++;
	}
}

void Connection::closeAll()
{
	this->m_refCount = 0;
	close();
}

void Connection::close()
{
	this->m_refCount--;
	if(this->m_refCount <= 0) {
		/* If the open() and close() number of calls done by a client
		 * is mismatched, all the remaining clients should be notified of the
		 * destruction. */
		this->m_refCount = 0;
		Q_EMIT aboutToBeDestroyed();
	}
}

#include "moc_connection.cpp"
