#include <GL/gl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtkgl/gtkglarea.h>
#include <stdio.h>
#include <math.h>
#include "game.h"
#include "gfx.h"

/* size of our main window */
#define WIDTH 800
#define HEIGHT 600

extern float rotx, roty, rotz;

static int button_press = 1, button_release = 0;
static int keep_going = 1;

void nlog(char *str, ...)
{
	va_list ap;

	va_start(ap, str);
	vfprintf(stderr, str, ap);
	va_end(ap);
	// TODO: log to a file or another (debug) window
}

void gl_destroy(void)
{
	keep_going = 0;
	gtk_main_quit();
}

static gboolean on_delete(GtkWindow *win __attribute__((unused)))
{
	// TODO: gtk_gl_area_make_current(glarea);
	gfx_kill();
	return TRUE;
}

static void on_realize(GtkGLArea *glarea)
{
	gtk_gl_area_make_current(glarea);

	nlog("OpenGL version %s (%s)\n",
			glGetString(GL_VERSION), glGetString(GL_RENDERER));
}

static gboolean on_expose(GtkWidget *widget, GdkEventExpose *event)
{
	GtkGLArea *glarea = GTK_GL_AREA(widget);
	if (event->count > 0)
		return TRUE;
	gtk_gl_area_make_current(glarea);
	game();
	gtk_gl_area_swapbuffers(glarea);
	return TRUE;
}

static void on_resize(GtkGLArea *glarea __attribute__((unused)),
		GdkEventConfigure *event,
		gpointer data __attribute__((unused)))
{
	nlog("resize %d,%d", event->width, event->height);
	// TODO: send update to gfx_reshape or game_reshape
	glViewport(0, 0, event->width, event->height);
}

static void on_scroll(GtkGLArea *glarea __attribute__((unused)),
		GdkEventScroll *event)
{
	nlog("scroll %s\n", event->direction == GDK_SCROLL_UP ? "up"
			: event->direction == GDK_SCROLL_DOWN ? "down"
			: "don't know");
}

static void on_button(GtkGLArea *glarea __attribute__((unused)),
		GdkEventButton *event, gpointer data)
{
	nlog("button %d %s\n", event->button,
			data == &button_press ? "pressed"
			: data == &button_release ? "released"
			: "don't know");
}

static void on_motion(GtkGLArea *glarea __attribute__((unused)),
		GdkEventMotion *event)
{
	nlog("motion %d,%d\n", event->x, event->y);
}

static gboolean on_key(GtkWidget *widget __attribute__((unused)),
		GdkEventKey *event, gpointer data __attribute__((unused)))
{
	if (event->keyval == GDK_KEY_Escape) {
		gtk_main_quit();
		return TRUE;
	}
	return FALSE;
}

int main(int argc, char **argv)
{
	if (!gtk_init_check(&argc, &argv)) {
		puts("Could not initialize GTK");
		return 1;
	}

	int attrlist[] = {
		GDK_GL_RGBA,
		GDK_GL_RED_SIZE, 1,
		GDK_GL_GREEN_SIZE, 1,
		GDK_GL_BLUE_SIZE, 1,
		GDK_GL_DEPTH_SIZE, 1,
		GDK_GL_DOUBLEBUFFER,
		GDK_GL_NONE
	};

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *glarea = gtk_gl_area_new(attrlist);

	gtk_container_add(GTK_CONTAINER(window), glarea);
	gtk_window_set_title(GTK_WINDOW(window), "gl");
	gtk_window_set_default_size(GTK_WINDOW(window), WIDTH, HEIGHT);
	// gtk_window_set_resizable (GTK_WINDOW(window), FALSE);
	gtk_quit_add_destroy(1, GTK_OBJECT(window));
	gtk_widget_set_usize(glarea, WIDTH, HEIGHT); /* minimum size */

	/* connect window signals */
	g_signal_connect(window, "delete-event", G_CALLBACK(on_delete), NULL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
	g_signal_connect(window, "key_press_event", G_CALLBACK(on_key), NULL);

	/* connect glarea signals */
	g_signal_connect(glarea, "realize", G_CALLBACK(on_realize), NULL);
	g_signal_connect(glarea, "expose_event", G_CALLBACK(on_expose), NULL);
	g_signal_connect(glarea, "configure-event", G_CALLBACK(on_resize), NULL);
	gtk_widget_add_events(glarea, GDK_SCROLL_MASK);
	g_signal_connect(glarea, "scroll-event", G_CALLBACK(on_scroll), NULL);
	gtk_widget_add_events(glarea, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	g_signal_connect(glarea, "button-press-event",
			G_CALLBACK(on_button), &button_press);
	g_signal_connect(glarea, "button-release-event",
			G_CALLBACK(on_button), &button_release);
	gtk_widget_add_events(glarea,
			GDK_POINTER_MOTION_MASK | GDK_BUTTON_MOTION_MASK);
	g_signal_connect(glarea, "motion-notify-event",
			G_CALLBACK(on_motion), NULL);

	/* initialize the app */
	gfx_init();
	game_init();

	/* show it */
	gtk_widget_show_all(window);

#if 1
	GTK_WIDGET_SET_FLAGS(glarea, GTK_CAN_FOCUS);
	gtk_widget_grab_focus(GTK_WIDGET(glarea));
#endif

	keep_going = 1;
	while (keep_going) {
		gtk_main_iteration_do(FALSE);
//		while (gtk_events_pending())
//			if (gtk_main_iteration())
//				goto out;
		gtk_widget_queue_draw(GTK_WIDGET(window));
	}

	game_kill();

	return 0;
}
