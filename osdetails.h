#ifndef OSDETAILS_H
#define OSDETAILS_H

#include <QObject>
#include <QScreen>

class OSDetails : public QObject
{
    Q_OBJECT
public:
    enum OSTYPE {
        UNKNOWN = 1,
        OS_ANDROID = 2
    };
    OSDetails(const QScreen *screen, QObject *parent = 0);
    Q_INVOKABLE unsigned int getOSName();
    Q_INVOKABLE qreal getDPI();

private:
    unsigned int m_osName;
    qreal m_dpi;
};

#endif // OSDETAILS_H
