#ifndef ADCACQUISITIONMANAGER_H
#define ADCACQUISITIONMANAGER_H

#include <QObject>
#include <gr-util/griiodevicesource.h>
#include <gr-util/grtopblock.h>
#include <gr-util/griiofloatchannelsrc.h>
#include <gr-util/grsignalpath.h>
#include <gr-util/grscaleoffsetproc.h>
#include <scopy-adcplugin_export.h>

namespace scopy {
using namespace grutil;
class AcqTreeNode;

class SCOPY_ADCPLUGIN_EXPORT AcqTree : public QObject {
	Q_OBJECT
public:
	AcqTree(QObject *parent = nullptr);;
	~AcqTree();

	QList<AcqTreeNode*> m_nodes;
	void reset();

};

class SCOPY_ADCPLUGIN_EXPORT AcqTreeNode : public QObject {
	Q_OBJECT
public:
	AcqTreeNode(QString name, QObject *parent = nullptr);
	~AcqTreeNode();

	void addChild(AcqTreeNode *t);
	bool removeChild(AcqTreeNode *t);
	AcqTree *tree();

	virtual void *data();

protected:
	AcqTree *m_tree;
	void *m_data;
	QString m_name;
	QList<AcqTreeNode*> m_children;

};

class SCOPY_ADCPLUGIN_EXPORT IIOContextNode : public AcqTreeNode {

};

class SCOPY_ADCPLUGIN_EXPORT GRTopBlockNode : public AcqTreeNode {
public:
	GRTopBlockNode(GRTopBlock *g, QObject *parent = nullptr);
	~GRTopBlockNode();
};

class SCOPY_ADCPLUGIN_EXPORT GRIIODeviceSourceNode : public AcqTreeNode {
public:
	GRIIODeviceSourceNode(GRIIODeviceSource *d, QObject *parent = nullptr);
	~GRIIODeviceSourceNode();


};

class SCOPY_ADCPLUGIN_EXPORT GRIIOFloatChannelNode: public AcqTreeNode {
public:
	GRIIOFloatChannelNode(GRIIOChannel *c, QObject *parent = nullptr);
	~GRIIOFloatChannelNode();

	GRSignalPath *signalPath() const;

	GRScaleOffsetProc *scOff() const;


private:
	GRSignalPath *m_signalPath;
	GRScaleOffsetProc *m_scOff;
};
/*
class IIODeviceNode : public AcqTreeNode {
public:
	IIODeviceNode(struct iio_device *dev, QString name, QObject *parent = nullptr) : AcqTreeNode(name,parent), m_dev(dev) {
		m_devId = iio_device_get_id(m_dev);
	}
	~IIODeviceNode() {}

     virtual void enable() override {};
     virtual void disable() override {};

private:
     size_t m_buffersize;
     QString m_devId;
     struct iio_device *m_dev;
     struct iio_buffer *m_buf;
};

class IIOChannelNode: public AcqTreeNode {
public:
     IIOChannelNode(struct iio_channel *ch, QString name, QObject *parent = nullptr) : AcqTreeNode(name,parent), m_ch(ch) {
	     m_chId= iio_channel_get_id(m_ch);
     }
     ~IIOChannelNode() {}

     virtual void enable() override { iio_channel_enable(m_ch);};
     virtual void disable() override { iio_channel_disable(m_ch);};

private:
     size_t m_buffersize;
     QString m_chId;
     struct iio_channel *m_ch;
};

 */
}

#endif // ADCACQUISITIONMANAGER_H
