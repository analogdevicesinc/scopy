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

#include "printplotmanager.h"

#include <QPdfWriter>
#include <QFileDialog>
#include <QDateTime>
#include <QCoreApplication>

using namespace scopy;

PrintPlotManager::PrintPlotManager(QObject *parent)
	: QObject{parent}
{}

void PrintPlotManager::printPlots(QList<PlotWidget *> plotList, QString toolName)
{
	// select folder where to save
	QString folderPath = QFileDialog::getExistingDirectory(
		nullptr, "Select Folder", "",
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);

	if(!folderPath.isEmpty()) {
		// use current date and tool name to crete the file name
		QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
		QString fileName = QString(folderPath + "/Scopy-" + toolName + "-" + date + ".pdf");

		if(!fileName.isEmpty()) {
			QPdfWriter pdfWriter(fileName);
			pdfWriter.setResolution(85);

			int widthInPixels = 500;
			int heightInPixels = 400;
			pdfWriter.setPageSize(QPageSize(QSize(widthInPixels / 3.78, heightInPixels / 3.78),
							QPageSize::Millimeter)); // 1 inch = 25.4mm

			QPainter painter(&pdfWriter);
			painter.setRenderHint(QPainter::Antialiasing);

			if(!plotList.isEmpty()) {
				plotList.first()->printPlot(&painter, m_printWithSymbols);
				// if there is more than one plot save it to same file
				for(int i = 1; i < plotList.length(); i++) {
					pdfWriter.newPage();
					plotList.at(i)->printPlot(&painter, m_printWithSymbols);
				}
			}
		}
	}
}

void PrintPlotManager::setPrintWithSymbols(bool printWithSymbols) { m_printWithSymbols = printWithSymbols; }

#include "moc_printplotmanager.cpp"
