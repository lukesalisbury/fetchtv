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

inline ArgumentStringType GetArgumentStringType( const QString & str) {
	static QRegExp datecheck("\\d{4}-\\d{2}-\\d{2}");
	static QRegExp numcheck("\\d+");
	if (datecheck.exactMatch(str))
		return AST_DATE;
	else if (numcheck.exactMatch(str))
		return AST_NUMBER;
	return AST_STRING;
}

QString GetRemainingTime( qint64 s ) {
	if ( s == INT64_MAX ) {
		return  QString("Unknown Time remaining");
	}

	qint64 hours, minutes, seconds;
	QString hour, minute, second;

	hours = s / 3600;
	minutes = s % 3600; // In seconds
	seconds = minutes % 60;
	minutes /= 60; // Convert to minutes

	// Not happy with this method, but it cleanest that I could think of
	hour = qAbs(hours) == 1 ? "hour" : "hours";
	minute = qAbs(minutes) < 2 ? "minute" : "minutes";
	second = qAbs(seconds) < 2 ? "second" : "seconds";

	//We abs the second value so we don't have -1 hour and -13 minutes
	if ( hours != 0 ) {
		return QString("%1 %3 and %2 %4 remaining").arg(hours).arg(qAbs(minutes)).arg(hour).arg(minute);
	} else if ( minutes != 0 ) {
		return QString("%1 %3 and %2 %4 remaining").arg(minutes).arg(qAbs(seconds)).arg(minute).arg(second);
	} else {
		return QString("%1 %2 remaining").arg(seconds).arg(second);
	}
}

BasicInfo Task::CDidlItem2BasicInfo( QString const& serverUUID, QtUPnP::CDidlItem const & didlItem ) {
	BasicInfo info;
	BasicInfo parent_info = get(serverUUID, didlItem.parentID());

	if ( didlItem.title().startsWith(parent_info.title) ) {
		info.filename = didlItem.title() + ".tts";
	} else {
		info.filename = parent_info.title + " " + didlItem.title() + ".tts";
	}
	info.title = didlItem.title();
	info.uri = didlItem.uri(0);
	info.filesize = didlItem.size();
	info.series = parent_info.title;
	info.date = didlItem.date();
	info.id = didlItem.id();

	//Strips invalid characters
	info.filename.replace(QRegExp("[<>:/\\\"|?*]"), "");

	return info;
}


BasicInfo Task::get( QString const& serverUUID, QString id ) {
	BasicInfo info;

	QtUPnP::CContentDirectory cd(upnp_cp);
	QtUPnP::CBrowseReply reply = cd.browse(serverUUID, id, QtUPnP::CContentDirectory::BrowseMetaData);

	QList<QtUPnP::CDidlItem> const & didlItems = reply.items();
	for (QtUPnP::CDidlItem const & didlItem : didlItems)
	{
		if ( didlItem.type() == QtUPnP::CDidlItem::StorageFolder) {
			info.title = didlItem.title();
		} else if ( didlItem.type() == QtUPnP::CDidlItem::Movie || didlItem.type() == QtUPnP::CDidlItem::VideoItem) {
			info = CDidlItem2BasicInfo(serverUUID, didlItem);
		}
	}
	return info;
}

QList<BasicInfo> Task::buildList(const QtUPnP::CDevice & device, QString id ) {
	QList<BasicInfo> list;

	if ( id.isEmpty() ) {
		QtUPnP::TMServices services = device.services();

		QMap<QString, QtUPnP::CService>::const_iterator i = services.constBegin();
		qint32 c = 0;
		while (i != services.constEnd()) {
			QtUPnP::CService service = i.value();
			if ( i.key() == "urn:upnp-org:serviceId:ContentDirectory") {
				id = QString::number(c);
			}
			++i;
			c++;
		}
	}

	QtUPnP::CContentDirectory cd(upnp_cp);
	QtUPnP::CBrowseReply reply = cd.browse(device.uuid(), id, QtUPnP::CContentDirectory::BrowseDirectChildren);

	QList<QtUPnP::CDidlItem> const & didlItems = reply.items();
	for (QtUPnP::CDidlItem const & didlItem : didlItems) {
		if ( didlItem.type() == QtUPnP::CDidlItem::StorageFolder) {
			QList<BasicInfo> child = this->buildList(device, didlItem.id());
			for (int i = 0; i < child.size(); ++i) {
				list.append(child.at(i));
			}
		} else if ( didlItem.type() == QtUPnP::CDidlItem::Movie || didlItem.type() == QtUPnP::CDidlItem::VideoItem) {
			BasicInfo info = CDidlItem2BasicInfo(device.uuid(), didlItem);
			list.append(info);
		}
	}
	return list;
}

void Task::list(QString const& serverUUID, QString id, QString outputPrefix ) {
	QtUPnP::CContentDirectory cd(upnp_cp);
	QtUPnP::CBrowseReply reply = cd.browse(serverUUID, id, QtUPnP::CContentDirectory::BrowseDirectChildren);

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
					  << QLocale::system().formattedDataSize(didlItem.size()).toStdString() << "\t"
					  << didlItem.date().toString().toStdString()
					  << std::endl;
		}
	}
}

void Task::retrievedContentList(QtUPnP::CDevice device) {
	QtUPnP::TMServices& services = device.services();

	QMap<QString, QtUPnP::CService>::const_iterator i = services.constBegin();
	qint32 c = 0;
	while (i != services.constEnd()) {
		QtUPnP::CService service = i.value();
		if ( i.key() == "urn:upnp-org:serviceId:ContentDirectory") {
			this->list(device.uuid(), QString::number(c));
		}
		++i;
		c++;
	}
}

void Task::newDevice( QString const & serverUUID) {
	QtUPnP::CDevice device = upnp_cp->device(serverUUID);
	if ( device.modelName().startsWith("Fetch")) {
		if ( this->has_device_ip ) {
			if ( this->requested_device == device.url().host() ) {
				std::cout << "Fetch STB Found at " << device.url().host().toStdString() << std::endl;
				founded_devices.append(device);
			} else {
				std::cout << "Fetch STB skipped at " << device.url().host().toStdString() << std::endl;
			}
		} else {
			std::cout << "Fetch STB Found at " << device.url().host().toStdString() << std::endl;
			founded_devices.append(device);
		}
	}
}


Task::Task(QCoreApplication * a, QObject * parent) : QObject(parent) {
	timer.start();

	this->app = a;

	this->action_method = &Task::actionHelp;

	// CLI options
	parser.addPositionalArgument("command", "download, list, help");
	parser.addPositionalArgument("id/date/series", "ID, Date (YYYY-MM-DD) or Series Name (Wrap text in quote). Multiple option can be used.");
	parser.addOptions({
		{{"d", "directory"}, "Download into <directory>.", "directory"},
		{"ip", "Fetch IP Address", "ip"},
		//{"csv", "Output as CSV"},
		{"resume", "[Untested] Resume downloads. May cause invalid videos"},
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
		upnp_cp->avDiscover();

		const QStringList positionalArguments = parser.positionalArguments();

		if ( parser.isSet("ip") ) {
			this->has_device_ip = true;
			this->requested_device = parser.value("ip");
		}

//		if ( parser.isSet("csv") ) {
//			this->output_as_csv = true;
//		}
		if ( parser.isSet("resume") ) {
			this->resume_downloads = true;
		}

		if (positionalArguments.isEmpty()) {
			QTimer::singleShot(scan_time, this, &Task::actionList);
		} else {
			QString action = positionalArguments.first();

			if ( action == "download") {
				if ( positionalArguments.size() >= 2 ) {
					QStringList::const_iterator constIterator= positionalArguments.constBegin();
					for (constIterator++; constIterator != positionalArguments.constEnd(); ++constIterator) {
						download_actions.push_back(*constIterator);
					}
					QTimer::singleShot(scan_time, this, &Task::actionPreDownload);
				} else {
					QTimer::singleShot(scan_time, this, &Task::actionHelp);
				}
			} else if ( action == "help") {
					QTimer::singleShot(0, this, &Task::actionHelp);
			} else {
				QTimer::singleShot(scan_time, this, &Task::actionList);
			}
		}
	} else {
		has_failed = true;
		std::cout << "UPNP failed. TODO: Write more detail message.";
		emit taskFailed();
	}
}

void Task::upnpError (int errorCode, QString const & errorString) {
	qDebug() << "upnpError - "  << errorCode << errorString;
}

void Task::networkError(QString const & deviceUUID, QNetworkReply::NetworkError errorCode, QString const & errorDesc) {
	Q_UNUSED(errorCode);
	qDebug() << "networkError - "  << deviceUUID << errorDesc;
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

void Task::nextDownload() {
	if ( download_actions.count() ) {
		this->actionDownload();
	} else {
		emit taskCompleted();
	}
}

void Task::actionPreDownload() {
	if ( founded_devices.size() ) {
		QList<QtUPnP::CDevice>::const_iterator d = founded_devices.constBegin();
		this->cached_info = this->buildList( *d, "" );
		this->nextDownload();
	} else {
		emit taskFailed();
	}
}

void Task::actionDownload() {
	if ( founded_devices.size() && download_actions.count()) {
		QString download = download_actions.takeFirst();
		QList<QtUPnP::CDevice>::const_iterator i = founded_devices.constBegin();

		QtUPnP::CDevice device = *i;

		switch (GetArgumentStringType(download)) {
			case AST_DATE:
				this->since_date = QDateTime::fromString(download, Qt::ISODate);
				for (int var = 0; var < this->cached_info.size(); ++var) {
					BasicInfo q = this->cached_info.at(var);
					if ( q.date > this->since_date ) {
						download_actions.push_back(q.id);
					}
				}
				this->nextDownload();
			break;

			case AST_STRING:
				for (int var = 0; var < this->cached_info.size(); ++var) {
					BasicInfo q = this->cached_info.at(var);
					if ( q.series.compare(download, Qt::CaseInsensitive) == 0 ) {
						download_actions.push_back(q.id);
					}
				}
				this->nextDownload();
			break;

			case AST_NUMBER:
				downloadStart(device, download.toULong());
			break;
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

void Task::downloadStart(QtUPnP::CDevice device, quint32 id) {
	BasicInfo info = get(device.uuid(), QString::number(id) );
	std::cout << "Downloading " << info.series.toStdString() << ":" << info.title.toStdString() << " \tSize: "
			  << QLocale::system().formattedDataSize(info.filesize).toStdString()
			  << std::endl;

	if ( !info.uri.isEmpty() && info.filesize > 0) {
		QDir dir;
		if ( parser.isSet("directory") ) {
			dir.setPath(parser.value("directory"));
		} else {
			dir.setPath(QCoreApplication::applicationDirPath());
		}

		QFileInfo path(dir, info.filename);
		current_file.setFileName( path.absoluteFilePath() );

		if ( this->resume_downloads && path.size() ) {
			current_file.open(QIODevice::WriteOnly|QIODevice::Append);
		} else {
			current_file.open(QIODevice::WriteOnly|QIODevice::Truncate);
		}
		if ( current_file.isOpen() ) {
			std::cout << "Url: " << info.uri.toStdString() << std::endl;

			this->downloaded = 0;
			this->continuing = false;

			QNetworkRequest request(info.uri);
			//Fetch STB have a problem with the last 28kb for a reason - 29084
			//Might be able to enable resume, by forgetting the first 188 packet
			if ( this->resume_downloads && path.size() ) {
				std::cout << "Resuming from " << QLocale::system().formattedDataSize(path.size() ).toStdString() << std::endl;
				QByteArray rangeHeaderValue = "bytes=" + QByteArray::number(path.size() - 188) + "-";
				request.setRawHeader("Range", rangeHeaderValue);
				this->continuing = true;
			}

			this->timer.start();
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

		} else {
			std::cout << "File " << info.filename.toStdString() << " can not be open."<< std::endl;
			nextDownload();
		}
	} else {
		emit taskFailed();
	}
}

void Task::downloadWrite() {
	QByteArray data = this->reply->readAll();

	//Write the current buffer to file
	if ( this->resume_downloads && this->downloaded <= 188) {
		//Strip MPEG Transport Stream when resuming (Maybe?)
		this->current_file.write(data.right(data.size()-188));
	} else {
		this->current_file.write(data);
	}
	downloaded += data.size();
}

void Task::downloadCompleted() {
	std::cout << "Saved to " << this->current_file.fileName().toStdString() << std::endl;

	// Remove the read event
	QObject::disconnect(this->reply, &QNetworkReply::readyRead, nullptr,nullptr);

	//Close Temporary File, rename file to original filename
	this->current_file.close();

	this->reply->deleteLater();
	if ( download_actions.count() ) {
		// More Downloads
		QTimer::singleShot(scan_time, this, &Task::actionDownload);
	} else {
		emit taskCompleted();
	}

}

void Task::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
	qint64 q = 0;
	qint64 bytes_left = bytesTotal - bytesReceived;
	qint64 left = INT64_MAX;

	if ( this->timer.elapsed() >= 1000) {
		q = bytesReceived / (this->timer.elapsed()/1000);
		left = bytes_left / (q ? q :1);
	}

	std::cout << ""
			  << QLocale::system().formattedDataSize(bytesReceived).toStdString()
			  << " of "
			  << QLocale::system().formattedDataSize(bytesTotal).toStdString()
			  << " at " << QLocale::system().formattedDataSize(q).toStdString()
			  << " per second. "
			  << GetRemainingTime(left).toStdString()
			  << "\t\t\t\r" << std::flush;
}

void Task::downloadError(QNetworkReply::NetworkError code) {
	if (code == QNetworkReply::NetworkError::RemoteHostClosedError) {
		//Ignore Error
		//emit fetchHasClosedConnection();
	} else {
		qDebug() << "error" << code;
		this->reply->deleteLater();
	}
}




