#ifndef BUFFERMENU_H
#define BUFFERMENU_H

#include "qiqutils.h"

#include <QWidget>
#include <QMap>
#include <QStringList>
#include <QSpinBox>
#include <menucombo.h>
#include <menuspinbox.h>

using namespace scopy::gui;

namespace scopy::qiqplugin {
class BufferMenu : public QWidget
{
	Q_OBJECT

public:
	BufferMenu(QWidget *parent = nullptr);
	~BufferMenu();

	void setAvailableChannels(const QMap<QString, QStringList> &channels);
	void updateChnList();

Q_SIGNALS:
	void bufferParamsChanged(BufferParams &params);

private Q_SLOTS:
	void onParamsChanged();

private:
	QStringList m_enChannels;
	QMap<QString, QStringList> m_availableChannels;
	MenuCombo *m_deviceCombo;
	MenuSpinbox *m_bufferSizeSpin;
	QWidget *m_chnList;

	void setupUI();
};

} // namespace scopy::qiqplugin

#endif // BUFFERMENU_H
