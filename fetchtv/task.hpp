/*****************************************************************************
Copyright © Luke Salisbury

Licensed under the under the ZLIB or GNU General Public License Version 3, at 
your option. This file may not be copied, modified, or distributed except 
according to those terms.
*****************************************************************************/
#ifndef TASK_HPP
#define TASK_HPP

#include <QObject>
#include <QCoreApplication>
#include <QNetworkReply>
#include <QCommandLineParser>
#include <QFile>


#include "../qtupnp/controlpoint.hpp"
#include "../qtupnp/device.hpp"

class BasicDevice {
public:
	QString uuid;
	QString browseid;
};

class BasicInfo {
public:
	QString title;
	QString uri;
	int64_t filesize = 0;
};

class Task : public QObject
{
	Q_OBJECT
public:
	Task(QCoreApplication * a, QObject * parent = nullptr);

signals:
	void taskCompleted();
	void taskFailed();
	void stepCompleted();
	void fetchHasClosedConnection();

public slots:
	void actionHelp();
	void actionLastID();
	void actionList();
	void actionDownload();

	void exitSuccessfully();
	void exitNotSoSuccessfully();

	void downloadWrite();
	void downloadCompleted();
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void downloadError(QNetworkReply::NetworkError code);

	void upnpError(int errorCode, QString const & errorString);
	void newDevice( QString const & msg);
	void networkError(QString const & deviceUUID, QNetworkReply::NetworkError errorCode, QString const & errorDesc);

private:
	BasicInfo get(QString const& serverUUID, QString id );
	void list(QString const& serverUUID, QString id , QString outputPrefix = "");
	void retrievedContentList(QtUPnP::CDevice device);
	uint32_t retrievedSystemUpdateID(QtUPnP::CDevice device);

	QNetworkAccessManager manager;
	QCoreApplication * app = nullptr;
	QCommandLineParser parser;
	QtUPnP::CControlPoint * upnp_cp = nullptr;

	QList<QtUPnP::CDevice> founded_devices;
	uint8_t active_steps = 0;
	qint32 scan_time = 2000;
	quint32 download_id = 0;
	QFile file;
	qint64 downloaded = 0;
	QNetworkReply * reply;
	QTime timer;
	bool hasFailed = false;
};

#endif // TASK_HPP
