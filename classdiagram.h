#ifndef CLASSDIAGRAM_H
#define CLASSDIAGRAM_H

#include "runner.h"

#include <QGraphicsItem>

class ClassDiagramEmitter : public QObject
{
    Q_OBJECT
signals:
    void classOpened(QString classPath);
};

class ClassEntry : public QGraphicsItem
{
public:
    ClassEntry(QString className = "", QString classPath = "", ClassDiagramEmitter *emitter = nullptr, Runner *runner = nullptr);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    QColor color = Qt::lightGray;
    QPointF lastClickPos = QPointF(0, 0);
    bool hoverOver = false;
    QString className;
    QString classPath;
    ClassDiagramEmitter *emitter;
    Runner *runner;
};

#endif // CLASSDIAGRAM_H
