#include "my_application.h"

#include <flutter_linux/flutter_linux.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#endif

#include "flutter/generated_plugin_registrant.h"
#include "src/sinosecu.h"

#include <iostream>
#include <memory>
#include <map>


static std::unique_ptr<Sinosecu> global_scanner_instance;

struct _MyApplication {
  GtkApplication parent_instance;
  char** dart_entrypoint_arguments;
};

G_DEFINE_TYPE(MyApplication, my_application, GTK_TYPE_APPLICATION)

// Method channel call handler
static void method_channel_call_handler(FlMethodChannel* channel,
                                        FlMethodCall* method_call,
                                        gpointer user_data) {
    const gchar* method_name = fl_method_call_get_name(method_call);
    FlValue* args = fl_method_call_get_args(method_call);

    if (strcmp(method_name, "initializeScanner") == 0) {
        if (!global_scanner_instance) {
            global_scanner_instance = std::make_unique<Sinosecu>();
        }
    }else {
        if (!global_scanner_instance) {
            std::cerr << "Linux side: global_scanner_instance is null for method " << method_name << std::endl;
            fl_method_call_respond(method_call, FL_METHOD_RESPONSE(fl_method_error_response_new("SCANNER_NOT_READY", "Scanner instance not available.", nullptr)), nullptr);
            return;
        }
    }
    FlMethodResponse* response = nullptr;

    if (strcmp(method_name, "initializeScanner") == 0{
        if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
            std::cerr << "Linux side: initializeScanner arguments are not a map." << std::endl;
            fl_method_call_respond(method_call, FL_METHOD_RESPONSE(fl_method_error_response_new("ARGUMENT_ERROR", "Expected map argument for initializeScanner", nullptr)), nullptr);
            return;
        }

        FlValue* user_id_value = fl_value_lookup_string(args, "userId");
        FlValue* n_type_value = fl_value_lookup_string(args, "nType");
        FlValue* sdk_dir_value = fl_value_lookup_string(args, "sdkDirectory");

        if (!user_id_value || fl_value_get_type(user_id_value) != FL_VALUE_TYPE_STRING ||
            !n_type_value || fl_value_get_type(n_type_value) != FL_VALUE_TYPE_INT ||
            !sdk_dir_value || fl_value_get_type(sdk_dir_value) != FL_VALUE_TYPE_STRING) {
            std::cerr << "Linux side: Missing or incorrect argument types for initializeScanner." << std::endl;
            response = FL_METHOD_RESPONSE(fl_method_error_response_new("ARGUMENT_ERROR", "Invalid arguments for initializeScanner", nullptr));
        } else {
            const char* userId_cstr = fl_value_get_string(user_id_value);
            int nType = fl_value_get_int(n_type_value);
            const char* sdkDirectory_cstr = fl_value_get_string(sdk_dir_value);

            std::cout << "Linux side: Calling initializeScanner with UserID: " << (userId_cstr ? userId_cstr : "NULL")
                      << ", nType: " << nType
                      << ", Directory: " << (sdkDirectory_cstr ? sdkDirectory_cstr : "NULL") << std::endl;

            int result = global_scanner_instance->initializeScanner(std::string(userId_cstr ? userId_cstr : ""), nType, std::string(sdkDirectory_cstr ? sdkDirectory_cstr : ""));
            response = FL_METHOD_RESPONSE(fl_method_success_response_new(fl_value_new_int(result)));
        }
    }

}

// Implements GApplication::activate.
static void my_application_activate(GApplication* application) {
  MyApplication* self = MY_APPLICATION(application);
  GtkWindow* window =
      GTK_WINDOW(gtk_application_window_new(GTK_APPLICATION(application)));

  gboolean use_header_bar = TRUE;
#ifdef GDK_WINDOWING_X11
  GdkScreen* screen = gtk_window_get_screen(window);
  if (GDK_IS_X11_SCREEN(screen)) {
    const gchar* wm_name = gdk_x11_screen_get_window_manager_name(screen);
    if (g_strcmp0(wm_name, "GNOME Shell") != 0) {
      use_header_bar = FALSE;
    }
  }
#endif
  if (use_header_bar) {
    GtkHeaderBar* header_bar = GTK_HEADER_BAR(gtk_header_bar_new());
    gtk_widget_show(GTK_WIDGET(header_bar));
    gtk_header_bar_set_title(header_bar, "Passport Scanner");
    gtk_header_bar_set_show_close_button(header_bar, TRUE);
    gtk_window_set_titlebar(window, GTK_WIDGET(header_bar));
  } else {
    gtk_window_set_title(window, "Passport Scanner");
  }

  gtk_window_set_default_size(window, 1280, 720);
  gtk_widget_show(GTK_WIDGET(window));

  g_autoptr(FlDartProject) project = fl_dart_project_new();
  fl_dart_project_set_dart_entrypoint_arguments(project, self->dart_entrypoint_arguments);

  FlView* view = fl_view_new(project);
  gtk_widget_show(GTK_WIDGET(view));
  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(view));

  fl_register_plugins(FL_PLUGIN_REGISTRY(view));

  // register Flutter plugins
    fl_register_plugins(FL_PLUGIN_REGISTRY(view));

    // Register custom platform channel
    FlEngine* engine = fl_view_get_engine(view);
    if (engine == nullptr) {
        std::cerr << "Linux side: Could not get Flutter engine." << std::endl;
        return;
    }

    g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
    if (codec == nullptr) {
        std::cerr << "Linux side: Could not create standard method codec." << std::endl;
        return;
    }

    FlBinaryMessenger* messenger = fl_engine_get_binary_messenger(engine);
    if (messenger == nullptr) {
        std::cerr << "Linux side: Could not get binary messenger from Flutter engine." << std::endl;
        return;
    }

    FlMethodChannel* channel = fl_method_channel_new(
            messenger,
            "sinosecu",
            FL_METHOD_CODEC(codec)
    );

    if (channel == nullptr) {
        std::cerr << "Linux side: Failed to create method channel." << std::endl;
        return;
    }

    // Update how we set the method call handler
    fl_method_channel_set_method_channel_call_handler(channel,
                                                      method_channel_call_handler,
                                              g_object_ref(self),
                                              g_object_unref);

    std::cout << "Linux side: SinoScanner platform channel registered successfully." << std::endl;

  gtk_widget_grab_focus(GTK_WIDGET(view));
}

// Implements GApplication::local_command_line.
static gboolean my_application_local_command_line(GApplication* application, gchar*** arguments, int* exit_status) {
  MyApplication* self = MY_APPLICATION(application);
  // Strip out the first argument as it is the binary name.
  self->dart_entrypoint_arguments = g_strdupv(*arguments + 1);

  g_autoptr(GError) error = nullptr;
  if (!g_application_register(application, nullptr, &error)) {
     g_warning("Failed to register: %s", error->message);
     *exit_status = 1;
     return TRUE;
  }

  g_application_activate(application);
  *exit_status = 0;

  return TRUE;
}

// Implements GApplication::startup.
static void my_application_startup(GApplication* application) {
  //MyApplication* self = MY_APPLICATION(object);

  // Perform any actions required at application startup.

  G_APPLICATION_CLASS(my_application_parent_class)->startup(application);
}

// Implements GApplication::shutdown.
static void my_application_shutdown(GApplication* application) {
  //MyApplication* self = MY_APPLICATION(object);

  // Perform any actions required at application shutdown.

  G_APPLICATION_CLASS(my_application_parent_class)->shutdown(application);
}

// Implements GObject::dispose.
static void my_application_dispose(GObject* object) {
  MyApplication* self = MY_APPLICATION(object);
  g_clear_pointer(&self->dart_entrypoint_arguments, g_strfreev);
  G_OBJECT_CLASS(my_application_parent_class)->dispose(object);
}

static void my_application_class_init(MyApplicationClass* klass) {
  G_APPLICATION_CLASS(klass)->activate = my_application_activate;
  G_APPLICATION_CLASS(klass)->local_command_line = my_application_local_command_line;
  G_APPLICATION_CLASS(klass)->startup = my_application_startup;
  G_APPLICATION_CLASS(klass)->shutdown = my_application_shutdown;
  G_OBJECT_CLASS(klass)->dispose = my_application_dispose;
}

static void my_application_init(MyApplication* self) {}

MyApplication* my_application_new() {
  // Set the program name to the application ID, which helps various systems
  // like GTK and desktop environments map this running application to its
  // corresponding .desktop file. This ensures better integration by allowing
  // the application to be recognized beyond its binary name.
  g_set_prgname(APPLICATION_ID);

  return MY_APPLICATION(g_object_new(my_application_get_type(),
                                     "application-id", APPLICATION_ID,
                                     "flags", G_APPLICATION_NON_UNIQUE,
                                     nullptr));
}
