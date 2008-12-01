#ifndef CTRL_VOL_H_
#define CTRL_VOL_H_

#include "gpib.h"
#include <QStringList>

class volctrl: public GPIBctrl
{
public:
	volctrl(QString GPIBID);
	~volctrl();
	double acquire();
};

#endif /*CTRL_VOL_H_*/
