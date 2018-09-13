/*****************************************************************************
Copyright © Luke Salisbury

Licensed under the under the ZLIB or GNU General Public License Version 3, at 
your option. This file may not be copied, modified, or distributed except 
according to those terms.
*****************************************************************************/
#include <QTimer>
#include <QCommandLineParser>
#include <iostream>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

#include "task.hpp"

#include "../qtupnp/contentdirectory.hpp"
#include "../qtupnp/browsereply.hpp"
#include "../qtupnp/didlitem.hpp"
#include "../qtupnp/action.hpp"

void Task::upnpError (int errorCode, QString const & errorString) {
	qDebug() << "upnpError - "  << errorCode << errorString;
}

BasicInfo Task::get(QString const& serverUUID, QString id ) {
	BasicInfo info;

	QtUPnP::CContentDirectory cd(upnp_cp);
	QtUPnP::CBrowseReply reply = cd.browse(serverUUID, id, QtUPnP::CContentDirectory::BrowseMetaData);

	QList<QtUPnP::CDidlItem> const & didlItems = reply.items();
	for (QtUPnP::CDidlItem const & didlItem : didlItems)
	{
		if ( didlItem.type() == QtUPnP::CDidlItem::StorageFolder) {
			info.title = didlItem.title();
		} else if ( didlItem.type() == QtUPnP::CDidlItem::Movie || didlItem.type() == QtUPnP::CDidlItem::VideoItem) {
			BasicInfo parent_info = get(serverUUID, didlItem.parentID());

			if ( didlItem.title().startsWith(parent_info.title) ) {
				info.title = didlItem.title() + ".tts";
			} else {
				info.title = parent_info.title + " " + didlItem.title() + ".tts";
			}
			info.uri = didlItem.uri(0);
			info.filesize = didlItem.size();
		}
	}
	return info;
}

void Task::list(QString const& serverUUID, QString id, QString outputPrefix ) {
	QtUPnP::CContentDirectory cd(upnp_cp);
	QtUPnP::CBrowseReply reply = cd.browse(serverUUID, id);

	QList<QtUPnP::CDidlItem> const & didlItems = reply.items();
	for (QtUPnP::CDidlItem const & didlItem : didlItems)
	{
		if ( didlItem.type() == QtUPnP::CDidlItem::StorageFolder) {
			std::cout << outputPrefix.toStdString() << didlItem.title().toStdString() << std::endl;
			QString q = outputPrefix + "\t";
			this->list(serverUUID, didlItem.id(), q);
		} else if ( didlItem.type() == QtUPnP::CDidlItem::Movie || didlItem.type() == QtUPnP::CDidlItem::VideoItem) {
			std::cout << outputPrefix.toStdString() << "[" << didlItem.id().toInt() << "] "
					  << didlItem.title().toStdString() << " [" << didlItem.duration().toStdString() << "] "
					  << QLocale::system().formattedDataSize(didlItem.size()).toStdString()
					  << std::endl;
			//std::cout << outputPrefix.toStdString() << "[" << didlItem.protocolInfo().toStdString() << "] "
			//		  << std::endl;
		}
	}
}

uint32_t Task::retrievedSystemUpdateID(QtUPnP::CDevice device) {
	QtUPnP::TMServices& services = device.services ();

	QMap<QString, QtUPnP::CService>::const_iterator i = services.constBegin();
	while (i != services.constEnd()) {
		QtUPnP::CService service = i.value();
		if ( i.key() == "urn:upnp-org:serviceId:ContentDirectory") {
			QtUPnP::CContentDirectory cd(upnp_cp);
			return static_cast<uint32_t>(cd.getSystemUpdateID(device.uuid()));
		}
		++i;
	}
	return 0;
}

void Task::retrievedContentList(QtUPnP::CDevice device) {
	QtUPnP::TMServices& services = device.services ();

	QMap<QString, QtUPnP::CService>::const_iterator i = services.constBegin();
	qint32 c = 0;
	while (i != services.constEnd()) {
		QtUPnP::CService service = i.value();
		if ( i.key() == "urn:upnp-org:serviceId:ContentDirectory") {
			this->list(device.uuid(),QString::number(c));
		}
		++i;
		c++;
	}
}

void Task::newDevice( QString const & serverUUID) {

	QtUPnP::CDevice device = upnp_cp->device(serverUUID);

	if ( device.modelName().startsWith("Fetch")) {
		qDebug() << "Fetch STB Found";
		founded_devices.append(device);
	}
}

void Task::networkError(QString const & deviceUUID, QNetworkReply::NetworkError errorCode, QString const & errorDesc) {
	Q_UNUSED(errorCode);
	qDebug() << "networkError - "  << deviceUUID << errorDesc;
}

Task::Task(QCoreApplication * a, QObject * parent) : QObject(parent) {
	timer.start();

	this->app = a;

	parser.addPositionalArgument("command", "download, lastid, list, help");
	parser.addPositionalArgument("id", "ID for download");
	parser.addOptions({
		{{"d", "directory"}, "Download into <directory>.", "directory"}
	});
	// Process the actual command line arguments given by the user
	if ( !parser.parse(QCoreApplication::arguments()) ) {
		std::cout << parser.helpText().toStdString() << std::endl;
		std::cout << parser.errorText().toStdString() << std::endl;

		emit taskFailed();
		return;
	}

	this->upnp_cp = new QtUPnP::CControlPoint(this);

	connect(this, &Task::taskCompleted, this, &Task::exitSuccessfully);
	connect(this, &Task::taskFailed, this, &Task::exitNotSoSuccessfully);

	connect(upnp_cp, SIGNAL(upnpError(int, QString const &) ), this, SLOT(upnpError(int, QString const &) ));
	connect(upnp_cp, SIGNAL(newDevice(QString const &) ), this, SLOT(newDevice(QString const &) ));
	connect(upnp_cp, SIGNAL(networkError(QString const &, QNetworkReply::NetworkError, QString const &) ), this, SLOT(networkError(QString const &, QNetworkReply::NetworkError, QString const &) ));



	if ( upnp_cp->initialize() ) {
		upnp_cp->discover();

		const QStringList positionalArguments = parser.positionalArguments();

		if (positionalArguments.isEmpty()) {
			QTimer::singleShot(scan_time, this, &Task::actionList);
		} else {
			QString action = positionalArguments.first();
			if ( action == "download") {
				if ( positionalArguments.size() >= 2 ) {
					download_id = positionalArguments.at(1).toUInt();
					QTimer::singleShot(scan_time, this, &Task::actionDownload);
				} else {
					QTimer::singleShot(scan_time, this, &Task::actionHelp);
				}
			} else if ( action == "lastid") {
				QTimer::singleShot(scan_time, this, &Task::actionLastID);
			} else if ( action == "help") {
				QTimer::singleShot(0, this, &Task::actionHelp);
			} else {
				QTimer::singleShot(scan_time, this, &Task::actionList);
			}
		}


	} else {
		hasFailed = true;
		std::cout << "UPNP failed. TODO: Write more detail message.";
		emit taskFailed();
	}
}

void Task::exitSuccessfully() {
	this->app->exit(0);
}
void Task::exitNotSoSuccessfully() {
	this->app->exit(1);
}

void Task::actionHelp() {
	std::cout << parser.helpText().toStdString() << std::endl;
	emit taskCompleted();
}

void Task::actionLastID() {
	QList<QtUPnP::CDevice>::const_iterator i = founded_devices.constBegin();
	std::cout << retrievedSystemUpdateID(*i) << std::endl;
	emit taskCompleted();
}


void Task::actionDownload() {
	if ( founded_devices.size() ) {
		QList<QtUPnP::CDevice>::const_iterator i = founded_devices.constBegin();

		QtUPnP::CDevice q = *i;
		BasicInfo info = get(q.uuid(), QString::number(download_id) );
		std::cout << "Downloading " << info.title.toStdString() << " \tSize: "
				  << QLocale::system().formattedDataSize(info.filesize).toStdString()
				  << std::endl;

		if ( !info.uri.isEmpty() && info.filesize > 0) {
			QDir dir;
			if ( parser.isSet("directory") ) {
				dir.setPath(parser.value("directory"));
			} else {
				dir.setPath(QCoreApplication::applicationDirPath());
			}

			QFileInfo path(dir, info.title);

			file.setFileName( path.absoluteFilePath() );
			file.open(QIODevice::WriteOnly|QIODevice::Truncate);
			if ( file.isOpen() ) {
				std::cout << "Url: " << info.uri.toStdString() << std::endl;

				QNetworkRequest request(info.uri);
				//Fetch STB have a problem with the last 28kb for a reason
				// Might be able to enable resume, by forgetting the first 188 packet
				QByteArray rangeHeaderValue = "bytes="
						+ QByteArray::number(0) + "-"
						+ QByteArray::number(1000000)
						+ "/"
						+ QByteArray::number(info.filesize + 29084);
				//request.setRawHeader("Range", rangeHeaderValue);

				reply = manager.get(request);

				if ( reply ) {
					connect(this, &Task::fetchHasClosedConnection, this, &Task::downloadCompleted);
					connect(reply, &QNetworkReply::downloadProgress, this, &Task::downloadProgress);
					connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
									this, SLOT(downloadError(QNetworkReply::NetworkError)));
					connect(reply, &QNetworkReply::finished, this, &Task::downloadCompleted);
					connect(reply, &QNetworkReply::readyRead, this, &Task::downloadWrite);
				} else {
					std::cout << "Invalid QNetworkReply" << std::endl;
				}

			}
		} else {
			emit taskFailed();
		}
} else {
	emit taskFailed();
}
}

void Task::actionList() {
	QList<QtUPnP::CDevice>::const_iterator i = founded_devices.constBegin();
	while (i != founded_devices.constEnd()) {
		retrievedContentList(*i);
		i++;
	}
	emit taskCompleted();
}

void Task::downloadWrite()
{
	QByteArray data = this->reply->readAll();

	//Write the current buffer to file
	this->file.write(data);

	downloaded += data.size();
}

void Task::downloadCompleted()
{
	std::cout << "Saved to " << this->file.fileName().toStdString() << std::endl;

	// Remove the read event
	QObject::disconnect(this->reply, &QNetworkReply::readyRead, nullptr,nullptr);

	//Close Temporary File, rename file to original filename
	this->file.close();

	this->reply->deleteLater();
	emit taskCompleted();

}

void Task::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	std::cout << ""
			  << QLocale::system().formattedDataSize(bytesReceived).toStdString()
			  << " of "
			  << QLocale::system().formattedDataSize(bytesTotal).toStdString() << std::endl;
}

void Task::downloadError(QNetworkReply::NetworkError code)
{
	if (code == QNetworkReply::NetworkError::RemoteHostClosedError) {
		//Ignore Error
		//emit fetchHasClosedConnection();
	} else {
		qDebug() << "error" << code;
		this->reply->deleteLater();
	}
}




