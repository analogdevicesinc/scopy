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

#include "apiObject.hpp"
#include "digitalchannel_manager.hpp"
#include "filter.hpp"
#include "tool.hpp"

#include <QList>
#include <QPair>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include <string>
#include <vector>

extern "C"
{
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
} // namespace Ui

namespace adiscope {
class DigitalIO;
class DigitalIO_API;

class DigitalIoGroup : public QWidget
{
	friend class DigitalIO_API;

	Q_OBJECT
	int nr_of_channels;
	int ch_mask;
	int io_mask;
	DigitalIO* dio;

public:
	DigitalIoGroup(QString label, int ch_mask, int io_mask, DigitalIO* dio, QWidget* parent = 0);
	~DigitalIoGroup();
	Ui::dioElement* ui;
	QList<QPair<QWidget*, Ui::dioChannel*>*> chui;

Q_SIGNALS:
	void slider(int val);

private Q_SLOTS:
	void on_lineEdit_editingFinished();
	void on_horizontalSlider_valueChanged(int value);
	void on_comboBox_activated(int index);
	void changeDirection();
};

class DigitalIO : public Tool
{
	friend class DigitalIO_API;
	friend class ToolLauncher_API;

	Q_OBJECT

private:
	Ui::DigitalIO* ui;
	Filter* filt;
	bool offline_mode;
	QList<DigitalIoGroup*> groups;
	QTimer* poll;
	DIOManager* diom;
	int polling_rate = 500; // ms

	QPair<QWidget*, Ui::dioChannel*>* findIndividualUi(int ch);

public:
	explicit DigitalIO(struct iio_context* ctx, Filter* filt, ToolMenuItem* toolMenuItem, DIOManager* diom,
			   QJSEngine* engine, ToolLauncher* parent, bool offline_mode = 0);
	~DigitalIO();
	void setDirection(int ch, int direction);
	void setOutput(int ch, int out);
	void setVisible(bool visible);

public Q_SLOTS:
	void run() override;
	void stop() override;
	void updateUi();
	void setDirection();
	void setOutput();
	void setSlider(int val);
	void lockUi();
	void startStop(bool);
Q_SIGNALS:
	void showTool();
};
} /* namespace adiscope */

#endif // DIGITAL_IO_H
