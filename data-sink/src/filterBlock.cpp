#include <include/data-sink/basicBlock.h>
#include <include/data-sink/filterBlock.h>
using namespace scopy::datasink;

FilterBlock::FilterBlock(bool copy, QString name)
	: BasicBlock(name)
	, m_copy(true)
	, m_data(BlockData())
{
	doCopy(copy);
}

FilterBlock::~FilterBlock() {}

void FilterBlock::doCopy(bool copy) { m_copy = copy; }

bool FilterBlock::areChannelsReady()
{
	for(auto it = m_connectedChannels.keyValueBegin(); it != m_connectedChannels.keyValueEnd(); ++it) {
		if(!it->second)
			return false;
	}

	return true;
}

// set data recieved to false for every channel
void FilterBlock::resetChannels()
{
	for(auto it = m_connectedChannels.keyValueBegin(); it != m_connectedChannels.keyValueEnd(); ++it) {
		it->second = false;
	}
	// m_data->clearVectors();
}

void FilterBlock::onNewData(ChannelDataVector chData, uint ch)
{
	// ChannelDataVector new_data = createData();
	// Q_EMIT newData(new_data, 0);
	// return;

	if(m_copy) {
		chData.setCopiedData(chData.data);
	}

	if(m_data.contains(ch))
		m_data[ch].data = chData.data;
	else
		m_data.insert(ch, chData);

	m_connectedChannels[ch] = true;
	if(areChannelsReady()) {
		ChannelDataVector new_data = createData();
		// new_data->clear();
		// delete new_data;

		if(new_data.data.empty()) {
			qWarning() << m_name << ": Empty buffer from filter!";
		} else {
			Q_EMIT newData(new_data, 0); // FORCED TO CH0 SINCE FILTERS ONLY SUPPORT 1 OUTPUT
		}
		resetChannels();
	}
}

void FilterBlock::addConnectedChannel(uint ch) { m_connectedChannels.insert(ch, false); }

void FilterBlock::removeConnectedChannel(uint ch) { m_connectedChannels.remove(ch); }
