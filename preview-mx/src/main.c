//Original copywrite Dolphin Oracle 2023
//preview window, no other function

//usually called by mx-tweak

#include <gtk/gtk.h>
#include <libintl.h>
#include <locale.h>
#include <gdk/gdkkeysyms.h>

#define _(STRING) gettext(STRING)

GtkWidget *g_button_close;
GtkWidget *g_check_button_checkbox;
GtkWidget *g_textbox;


int main(int argc, char *argv[])
{
    GtkBuilder      *builder; 
    GtkWidget       *window;
    
    //Setting the i18n environment
    setlocale (LC_ALL, "");
    bindtextdomain ("preview-mx", "/usr/share/locale/");
    textdomain ("preview-mx");

    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("/usr/share/preview-mx/glade/preview.glade");
    //builder = gtk_builder_new_from_file("/home/dolphin/development/preview-mx/assets/glade/preview.glade");

    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);
    
    // get pointers to the gui pieces
    g_button_close = GTK_WIDGET(gtk_builder_get_object(builder, "button_close"));
    g_check_button_checkbox = GTK_WIDGET(gtk_builder_get_object(builder, "checkbox1"));
    g_textbox = GTK_WIDGET(gtk_builder_get_object(builder, "textbox1"));
    //translator comment: Close as in quit application
    gtk_button_set_label (GTK_BUTTON (g_button_close), _("Close"));
    //translator comment: Preview means to see before using.
    gtk_window_set_title (GTK_WINDOW (window), _("Preview"));
    //translator comment: Preview means to see before using.
    gtk_button_set_label (GTK_BUTTON (g_check_button_checkbox), _("Preview"));
    //translator comment: Preview means to see before using.
    gtk_entry_set_text (GTK_ENTRY (g_textbox), _("Preview"));

    g_object_unref(builder);

    gtk_widget_show(window);                
    gtk_main();

    return 0;
}

// called when window is closed
void on_window_main_destroy()
{
    gtk_main_quit();
}

//close button
void on_button_close_clicked()
{
    gtk_main_quit();
}

//on loss of focus
void on_focus_out(){
	gtk_main_quit();
}

