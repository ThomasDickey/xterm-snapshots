/* $XFree86: xc/programs/xterm/proto.h,v 3.0 1996/01/10 05:51:43 dawes Exp $ */

#ifndef included_proto_h
#define included_proto_h

#include <X11/Xfuncproto.h>

#if NeedFunctionPrototypes
#define PROTO(p) p
#else
#define PROTO(p) ()
#endif

#define NOPROTO(p) (/*nothing*/)

#define PROTO_XT_ACTIONS_ARGS \
	PROTO((Widget w, XEvent *event, String *params, Cardinal *num_params))

#define PROTO_XT_CALLBACK_ARGS \
	PROTO((Widget gw, XtPointer closure, XtPointer data))

#define PROTO_XT_CVT_SELECT_ARGS \
	PROTO((Widget w, Atom *selection, Atom *target, Atom *type, XtPointer *value, unsigned long *length, int *format))

#define PROTO_XT_EV_HANDLER_ARGS \
	PROTO((Widget w, XtPointer closure, XEvent *event, Boolean *cont))

#define PROTO_XT_SEL_CB_ARGS \
	PROTO((Widget w, XtPointer client_data, Atom *selection, Atom *type, XtPointer value, unsigned long *length, int *format))

#ifdef SIGNALRETURNSINT
#define SIGNAL_T int
#define SIGNAL_RETURN return 0
#else
#define SIGNAL_T void
#define SIGNAL_RETURN return
#endif

#undef  Size_t
#ifndef X_NOT_STDC_ENV
#define Size_t size_t
#else
#define Size_t int
#endif

#endif/*included_proto_h*/
