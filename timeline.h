#ifndef TIMELINE_H
#define TIMELINE_H

#include <QObject>
#include <QDateTime>

class Timeline : public QObject
{
    Q_OBJECT
public:
    Timeline();
    double currentTime() const {return m_currentTime/1000.0d-J2000;}
    void realTime();
    qint64 rate() {return m_speedRate;}
    void realRate() {m_speedRate = 1; emit rateChanged();}
    void speedUp();
    void speedDown();
    void pause() {m_speedRate = 0; emit rateChanged();}
    QDateTime dateTime() const;
    void setDateTime(const QDateTime& dateTime);

signals:
    void tick();
    void rateChanged();

protected:
    void timerEvent(QTimerEvent *event);

private:
    double J2000;
    qint64 m_currentTime;
    qint64 m_speedRate;
};

#endif // TIMELINE_H
