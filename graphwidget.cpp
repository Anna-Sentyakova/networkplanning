/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include "graphwidget.h"
#include <QDebug>
#include <cmath>

void GraphWidget::setModel(NetModel* model)
{

	_model=model;
	if (!model) return;

	QList<Event*>* events = _model->getEvents();
	foreach(Event* ev,*events)
	{
		EventWidget *ic = new EventWidget(ev,model,this);
                //ic->setText(QString::number(ev->getN()));
		ic->move(ev->getPoint());
		ic->show();
		ic->setAttribute(Qt::WA_DeleteOnClose);
	}
        connect(model, SIGNAL(eventNameChanged(QObject *, Event *, const QString &)), this, SLOT(eventNameChanged(QObject *, Event *, const QString &)));
}

void GraphWidget::updatePositions()
{
	
}

//! [0]
GraphWidget::GraphWidget(QWidget *parent)
    : QFrame(parent)
{
    _model=0;
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);

    setAcceptDrops(true);
/*
    QLabel *boatIcon = new QLabel(this);
    boatIcon->setPixmap(QPixmap(":/images/boat.png"));
    boatIcon->move(20, 20);
    boatIcon->show();
    boatIcon->setAttribute(Qt::WA_DeleteOnClose);

    QLabel *carIcon = new QLabel(this);
    carIcon->setPixmap(QPixmap(":/images/car.png"));
    carIcon->move(120, 20);
    carIcon->show();
    carIcon->setAttribute(Qt::WA_DeleteOnClose);

    QLabel *houseIcon = new QLabel(this);
    houseIcon->setPixmap(QPixmap(":/images/house.png"));
    houseIcon->move(20, 120);
    houseIcon->show();
    houseIcon->setAttribute(Qt::WA_DeleteOnClose);*/
}
//! [0]

void GraphWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
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
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
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

        QLabel *newIcon = new QLabel(this);
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
    } else {
        event->ignore();
    }

}

//! [1]
void GraphWidget::mousePressEvent(QMouseEvent *event)
{
    QLabel *child = static_cast<QLabel*>(childAt(event->pos()));
    if (!child)
        return;

/*    QPixmap pixmap = *child->pixmap();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << pixmap << QPoint(event->pos() - child->pos()
);*/
//! [1]

//! [2]
    QMimeData *mimeData = new QMimeData;
    //mimeData->setData("application/x-dnditemdata", itemData);
//! [2]
        
//! [3]
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    //drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos() - child->pos());
//! [3]

    /*QPixmap tempPixmap = pixmap;
    QPainter painter;
    painter.begin(&tempPixmap);
    painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 127));
    painter.end();

    child->setPixmap(tempPixmap);

    if (drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction) == Qt::MoveAction)
        child->close();
    else {
        child->show();
        child->setPixmap(pixmap);
    }*/

}

template<class Ty>
        Ty sqr(Ty x)
{
    return x*x;
}

QLine GraphWidget::adjustLine(QPoint p1,QPoint p2,int margin)
{
    QPoint vec = p2-p1;
    double len = sqrt(  sqr(vec.x())+sqr(vec.y()) );
    //QPoint shift = (vec/len)*static_cast<double>(margin);
    double sx = static_cast<double>(vec.x())/len*static_cast<double>(margin);
    double sy = static_cast<double>(vec.y())/len*static_cast<double>(margin);
    QPoint shift(sx,sy);
    return QLine(p1+shift,p2-shift);
}

void GraphWidget::paintEvent(QPaintEvent *event)
{
	
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
                                //painter.drawLine();
                            drawArrow(painter,adjustLine(be->getPoint()+center,ee->getPoint()+center,15));
			}
		}

	}
	painter.end();
}

void GraphWidget::drawArrow(QPainter &p,QLine l) const
{
    p.translate(l.p2());
    double angle = atan2(l.dx(),l.dy())/M_PI*180.0;
    p.rotate(-angle);
    double len = sqrt(  sqr(l.dx())+sqr(l.dy()) );
    //QPoint end
    p.drawLine(QPoint(0,0), QPoint(0,-len));
    p.drawLine(0,0,4,-10);
    p.drawLine(0,0,-4,-10);
    //p.drawLine(
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
