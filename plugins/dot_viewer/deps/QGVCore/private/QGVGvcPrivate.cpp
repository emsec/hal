#include "QGVGvcPrivate.h"

QGVGvcPrivate::QGVGvcPrivate(GVC_t *context)
{
		setContext(context);
}

void QGVGvcPrivate::setContext(GVC_t *context)
{
	_context = context;
}

GVC_t* QGVGvcPrivate::context() const
{
	return _context;
}
