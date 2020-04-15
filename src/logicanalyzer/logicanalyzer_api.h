#ifndef LOGICANALYZER_API_H
#define LOGICANALYZER_API_H

#include "logic_analyzer.h"
#include "apiObject.hpp"

namespace adiscope {

class LogicAnalyzer_API : public ApiObject
{
	Q_OBJECT
public:
	explicit LogicAnalyzer_API(logic::LogicAnalyzer *logic):
	ApiObject(), m_logic(logic) {}
	~LogicAnalyzer_API() {}

private:
	logic::LogicAnalyzer *m_logic;
};
}

#endif // LOGICANALYZER_API_H
