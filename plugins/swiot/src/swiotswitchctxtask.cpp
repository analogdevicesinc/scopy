#include "swiotswitchctxtask.h"
#include <iioutil/contextprovider.h>
#include "src/swiot_logging_categories.h"

using namespace adiscope::swiot;

SwiotSwitchCtxTask::SwiotSwitchCtxTask(QString uri) : QThread()
{
	m_uri = uri;
}

void SwiotSwitchCtxTask::run()
{
	iio_context *ctx = ContextProvider::GetInstance()->open(m_uri);
	if(!ctx)
		goto finish;
	//need to be changed to CAT_SWIOTPLUGIN
	qDebug(CAT_SWIOT_AD74413R)<<"Context has been changed";

	Q_EMIT contextSwitched();
finish:
	ContextProvider::GetInstance()->close(m_uri);
	return;
}

