/* GStreamer
 * Copyright (C) 2016 Centricular Ltd.
 * Author: Arun Raghavan <arun@centricular.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#define GST_NET_API             /* empty */

/* We need the internal netclock estimation function to (re)run the code on
 * captured samples, plus its dependencies for the build to succeed. */
#include "../../libs/gst/net/gstntppacket.c"
#include "../../libs/gst/net/gstnettimepacket.c"
#include "../../libs/gst/net/gstnetclientclock.c"
#include "../../libs/gst/net/gstnetutils.c"

static gchar *input = NULL;
static gboolean debug = FALSE;
static gint rtt_limit = 0;

static GOptionEntry entries[] = {
  {"input", 'i', 0, G_OPTION_ARG_FILENAME, &input,
        "Clock reading file containing one local and remote time readings, one "
        "per line",
      "FILE"},
  {"rtt-limit", 'r', 0, G_OPTION_ARG_INT64, &rtt_limit,
      "Round trip time limit on packets (in ms)", "MSEC"},
  {"debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "Verbose debug output", NULL},
  {NULL,}
};

/* g_ascii_string_to_unsigned is available since 2.54. Get rid of this wrapper
 * when we bump the version in 1.18 */
#if !GLIB_CHECK_VERSION(2,54,0)
#define g_ascii_string_to_unsigned parse_ascii_string_to_unsigned
static gboolean
parse_ascii_string_to_unsigned (const gchar * str, guint base, guint64 min,
    guint64 max, guint64 * out_num, GError ** error)
{
  gchar *endptr = NULL;
  *out_num = g_ascii_strtoull (str, &endptr, base);
  if (errno)
    return FALSE;
  if (endptr == str)
    return FALSE;
  return TRUE;
}
#endif

static gboolean
parse_time_values (const gchar * line, GstClockTime * local_1,
    GstClockTime * remote_1, GstClockTime * remote_2, GstClockTime * local_2)
{
  gchar **split;
  gboolean ret = FALSE;

  if (!line)
    return FALSE;

  split = g_strsplit (line, " ", -1);

  if (g_strv_length (split) != 4)
    goto out;

  if (!g_ascii_string_to_unsigned (split[0], 10, 0, G_MAXUINT64, local_1, NULL))
    goto out;

  if (!g_ascii_string_to_unsigned (split[1], 10, 0, G_MAXUINT64, remote_1,
          NULL))
    goto out;

  if (!g_ascii_string_to_unsigned (split[2], 10, 0, G_MAXUINT64, remote_2,
          NULL))
    goto out;

  if (!g_ascii_string_to_unsigned (split[3], 10, 0, G_MAXUINT64, local_2, NULL))
    goto out;

  ret = TRUE;
out:
  g_strfreev (split);
  return ret;
}

int
main (int argc, char *argv[])
{
  GstNetClientInternalClock *clock;
  GstBus *bus;
  GIOChannel *channel;
  GIOStatus status;
  GError *error = NULL;
  GOptionContext *context;
  gchar *line;
  int ret = 1;

  context = g_option_context_new (NULL);
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gst_init_get_option_group ());

  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_print ("Failed to parse options: %s\n\n", error->message);
    g_error_free (error);
    return 1;
  }

  if (input) {
    if (!(channel = g_io_channel_new_file (input, "r", NULL))) {
      g_print ("Could not read input file: %s\n", input);
      return 1;
    }
  } else {
    if (!(channel = g_io_channel_unix_new (0))) {
      g_print ("Could not read stdin");
      return 1;
    }
  }

  clock = g_object_new (GST_TYPE_NET_CLIENT_INTERNAL_CLOCK, NULL);
  bus = gst_bus_new ();

  /* FIXME: Find a way to do this without touching the structure internals */
  if (rtt_limit)
    clock->roundtrip_limit = rtt_limit * GST_MSECOND;
  clock->busses = g_list_prepend (clock->busses, bus);

  while ((status = g_io_channel_read_line (channel, &line, NULL, NULL,
              &error)) == G_IO_STATUS_NORMAL) {
    GstClockTime local_1, remote_1, remote_2, local_2;
    GstMessage *message;

    if (!parse_time_values (line, &local_1, &remote_1, &remote_2, &local_2)) {
      g_print ("Failed to get local/remote time values from: %s\n", line);
      goto done;
    }

    if (debug)
      g_print ("%s", line);

    gst_net_client_internal_clock_observe_times (clock, local_1, remote_1,
        remote_2, local_2);

    g_free (line);

    if ((message = gst_bus_pop_filtered (bus, GST_MESSAGE_ELEMENT))) {
      const GstStructure *st;
      gchar *str;

      st = gst_message_get_structure (message);
      str = gst_structure_to_string (st);

      g_print ("%s\n", str);

      g_free (str);
      gst_message_unref (message);
    }
  }

  if (status == G_IO_CHANNEL_ERROR) {
    g_print ("Error reading file: %s\n", error->message);
    g_error_free (error);
    goto done;
  }

  g_io_channel_unref (channel);
  g_free (input);
  gst_object_unref (bus);

  ret = 0;

done:
  return ret;
}
