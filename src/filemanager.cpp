/*
 * Copyright 2018 Analog Devices, Inc.
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

#include "filemanager.h"
#include "config.h"

#include <QDebug>
#include <QFile>
#include <QDate>

using namespace adiscope;

FileManager::FileManager(QString toolName) :
        hasHeader(false),
        sampleRate(0),
	nrOfSamples(0),
	toolName(toolName)
{

}

FileManager::~FileManager()
{

}

void FileManager::open(QString fileName,
                                  FileManager::FilePurpose filepurpose)
{
        //read the data from the file if filepurpose is Import
        //throws exception if the file is corrupted, has a header but not the scopy one
        //columns with different sizes etc..

        QVector<QVector<QString>> raw_data;

        openedFor = filepurpose;

        if (fileName.endsWith(".csv")) {
                separator = ",";
                fileType = CSV;
        } else if (fileName.endsWith(".txt")) {
                separator = "\t";
                fileType = TXT;
                //find sep to read txt files
        }

        //clear previous data if the manager was used for other exports
        data.clear();
        columnNames.clear();
        this->filename = fileName;

        if (filepurpose == IMPORT) {

                if (fileName.isEmpty()) {
                        throw FileManagerException("No file selected");
                }

                QFile file(fileName);
                if (!file.open(QIODevice::ReadOnly)) {
                        throw FileManagerException("Can't open selected file");
                }

                QTextStream in(&file);

                for (int i = 0; i < data.size(); ++i)
                        data[i].clear();

                while (!in.atEnd()) {
                        QVector<QString> line_data;
                        QString line = in.readLine();
                        QStringList list = line.split(separator, QString::SkipEmptyParts);
                        for (QString list_item : list) {
                                line_data.push_back(list_item);
                        }

                        raw_data.push_back(line_data);
                }

                //check if it has a header or not
                /*
                *  Header format
                *
                *       ;Scopy version <separator> abcdefg
                *       ;Exported on <separator> Wed Apr 4 13:49:01 2018
                *       ;Device <separator> M2K
                *       ;Nr of samples <separator> 1234
                *       ;Sample rate <separator> 1234 or 0 if it does not have samp. rate
                *       ;Tool: <separator> Oscilloscope/ Spectrum ...
                *       ;Additional Information
                */

                hasHeader = ScopyFileHeader::hasValidHeader(raw_data);
                try {
                        if (hasHeader) {

                                format = SCOPY;

                                //first column in data is the time!!! when retrieving channel data start from data[1]
				for (int i = 1; i < raw_data[6].size(); ++i) {
					additionalInformation.push_back(raw_data[6][i]);
				}

                                sampleRate = std::stod(raw_data[4][1].toStdString());
                                //should be 0 if read from network/spectrum analyzer exported file

                                for (int j = 1; j < raw_data[7].size(); ++j)
                                        columnNames.push_back(raw_data[7][j]);

                                data.resize(raw_data.size() - 8);
                                for (int i = 0; i < data.size(); ++i) {
                                        data[i].resize(raw_data[i + 8].size() - 1);
                                }

                                for (int i = 8; i < raw_data.size(); ++i) {
                                        for (int j = 1; j < raw_data[i].size(); ++j) {
						data[i - 8][j - 1] = raw_data[i][j].toDouble();
                                        }
                                }
                        } else {

                                format = RAW;

                                data.resize(raw_data.size());
                                for (int i = 0; i < data.size(); ++i) {
                                        data[i].resize(raw_data[i].size());
                                }

                                for (int i = 0; i < raw_data.size(); ++i) {
                                        for (int j = 0; j < raw_data[i].size(); ++j) {
                                              data[i][j] = std::stod(raw_data[i][j].toStdString());
                                        }
                                }
                        }
                } catch (std::invalid_argument &i) {
                        // if stod fails to convert a value to double it will throw an invalid argument exception
                        // in this case it means that the file is corrupted
                        throw FileManagerException("File is corrupted!");
                }

                nrOfSamples = data.size();
        }
}

void FileManager::save(QVector<double> data, QString name)
{
        this->columnNames.push_back(name);

        if (this->data.size() == 0) {
                this->data.resize(data.size());
        }

        for (int i = 0; i < data.size(); ++i) {
                this->data[i].push_back(data[i]);
        }
}

void FileManager::save(QVector<QVector<double> > data, QStringList columnNames)
{
        for (auto &column : data) {
                this->data.push_back(column);
        }

        for (auto &column_name : columnNames) {
                this->columnNames.push_back(column_name);
        }
}

QVector<double> FileManager::read(int index)
{
        if (index < 0 || index + 1 >= data.size()) {
                return QVector<double>();
        }

        if (hasHeader) {
                index++;
        }

        QVector<double> channel_data;
        for (int i = 0; i < nrOfSamples; ++i) {
                channel_data.push_back(data[i][index]);
        }

        return channel_data;
}

QVector<QVector<double>> FileManager::read()
{
        return data;
}

void FileManager::setColumnName(int index, QString name)
{
        if (index < 0 || index >= data.size()) {
                return;
        }

        columnNames[index] = name;
}

QString FileManager::getColumnName(int index)
{
        if (index < 0 || index >= columnNames.size()) {
                return "";
        }

        if (hasHeader) {
                index++;
        }

        return columnNames[index];
}

double FileManager::getSampleRate() const
{
        return sampleRate;
}

void FileManager::setSampleRate(double sampleRate)
{
        this->sampleRate = sampleRate;
}

double FileManager::getNrOfSamples() const
{
        return nrOfSamples;
}

int FileManager::getNrOfChannels() const
{
        if (hasHeader) {
                return data[0].size() - 1;
        } else {
                return data[0].size();
        }
}

void FileManager::performWrite()
{
        if (openedFor == IMPORT) {
                qDebug() << "Can't write when opened for import!";
                return;
        }

        QFile exportFile(filename);
        exportFile.open(QIODevice::WriteOnly);
        QTextStream exportStream(&exportFile);


        QStringList header = ScopyFileHeader::getHeader();

        //prepare header
        exportStream << header[0] << separator << QString(SCOPY_VERSION_GIT) << "\n";
        exportStream << header[1] << separator << QDate::currentDate().toString("dddd MMMM dd/MM/yyyy") << "\n";
        exportStream << header[2] << separator << "M2K" << "\n";
        exportStream << header[3] << separator << data.size() << "\n";
        exportStream << header[4] << separator << sampleRate << "\n";
        exportStream << header[5] << separator << toolName << "\n";
	exportStream << header[6] << separator << additionalInformation[0] << "\n";

        //column names row
        exportStream << "Sample" << separator;
	bool skipFirstSeparator=true;
        for (QString columnName : columnNames) {
		if(!skipFirstSeparator)
			exportStream << separator;
		exportStream << columnName;
		skipFirstSeparator = false;
        }
        exportStream << "\n";

        for (int i = 0; i < data.size(); ++i) {
		skipFirstSeparator = true;
                exportStream << QString::number(i) << separator;
                for (int j = 0; j < data[i].size(); ++j) {
			if(!skipFirstSeparator)
				exportStream << separator;
			exportStream << data[i][j];
			skipFirstSeparator = false;
                }
                exportStream << "\n";
        }

        exportFile.close();
}

QStringList FileManager::getAdditionalInformation() const
{
        return additionalInformation;
}

void FileManager::setAdditionalInformation(const QString &value)
{

	additionalInformation.push_back(value);
}

FileManager::FileFormat FileManager::getFormat() const
{
        return format;
}

void FileManager::setFormat(const FileManager::FileFormat &value)
{
        format = value;
}

bool ScopyFileHeader::hasValidHeader(QVector<QVector<QString>> data)
{

        QStringList header_elements = getHeader();

        if (data.size() < header_elements.size()) {
                return false;
        }

        for (int i = 0; i < header_elements.size(); ++i) {
                if (data[i][0] != header_elements[i]) {
                        return false;
                }
        }

        return true;
}

QStringList ScopyFileHeader::getHeader()
{

        QStringList header_elements = QStringList() << ";Scopy version"
                                         << ";Exported on"
                                         << ";Device"
                                         << ";Nr of samples"
                                         << ";Sample rate"
                                         << ";Tool"
                                         << ";Additional Information";
        return header_elements;
}
