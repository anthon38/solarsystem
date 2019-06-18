#include "pickable.h"
Pickable::Pickable(QObject *parent)
    : Renderable(parent)
{
}

void Pickable::setColor(int id)
{
    m_color = QVector3D((id >> 16) & 255, (id >> 8) & 255, id & 255)/255;
}

void Pickable::setColor(const QVector3D &color)
{
    m_color = color;
}
