#pragma once

#include "DataKey.h"
#include "ProcessorBlock.h"

#include <atomic>
#include <QList>

namespace scopy {
namespace adc {
namespace sim {

// Processor that applies per-channel linear transform: output[i] = scale * input[i] + offset.
// Reads <inputKey> from the DataStore, writes the result to <outputKey>.
// scale and offset are set from the GUI thread and read from the worker thread via atomics.
class ScaleOffsetProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	struct ChannelConfig
	{
		DataKey            inputKey;
		DataKey            outputKey;
		QString            label;
		std::atomic<float> scale{1.0f};
		std::atomic<float> offset{0.0f};

		// Non-copyable, non-movable (atomic members)
		ChannelConfig(const ChannelConfig &) = delete;
		ChannelConfig &operator=(const ChannelConfig &) = delete;
		ChannelConfig(DataKey in, DataKey out, QString lbl)
			: inputKey(std::move(in))
			, outputKey(std::move(out))
			, label(std::move(lbl))
		{}
	};

	explicit ScaleOffsetProcessor(const QString &name, QObject *parent = nullptr);
	~ScaleOffsetProcessor() override;

	// Register a channel. Returns the config so the widget can bind spinboxes directly.
	ChannelConfig *addChannel(const DataKey &inputKey, const DataKey &outputKey,
				  const QString &label = {});

	QList<ChannelConfig *> channels() const { return m_channels; }

	void     process(DataStore *store) override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

private:
	QList<ChannelConfig *> m_channels; // owned
};

} // namespace sim
} // namespace adc
} // namespace scopy
