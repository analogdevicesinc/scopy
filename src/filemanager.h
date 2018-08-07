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

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QVector>
#include <QStringList>

#include <exception>
#include <iostream>


namespace adiscope {
class FileManager
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

        QString getAdditionalInformation() const;
        void setAdditionalInformation(const QString& value);

        FileFormat getFormat() const;
        void setFormat(const FileFormat &value);

private:

        QVector<QVector<double>> data;
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
        QString additionalInformation;

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
