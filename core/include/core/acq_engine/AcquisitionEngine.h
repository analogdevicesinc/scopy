#pragma once

#include "scopy-core_export.h"

#include "AcquisitionError.h"
#include "DataStore.h"
#include "ProcessorBlock.h"
#include "SourceBlock.h"

#include <atomic>
#include <QList>
#include <QObject>
#include <QSet>
#include <QThread>

namespace scopy {
namespace acq {

class SCOPY_CORE_EXPORT AcquisitionEngine : public QObject
{
	Q_OBJECT
public:
	explicit AcquisitionEngine(DataStore *store, QObject *parent = nullptr);
	~AcquisitionEngine() override;

	void                        addSource(SourceBlock *src);
	void                        removeSource(SourceBlock *src);
	const QList<SourceBlock *> &sources() const;

	void                           addProcessor(ProcessorBlock *proc);
	void                           removeProcessor(ProcessorBlock *proc);
	const QList<ProcessorBlock *> &processors() const;

	enum class Mode { Continuous, Triggered };
	void setMode(Mode m);
	Mode mode() const;

	bool        isRunning() const;
	void        setBufferSize(std::size_t size);
	std::size_t bufferSize() const;
	void         setMaxFPS(unsigned int fps);
	unsigned int maxFPS() const;

	void single(unsigned int count = 1);
	void run();
	void stop();

Q_SIGNALS:
	void started();
	void stopped();
	void forceStopped();
	void cycleComplete();
	void error(int severity, const QString &id, const QString &message);

private:
	void startLoop(int acqCount);
	void loop();
	bool runProcessors();
	void safeOnStop(SourceBlock *src);

	DataStore              *m_store;
	QList<SourceBlock *>    m_sources;
	QList<ProcessorBlock *> m_processors;
	QThread                *m_thread{nullptr};
	std::atomic<bool>       m_running{false};
	std::atomic<bool>       m_faultStop{false};
	std::atomic<Mode>       m_mode{Mode::Triggered};
	int                     m_acqCount{0};
	std::size_t             m_bufferSize{1024};
	unsigned int            m_maxFPS{0};
};

} // namespace acq
} // namespace scopy
