
// #include <include/data-sink/blockManager.h>
// #include <include/data-sink/proxyblock.h>
// #include <include/data-sink/scaleoffsetfilter.h>
// using namespace scopy::datasink;

// // ScaleOffsetFilter Implementation
// ScaleOffsetFilter::ScaleOffsetFilter(QString name)
// 	: FilterBlock(false, name)
// 	, m_scale(1.0)
// 	, m_offset(0.0)
// {
// }

// ScaleOffsetFilter::~ScaleOffsetFilter()
// {
// }

// void ScaleOffsetFilter::setScale(double scale)
// {
// 	// QMutexLocker locker(&m_paramMutex);
// 	m_scale = scale;
// }

// void ScaleOffsetFilter::setOffset(double offset)
// {
// 	// QMutexLocker locker(&m_paramMutex);
// 	m_offset = offset;
// }

// ChannelDataVector* ScaleOffsetFilter::createData()
// {
// 	for(int i = 0; i < m_data->size(); ++i) {
// 		for(auto chData : *m_data)
// 			if(chData == firstCh) {
// 				new_data->data.push_back(chData->data[i]);
// 			} else {
// 				new_data->data[i] += chData->data[i];
// 			}
// 	}

// 	return nullptr;
// }

// // ScaleOffsetProc Implementation
// ScaleOffsetProc::ScaleOffsetProc(QString name, QObject *parent)
// 	: ProxyBlock(parent)
// 	, m_filter(nullptr)
// {
// 	createFilterBlock();
// }

// ScaleOffsetProc::~ScaleOffsetProc()
// {
// 	destroyFilterBlock();
// }

// void ScaleOffsetProc::createFilterBlock()
// {
// 	if (!m_filter) {
// 		m_filter = new ScaleOffsetFilter("ScaleOffset");
// 		m_built = true;
// 	}
// }

// void ScaleOffsetProc::destroyFilterBlock()
// {
// 	if (m_filter) {
// 		delete m_filter;
// 		m_filter = nullptr;
// 		m_built = false;
// 	}
// }

// void ScaleOffsetProc::setScale(double scale)
// {
// 	if (m_filter) {
// 		m_filter->setScale(scale);
// 	}
// }

// void ScaleOffsetProc::setOffset(double offset)
// {
// 	if (m_filter) {
// 		m_filter->setOffset(offset);
// 	}
// }

// double ScaleOffsetProc::getScale() const
// {
// 	return m_filter ? m_filter->getScale() : 1.0;
// }

// double ScaleOffsetProc::getOffset() const
// {
// 	return m_filter ? m_filter->getOffset() : 0.0;
// }
