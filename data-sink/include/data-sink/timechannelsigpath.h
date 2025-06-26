#ifndef TIMECHANNELSIGPATH_H
#define TIMECHANNELSIGPATH_H
#include "blockManager.h"
#include "scaleoffsetfilter.h"
#include "scopy-data-sink_export.h"
#include "sourceBlock.h"

#include <QObject>
#include <channelcomponent.h>

/**
 * Time channel signal path - manages a complete signal processing chain for a single channel
 */
namespace scopy::datasink {
// Offset filter implementation
class SCOPY_DATA_SINK_EXPORT OffsetFilter : public FilterBlock
{
	Q_OBJECT
public:
	OffsetFilter(QString name = "OffsetFilter");

	void setOffset(double offset);
	void setScale(double scale);
	double getOffset() const;
	double getScale() const;

protected:
	ChannelDataVector* createData() override;

private:
	double m_offset;
	double m_scale;
};

// Simplified proxy block concept (no longer needed for GR, but keeping interface similarity)
class SCOPY_DATA_SINK_EXPORT ProxyBlock : public QObject
{
	Q_OBJECT
public:
	ProxyBlock(QObject *parent = nullptr);
	virtual ~ProxyBlock();

	void setEnabled(bool v);
	bool enabled() const;

Q_SIGNALS:
	void enabledChanged(bool enabled);
	void requestRebuild();

protected:
	bool m_enabled;
};

// Signal path equivalent - manages the chain of processing
class SCOPY_DATA_SINK_EXPORT SignalPath : public ProxyBlock
{
	Q_OBJECT
public:
	SignalPath(QString name, QObject *parent = nullptr);

	void setManager(BlockManager *manager);
	BlockManager *manager() const;

	QString name() const;

	void addFilter(FilterBlock *filter);
	void removeFilter(FilterBlock *filter);
	QList<FilterBlock*> filters() const;

protected:
	QString m_name;
	BlockManager *m_manager;
	QList<FilterBlock*> m_filters;
};

// Main replacement for GRTimeChannelSigpath
class SCOPY_DATA_SINK_EXPORT TimeChannelSigpath : public QObject
{
	Q_OBJECT
public:
	TimeChannelSigpath(QString name, ChannelComponent *ch, SourceBlock *sourceBlock,
			   uint sourceChannel, uint outputChannel, BlockManager *manager, QObject *parent = nullptr);
	~TimeChannelSigpath();

	// Enable/disable the signal path
	void enable();
	void disable();
	bool isEnabled() const;

	// Offset control
	void setOffset(double offset);
	void setScale(double scale);
	double getOffset() const;
	double getScale() const;

	// Access to components
	SignalPath *sigpath();
	ChannelComponent *channelComponent() const;
	SourceBlock *sourceBlock() const;
	uint sourceChannel() const;
	uint outputChannel() const;
	OffsetFilter *offsetFilter() const;

private Q_SLOTS:
	void onManagerNewData(ChannelDataVector *data, uint ch);

private:
	QString m_name;
	ChannelComponent *m_ch;
	SourceBlock *m_sourceBlock;
	uint m_sourceChannel;
	uint m_outputChannel;
	BlockManager *m_manager;
	SignalPath *m_signalPath;
	OffsetFilter *m_offsetFilter;
	bool m_enabled;
	bool m_filterConnected;

	void forwardDataToChannel(ChannelDataVector *data);
};
}

#endif // TIMECHANNELSIGPATH_H
