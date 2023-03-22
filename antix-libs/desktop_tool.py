#!/usr/bin/env python3

import gtk
import os
import textwrap
import gobject
       
# For debug
def get_icon_info(icon_name, size = 48):
    icon_theme = gtk.icon_theme_get_default()
    try:
        flags = 0
        pixbuf = icon_theme.load_icon(icon_name, size, flags)
        icon_info = icon_theme.lookup_icon(icon_name, size, flags)
        sz = icon_info.get_base_size()
        fname = icon_info.get_filename()
        print("get_icon_info: icon_name: " + icon_name + " Filename: " + fname + " base size: %d" % sz)
    except gobject.GError as exc:
        print("get_icon_info: icon_name: " + icon_name + " not present in theme ") #, exc
        pass

# Return a pixbuf from icon name
def get_icon(name, size):
    """ Return a gtk.gdk.Pixbuf """
    found = False
    if name.startswith('/'):
        try:
            # name is an abslolute pathname to a file
            pixbuf = gtk.gdk.pixbuf_new_from_file(name)
            pixbuf = pixbuf.scale_simple(size, size, gtk.gdk.INTERP_BILINEAR)
            found = True
        except:
            # print "get_icon: Cannot get pixbuf from file: " + name
            name = os.basename(name).splitext()[0]
    
    if not found:
        theme = gtk.icon_theme_get_default()
        try:
            pixbuf = theme.load_icon(name, size, gtk.ICON_LOOKUP_USE_BUILTIN)
        except:
            try:
                filename = os.path.join('/usr/share/pixmaps', name)
                try:
                    pixbuf = gtk.gdk.pixbuf_new_from_file(filename)
                    found = True
                except:
                    # print "get_icon: [%s] not present in theme" % name
                    pixbuf = theme.load_icon('applications-system', size, gtk.ICON_LOOKUP_USE_BUILTIN)
            except:
                pixbuf = theme.load_icon(gtk.STOCK_PREFERENCES, 16, 0)
    # print "get_icon(name, size): icon name: %s pixbuf size: %dx%d" % (name, pixbuf.get_width(), pixbuf.get_height())
    if pixbuf.get_width() < 48 or pixbuf.get_height() < 48:
        pixbuf = pixbuf.scale_simple(size, size, gtk.gdk.INTERP_BILINEAR)
    if pixbuf.get_width() > 48 or pixbuf.get_height() > 48:
        pixbuf = pixbuf.scale_simple(size, size, gtk.gdk.INTERP_BILINEAR)
    return pixbuf


# My widget: icon and label wrapped in a EventBox
class DesktopToolWidget(gtk.EventBox):
    
    def __init__(self, label, icon_name, icon_size=48, 
                 orientation = gtk.ORIENTATION_VERTICAL, border=4,
                 wrap = 0):
        
        self.callback = None
        
        gtk.EventBox.__init__(self)
        self.set_border_width(0)

        if orientation == gtk.ORIENTATION_VERTICAL:
            # Label under Image
            box = gtk.VBox(False, border)
            box.set_border_width(int(border/2))
            box.set_spacing(border*2)
            box.show()
        else:
            # Label right side of Image
            box = gtk.HBox(False, border)
            box.set_border_width(int(border/2))
            box.set_spacing(border*2)
            box.show()

        btn_image = gtk.Image()
        # get_icon_info(icon_name, icon_size)
        pixbuf = get_icon(icon_name, icon_size)
        btn_image.set_from_pixbuf(pixbuf)
            
        btn_image.show()
        btn_image.set_size_request(icon_size, icon_size)
        box.pack_start(btn_image, False, False, 5)
        
        if label is not None:
            self.btn_label = gtk.Label()
            self.btn_label.set_use_markup(True)
            self.btn_label.set_line_wrap(True)
            if '&' in label:
                label = label.replace('&', '&amp;')
            label = label.replace("\\n", '\n')
            if wrap:
                label = textwrap.fill(label, wrap)
            self.btn_label.set_markup(label)
                
            if orientation == gtk.ORIENTATION_VERTICAL:
                self.btn_label.set_alignment(0.5, 0)
                self.btn_label.set_justify(gtk.JUSTIFY_CENTER)            
                #align = gtk.Alignment(1, 0, 0.5, 0.5)
                #align.show()
                #align.add(self.btn_label)
                box.pack_start(self.btn_label, False, False)
            else:
                self.btn_label.set_alignment(0.2, 1)
                align = gtk.Alignment(1, 0, 0.5, 0.5)
                align.show()
                align.add(self.btn_label)
                box.pack_start(align, False, False)

        # Manage click
        self.connect("button-press-event", self.on_button_press)
        # Manage selection
        self.connect("enter-notify-event", self.on_enter_notify)
        self.connect("leave-notify-event", self.on_leave_notify)
        
        self.add(box)
        self.show_all()

    def on_button_press(self, w, event):
        print("on_button_press button 1 pressed: no callback")
        if (event.type == gtk.gdk.BUTTON_PRESS  and  event.button == 1):
            print(w) 
            if not w.callback:
                print("button 1 pressed: no callback")
            else:
                print("*** call me ***")
                w.callback(self.data)
                
    def on_enter_notify(self, w, event):
        # Change background of the EventBox
        self.selected_bg = self.style.bg[gtk.STATE_SELECTED]
        self.modify_bg(gtk.STATE_NORMAL, self.selected_bg)
        # Change foreground of the Label
        self.selected_fg = self.btn_label.style.fg[gtk.STATE_SELECTED]
        self.btn_label.modify_fg(gtk.STATE_NORMAL, self.selected_fg)
   
    def on_leave_notify(self, w, event):
        # Restore defaults
        self.modify_bg(gtk.STATE_NORMAL, None)
        self.btn_label.modify_fg(gtk.STATE_NORMAL, None)
   
    def set_callback(self, callback, w, data):
        # print "set_callback: data: " + data
        self.widget = w
        self.callback = callback
        self.data = data

# Test me
if __name__ == "__main__":

    class Test:
        Value = 5
        def do_something(self, args):
            print("do_something: args: " + args + " value: %d" % self.Value)

    window = gtk.Window()
    window.set_title("DesktopToolWidget Test");
    window.set_border_width(10);

    vbox = gtk.VBox()
    vbox.set_spacing(8)

    icon_size = 48

    # Button 1
    icon_button1 = DesktopToolWidget("<b>Hello</b>\nIconButton", 'vlc', icon_size)
    icon_button1.set_size_request(150, 150)
    vbox.pack_start(icon_button1)
    # Attach a callback
    test = Test()
    icon_button1.set_callback(test.do_something, icon_button1, "Hello")
    
    # Button 2
    label = "<b>Hello</b>\nIconButton Horizontal"
    icon_button2 = DesktopToolWidget(label, 'synaptic', icon_size, gtk.ORIENTATION_HORIZONTAL)
    icon_button2.set_size_request(200, 100)
    vbox.pack_start(icon_button2)

    # Button 3
    label = "<b>Hello</b>This is a very long label a very long label a very long label"
    icon_button3 = DesktopToolWidget(label, 'synaptic', icon_size, gtk.ORIENTATION_HORIZONTAL, wrap = 30)
    icon_button3.set_size_request(200, 100)
    vbox.pack_start(icon_button3)

    window.add(vbox)
    vbox.show()
    window.show()
    gtk.main()
