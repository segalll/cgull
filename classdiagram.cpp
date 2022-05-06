#include "classdiagram.h"

#include <QFile>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>

ClassEntry::ClassEntry(QString className, QString classPath, ClassDiagramEmitter* emitter, Runner* runner) : m_className(className), m_classPath(classPath), m_emitter(emitter), m_runner(runner) {
    setAcceptHoverEvents(true);
    setData(0, className);
}

void ClassEntry::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_hoverOver = true;
    update();
}

void ClassEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
    m_hoverOver = false;
    update();
}

void ClassEntry::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && contains(event->pos())) {
        m_lastClickPos = event->pos();
    }
}

constexpr int roundToMultiple(int n, int spacing) {
    int a = (n / spacing) * spacing;
    int b = a + spacing;

    return (n - a > b - n) ? b : a;
}

void ClassEntry::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        const QPointF delta = event->pos() - m_lastClickPos;
        moveBy(roundToMultiple(delta.x(), 10), roundToMultiple(delta.y(), 10));
    }
}

void ClassEntry::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if (contains(event->pos())) {
        emit m_emitter->classOpened(m_classPath);
    }
}

void ClassEntry::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QMenu* m = new QMenu;
    m->addAction("void main(String[] args)", [this]() {
        m_runner->show();
        m_runner->run(m_className);
    });
    QAction* d = new QAction("Delete");
    d->connect(d, &QAction::triggered, [this]() {
        QFile::remove(m_classPath);
        QFile::remove(m_classPath.chopped(5) + ".class");
        emit m_emitter->classClosed(m_className);
        delete this;
    });
    m->addAction(d);
    m->setDefaultAction(d);
    m->setStyleSheet("QMenu::item:default { color: #ff3333; }");
    m->exec(event->screenPos());
}

QRectF ClassEntry::boundingRect() const {
    return QRectF(0, 0, 120, 70);
}

void ClassEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(m_hoverOver ? m_color.lighter(130) : m_color);
    painter->setRenderHint(QPainter::Antialiasing);

    QRectF rect(0, 0, 120, 70);
    painter->drawRoundedRect(rect, 4, 4, Qt::RelativeSize);
    painter->setPen(Qt::black);
    painter->drawText(rect, Qt::AlignCenter, m_className);
}
