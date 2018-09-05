#include <QApplication>
#include <QWidget>
#include <QPalette>
#include <QLabel>
#include <QtGui>
#include <QFont>
#include <QPainter>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

QLabel *screen;

class QtThread : public QThread {
	public:
        	void run();
};



int main(int argc, char **argv){
	QApplication app(argc, argv);

	QWidget w;
	QPalette p = w.palette();
	p.setColor(QPalette::Window, Qt::blue);
	w.setPalette(p);
	w.setAutoFillBackground(true);
	w.showFullScreen();

	screen = new QLabel;	
	screen -> resize(500,300);
	screen -> setAlignment(Qt::AlignCenter);
	screen->show();

	QtThread *thread = new QtThread;
	thread->start();

	return app.exec();
}


void QtThread::run()
{
	int pFile;

	char line[32];
	char count[6];
	char direction[6]; 
	char speed[6];
	char state[6];
	char brightness[6];
	char display[128];

	while(true)
	{
		system("cat /dev/mygpio > lcd.txt");
		pFile = open("lcd.txt", O_RDWR);
		read(pFile, line, 32);

		sscanf(line, "%s\t%s\t%s\t%s\t%s", count, speed, state, direction, brightness);
		sprintf(display,"Count: %s\nDirection: %s\nSpeed: %s\nBrightness: %s\nOn/Off: %s", count, direction, speed, brightness, state);

		QFont font = screen->font();
		font.setPointSize(20);
		screen->setFont(font);
		screen->setText(display);

		system ("rm lcd.txt");
	}
}



