#include "QGVNodePrivate.h"

QGVNodePrivate::QGVNodePrivate(Agnode_t *node, Agraph_t *parent) : 
    _node(node), _parent(parent)
{
}

void QGVNodePrivate::setNode(Agnode_t *node)
{
    _node = node;
}

Agnode_t* QGVNodePrivate::node() const
{
    return _node;
}

Agraph_t* QGVNodePrivate::graph() const
{
    return _parent;
}
