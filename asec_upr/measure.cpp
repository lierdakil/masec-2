#include "measure.h"
#include "sleep.h"
//TODO: Fix comments...

#define draw_x(x) emit line(x,0,x,-127,Qt::SolidLine)
#define draw_y(y) emit line(0,y,2500,y,Qt::SolidLine)

QList<qreal> sm_diff(QByteArray data, int nr)//nr - Depth of smoothing in one direction
{
	QList<qreal> diff;

	for (int i=0;i<data.count();i++)
	{
		if(i<nr || i>=data.count()-nr)
		{
			diff<<0;
			continue;
		}
		double a=0;
		double b=0;
		for (int k=-nr;k<=nr;k++)
		{
			a+=k*data[i+k]-k*data[i+k]/nr;
			b+=k*k;
		}

		diff<<a/b*10;
	}
	return diff;
}

cmeasure::cmeasure(QString oscstr, QString genstr, QString volstr, double sf, double ff, double epsilon, double volts1, int sm1, int sm2, QObject *parent)
{
    connect(this,SIGNAL(path(QList<qreal>,QPen)),parent,SLOT(path(QList<qreal>,QPen)),Qt::QueuedConnection);
    connect(this,SIGNAL(path(QByteArray,QPen)),parent,SLOT(path(QByteArray,QPen)),Qt::QueuedConnection);
    connect(this,SIGNAL(line(qreal,qreal,qreal,qreal,QPen)),parent,SLOT(line(qreal,qreal,qreal,qreal,QPen)),Qt::QueuedConnection);
	this->oscstr=oscstr;
	gen = new genctrl(genstr);
	vol = new volctrl(volstr);
	fsf=sf;
	fff=ff;
	this->epsilon=epsilon;
	this->volts1=volts1;
	this->sm1=sm1;
	this->sm2=sm2;
	findresonance();
}

cmeasure::~cmeasure()
{
	delete gen;
	delete vol;
}

QByteArray cmeasure::sweep()
{
	osc = new oscctrl((char*)oscstr.toAscii().data());
	osc->setch1(volts1);

	int mini=0;
	int maxi=0;
	int starti, stopi;

	QByteArray data;

	while(!(mini>maxi))
	{
		gen->setsweep(fsf,fff);
		osc->wait("READY");
		gen->startsweep();
		osc->wait("READY");
		data = osc->readcurve();

/*		    max
 *		     _
 *		    / \
 *		___/   |   _______
 *		        \_/
 *		        min
 *
 *         /\     _
 *		---  \   / -------
 *            \_/
 *		    diff_min/
 *
 */

		emit path(data,QPen(Qt::green));

		QList<qreal> diff = sm_diff(data,sm1);

		emit path(diff,QPen(Qt::darkYellow));

		double min_diff_val=0;
		int min_diff_index;

		//find diff_min
		for (int i=0;i<diff.count();i++)
			if (diff[i]<min_diff_val)
			{
				min_diff_val = diff[i];
				min_diff_index = i;
			}

		//find diff=0 right from diff_min <=> min
		for (int i=min_diff_index;i<diff.count()-1;i++)
			if (diff[i]<=0 && diff[i+1]>0)
			{
				mini = i;
				break;
			}

		//find diff=0 left from diff_min <=> max
		for (int i=min_diff_index;i>0;i--)
			if (diff[i-1]>=0 && diff[i]<0)
			{
				maxi = i;
				break;
			}

//		starti=2*maxi-min_diff_index;//double interval
//		stopi=2*mini-min_diff_index;
		starti=2*maxi-mini;
		draw_x(starti);
		stopi=2*mini-maxi;
		draw_x(stopi);
	}
	//calculate coefficents to convert index to frequency
	double kt = (fff-fsf)/data.count();
	sff = kt*(stopi)+fsf;
	ssf = kt*(starti)+fsf;
	k = (sff-ssf)/data.count();

	int max_data=0;
	for(int i=0;i<data.count();i++)
		if(data.at(i)>max_data)
			max_data=data.at(i);

//	double curv=osc->getch1();
//	double onediv=curv/25.4;
//	double maxv=max_data*onediv;
//	double newv=maxv/4;
	osc->setch1(osc->getch1()*max_data/101.6);
	k2=osc->getch1()/25.4;

	gen->setsweep(ssf,sff);
	osc->wait("READY");
	gen->startsweep();
	osc->wait("READY");

	QByteArray data2 = osc->readcurve();

	delete osc;
	return data2;
}

float cmeasure::getamplonf(float freq)
{
	gen->setfreq(freq);
	return vol->acquire();
}

float cmeasure::golden(float a, float b, float epsilon, bool max)
{
	//Метод золотого сечения
	const double phi = 1.61803398874989484;

	float x1=b-(b-a)/phi;
	float x2=a+(b-a)/phi;

	float y1=getamplonf(x1);
	float y2=getamplonf(x2);


	while ((b-a)>=epsilon*2)
	{
		if ( (y1>y2 && max) || (y1<y2 && !max))
		{
			b=x2;
			x2=x1;
			y2=y1;
			x1=a+b-x2;
			y1=getamplonf(x1);
		}
		else
		{
			a=x1;
			x1=x2;
			y1=y2;
			x2=b-(x1-a);
			y2=getamplonf(x2);
		}
	}

	return (a+b)/2;
}

void cmeasure::findresonance()
{
	int xmin=0,xmax1=0,xfmax=0,xmax2=0;

	QByteArray dat;
	QList<qreal> diff;

	while ((xfmax <= xmax1)||(xfmax >= xmin))
	{
		//draw X axis
		dat = sweep();

		emit line(0,0,dat.count(),0,Qt::SolidLine);

		int fmax=0;
		for (int i=0; i<dat.count();i++)
		{
			if (fmax<dat[i])
			{
				fmax=dat[i];
				xfmax=i;
			}
		}

		emit path(dat,QPen(Qt::blue));

		diff=sm_diff(dat,sm2);

		emit path(diff, QPen(Qt::red));

		float min=255;
		float max1=0;
		float max2=0;

		for (int i=0;i<dat.count();i++)
			if (min>diff[i])
			{
				min=diff[i];
				xmin=i;
			}

		for (int i=0;i<xmin;i++)
			if (max1<diff[i])
			{
				max1=diff[i];
				xmax1=i;
			}

		for (int i=xmin;i<dat.count();i++)
			if (max2<diff[i])
			{
				max2=diff[i];
				xmax2=i;
			}

		draw_x(xmin);
		draw_x(xmax1);
		draw_x(xmax2);
	}

	for(int i=0; i<dat.count(); i++)
	{
		curve<<QPair<double,double>(k*i+ssf, dat[i]*k2);
	}

	gen->sweepoff();

#define GOLDEN

#ifdef GOLDEN
	rf = golden(k*xmax1+ssf,k*xmin+ssf,epsilon,true);
#else
	rf=0;
	//left to right
	for(int i=xmax1;i<xmin;i++)
	{
		if(diff[i]<0)
		{
			rf=(qreal)i/(qreal)2;
			break;
		}
	}
	//right to left
	for(int i=xmin;i>xmax1;i--)
	{
		if(diff[i]>0)
		{
			rf+=(qreal)i/(qreal)2;
			break;
		}
	}
	/* rf= median of two points closest to edges of
	 * segment xmax1-xmin, where diff changes sign.
	 */
	rf=k*rf+ssf;
#endif
	ra=getamplonf(rf);

	draw_x((rf-ssf)/k);

#ifdef GOLDEN
	af = golden(k*xmin+ssf,k*xmax2+ssf,epsilon,false);
#else
	af=0;
	//left to right
	for(int i=xmin;i<xmax2;i++)
	{
		if(diff[i]>0)
		{
			af+=(qreal)i/(qreal)2;
			break;
		}
	}
	//right to left
	for(int i=xmax2;i>xmin;i--)
	{
		if(diff[i]<0)
		{
			af+=(qreal)i/(qreal)2;
			break;
		}
	}
	/* af= median of two points closest to edges of
	 * segment xmin-xmax2, where diff changes sign.
	 */
	af=k*af+ssf;
#endif
	aa=getamplonf(af);

	gen->sweepon();
	draw_x((af-ssf)/k);

	draw_y(-aa/k2);
	draw_y(-ra/k2);
}

