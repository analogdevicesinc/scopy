#include "emptydatastrategy.h"

using namespace scopy;

EmptyDataStrategy::EmptyDataStrategy(QObject *parent)
	: QObject{parent}
	, m_data("")
	, m_previousData("")
{}

QString EmptyDataStrategy::data() { return m_data; }

QString EmptyDataStrategy::optionalData() { return ""; }

int EmptyDataStrategy::write(QString data)
{
	m_previousData = data;
	m_data = data;
	return 0;
}

QPair<QString, QString> EmptyDataStrategy::read() { return {m_data, ""}; }

void EmptyDataStrategy::writeAsync(QString data)
{
	int res = write(data);
	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_data, data, (int)(res), false);
	readAsync();
}

void EmptyDataStrategy::readAsync()
{
	QPair<QString, QString> res = read();
	Q_EMIT emitStatus(QDateTime::currentDateTime(), m_previousData, m_data, 0, true);
	Q_EMIT sendData(res.first, res.second);
}

#include "moc_emptydatastrategy.cpp"
