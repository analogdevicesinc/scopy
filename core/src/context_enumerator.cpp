#include <iio.h>

#include <QtConcurrentRun>

#include <iostream>
#include <scopy/core/context_enumerator.hpp>

constexpr int TIMER_TIMEOUT_MS = 5000;

using namespace scopy::core;

ContextEnumerator::ContextEnumerator()
{
	connect(&m_searchTimer, &QTimer::timeout, this, &scopy::core::ContextEnumerator::search);
	connect(&m_watcher, &QFutureWatcher<QVector<QString>>::finished, this,
		[=]() { Q_EMIT printData(m_future.result()); });
	start();
}

void ContextEnumerator::search()
{
	m_searchTimer.stop();
	m_future = QtConcurrent::run(this, &ContextEnumerator::searchDevices);
	m_watcher.setFuture(m_future);
}

QStringList ContextEnumerator::searchDevices()
{
	struct iio_context_info** info;
	unsigned int nb_contexts;
	QStringList uris;

	struct iio_scan_context* scan_ctx = iio_create_scan_context("usb", 0);

	if (!scan_ctx) {
		std::cerr << "Unable to create scan context!" << std::endl;
		return uris;
	}

	ssize_t ret = iio_scan_context_get_info_list(scan_ctx, &info);

	if (ret < 0) {
		std::cerr << "Unable to scan!" << std::endl;
		goto out_destroy_context;
	}

	nb_contexts = static_cast<unsigned int>(ret);

	for (unsigned int i = 0; i < nb_contexts; i++)
		uris.append(QString(iio_context_info_get_uri(info[i])));

	iio_context_info_list_free(info);
out_destroy_context:
	iio_scan_context_destroy(scan_ctx);
	return uris;
}

void ContextEnumerator::start() { m_searchTimer.start(TIMER_TIMEOUT_MS); }
