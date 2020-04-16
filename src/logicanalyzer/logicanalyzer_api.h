#ifndef LOGICANALYZER_API_H
#define LOGICANALYZER_API_H

#include "logic_analyzer.h"
#include "apiObject.hpp"

namespace adiscope {
namespace logic {
class LogicAnalyzer_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(double sampleRate READ getSampleRate WRITE setSampleRate);
	Q_PROPERTY(int bufferSize READ getBufferSize WRITE setBufferSize);

public:
	explicit LogicAnalyzer_API(logic::LogicAnalyzer *logic):
	ApiObject(), m_logic(logic) {}
	~LogicAnalyzer_API() {}

	double getSampleRate() const;
	void setSampleRate(double sampleRate);

	int getBufferSize() const;
	void setBufferSize(int bufferSize);

private:
	logic::LogicAnalyzer *m_logic;
};
}
}

#endif // LOGICANALYZER_API_H
