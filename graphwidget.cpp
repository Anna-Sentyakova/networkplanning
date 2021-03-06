#include <QtGui>

#include "graphwidget.h"
#include "arrowpointwidget.h"
#include <QDebug>
#include <math.h>

void GraphWidget::setModel(NetModel* model)
{
    if (_model!=0)
    {
        foreach(QObject* o, children())
        {
            delete o;
        }
        disconnect(0,this,0);
    }
    _model=model;
    if (!model) return;

    QList<Event*>* events = _model->getEvents();
    foreach(Event* ev, *events)
    {
        EventAdd(0,ev);
    }
    connect(model, SIGNAL(eventNameChanged(QObject *, Event *, const QString &)), this, SLOT(eventNameChanged(QObject *, Event *, const QString &)));
    connect(model, SIGNAL(eventIdChanged (QObject *, Event *, const int)), this, SLOT(eventIdChanged(QObject *, Event *, const int)));
    connect(model, SIGNAL(afterEventAdd(QObject*,Event*)), this, SLOT(EventAdd(QObject*,Event*)));
    connect(model, SIGNAL(afterEventInsert(QObject*,Event*,int)), this, SLOT(EventAdd(QObject*,Event*,int)));
    connect(model, SIGNAL(beforeEventDelete(QObject*,Event*)),this, SLOT(DeleteEvent(QObject*,Event*)));
    connect(model, SIGNAL(updated()),this,SLOT(update()));
    connect(model, SIGNAL(beforeClear()),this,SLOT(clearModel()));
}

void GraphWidget::clearModel()
{
    foreach(QObject* o, children())
    {
        delete o;
    }
    update();
}

void GraphWidget::updatePositions()
{
}

void GraphWidget::EventAdd(QObject *, Event * ev)
{
    EventWidget *ic = new EventWidget(ev,_model,this);
    ic->move(ev->getPoint());
    ic->show();
    ic->setAttribute(Qt::WA_DeleteOnClose);
    recreatePoints();
    ic->update();
}

void GraphWidget::EventAdd(QObject *, Event * ev,int)
{
    EventAdd(0,ev);
}

GraphWidget::GraphWidget(QWidget *parent)
    : QFrame(parent)
{
    _model=0;
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAcceptDrops(true);
}

void GraphWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")||event->mimeData()->hasFormat("application/x-arrowpointdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void GraphWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")||event->mimeData()->hasFormat("application/x-arrowpointdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void GraphWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QPixmap pixmap;
        QPoint offset;
        dataStream >> pixmap >> offset;
        Event * chev = 0;
        dataStream.readRawData(reinterpret_cast<char*>(&chev),sizeof(chev));
        EventWidget *newIcon = new EventWidget(chev,_model,this);
        newIcon->setPixmap(pixmap);
        newIcon->move(event->pos() - offset);
        newIcon->show();
        newIcon->setAttribute(Qt::WA_DeleteOnClose);

        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else
        if (event->mimeData()->hasFormat("application/x-arrowpointdata")) {
        event->setDropAction(Qt::MoveAction);
        EventWidget *child = qobject_cast<EventWidget*>(childAt(event->pos()));
        if (child) child->dropEvent(event);
    } else
    {
        event->ignore();
    }
    update();
    recreatePoints();

}

void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    EventWidget *child = qobject_cast<EventWidget*>(childAt(event->pos()));
    arrowpointwidget *child1 = qobject_cast<arrowpointwidget*>(childAt(event->pos()));
    if (!(child||child1))
        return;
    QLabel *achild=child?static_cast<QLabel*> (child):static_cast<QLabel*> (child1);
    qDebug() << achild;
    qDebug() << child;
    QPixmap pixmap = *achild->pixmap();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);

    dataStream << pixmap << QPoint(event->pos() - achild->pos());
    QMimeData *mimeData = new QMimeData;
    if (child)
    {
        Event * chev = child->event();
        dataStream.writeRawData(reinterpret_cast<char*>(&chev),sizeof(chev));
        mimeData->setData("application/x-dnditemdata", itemData);
    }
    if (child1)
    {
        Event * chev = child1->ev;
        Operation * chop = child1->op;
        dataStream << static_cast<int>(child1->role);
        dataStream.writeRawData(reinterpret_cast<char*>(&chev  ),sizeof(chev));
        dataStream.writeRawData(reinterpret_cast<char*>(&chop  ),sizeof(chop));
        mimeData->setData("application/x-arrowpointdata", itemData);
    }

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos() - achild->pos());

    QPixmap tempPixmap = pixmap;
    QPainter painter;
    painter.begin(&tempPixmap);
    painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 255));
    painter.end();

    achild->setPixmap(tempPixmap);

    if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
        achild->close();
    else {
        achild->show();
        achild->setPixmap(pixmap);
    }
    recreatePoints();
}

template<class Ty>
        Ty sqr(Ty x)
{
    return x*x;
}

QLine GraphWidget::adjustLine(QPoint p1, QPoint p2, int margin)
{
    QPoint vec = p2 - p1;
    int x = vec.x(), y = vec.y();
    double len = sqrt((double)(x * x + y * y));
    double sx = static_cast<double>(x) / len * static_cast<double>(margin);
    double sy = static_cast<double>(y) / len * static_cast<double>(margin);
    QPoint shift(sx, sy);
    return QLine(p1 + shift, p2 - shift);
}

void GraphWidget::recreatePoints()
{
    foreach(QObject* qo,children())
    {
        arrowpointwidget * ap = qobject_cast<arrowpointwidget *>(qo);
        delete ap;
    }
    if (!_model) return;
    foreach(Operation * op,*_model->getOperations())
    {
        Event *be = op->getBeginEvent();
        Event *ee = op->getEndEvent();
        QPoint center = QPoint(15,15);
        if (be&&ee) {
            QLine l = adjustLine(be->getPoint()+center,ee->getPoint()+center,15);
            arrowpointwidget* ap = new arrowpointwidget(arrowpointwidget::EEnd,op,ee,this);
            ap->move(l.p2());
            ap->show();
            ap = new arrowpointwidget(arrowpointwidget::EBegin,op,be,this);
            ap->move(l.p1());
            ap->show();
        }
    }
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    qDebug() << "paint";
    if (!_model) return;
    QPainter painter;
    painter.begin(this);

    QList<Event*>* events = _model->getEvents();
    foreach(Event* ev,*events)
    {
        QList<Operation*> ops = ev->getOutOperations();
        foreach(Operation * op,ops)
        {
            Event *be = op->getBeginEvent();
            Event *ee = op->getEndEvent();
            QPoint center = QPoint(15,15);
            if (be&&ee) {
                drawArrow(painter,adjustLine(be->getPoint()+center,ee->getPoint()+center,15));
            }
        }
    }
    painter.end();
}

void GraphWidget::drawArrow(QPainter &p,QLine l)
{
    p.translate(l.p2());
    int dx = l.dx(), dy = l.dy();
    double angle = atan2((double)dx, (double)dy) / M_PI * 180.0;
    p.rotate(-angle);
    double len = sqrt((double)(dx * dx + dy * dy));
    p.drawLine(QPoint(0, 0), QPoint(0, -len));
    p.drawLine(0, 0, 4, -10);
    p.drawLine(0, 0, -4, -10);
    p.resetTransform();
}

void GraphWidget::eventNameChanged(QObject *, Event * event, const QString &)
{
    QObjectList ch = children();
    foreach(QObject* qo,ch)
    {
        EventWidget* ev = qobject_cast<EventWidget*>(qo);
        if (ev&&ev->wrapsEvent(event)) {
            ev->update();
        }
    }
}

void GraphWidget::eventIdChanged(QObject *, Event * event, const int id)
{
    Q_UNUSED(id);
    QObjectList ch = children();
    foreach(QObject* qo,ch)
    {
        EventWidget* ev = qobject_cast<EventWidget*>(qo);
        if (ev&&ev->wrapsEvent(event)) {
            ev->update();
        }
    }
}

void GraphWidget::DeleteEvent(QObject *, Event * event)
{
    QObjectList ch = children();
    foreach(QObject* qo,ch)
    {
        EventWidget* ev = qobject_cast<EventWidget*>(qo);
        if (ev&&ev->wrapsEvent(event)) {
            delete ev;
        }
    }
    recreatePoints();
}
