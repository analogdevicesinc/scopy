// #ifndef SCALEOFFSETFILTER_H
// #define SCALEOFFSETFILTER_H
// #include "scopy-data-sink_export.h"
// #include "filterBlock.h"
// #include "proxyblock.h"

// #include <QMutex>

// /**
//  * Scale and offset filter block
//  */
// namespace scopy::datasink {
// class SCOPY_DATA_SINK_EXPORT ScaleOffsetFilter : public FilterBlock
// {
// 	Q_OBJECT
// public:
// 	ScaleOffsetFilter(QString name = "ScaleOffset");
// 	virtual ~ScaleOffsetFilter();

// 	void setScale(double scale);
// 	void setOffset(double offset);
// 	double getScale() const { return m_scale; }
// 	double getOffset() const { return m_offset; }

// protected:
// 	ChannelDataVector* createData() override;

// private:
// 	double m_scale;
// 	double m_offset;
// 	QMutex m_paramMutex;
// };

// /**
//  * Proxy block wrapper for ScaleOffsetFilter
//  */
// class SCOPY_DATA_SINK_EXPORT ScaleOffsetProc : public ProxyBlock
// {
// 	Q_OBJECT
// public:
// 	ScaleOffsetProc(QString name, QObject *parent = nullptr);
// 	virtual ~ScaleOffsetProc();

// 	void setScale(double scale);
// 	void setOffset(double offset);
// 	double getScale() const;
// 	double getOffset() const;

// 	FilterBlock* getFilterBlock() override { return m_filter; }

// protected:
// 	void createFilterBlock() override;
// 	void destroyFilterBlock() override;

// private:
// 	ScaleOffsetFilter* m_filter;
// };
// }

// #endif // SCALEOFFSETFILTER_H
