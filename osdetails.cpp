#include "osdetails.h"

OSDetails::OSDetails(const QScreen *screen, QObject *parent) :
    QObject(parent)
{
    int osName = UNKNOWN;
    #ifdef Q_OS_ANDROID
         osName = OS_ANDROID;
    #endif
    m_osName = osName;

    m_dpi = screen->physicalDotsPerInch();
}

unsigned int OSDetails::getOSName()
{
    return m_osName;
}

qreal OSDetails::getDPI()
{
    return m_dpi;
}
