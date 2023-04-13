/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "printableplot.h"
#include <QDateTime>
#include <QFileDialog>
#include <QImageWriter>

using namespace scopy;

PrintablePlot::PrintablePlot(QWidget *parent) :
	BasicPlot(parent),
	d_plotRenderer(new QwtPlotRenderer(this)),
	d_useNativeDialog(true)
{
	dropBackground(true);
}

void PrintablePlot::dropBackground(bool drop)
{
	d_plotRenderer.setDiscardFlag(QwtPlotRenderer::DiscardBackground, drop);
	d_plotRenderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground, drop);
}

void PrintablePlot::setUseNativeDialog(bool nativeDialog)
{
	d_useNativeDialog = nativeDialog;
}

void PrintablePlot::printPlot(const QString& toolName)
{
	legendDisplay = new QwtLegend(this);
	legendDisplay->setDefaultItemMode(QwtLegendData::ReadOnly);
	insertLegend(legendDisplay, QwtPlot::TopLegend);

	updateLegend();

	QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");

	QString fileName = "Scopy-" + toolName + "-" + date;

	// https://github.com/osakared/qwt/blob/qwt-6.1-multiaxes/src/qwt_plot_renderer.cpp#L1023
	// QwtPlotRenderer does not expose an option to select which file dialog to use
	// native or qt based. So we reconstruct the file formats used by it and use our own
	// call of QFileDialog::getSaveFileName(...) where we take into account the d_useNativeDialog
	// boolean
	const QList<QByteArray> imageFormats =
			QImageWriter::supportedImageFormats();
	QStringList filter;
	filter += QString( "PDF " ) + tr( "Documents" ) + " (*.pdf)";
	filter += QString( "SVG " ) + tr( "Documents" ) + " (*.svg)";
	filter += QString( "Postscript " ) + tr( "Documents" ) + " (*.ps)";

	if ( imageFormats.size() > 0 ) {
		for ( int i = 0; i < imageFormats.size(); i++ ) {
			filter += (imageFormats[i].toUpper() + " "
                    + tr("Image") + " (*." +  imageFormats[i] + ")");
		}

	}

	QString selectedFilter = QString( "PDF " ) + tr( "Documents" ) + " (*.pdf)";
	fileName = QFileDialog::getSaveFileName(
				nullptr, tr( "Export File Name" ), fileName,
				filter.join( ";;" ), &selectedFilter,
				(d_useNativeDialog ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if (fileName.split(".").size() <= 1) {
		// file name w/o extension. Let's append it
		QString ext = selectedFilter.split(".")[1].split(")")[0];
		fileName += "." + ext;
	}

	d_plotRenderer.renderDocument(this, fileName, QSizeF( 300, 200 ));

	insertLegend(nullptr);
}

#include "moc_printableplot.cpp"
