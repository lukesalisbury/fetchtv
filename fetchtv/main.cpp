
#include <QCoreApplication>
#include <QCommandLineParser>

#include "task.hpp"


int main(int argc, char *argv[]) {
	QCoreApplication a(argc, argv);

	QCoreApplication::setApplicationName("fetchtv");
	QCoreApplication::setApplicationVersion("20190125");
	new Task(&a);

	return a.exec();
}
