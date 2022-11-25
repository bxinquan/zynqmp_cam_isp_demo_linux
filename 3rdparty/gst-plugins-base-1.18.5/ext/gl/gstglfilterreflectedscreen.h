/*
 * GStreamer
 * Copyright (C) 2008 Pierre Pouzol<pierre.pouzol@hotmail.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_GL_FILTERREFLECTEDSCREEN_H_
#define _GST_GL_FILTERREFLECTEDSCREEN_H_

#include <gst/gl/gstglfilter.h>

G_BEGIN_DECLS

#define GST_TYPE_GL_FILTER_REFLECTED_SCREEN          (gst_gl_filter_reflected_screen_get_type())
#define GST_GL_FILTER_REFLECTED_SCREEN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_GL_FILTER_REFLECTED_SCREEN,GstGLFilterReflectedScreen))
#define GST_IS_GL_FILTER_REFLECTED_SCREEN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_GL_FILTER_REFLECTED_SCREEN))
#define GST_GL_FILTER_REFLECTED_SCREEN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass) ,GST_TYPE_GL_FILTER_REFLECTED_SCREEN,GstGLFilterReflectedScreenClass))
#define GST_IS_GL_FILTER_REFLECTED_SCREEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass) ,GST_TYPE_GL_FILTER_REFLECTED_SCREEN))
#define GST_GL_FILTER_REFLECTED_SCREEN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj) ,GST_TYPE_GL_FILTER_REFLECTED_SCREEN,GstGLFilterReflectedScreenClass))

typedef struct _GstGLFilterReflectedScreen GstGLFilterReflectedScreen;
typedef struct _GstGLFilterReflectedScreenClass GstGLFilterReflectedScreenClass;

struct _GstGLFilterReflectedScreen
{
	GstGLFilter filter;
	gdouble fovy;
	gdouble aspect;
	gdouble znear;
	gdouble zfar;

	gboolean active_graphic_mode;
	gboolean separated_screen;
	gboolean show_floor;
};

struct _GstGLFilterReflectedScreenClass
{
	GstGLFilterClass filter_class;
};

GType gst_gl_filter_reflected_screen_get_type (void);

G_END_DECLS

#endif /* _GST_GLFILTERREFLECTEDSCREEN_H_ */

