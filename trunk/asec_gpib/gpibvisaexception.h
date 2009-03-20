#ifndef GPIBVISAEXCEPTION_H
#define GPIBVISAEXCEPTION_H

#include "gpibgenericexception.h"
#include "visa.h"

class ASEC_GPIBSHARED_EXPORT GPIBVISAException : public GPIBGenericException
{
public:
    GPIBVISAException(ViStatus status, ViSession session);
};

#endif // GPIBVISAEXCEPTION_H
