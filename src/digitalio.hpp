/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef DIGITAL_IO_H
#define DIGITAL_IO_H

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include <vector>
#include <string>
#include <QList>
#include <QPair>
#include <QTimer>
#include "filter.hpp"
#include "digitalchannel_manager.hpp"

#include "apiObject.hpp"
#include "tool.hpp"


extern "C" {
	struct iio_context;
	struct iio_device;
	struct iio_channel;
	struct iio_buffer;
}

namespace Ui {
class DigitalIO;
class DigitalIoMenu;
class dioElement;
class dioGroup;
class dioChannel;
}

namespace adiscope {
class DigitalIO;
class DigitalIO_API;

class DigitalIoGroup : public QWidget
{
	friend class DigitalIO_API;

	Q_OBJECT
	int nr_of_channels;
	int ch_mask;
	int channels;
	int io_mask;
	int mode;
	DigitalIO *dio;
public:
	DigitalIoGroup(QString label, int ch_mask, int io_mask, DigitalIO *dio,
	               QWidget *parent=0);
	~DigitalIoGroup();
	Ui::dioElement *ui;
	QList<QPair<QWidget *,Ui::dioChannel *>*> chui;

Q_SIGNALS:
	void slider(int val);

private Q_SLOTS:
	void on_lineEdit_editingFinished();
	void on_horizontalSlider_valueChanged(int value);
	void on_comboBox_activated(int index);
	void on_inout_clicked();
};

class DigitalIO : public Tool
{
	friend class DigitalIO_API;
	friend class ToolLauncher_API;

	Q_OBJECT

private:
	Ui::DigitalIO *ui;
	Filter *filt;
	bool offline_mode;
	QList<DigitalIoGroup *> groups;
	QTimer *poll;
	DIOManager *diom;
	int polling_rate = 500; // ms

	QPair<QWidget *,Ui::dioChannel *>  *findIndividualUi(int ch);

public:
	explicit DigitalIO(struct iio_context *ctx, Filter *filt, QPushButton *runBtn,
	                   DIOManager *diom, QJSEngine *engine,
	                   ToolLauncher *parent, bool offline_mode = 0);
	~DigitalIO();
	void setDirection(int ch, int direction);
	void setOutput(int ch, int out);
	void setVisible(bool visible);

public Q_SLOTS:
	void updateUi();
	void setDirection();
	void setOutput();
	void setSlider(int val);
	void lockUi();
	void btnRunStop_toggled();
};

class DigitalIO_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QList<bool> dir READ direction WRITE setDirection SCRIPTABLE true);
	Q_PROPERTY(QList<bool> out READ output    WRITE setOutput SCRIPTABLE true);
	Q_PROPERTY(QList<bool> group READ grouped WRITE setGrouped SCRIPTABLE false);

public:
	explicit DigitalIO_API(DigitalIO *dio) : ApiObject(), dio(dio) {}
	~DigitalIO_API() {}

	QList<bool> direction() const;
	void setDirection(const QList<bool>& list);
	QList<bool> output() const;
	void setOutput(const QList<bool>& list);
	void setOutput(int ch, int direction);
	QList<bool> grouped() const;
	void setGrouped(const QList<bool>& grouped);


private:
	DigitalIO *dio;
};

} /* namespace adiscope */

#endif // DIGITAL_IO_H

