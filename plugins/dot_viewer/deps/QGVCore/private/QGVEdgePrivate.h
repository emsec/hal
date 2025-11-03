#ifndef QGVEDGEPRIVATE_H
#define QGVEDGEPRIVATE_H

#include <cgraph.h>

class QGVEdgePrivate
{
	public:
		QGVEdgePrivate(Agedge_t *edge = NULL);

		void setEdge(Agedge_t *edge);
		Agedge_t* edge() const;

        Agnode_t* endpoint(bool head) const;
	private:
		Agedge_t* _edge;
};

#endif // QGVEDGEPRIVATE_H
