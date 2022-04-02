#ifndef CLASSDIAGRAM_H
#define CLASSDIAGRAM_H

#include <QGraphicsItem>

class ClassDiagramEmitter : public QObject
{
    Q_OBJECT
signals:
    void classOpened(QString className);
};

class ClassEntry : public QGraphicsItem
{
public:
    ClassEntry(QString className = "", ClassDiagramEmitter *emitter = nullptr);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    QColor color = Qt::lightGray;
    QPointF lastClickPos = QPointF(0, 0);
    bool hoverOver = false;
    QString className;
    ClassDiagramEmitter *emitter;
};

#endif // CLASSDIAGRAM_H
