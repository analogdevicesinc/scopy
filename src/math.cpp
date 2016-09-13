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

#include "dynamicWidget.hpp"
#include "math.hpp"

#include <QLocale>
#include <QMenu>

#include <gnuradio/iio/math.h>

using namespace adiscope;

Math::Math(QWidget *parent, unsigned int num_inputs) : QWidget(parent),
	num_inputs(num_inputs)
{
	if (num_inputs == 0)
		throw std::runtime_error("Math widget used with zero inputs");

	ui.setupUi(this);

	QMenu *menuCos = new QMenu(ui.btnCos);
	menuCos->addAction("cos");
	menuCos->addAction("acos");
	menuCos->addAction("cosh");
	ui.btnCos->setMenu(menuCos);

	QMenu *menuSin = new QMenu(ui.btnSin);
	menuSin->addAction("sin");
	menuSin->addAction("asin");
	menuSin->addAction("sinh");
	ui.btnSin->setMenu(menuSin);

	QMenu *menuTan = new QMenu(ui.btnTan);
	menuTan->addAction("tan");
	menuTan->addAction("atan");
	menuTan->addAction("tanh");
	ui.btnTan->setMenu(menuTan);

	QMenu *menuLog = new QMenu(ui.btnLog);
	menuLog->addAction("log");
	menuLog->addAction("log10");
	ui.btnLog->setMenu(menuLog);

	QMenu *menuExp = new QMenu(ui.btnExp);
	menuExp->addAction("exp");
	menuExp->addAction("sqrt");
	ui.btnExp->setMenu(menuExp);

	if (num_inputs > 1) {
		QMenu *menuT = new QMenu(ui.btnT);

		for (unsigned int i = 0; i < num_inputs; i++)
			menuT->addAction(QString("t%1").arg(i));
		ui.btnT->setMenu(menuT);

		connect(menuT, SIGNAL(triggered(QAction *)),
				this, SLOT(handleMenuButtonT(QAction *)));
	} else {
		ui.btnT->setProperty("token", QVariant("t"));
		connect(ui.btnT, SIGNAL(clicked()), this, SLOT(handleButton()));
	}

	ui.btn0->setProperty("token", QVariant("0"));
	ui.btn1->setProperty("token", QVariant("1"));
	ui.btn2->setProperty("token", QVariant("2"));
	ui.btn3->setProperty("token", QVariant("3"));
	ui.btn4->setProperty("token", QVariant("4"));
	ui.btn5->setProperty("token", QVariant("5"));
	ui.btn6->setProperty("token", QVariant("6"));
	ui.btn7->setProperty("token", QVariant("7"));
	ui.btn8->setProperty("token", QVariant("8"));
	ui.btn9->setProperty("token", QVariant("9"));
	ui.btnE->setProperty("token", QVariant("e"));
	ui.btnPi->setProperty("token", QVariant("pi"));
	ui.btnOpenP->setProperty("token", QVariant("("));
	ui.btnCloseP->setProperty("token", QVariant(")"));
	ui.btnPlus->setProperty("token", QVariant("+"));
	ui.btnMinus->setProperty("token", QVariant("-"));
	ui.btnMult->setProperty("token", QVariant("*"));
	ui.btnDiv->setProperty("token", QVariant("/"));
	ui.btnPow->setProperty("token", QVariant("^"));

	QLocale locale;
	QString separator(locale.decimalPoint());
	ui.btnComma->setProperty("token", QVariant(separator));
	ui.btnComma->setText(separator);

	connect(ui.btn0, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn1, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn2, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn3, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn4, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn5, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn6, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn7, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn8, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btn9, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnE, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnPi, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnOpenP, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnCloseP, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnComma, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnPlus, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnMinus, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnMult, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnDiv, SIGNAL(clicked()), this, SLOT(handleButton()));
	connect(ui.btnPow, SIGNAL(clicked()), this, SLOT(handleButton()));

	connect(menuCos, SIGNAL(triggered(QAction *)),
			this, SLOT(handleMenuButton(QAction *)));
	connect(menuSin, SIGNAL(triggered(QAction *)),
			this, SLOT(handleMenuButton(QAction *)));
	connect(menuTan, SIGNAL(triggered(QAction *)),
			this, SLOT(handleMenuButton(QAction *)));
	connect(menuLog, SIGNAL(triggered(QAction *)),
			this, SLOT(handleMenuButton(QAction *)));
	connect(menuExp, SIGNAL(triggered(QAction *)),
			this, SLOT(handleMenuButton(QAction *)));

	connect(ui.btnClear, SIGNAL(clicked()), ui.function, SLOT(clear()));
	connect(ui.btnBackspace, SIGNAL(clicked()), this, SLOT(delLastChar()));

	connect(ui.btnApply, SIGNAL(clicked()), this, SLOT(validateFunction()));
	connect(ui.function, SIGNAL(returnPressed()),
			this, SLOT(validateFunction()));

	connect(ui.function, SIGNAL(textChanged(const QString&)),
			this, SLOT(resetState()));
}

void Math::handleButton()
{
	QPushButton *btn = static_cast<QPushButton *>(QObject::sender());
	QString token = btn->property("token").toString();

	ui.function->insert(token);
}

void Math::handleMenuButton(QAction *action)
{
	ui.function->insert(action->text() + "(");
}

void Math::handleMenuButtonT(QAction *action)
{
	ui.function->insert(action->text());
}

void Math::validateFunction()
{
	QString function = ui.function->text();

	try {
		gr::iio::iio_math::make(function.toStdString(), num_inputs);

		Q_EMIT functionValid(function);

		setDynamicProperty(ui.function, "valid", true);
		setDynamicProperty(ui.btnApply, "valid", true);
	} catch (std::exception ex) {
		setDynamicProperty(ui.function, "invalid", true);
		setDynamicProperty(ui.btnApply, "invalid", true);
	}
}

void Math::resetState()
{
	setDynamicProperty(ui.function, "valid", false);
	setDynamicProperty(ui.btnApply, "valid", false);
	setDynamicProperty(ui.function, "invalid", false);
	setDynamicProperty(ui.btnApply, "invalid", false);

	Q_EMIT stateReseted();
}

void Math::setFunction(const QString& function)
{
	ui.function->setText(function);
}

void Math::delLastChar()
{
	ui.function->backspace();
}
