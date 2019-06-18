#include "timeline.h"

static const int interval = 10; //ms

Timeline::Timeline()
    : J2000(946727935.0)
{
    realTime();
    startTimer(interval);
}

void Timeline::realTime()
{
    realRate();
    m_currentTime = QDateTime::currentMSecsSinceEpoch();
}

void Timeline::speedUp()
{
    if (m_speedRate < -1)
        m_speedRate /= 10;
    else if (m_speedRate < 1)
        m_speedRate += 1;
    else if (m_speedRate < 1000000000)
        m_speedRate *= 10;
    emit rateChanged();
}

void Timeline::speedDown()
{
    if (m_speedRate > 1)
        m_speedRate /= 10;
    else if (m_speedRate > -1)
        m_speedRate -= 1;
    else if (m_speedRate > -1000000000)
        m_speedRate *= 10;
    emit rateChanged();
}

QDateTime Timeline::dateTime() const
{
    return QDateTime::fromMSecsSinceEpoch(m_currentTime);
}

void Timeline::setDateTime(const QDateTime &dateTime)
{
    m_currentTime = dateTime.toMSecsSinceEpoch();
}

void Timeline::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    m_currentTime += m_speedRate*interval;
    emit tick();
}
