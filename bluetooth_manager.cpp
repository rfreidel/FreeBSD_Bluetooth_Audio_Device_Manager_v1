#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <gtk/gtk.h>

// Configure logging to user-level log file
std::ofstream log_file("bluetooth_manager.log", std::ios_base::app | std::ios_base::out);

void log_action(const std::string &action) {
    log_file << action << std::endl;
}

void log_error(const std::string &error) {
    log_file << "ERROR: " << error << std::endl;
}

bool is_service_running(const std::string &service_name) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(("service " + service_name + " status").c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result.find("is running") != std::string::npos;
}

std::vector<std::string> execute_command(const std::string &command) {
    std::array<char, 128> buffer;
    std::string result;
    std::vector<std::string> output;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        output.push_back(buffer.data());
    }
    return output;
}

static void scan_devices(GtkWidget *widget, gpointer data) {
    GtkListStore *list_store = GTK_LIST_STORE(data);
    gtk_list_store_clear(list_store);

    try {
        // Check if hcsecd service is running
        if (!is_service_running("hcsecd")) {
            auto result = execute_command("service hcsecd onestart");
            log_action("Started hcsecd service");
        } else {
            log_action("hcsecd service already running");
        }

        // Start Bluetooth service
        auto bluetooth_result = execute_command("service bluetooth onestart ubt0");
        log_action("Bluetooth service started");

        // List Bluetooth devices
        auto devices = execute_command("hccontrol -n ubt0hci inquiry");
        for (const auto &device : devices) {
            GtkTreeIter iter;
            gtk_list_store_append(list_store, &iter);
            gtk_list_store_set(list_store, &iter, 0, device.c_str(), -1);
        }
        log_action("Scanned for Bluetooth devices");
    } catch (const std::exception &e) {
        std::string error_message = std::string("Failed to scan devices: ") + e.what();
        std::cerr << error_message << std::endl;
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", error_message.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        log_error(error_message);
    }
}

static void connect_device(GtkWidget *widget, gpointer data) {
    GtkTreeView *tree_view = GTK_TREE_VIEW(data);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(tree_view);
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gchar *device;
        gtk_tree_model_get(model, &iter, 0, &device, -1);

        try {
            std::string command = "hccontrol -n ubt0hci create_connection " + std::string(device);
            auto result = execute_command(command);
            GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Connected to %s", device);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            log_action("Connected to " + std::string(device));
        } catch (const std::exception &e) {
            std::string error_message = std::string("Failed to connect to device: ") + e.what();
            std::cerr << error_message << std::endl;
            GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, "%s", error_message.c_str());
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            log_error(error_message);
        }

        g_free(device);
    } else {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE, "No device selected");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        log_action("No device selected for connection");
    }
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    log_action("Bluetooth Manager started");

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Bluetooth Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *scan_button = gtk_button_new_with_label("Scan for Devices");
    gtk_box_pack_start(GTK_BOX(vbox), scan_button, FALSE, FALSE, 5);

    GtkListStore *list_store = gtk_list_store_new(1, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Devices", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    gtk_box_pack_start(GTK_BOX(vbox), tree_view, TRUE, TRUE, 5);

    GtkWidget *connect_button = gtk_button_new_with_label("Connect");
    gtk_box_pack_start(GTK_BOX(vbox), connect_button, FALSE, FALSE, 5);

    g_signal_connect(scan_button, "clicked", G_CALLBACK(scan_devices), list_store);
    g_signal_connect(connect_button, "clicked", G_CALLBACK(connect_device), tree_view);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
