/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef ADCACQUISITIONMANAGER_H
#define ADCACQUISITIONMANAGER_H

#include <QObject>
#include <scopy-adc_export.h>
#include <QMap>
#include <interfaces.h>
#include <adcinterfaces.h>
#include <synccontroller.h>
#include <customSourceBlocks.h>

namespace scopy {
namespace adc {
class AcqTreeNode;

class SCOPY_ADC_EXPORT AcqTreeNode : public QObject
{
	Q_OBJECT
public:
	AcqTreeNode(QString name, QObject *parent = nullptr);
	~AcqTreeNode();

	void addTreeChild(AcqTreeNode *t);
	bool removeTreeChild(AcqTreeNode *t);
	// AcqTree *tree();

	virtual void *data();
	QList<AcqTreeNode *> bfs();

	QString name() const;

	AcqTreeNode *treeParent() const;
	void setTreeParent(AcqTreeNode *newTreeParent);
	QList<AcqTreeNode *> treeChildren() const;
	AcqTreeNode *treeRoot();

Q_SIGNALS:
	void newChild(AcqTreeNode *);
	void deletedChild(AcqTreeNode *);

protected:
	// AcqTree *m_tree;
	void *m_data;
	QString m_name;
	QList<AcqTreeNode *> m_treeChildren;
	AcqTreeNode *m_treeParent;
};

class SCOPY_ADC_EXPORT IIOContextNode : public AcqTreeNode
{};

class SCOPY_ADC_EXPORT TopBlockNode : public AcqTreeNode
{
public:
	TopBlockNode(QString name, QObject *parent = nullptr);
	~TopBlockNode();
	SyncController *sync() const;
	iio_context *ctx() const;
	datasink::BlockManager* manager();
	void setCtx(iio_context *ctx);

private:
	datasink::BlockManager* m_manager;
	SyncController *m_sync;
	iio_context *m_ctx;
};

class SCOPY_ADC_EXPORT IIODeviceSourceNode : public AcqTreeNode
{
public:
	IIODeviceSourceNode(datasink::SourceBlock* source, TopBlockNode *top, QString name, QObject *parent = nullptr);
	~IIODeviceSourceNode();
	TopBlockNode *top() const;
	datasink::IIOSourceBlock *source() const;

private:
	datasink::SourceBlock* m_source;
	TopBlockNode *m_top;
};

class SCOPY_ADC_EXPORT IIOFloatChannelNode : public AcqTreeNode
{
public:
	IIOFloatChannelNode(datasink::SourceBlock* source, TopBlockNode *top, QString name, QObject *parent = nullptr);
	~IIOFloatChannelNode();
	datasink::SourceBlock* source() const;
	TopBlockNode *top() const;

private:
	datasink::SourceBlock* m_source;
	TopBlockNode *m_top;
};

class SCOPY_ADC_EXPORT ImportFloatChannelNode : public AcqTreeNode
{
public:
	// ImportFloatChannelNode()
	ImportFloatChannelNode(SnapshotRecipe, QObject *parent = nullptr);
	~ImportFloatChannelNode();
	SnapshotRecipe recipe() const;

private:
	SnapshotRecipe m_recipe;
};

class SCOPY_ADC_EXPORT AcqNodeChannelAware
{
public:
	virtual void addChannel(AcqTreeNode *c) = 0;
	virtual void removeChannel(AcqTreeNode *c) = 0;
};

/*
class IIODeviceNode : public AcqTreeNode {
public:
	IIODeviceNode(struct iio_device *dev, QString name, QObject *parent = nullptr) : AcqTreeNode(name,parent),
m_dev(dev) { m_devId = iio_device_get_id(m_dev);
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
     IIOChannelNode(struct iio_channel *ch, QString name, QObject *parent = nullptr) : AcqTreeNode(name,parent),
m_ch(ch) { m_chId= iio_channel_get_id(m_ch);
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
} // namespace adc
} // namespace scopy

#endif // ADCACQUISITIONMANAGER_H
