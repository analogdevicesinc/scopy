#include "printplotmanager.h"

#include <QPdfWriter>
#include <QFileDialog>
#include <QDateTime>
#include <QCoreApplication>

using namespace scopy;

PrintPlotManager::PrintPlotManager(QObject *parent)
	: QObject{parent}
{
	m_printBtn = new PrintBtn();
}

PrintBtn *PrintPlotManager::getPrintBtn() { return m_printBtn; }

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
			pdfWriter.setPageSizeMM(QSize(widthInPixels / 3.78, heightInPixels / 3.78)); // 1 inch = 25.4 mm

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
