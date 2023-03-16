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

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QVector>
#include <QStringList>

#include <exception>
#include <iostream>
#include "scopygui_export.h"


namespace adiscope {
class SCOPYGUI_EXPORT FileManager
{
public:

	enum FilePurpose {
		EXPORT,
		IMPORT
	};

	enum FileFormat {
		RAW,
		SCOPY
	};

	enum FileType {
		CSV,
		TXT
	};

	FileManager(QString toolName);
	~FileManager();

	void open(QString fileName, FileManager::FilePurpose filepurpose = EXPORT);

	void save(QVector<double> data, QString name);
	void save(QVector<QVector<double>> data, QVector<QVector<QString>> decoder_data, QStringList column_names);
	void save(QVector<QVector<double>> data, QStringList column_names);

	QVector<double> read(int index);
	QVector<QVector<double>> read();

	void setColumnName(int index, QString name);
	QString getColumnName(int index);

	double getSampleRate() const;
	void setSampleRate(double sampleRate);

	double getNrOfSamples() const;
	int getNrOfChannels() const;

	void performWrite();
	void performDecoderWrite(bool skip_empty_lines = false);

	QStringList getAdditionalInformation() const;
	void setAdditionalInformation(const QString& value);

	FileFormat getFormat() const;
	void setFormat(const FileFormat &value);

private:

	QVector<QVector<double>> data;
	QVector<QVector<QString>> decoder_data;
	QStringList columnNames;
	QString filename;
	bool hasHeader;
	double sampleRate;
	double nrOfSamples;
	FilePurpose openedFor;
	FileFormat format;
	FileType fileType;
	QString separator;
	QString toolName;
	QStringList additionalInformation;

};

class ScopyFileHeader {
public:
	static bool hasValidHeader(QVector<QVector<QString>> data);
	static QStringList getHeader();
};

class FileManagerException : public std::runtime_error {
public:
	FileManagerException(const char* msg) : std::runtime_error(msg) {}
	~FileManagerException() throw() {}
};
}

#endif // FILEMANAGER_H
