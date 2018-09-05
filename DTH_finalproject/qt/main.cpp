#include <QApplication>
#include <QWidget>
#include <QPalette>
#include <QLabel>
#include <QtGui>
#include <QFont>
#include <QPainter>
#include <QBrush>
#include <QPixmap>
#include <QPicture>


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
QPainter *p;
QPicture *pi;

int width;
int height;


class QtThread : public QThread {
	public:
        	void run();
};


int main(int argc, char **argv){
	QApplication app(argc, argv);

	QWidget w;
	QPalette pal = w.palette();
	pal.setColor(QPalette::Window, Qt::blue);
	w.setPalette(pal);
	w.setAutoFillBackground(true);
	w.showFullScreen();

	screen = new QLabel;
	pi = new QPicture;
	p = new QPainter(pi);
	QRect bRect(0,0,450,250);
	bRect = bRect.normalized();
	pi->setBoundingRect(bRect);

	screen->showFullScreen();
	screen->setAlignment(Qt::AlignCenter);
	screen->show();

	QtThread *thread = new QtThread;
	thread->start();
	return app.exec();

}

void QtThread::run()
{
	FILE *pFile;
	char display[128];
	char line[32];
	QBrush br, br2, br3;
	br.setColor(Qt::red);
	br.setStyle(Qt::SolidPattern);

	br2.setColor(Qt::blue);
	br2.setStyle(Qt::SolidPattern);
	
	br3.setColor(Qt::black);
	br3.setStyle(Qt::SolidPattern);

	while(true)
	{
		QVector<QRectF> snake;
		QVector<QRectF>::iterator it;
		QVector<QRectF>::iterator it2;
		it2 = snake.begin();
		if(it2->width() == 0 && it2->height() == 0)
		{
			snake.clear();
			delete(p);
			p = new QPainter(pi);
		}

		int i = 0;
		system("cat /dev/mysnake > lcd.log");
		pFile = fopen("lcd.log", "r");
		while(fgets(line, 32, pFile) != NULL)//!eof(pFile))
		{
			sscanf(line, "%d, %d\n", &width, &height);
			QRectF temp(25+15*(width-1), 25 + 15*(height-1), 15,15);
			temp = temp.normalized();
			//printf("%d, %d\n", 25+15*(width-1), 25 + 15*(height-1));
			snake << temp;
			i++;
		}
		for(it = snake.begin(); it != snake.end(); it++)
		{
			if(it == snake.begin())
			{
				p->setBrush(br);
				p->drawRect(*it);
			}
			else
			{
				p->setBrush(br2);
				p->drawRect(*it);
			}

			QRectF border1(430,0,25,250);
			QRectF border2(0,0,25,250);
			QRectF border3(0,235,455,25);
			QRectF border4(0,0,450,25);
			border1 = border1.normalized();
			border2 = border2.normalized();
			border3 = border3.normalized();
			border4 = border4.normalized();
			p->setBrush(br3);
			p->drawRect(border1);
			p->drawRect(border2);
			p->drawRect(border3);
			p->drawRect(border4);				
		}
		if( i != 0)
		{
			printf("\n");
			p->end();
			screen->setPicture(*pi);
			delete(p);
			p = new QPainter(pi);
		}
		//system ("rm lcd.txt");
	}
}

