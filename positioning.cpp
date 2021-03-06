#include "positioning.h"
#include "eventwidget.h"
#include <QDebug>

void PlanarPosition::position(NetModel* model)
{
	typedef QSet<Event*> evlist;
	Event* begin = model->getBeginEvent();
    if (!begin)
        return ;
	evlist wave;
	evlist nextWave;
	evlist processed;
	wave.insert(begin);
	int x=10;
	int y=10;
	begin->getPoint()=QPoint(x,y);
        x+=150;
	processed.insert(begin);
	while(!wave.isEmpty()) {
		nextWave.clear();
		y=10;
		foreach(Event* cev,wave)
		{
			QList<Operation*> ops = cev->getOutOperations();
			qDebug() << ops.count();
			foreach(Operation *op,ops)
			{
				Event *end=op->getEndEvent();
				if (end&&!processed.contains(end)) {
					end->getPoint()=QPoint(x,y);
					processed.insert(end);
					nextWave.insert(end);
                    y+=150;
				}
			}
		}
        x+=60;
		wave=nextWave;
	}
}
