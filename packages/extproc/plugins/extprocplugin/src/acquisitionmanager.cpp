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

#include "acquisitionmanager.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_ACQUISITION_MANAGER, "AcquisitionManager");

using namespace scopy::extprocplugin;

AcquisitionManager::AcquisitionManager(iio_context *ctx, QObject *parent)
	: QObject(parent)
	, m_activeSource(IIO_DEVICE)
{
	// Create component instances
	m_iioManager = new IIOManager(ctx, this);
	m_fileReader = new IQBinReader(this);

	// Setup signal connections
	setupConnections();

	// Start with IIO device as default active source
	connectActiveSource();

	qInfo(CAT_ACQUISITION_MANAGER) << "AcquisitionManager created with IIO_DEVICE as default source";
}

AcquisitionManager::~AcquisitionManager() { disconnectAll(); }

void AcquisitionManager::setActiveSource(AcquisitionSource source)
{
	if(m_activeSource == source) {
		return; // No change needed
	}

	qInfo(CAT_ACQUISITION_MANAGER) << "Switching acquisition source from" << m_activeSource << "to" << source;

	// Stop current acquisition
	startAcquisition(false);

	// Disconnect current source
	disconnectAll();

	// Switch source
	m_activeSource = source;

	// Connect new source
	connectActiveSource();

	Q_EMIT sourceChanged(m_activeSource);
	qInfo(CAT_ACQUISITION_MANAGER) << "Successfully switched to acquisition source:" << source;
}

AcquisitionManager::AcquisitionSource AcquisitionManager::getActiveSource() const { return m_activeSource; }

IIOManager *AcquisitionManager::getIIOManager() const { return m_iioManager; }

IQBinReader *AcquisitionManager::getFileReader() const { return m_fileReader; }

QMap<QString, QList<ChannelInfo>> AcquisitionManager::getAvailableChannels()
{
	if(m_activeSource == IIO_DEVICE) {
		return m_iioManager->getAvailableChannels();
	} else {
		// For file sources, return empty map or construct from file info
		// Could be enhanced to return channel info from IQ file header
		return {};
	}
}

void AcquisitionManager::startAcquisition(bool enable)
{
	if(m_activeSource == IIO_DEVICE) {
		m_iioManager->startAcq(enable);
	} else if(m_activeSource == IQ_FILE) {
		m_fileReader->startAcq(enable);
	}

	qDebug(CAT_ACQUISITION_MANAGER) << "Acquisition" << (enable ? "started" : "stopped")
					<< "for source:" << m_activeSource;
}

void AcquisitionManager::onDataRequest()
{
	if(m_activeSource == IIO_DEVICE) {
		m_iioManager->onDataRequest();
	} else if(m_activeSource == IQ_FILE) {
		m_fileReader->onDataRequest();
	}
}

void AcquisitionManager::onBufferParamsChanged(const BufferParams &params)
{
	// Update both components - they can handle params even when not active
	m_iioManager->onBufferParamsChanged(params);
	m_fileReader->onBufferParamsChanged(params);

	qDebug(CAT_ACQUISITION_MANAGER) << "Buffer params updated for both sources";
}

void AcquisitionManager::setupConnections()
{
	// Setup connections for both components
	// These are permanent connections - we'll control which ones are active
	connect(m_iioManager, &IIOManager::inputFormatChanged, this, &AcquisitionManager::onIIOInputFormatChanged);
	connect(m_iioManager, &IIOManager::dataReady, this, &AcquisitionManager::onIIODataReady);

	connect(m_fileReader, &IQBinReader::inputFormatChanged, this, &AcquisitionManager::onFileInputFormatChanged);
	connect(m_fileReader, &IQBinReader::dataReady, this, &AcquisitionManager::onFileDataReady);
}

void AcquisitionManager::disconnectAll()
{
	// Note: We don't actually disconnect here since we use slot filtering
	// The slot methods will check m_activeSource to decide whether to forward signals
}

void AcquisitionManager::connectActiveSource()
{
	// Connection filtering is handled in the slot methods based on m_activeSource
	// This method could be used for additional source-specific setup if needed

	if(m_activeSource == IIO_DEVICE) {
		qDebug(CAT_ACQUISITION_MANAGER) << "IIO_DEVICE source is now active";
	} else if(m_activeSource == IQ_FILE) {
		qDebug(CAT_ACQUISITION_MANAGER) << "IQ_FILE source is now active";
	}
}

void AcquisitionManager::onIIOInputFormatChanged(const InputConfig &config)
{
	// Only forward signal if IIO is the active source
	if(m_activeSource == IIO_DEVICE) {
		Q_EMIT inputFormatChanged(config);
	}
}

void AcquisitionManager::onIIODataReady(QVector<QVector<float>> &inputData)
{
	// Only forward signal if IIO is the active source
	if(m_activeSource == IIO_DEVICE) {
		Q_EMIT dataReady(inputData);
	}
}

void AcquisitionManager::onFileInputFormatChanged(const InputConfig &config)
{
	// Only forward signal if file is the active source
	if(m_activeSource == IQ_FILE) {
		Q_EMIT inputFormatChanged(config);
	}
}

void AcquisitionManager::onFileDataReady(QVector<QVector<float>> &inputData)
{
	// Only forward signal if file is the active source
	if(m_activeSource == IQ_FILE) {
		Q_EMIT dataReady(inputData);
	}
}
