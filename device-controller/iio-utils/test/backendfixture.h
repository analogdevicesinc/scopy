#pragma once

// Shared harness for the per-interface backend conformance tests. Each test file
// is data-driven over whichever libiio backends actually load (v0, v1), and skips
// cleanly when a plugin or a live context isn't available.
//
// URI comes from the DC_TEST_URI env var (default ip:192.168.2.1). Set it to point
// at a reachable context (or a local iiod / network device) to exercise the live
// paths; otherwise the context-dependent rows QSKIP.

#include "iioutil/iiobackendloader.h"
#include "iioutil/ibackend.h"
#include "iioutil/icontextops.h"

#include <QProcessEnvironment>
#include <QString>
#include <QTest>

namespace dctest {

inline QString testUri()
{
	const QString env = QProcessEnvironment::systemEnvironment().value(QStringLiteral("DC_TEST_URI"));
	return env.isEmpty() ? QStringLiteral("ip:192.168.2.1") : env;
}

// Adds one "version" column row per backend plugin that is loadable right now.
inline void addBackendRows()
{
	QTest::addColumn<int>("version");
	auto *loader = scopy::iio::IIOBackendLoader::instance();
	if(loader->load(scopy::iio::LibiioVersion::V0)) {
		QTest::newRow("v0") << static_cast<int>(scopy::iio::LibiioVersion::V0);
	}
	if(loader->load(scopy::iio::LibiioVersion::V1)) {
		QTest::newRow("v1") << static_cast<int>(scopy::iio::LibiioVersion::V1);
	}
}

// Loads the backend for the given version. Returns nullptr if the plugin is absent.
inline scopy::iio::IBackend *loadBackend(scopy::iio::LibiioVersion v)
{
	auto *loader = scopy::iio::IIOBackendLoader::instance();
	if(!loader->load(v) || !loader->isLoaded()) {
		return nullptr;
	}
	return loader->backend();
}

// Opens a live context at the test URI. Returns a null handle if unreachable.
inline scopy::iio::ContextHandle openContext(scopy::iio::IBackend *b)
{
	return b->contextOps()->createContext(testUri());
}

// True when the caller has opted into state-mutating conformance paths (buffer
// open/refill/push, attr/register writes, channel enable/disable, trigger set).
// These allocate kernel buffers, do DMA, or change device state, so they only run
// against a context the user has explicitly marked safe (a dummy/loopback device).
inline bool mutationAllowed()
{
	const QString v = QProcessEnvironment::systemEnvironment().value(QStringLiteral("DC_TEST_ALLOW_MUTATION"));
	return v == QStringLiteral("1") || v.compare(QStringLiteral("true"), Qt::CaseInsensitive) == 0;
}

} // namespace dctest

// Convenience: fetch the row's backend or QSKIP. Declares `b` (IBackend*) in scope.
#define DC_REQUIRE_BACKEND()                                                                                           \
	QFETCH(int, version);                                                                                          \
    scopy::iio::IBackend *b = dctest::loadBackend(static_cast<scopy::iio::LibiioVersion>(version));                \
	if(!b) {                                                                                                       \
		QSKIP("backend plugin not available");                                                                 \
	}

// Convenience: open a live context or QSKIP. Declares `ctx` (ContextHandle) in scope.
#define DC_REQUIRE_CONTEXT()                                                                                           \
	scopy::iio::ContextHandle ctx = dctest::openContext(b);                                                        \
	if(!ctx.ptr) {                                                                                                 \
		QSKIP("no live context at test URI (set DC_TEST_URI)");                                                \
	}

// Convenience: QSKIP unless the caller opted into state-mutating paths. Use at the
// top of any slot that allocates buffers, writes attrs/registers, or flips device
// state, AFTER DC_REQUIRE_CONTEXT so the context is torn down cleanly on skip.
#define DC_REQUIRE_MUTATION()                                                                                          \
	if(!dctest::mutationAllowed()) {                                                                               \
		ctxOps->destroyContext(ctx);                                                                           \
		QSKIP("mutation tests disabled (set DC_TEST_ALLOW_MUTATION=1 against a safe device)");                 \
	}
