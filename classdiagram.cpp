#include "classdiagram.h"

#include <QFile>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QMenu>

ClassEntry::ClassEntry(QString className, QString classPath, ClassDiagramEmitter *emitter, Runner *runner) : className(className), classPath(classPath), emitter(emitter), runner(runner)
{
    setAcceptHoverEvents(true);
}

void ClassEntry::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    hoverOver = true;
    update();
}

void ClassEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    hoverOver = false;
    update();
}

void ClassEntry::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && contains(event->pos()))
    {
        lastClickPos = event->pos();
    }
}

int roundToMultiple(int n, int spacing)
{
    int a = (n / spacing) * spacing;
    int b = a + spacing;

    return (n - a > b - n)? b : a;
}

void ClassEntry::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        const QPointF delta = event->pos() - lastClickPos;
        moveBy(roundToMultiple(delta.x(), 10), roundToMultiple(delta.y(), 10));
    }
}

void ClassEntry::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (contains(event->pos()))
    {
        emit emitter->classOpened(classPath);
    }
}

void ClassEntry::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    QMenu* m = new QMenu;
    m->addAction("void main(String[] args)", [this]() {
        runner->show();
        runner->run(className);
    });
    QAction* d = new QAction("Delete");
    d->connect(d, &QAction::triggered, [this]() {
        QFile::remove(classPath);
        QFile::remove(classPath.chopped(5) + ".class");
        delete this;
    });
    m->addAction(d);
    m->setDefaultAction(d);
    m->setStyleSheet("QMenu::item:default { color: #ff3333; }");
    m->exec(event->screenPos());
}

QRectF ClassEntry::boundingRect() const
{
    return QRectF(0, 0, 120, 70);
}

void ClassEntry::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->setBrush(hoverOver ? color.lighter(130) : color);
    painter->setRenderHint(QPainter::Antialiasing);
    QRectF rect(0, 0, 120, 70);
    painter->drawRoundedRect(rect, 4, 4, Qt::RelativeSize);
    painter->setPen(Qt::black);
    painter->drawText(rect, Qt::AlignCenter, className);
}
