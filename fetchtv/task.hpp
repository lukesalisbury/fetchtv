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
#include <QRegExp>

#include "../qtupnp/controlpoint.hpp"
#include "../qtupnp/device.hpp"

enum ArgumentStringType {
	AST_NUMBER,
	AST_DATE,
	AST_STRING
};


struct TaskAction {
	QString type;
	quint32 id;
};

class BasicDevice {
public:
	QString uuid;
	QString browseid;
};

class BasicInfo {
public:
	QString title;
	QString series;
	QString uri;
	QString filename;
	QString id;
	QDateTime date;
	int64_t filesize = 0;
	QString toString() {
		return QString("%1 [%2] %3 %4 %5").arg(this->id).arg(this->series).arg(this->title).arg(this->date.toString()).arg(this->filename);
	}
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
	void actionList();
	void actionDownload();
	void actionPreDownload();

	void exitSuccessfully();
	void exitNotSoSuccessfully();

	void downloadWrite();
	void downloadCompleted();
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void downloadError(QNetworkReply::NetworkError code);

	void upnpError(int errorCode, QString const & errorString);
	void newDevice( QString const & msg);
	void networkError(QString const & deviceUUID, QNetworkReply::NetworkError errorCode, QString const & errorDesc);

	private slots:


	private:
	BasicInfo CDidlItem2BasicInfo(QString const& serverUUID, const QtUPnP::CDidlItem & didlItem);
	BasicInfo get(QString const& serverUUID, QString id );
	QList<BasicInfo> buildList(const QtUPnP::CDevice & device, QString id);
	void list(QString const& serverUUID, QString id , QString outputPrefix = "");
	void retrievedContentList(QtUPnP::CDevice device);
	void downloadStart(QtUPnP::CDevice, quint32 id);
	void nextDownload();

	QNetworkAccessManager manager;
	QCoreApplication * app = nullptr;
	QCommandLineParser parser;
	QtUPnP::CControlPoint * upnp_cp = nullptr;

	QList<QString> download_actions;
	QList<QtUPnP::CDevice> founded_devices;
	QList<BasicInfo> cached_info;
	QString requested_device = "";

	QNetworkReply * reply;
	QTime timer;
	QDateTime since_date;

	QFile current_file;
	qint32 scan_time = 2000;
	qint64 downloaded = 0;

	bool has_failed = false;
	bool has_device_ip = false;
	bool output_as_csv = false;
	bool resume_downloads = false;
	bool continuing = false;
	void (Task::*action_method)();


};

#endif // TASK_HPP
