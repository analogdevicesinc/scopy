#ifndef TOOL_LAUNCHER_DRAWAREA_H
#define TOOL_LAUNCHER_DRAWAREA_H

#include <QWidget>
#include <QLabel>
#include <QMap>

namespace scopy::swiot {
class DrawArea : public QWidget {
Q_OBJECT

public:
	explicit DrawArea(QWidget *parent = nullptr);

	~DrawArea() override;

	enum ChannelName : int {
		AD74413R = 0,
		MAX14906 = 1
	};

	void activateConnection(int channelIndex, ChannelName channelName);
	void deactivateConnections();

private:
	QString m_filePath;
	QImage *m_boardImage;
	const QImage *m_baseImage;

	QLabel *m_drawLabel;

	// channel-device pairs
	QMap<QPair<int, int>, QPixmap *> *m_connectionsMap;
};
}

#endif //TOOL_LAUNCHER_DRAWAREA_H
