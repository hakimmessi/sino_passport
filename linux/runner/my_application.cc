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

// Helper function to create error response
static FlMethodResponse* create_error_response(const char* code, const char* message, const char* details = nullptr) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(code, message, details ? fl_value_new_string(details) : nullptr));
}

// Helper function to create success response
static FlMethodResponse* create_success_response(FlValue* result) {
    return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

// Helper function to validate scanner instance
static bool ensure_scanner_ready(FlMethodCall* method_call, const char* method_name) {
    if (!global_scanner_instance) {
        std::cerr << "Linux side: Scanner instance not available for method " << method_name << std::endl;
        fl_method_call_respond(method_call, create_error_response("SCANNER_NOT_READY", "Scanner instance not available"), nullptr);
        return false;
    }
    return true;
}

// Helper function to validate map arguments
static bool validate_map_args(FlValue* args, FlMethodCall* method_call, const char* method_name) {
    if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
        std::cerr << "Linux side: " << method_name << " arguments are not a map" << std::endl;
        fl_method_call_respond(method_call, create_error_response("ARGUMENT_ERROR", "Expected map argument"), nullptr);
        return false;
    }
    return true;
}

// Method channel call handler
static void method_channel_call_handler(FlMethodChannel* channel,
                                        FlMethodCall* method_call,
                                        gpointer user_data) {
    const gchar* method_name = fl_method_call_get_name(method_call);
    FlValue* args = fl_method_call_get_args(method_call);

    std::cout << "Linux side: Received method call: " << method_name << std::endl;

    FlMethodResponse* response = nullptr;

    try {
        // Handle initializeScanner - creates instance if needed
        if (strcmp(method_name, "initializeScanner") == 0) {
            if (!global_scanner_instance) {
                global_scanner_instance = std::make_unique<Sinosecu>();
                std::cout << "Linux side: Created new scanner instance" << std::endl;
            }

            if (!validate_map_args(args, method_call, method_name)) return;

            FlValue* user_id_value = fl_value_lookup_string(args, "userId");
            FlValue* n_type_value = fl_value_lookup_string(args, "nType");
            FlValue* sdk_dir_value = fl_value_lookup_string(args, "sdkDirectory");

            // Validate required parameters
            if (!user_id_value || fl_value_get_type(user_id_value) != FL_VALUE_TYPE_STRING) {
                response = create_error_response("ARGUMENT_ERROR", "Missing or invalid 'userId' parameter");
            }
            else if (!n_type_value || fl_value_get_type(n_type_value) != FL_VALUE_TYPE_INT) {
                response = create_error_response("ARGUMENT_ERROR", "Missing or invalid 'nType' parameter");
            }
            else if (!sdk_dir_value || fl_value_get_type(sdk_dir_value) != FL_VALUE_TYPE_STRING) {
                response = create_error_response("ARGUMENT_ERROR", "Missing or invalid 'sdkDirectory' parameter");
            }
            else {
                const char* userId_cstr = fl_value_get_string(user_id_value);
                int nType = fl_value_get_int(n_type_value);
                const char* sdkDirectory_cstr = fl_value_get_string(sdk_dir_value);

                std::cout << "Linux side: Initializing scanner with:" << std::endl;
                std::cout << "  UserID: " << userId_cstr << std::endl;
                std::cout << "  nType: " << nType << std::endl;
                std::cout << "  Directory: " << sdkDirectory_cstr << std::endl;

                int result = global_scanner_instance->initializeScanner(
                        std::string(userId_cstr), nType, std::string(sdkDirectory_cstr)
                );

                // Create detailed response
                FlValue* result_map = fl_value_new_map();
                fl_value_set_string_take(result_map, "result", fl_value_new_int(result));
                fl_value_set_string_take(result_map, "success", fl_value_new_bool(result == Sinosecu::SUCCESS));

                if (result == Sinosecu::SUCCESS) {
                    fl_value_set_string_take(result_map, "message", fl_value_new_string("Scanner initialized successfully"));

                    // Add device information if available
                    std::string serialNumber = global_scanner_instance->getDeviceSerialNumber();
                    std::string deviceModel = global_scanner_instance->getDeviceModel();

                    if (!serialNumber.empty()) {
                        fl_value_set_string_take(result_map, "serialNumber", fl_value_new_string(serialNumber.c_str()));
                    }
                    if (!deviceModel.empty()) {
                        fl_value_set_string_take(result_map, "deviceModel", fl_value_new_string(deviceModel.c_str()));
                    }
                } else {
                    std::string error_msg = global_scanner_instance->getLastError();
                    fl_value_set_string_take(result_map, "message", fl_value_new_string(error_msg.c_str()));
                    fl_value_set_string_take(result_map, "errorCode", fl_value_new_int(result));
                }

                response = create_success_response(result_map);
            }
        }

            // Handle checkDeviceStatus
        else if (strcmp(method_name, "checkDeviceStatus") == 0) {
            if (!ensure_scanner_ready(method_call, method_name)) return;

            int status = global_scanner_instance->checkDeviceStatus();

            FlValue* result_map = fl_value_new_map();
            fl_value_set_string_take(result_map, "status", fl_value_new_int(status));

            const char* status_message;
            switch (status) {
                case Sinosecu::DEVICE_CONNECTED:
                    status_message = "Device connected and ready";
                    break;
                case Sinosecu::DEVICE_DISCONNECTED:
                    status_message = "Device disconnected";
                    break;
                case Sinosecu::DEVICE_NEEDS_REINIT:
                    status_message = "Device needs reinitialization";
                    break;
                default:
                    status_message = "Unknown device status";
                    break;
            }

            fl_value_set_string_take(result_map, "message", fl_value_new_string(status_message));
            response = create_success_response(result_map);
        }

            // Handle detectDocument
        else if (strcmp(method_name, "detectDocument") == 0) {
            if (!ensure_scanner_ready(method_call, method_name)) return;

            int detection_result = global_scanner_instance->detectDocument();

            FlValue* result_map = fl_value_new_map();
            fl_value_set_string_take(result_map, "detectionResult", fl_value_new_int(detection_result));

            const char* detection_message;
            bool document_present = false;

            switch (detection_result) {
                case Sinosecu::DOC_NOT_DETECTED:
                    detection_message = "No document detected";
                    break;
                case Sinosecu::DOC_PLACED:
                    detection_message = "Document placed on scanner";
                    document_present = true;
                    break;
                case Sinosecu::DOC_REMOVED:
                    detection_message = "Document removed from scanner";
                    break;
                case Sinosecu::PHONE_BARCODE_DETECTED:
                    detection_message = "Mobile phone barcode detected";
                    document_present = true;
                    break;
                default:
                    detection_message = "Unknown detection result";
                    break;
            }

            fl_value_set_string_take(result_map, "message", fl_value_new_string(detection_message));
            fl_value_set_string_take(result_map, "documentPresent", fl_value_new_bool(document_present));

            response = create_success_response(result_map);
        }

            // Handle processDocument
        else if (strcmp(method_name, "processDocument") == 0) {
            if (!ensure_scanner_ready(method_call, method_name)) return;

            int process_result = global_scanner_instance->processDocument();

            FlValue* result_map = fl_value_new_map();
            fl_value_set_string_take(result_map, "processResult", fl_value_new_int(process_result));
            fl_value_set_string_take(result_map, "success", fl_value_new_bool(process_result > 0));

            if (process_result > 0) {
                // Get REAL data from extract()
                std::string realPassportNumber = global_scanner_instance->extract();

                FlValue* passport_map = fl_value_new_map();
                fl_value_set_string_take(passport_map, "passportNumber", fl_value_new_string(realPassportNumber.c_str()));

                fl_value_set_string_take(result_map, "passportData", passport_map);
                fl_value_set_string_take(result_map, "message", fl_value_new_string("Document processed successfully"));
            } else {
                std::string error_msg = global_scanner_instance->getLastError();
                fl_value_set_string_take(result_map, "message", fl_value_new_string(error_msg.c_str()));
            }

            response = create_success_response(result_map);
        }

            // Handle getDeviceInfo
        else if (strcmp(method_name, "getDeviceInfo") == 0) {
            if (!ensure_scanner_ready(method_call, method_name)) return;

            FlValue* info_map = fl_value_new_map();

            std::string serialNumber = global_scanner_instance->getDeviceSerialNumber();
            std::string deviceModel = global_scanner_instance->getDeviceModel();
            bool is_ready = global_scanner_instance->isReady();

            fl_value_set_string_take(info_map, "serialNumber", fl_value_new_string(serialNumber.c_str()));
            fl_value_set_string_take(info_map, "deviceModel", fl_value_new_string(deviceModel.c_str()));
            fl_value_set_string_take(info_map, "isReady", fl_value_new_bool(is_ready));

            response = create_success_response(info_map);
        }

            // Handle playBuzzer
        else if (strcmp(method_name, "playBuzzer") == 0) {
            if (!ensure_scanner_ready(method_call, method_name)) return;

            int duration = 100; // Default duration

            if (args && fl_value_get_type(args) == FL_VALUE_TYPE_MAP) {
                FlValue* duration_value = fl_value_lookup_string(args, "duration");
                if (duration_value && fl_value_get_type(duration_value) == FL_VALUE_TYPE_INT) {
                    duration = fl_value_get_int(duration_value);
                }
            }

            int result = global_scanner_instance->playBuzzer(duration);

            FlValue* result_map = fl_value_new_map();
            fl_value_set_string_take(result_map, "result", fl_value_new_int(result));
            fl_value_set_string_take(result_map, "success", fl_value_new_bool(result == Sinosecu::SUCCESS));

            response = create_success_response(result_map);
        }

            // Handle releaseScanner
        else if (strcmp(method_name, "releaseScanner") == 0) {
            if (global_scanner_instance) {
                global_scanner_instance->releaseScanner();
                global_scanner_instance.reset();
                std::cout << "Linux side: Scanner instance released" << std::endl;
            }

            FlValue* result_map = fl_value_new_map();
            fl_value_set_string_take(result_map, "success", fl_value_new_bool(true));
            fl_value_set_string_take(result_map, "message", fl_value_new_string("Scanner released successfully"));

            response = create_success_response(result_map);
        }

            // Handle unknown methods
        else {
            std::cerr << "Linux side: Unknown method called: " << method_name << std::endl;
            response = create_error_response("METHOD_NOT_FOUND", "Unknown method", method_name);
        }

    } catch (const std::exception& e) {
        std::cerr << "Linux side: Exception in method " << method_name << ": " << e.what() << std::endl;
        response = create_error_response("NATIVE_EXCEPTION", "Native code exception", e.what());
    } catch (...) {
        std::cerr << "Linux side: Unknown exception in method " << method_name << std::endl;
        response = create_error_response("UNKNOWN_EXCEPTION", "Unknown native exception");
    }

    // Always respond to the method call
    if (response) {
        fl_method_call_respond(method_call, response, nullptr);
    } else {
        // Fallback response if none was created
        fl_method_call_respond(method_call, create_error_response("INTERNAL_ERROR", "No response generated"), nullptr);
    }

    std::cout << "Linux side: Method call " << method_name << " completed" << std::endl;
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



    fl_method_channel_set_method_call_handler(channel,
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

