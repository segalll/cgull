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

void ClassEntry::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    Q_UNUSED(event);
    m_hoverOver = false;
    update();
}

void ClassEntry::mousePressEvent(QGraphicsSceneMouseEvent* event) {
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
        emit m_emitter->classMoved(m_className, pos().toPoint());
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
    return QRectF(0, 0, 118, 68);
}

void ClassEntry::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(m_hoverOver ? m_color.lighter(130) : m_color);
    painter->setRenderHint(QPainter::Antialiasing);

    QRectF rect(0, 0, 118, 68);
    painter->drawRoundedRect(rect, 4, 4, Qt::RelativeSize);

    if (data(1).toBool()) {
        painter->setPen(QPen(Qt::darkRed, 3));
        const static QList<QLine> errorLines = {
            QLine(1, 51, 17, 67),
            QLine(1, 35, 33, 67),
            QLine(1, 19, 49, 67),
            QLine(13, 16, 65, 67),
            QLine(29, 16, 81, 67),
            QLine(45, 16, 97, 67),
            QLine(61, 16, 113, 67),
            QLine(77, 16, 117, 55),
            QLine(93, 16, 117, 39),
            QLine(109, 16, 117, 23),
            QLine(1, 28, 13, 16),
            QLine(1, 44, 29, 16),
            QLine(1, 60, 45, 16),
            QLine(9, 67, 61, 16),
            QLine(25, 67, 77, 16),
            QLine(41, 67, 93, 16),
            QLine(57, 67, 109, 16),
            QLine(73, 67, 117, 24),
            QLine(89, 67, 117, 40),
            QLine(105, 67, 117, 56)
        };
        painter->drawLines(errorLines);
    } else if (data(2).toBool()) {
        painter->setPen(QPen(Qt::darkGray, 3));
        const static QList<QLine> uncompiledLines = {
            QLine(1, 28, 13, 16),
            QLine(1, 42, 27, 16),
            QLine(1, 56, 41, 16),
            QLine(3, 67, 55, 16),
            QLine(17, 67, 69, 16),
            QLine(31, 67, 83, 16),
            QLine(45, 67, 97, 16),
            QLine(59, 67, 111, 16),
            QLine(73, 67, 117, 24),
            QLine(87, 67, 117, 38),
            QLine(101, 67, 117, 52),
            QLine(115, 67, 117, 66)
        };
        painter->drawLines(uncompiledLines);
    }

    painter->setPen(QPen(QColor(40, 40, 40, 255), 2));
    painter->setBrush(QColor(0, 0, 0, 0));
    painter->drawRoundedRect(rect, 4, 4, Qt::RelativeSize);
    painter->drawLine(0, 15, 118, 15);

    painter->setPen(Qt::black);
    painter->drawText(QRect(0, 0, 120, 15), Qt::AlignCenter, m_className);
}
