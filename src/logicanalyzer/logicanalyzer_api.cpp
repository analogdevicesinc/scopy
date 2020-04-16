#include "logicanalyzer_api.h"

using namespace adiscope::logic;

double LogicAnalyzer_API::getSampleRate() const
{
	return m_logic->m_sampleRateButton->value();
}

void LogicAnalyzer_API::setSampleRate(double sampleRate)
{
	m_logic->m_sampleRateButton->setValue(sampleRate);
}

int LogicAnalyzer_API::getBufferSize() const
{
	return m_logic->m_bufferSizeButton->value();
}

void LogicAnalyzer_API::setBufferSize(int bufferSize)
{
	m_logic->m_bufferSizeButton->setValue(bufferSize);
}
