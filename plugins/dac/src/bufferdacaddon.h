#ifndef BUFFERDACADDON_H_
#define BUFFERDACADDON_H_

#include <toolbuttons.h>
#include <menuonoffswitch.h>
#include <iio-widgets/iiowidget.h>

#include <QWidget>
#include <QTextBrowser>

#include "filebrowser.h"
#include "dacaddon.h"

namespace scopy {
namespace dac {
class DacDataModel;
class DataBuffer;
class TxNode;
class BufferDacAddon : public DacAddon
{
	Q_OBJECT
public:
	BufferDacAddon(DacDataModel *model, QWidget *parent = nullptr);
	virtual ~BufferDacAddon();
	virtual void enable(bool enable);

Q_SIGNALS:
	void log(QString log);
	void toggleCyclic(bool toggled);

public Q_SLOTS:
	void dataReload();
	void onLoadFailed();
	void onLoadFinished();

private Q_SLOTS:
	void load(QString path);
	void updateGuiStrategyWidget();
	void forwardSamplingFrequencyChange(QDateTime timestamp, QString oldData, QString newData, int retCode,
					    bool readOp);

private:
	DacDataModel *m_model;
	QWidget *m_optionalGuiStrategy;
	DataBuffer *m_dataBuffer;
	RunBtn *m_runBtn;
	TitleSpinBox *m_bufferSizeSpin;
	TitleSpinBox *m_fileSizeSpin;
	TitleSpinBox *m_kernelCountSpin;
	MenuOnOffSwitch *m_cyclicBtn;
	QTextBrowser *m_logText;
	FileBrowser *fm;

	void runBtnToggled(bool toggled);
	QWidget *createMenu(TxNode *node);
	QWidget *createAttrMenu(TxNode *node, QWidget *parent);
	void detectSamplingFrequency(IIOWidget *w);
};
} // namespace dac
} // namespace scopy

#endif // BUFFERDACADDON_H_
