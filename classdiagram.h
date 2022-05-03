#ifndef CLASSDIAGRAM_H
#define CLASSDIAGRAM_H

#include "runner.h"

#include <QGraphicsItem>

class ClassDiagramEmitter : public QObject {
    Q_OBJECT
signals:
    void classOpened(QString classPath);
};

class ClassEntry : public QGraphicsItem {
public:
    ClassEntry(QString className = "", QString classPath = "", ClassDiagramEmitter* emitter = nullptr, Runner* runner = nullptr);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
private:
    QColor m_color = Qt::lightGray;
    QPointF m_lastClickPos;
    bool m_hoverOver = false;
    QString m_className;
    QString m_classPath;
    ClassDiagramEmitter* m_emitter;
    Runner* m_runner;
};

#endif // CLASSDIAGRAM_H
