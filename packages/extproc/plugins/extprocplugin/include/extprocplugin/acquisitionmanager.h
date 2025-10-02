/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef ACQUISITIONMANAGER_H
#define ACQUISITIONMANAGER_H

#include "extprocutils.h"
#include "inputconfig.h"
#include "iiomanager.h"
#include "iqbinreader.h"

#include <QObject>
#include <iio.h>

namespace scopy::extprocplugin {

class AcquisitionManager : public QObject
{
	Q_OBJECT

public:
	enum AcquisitionSource
	{
		IIO_DEVICE, // Live acquisition from IIO devices
		IQ_FILE	    // Playback from IQ binary files
	};

	explicit AcquisitionManager(iio_context *ctx, QObject *parent = nullptr);
	~AcquisitionManager();

	// Source management
	void setActiveSource(AcquisitionSource source);
	AcquisitionSource getActiveSource() const;

	// Component access
	IIOManager *getIIOManager() const;
	IQBinReader *getFileReader() const;

	// Unified acquisition interface
	QMap<QString, QList<ChannelInfo>> getAvailableChannels();

public Q_SLOTS:
	void startAcquisition(bool enable);
	void onDataRequest();
	void onBufferParamsChanged(const BufferParams &params);

Q_SIGNALS:
	void inputFormatChanged(const InputConfig &config);
	void dataReady(QVector<QVector<float>> &inputData);
	void sourceChanged(AcquisitionSource newSource);

private Q_SLOTS:
	void onIIOInputFormatChanged(const InputConfig &config);
	void onIIODataReady(QVector<QVector<float>> &inputData);
	void onFileInputFormatChanged(const InputConfig &config);
	void onFileDataReady(QVector<QVector<float>> &inputData);

private:
	void setupConnections();
	void disconnectAll();
	void connectActiveSource();

	AcquisitionSource m_activeSource;
	IIOManager *m_iioManager;
	IQBinReader *m_fileReader;
};

} // namespace scopy::extprocplugin

#endif // ACQUISITIONMANAGER_H
