/* GStreamer
 * Copyright (C) 1999 Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) 2005-2014 Tim-Philipp Müller <tim@centricular.net>
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

/**
 * SECTION:element-timeoverlay
 * @title: timeoverlay
 * @see_also: #GstBaseTextOverlay, #GstClockOverlay
 *
 * This element overlays the buffer time stamps of a video stream on
 * top of itself. You can position the text and configure the font details
 * using its properties.
 *
 * By default, the time stamp is displayed in the top left corner of the picture,
 * with some padding to the left and to the top.
 *
 * |[
 * gst-launch-1.0 -v videotestsrc ! timeoverlay ! autovideosink
 * ]|
 * Display the time stamps in the top left corner of the video picture.
 * |[
 * gst-launch-1.0 -v videotestsrc ! timeoverlay halignment=right valignment=bottom text="Stream time:" shaded-background=true font-desc="Sans, 24" ! autovideosink
 * ]|
 * Another pipeline that displays the time stamps with some leading
 * text in the bottom right corner of the video picture, with the background
 * of the text being shaded in order to make it more legible on top of a
 * bright video background.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/video/video.h>

#include "gsttimeoverlay.h"

#define DEFAULT_TIME_LINE GST_TIME_OVERLAY_TIME_LINE_BUFFER_TIME
#define DEFAULT_SHOW_TIMES_AS_DATES FALSE
#define DEFAULT_DATETIME_FORMAT "%F %T" /* YYYY-MM-DD hh:mm:ss */

enum
{
  PROP_0,
  PROP_TIME_LINE,
  PROP_SHOW_TIMES_AS_DATES,
  PROP_DATETIME_EPOCH,
  PROP_DATETIME_FORMAT,
};

#define gst_time_overlay_parent_class parent_class
G_DEFINE_TYPE (GstTimeOverlay, gst_time_overlay, GST_TYPE_BASE_TEXT_OVERLAY);

static void gst_time_overlay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_time_overlay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

#define GST_TYPE_TIME_OVERLAY_TIME_LINE (gst_time_overlay_time_line_type())
static GType
gst_time_overlay_time_line_type (void)
{
  static GType time_line_type = 0;
  static const GEnumValue modes[] = {
    {GST_TIME_OVERLAY_TIME_LINE_BUFFER_TIME, "buffer-time", "buffer-time"},
    {GST_TIME_OVERLAY_TIME_LINE_STREAM_TIME, "stream-time", "stream-time"},
    {GST_TIME_OVERLAY_TIME_LINE_RUNNING_TIME, "running-time", "running-time"},
    {GST_TIME_OVERLAY_TIME_LINE_TIME_CODE, "time-code", "time-code"},
    {0, NULL, NULL},
  };

  if (!time_line_type) {
    time_line_type = g_enum_register_static ("GstTimeOverlayTimeLine", modes);
  }
  return time_line_type;
}

static gchar *
gst_time_overlay_render_time (GstTimeOverlay * overlay, GstClockTime time)
{
  guint hours, mins, secs, msecs;

  if (!GST_CLOCK_TIME_IS_VALID (time))
    return g_strdup ("");

  hours = (guint) (time / (GST_SECOND * 60 * 60));
  mins = (guint) ((time / (GST_SECOND * 60)) % 60);
  secs = (guint) ((time / GST_SECOND) % 60);
  msecs = (guint) ((time % GST_SECOND) / (1000 * 1000));

  return g_strdup_printf ("%u:%02u:%02u.%03u", hours, mins, secs, msecs);
}

/* Called with lock held */
static gchar *
gst_time_overlay_get_text (GstBaseTextOverlay * overlay,
    GstBuffer * video_frame)
{
  GstTimeOverlay *self = GST_TIME_OVERLAY (overlay);
  GstTimeOverlayTimeLine time_line;
  gchar *time_str, *txt, *ret;

  overlay->need_render = TRUE;

  time_line = g_atomic_int_get (&GST_TIME_OVERLAY_CAST (overlay)->time_line);
  if (time_line == GST_TIME_OVERLAY_TIME_LINE_TIME_CODE) {
    GstVideoTimeCodeMeta *tc_meta =
        gst_buffer_get_video_time_code_meta (video_frame);
    if (!tc_meta) {
      GST_DEBUG ("buffer without valid timecode");
      return g_strdup ("00:00:00:00");
    }
    time_str = gst_video_time_code_to_string (&tc_meta->tc);
    GST_DEBUG ("buffer with timecode %s", time_str);
  } else {
    GstClockTime ts, ts_buffer;
    GstSegment *segment = &overlay->segment;

    ts_buffer = GST_BUFFER_TIMESTAMP (video_frame);

    if (!GST_CLOCK_TIME_IS_VALID (ts_buffer)) {
      GST_DEBUG ("buffer without valid timestamp");
      return g_strdup ("");
    }

    GST_DEBUG ("buffer with timestamp %" GST_TIME_FORMAT,
        GST_TIME_ARGS (ts_buffer));

    switch (time_line) {
      case GST_TIME_OVERLAY_TIME_LINE_STREAM_TIME:
        ts = gst_segment_to_stream_time (segment, GST_FORMAT_TIME, ts_buffer);
        break;
      case GST_TIME_OVERLAY_TIME_LINE_RUNNING_TIME:
        ts = gst_segment_to_running_time (segment, GST_FORMAT_TIME, ts_buffer);
        break;
      case GST_TIME_OVERLAY_TIME_LINE_BUFFER_TIME:
      default:
        ts = ts_buffer;
        break;
    }

    if (self->show_times_as_dates) {
      GDateTime *datetime;

      datetime =
          g_date_time_add_seconds (self->datetime_epoch,
          (gdouble) GST_BUFFER_TIMESTAMP (video_frame) / GST_SECOND);
      time_str = g_date_time_format (datetime, self->datetime_format);
      g_date_time_unref (datetime);
    } else {
      time_str = gst_time_overlay_render_time (GST_TIME_OVERLAY (overlay), ts);
    }
  }

  txt = g_strdup (overlay->default_text);

  if (txt != NULL && *txt != '\0') {
    ret = g_strdup_printf ("%s %s", txt, time_str);
  } else {
    ret = time_str;
    time_str = NULL;
  }

  g_free (txt);
  g_free (time_str);

  return ret;
}

static void
gst_time_overlay_finalize (GObject * gobject)
{
  GstTimeOverlay *self = GST_TIME_OVERLAY (gobject);

  g_date_time_unref (self->datetime_epoch);
  g_free (self->datetime_format);
  G_OBJECT_CLASS (parent_class)->finalize (gobject);
}

static void
gst_time_overlay_class_init (GstTimeOverlayClass * klass)
{
  GstElementClass *gstelement_class;
  GstBaseTextOverlayClass *gsttextoverlay_class;
  GObjectClass *gobject_class;

  gsttextoverlay_class = (GstBaseTextOverlayClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;

  gst_element_class_set_static_metadata (gstelement_class, "Time overlay",
      "Filter/Editor/Video",
      "Overlays buffer time stamps on a video stream",
      "Tim-Philipp Müller <tim@centricular.net>");

  gsttextoverlay_class->get_text = gst_time_overlay_get_text;

  gobject_class->finalize = gst_time_overlay_finalize;
  gobject_class->set_property = gst_time_overlay_set_property;
  gobject_class->get_property = gst_time_overlay_get_property;

  g_object_class_install_property (gobject_class, PROP_TIME_LINE,
      g_param_spec_enum ("time-mode", "Time Mode", "What time to show",
          GST_TYPE_TIME_OVERLAY_TIME_LINE, DEFAULT_TIME_LINE,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DATETIME_EPOCH,
      g_param_spec_boxed ("datetime-epoch", "Datetime Epoch",
          "When showing times as dates, the initial date from which time "
          "is counted, if not specified prime epoch is used (1900-01-01)",
          G_TYPE_DATE_TIME, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_DATETIME_FORMAT,
      g_param_spec_string ("datetime-format", "Datetime Format",
          "When showing times as dates, the format to render date and time in",
          DEFAULT_DATETIME_FORMAT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SHOW_TIMES_AS_DATES,
      g_param_spec_boolean ("show-times-as-dates", "Show times as dates",
          "Whether to display times, counted from datetime-epoch, as dates",
          DEFAULT_SHOW_TIMES_AS_DATES,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_type_mark_as_plugin_api (GST_TYPE_TIME_OVERLAY_TIME_LINE, 0);
}

static void
gst_time_overlay_init (GstTimeOverlay * overlay)
{
  GstBaseTextOverlay *textoverlay;
  PangoContext *context;
  PangoFontDescription *font_description;

  textoverlay = GST_BASE_TEXT_OVERLAY (overlay);

  textoverlay->valign = GST_BASE_TEXT_OVERLAY_VALIGN_TOP;
  textoverlay->halign = GST_BASE_TEXT_OVERLAY_HALIGN_LEFT;

  overlay->time_line = DEFAULT_TIME_LINE;
  overlay->show_times_as_dates = DEFAULT_SHOW_TIMES_AS_DATES;
  overlay->datetime_epoch = g_date_time_new_utc (1900, 1, 1, 0, 0, 0);
  overlay->datetime_format = g_strdup (DEFAULT_DATETIME_FORMAT);

  context = textoverlay->pango_context;

  pango_context_set_language (context, pango_language_from_string ("en_US"));
  pango_context_set_base_dir (context, PANGO_DIRECTION_LTR);

  font_description = pango_font_description_new ();
  pango_font_description_set_family_static (font_description, "Monospace");
  pango_font_description_set_style (font_description, PANGO_STYLE_NORMAL);
  pango_font_description_set_variant (font_description, PANGO_VARIANT_NORMAL);
  pango_font_description_set_weight (font_description, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_stretch (font_description, PANGO_STRETCH_NORMAL);
  pango_font_description_set_size (font_description, 18 * PANGO_SCALE);
  pango_context_set_font_description (context, font_description);
  pango_font_description_free (font_description);
}

static void
gst_time_overlay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstTimeOverlay *overlay = GST_TIME_OVERLAY (object);

  switch (prop_id) {
    case PROP_TIME_LINE:
      g_atomic_int_set (&overlay->time_line, g_value_get_enum (value));
      break;
    case PROP_SHOW_TIMES_AS_DATES:
      overlay->show_times_as_dates = g_value_get_boolean (value);
      break;
    case PROP_DATETIME_EPOCH:
      g_date_time_unref (overlay->datetime_epoch);
      overlay->datetime_epoch = (GDateTime *) g_value_dup_boxed (value);
      break;
    case PROP_DATETIME_FORMAT:
      g_free (overlay->datetime_format);
      overlay->datetime_format = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_time_overlay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstTimeOverlay *overlay = GST_TIME_OVERLAY (object);

  switch (prop_id) {
    case PROP_TIME_LINE:
      g_value_set_enum (value, g_atomic_int_get (&overlay->time_line));
      break;
    case PROP_SHOW_TIMES_AS_DATES:
      g_value_set_boolean (value, overlay->show_times_as_dates);
      break;
    case PROP_DATETIME_EPOCH:
      g_value_set_boxed (value, overlay->datetime_epoch);
      break;
    case PROP_DATETIME_FORMAT:
      g_value_set_string (value, overlay->datetime_format);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
