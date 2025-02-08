#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <array>
#include <gtk/gtk.h>

// Logging functions
void log_action(const std::string& action) {
    std::ofstream log_file("bluetooth_manager.log", std::ios_base::app);
    log_file << action << std::endl;
}

void log_error(const std::string& error) {
    std::ofstream log_file("bluetooth_manager.log", std::ios_base::app);
    log_file << error << std::endl;
}

// Function to execute shell commands and get output
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// Function to check if a service is running
bool is_service_running(const std::string& service_name) {
    std::string command = "service " + service_name + " status";
    std::string output = exec(command.c_str());
    return output.find("is running") != std::string::npos;
}

// Callback functions for GTK buttons
void scan_devices(GtkWidget* widget, gpointer data);
void connect_device(GtkWidget* widget, gpointer data);

// Class to manage Bluetooth devices using GTK
class BluetoothManager {
public:
    BluetoothManager();
    void run();

private:
    GtkWidget* window;
    GtkWidget* listbox;
    GtkWidget* scan_button;
    GtkWidget* connect_button;

    void setup_ui();
    void scan_devices();
    void connect_device();
};

BluetoothManager::BluetoothManager() {
    setup_ui();
}

void BluetoothManager::setup_ui() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Bluetooth Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    GtkWidget* vbox = gtk_vbox_new(FALSE, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    scan_button = gtk_button_new_with_label("Scan for Devices");
    g_signal_connect(scan_button, "clicked", G_CALLBACK(scan_devices), this);
    gtk_box_pack_start(GTK_BOX(vbox), scan_button, FALSE, FALSE, 0);

    listbox = gtk_list_box_new();
    gtk_box_pack_start(GTK_BOX(vbox), listbox, TRUE, TRUE, 0);

    connect_button = gtk_button_new_with_label("Connect");
    g_signal_connect(connect_button, "clicked", G_CALLBACK(connect_device), this);
    gtk_box_pack_start(GTK_BOX(vbox), connect_button, FALSE, FALSE, 0);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}

void BluetoothManager::run() {
    gtk_widget_show_all(window);
    gtk_main();
}

void BluetoothManager::scan_devices() {
    gtk_list_box_invalidate_filter(GTK_LIST_BOX(listbox));
    try {
        if (!is_service_running("hcsecd")) {
            std::string command = "sudo service hcsecd onestart";
            std::string output = exec(command.c_str());
            if (output.find("failed") != std::string::npos) {
                throw std::runtime_error(output);
            }
            log_action("Started hcsecd service");
        } else {
            log_action("hcsecd service already running");
        }

        std::string command = "sudo service bluetooth onestart ubt0";
        std::string output = exec(command.c_str());
        if (output.find("failed") != std::string::npos) {
            throw std::runtime_error(output);
        }
        log_action("Bluetooth service started");

        command = "sudo hccontrol -n ubt0hci inquiry";
        output = exec(command.c_str());
        std::istringstream iss(output);
        std::string line;
        while (std::getline(iss, line)) {
            GtkWidget* row = gtk_label_new(line.c_str());
            gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);
        }
        log_action("Scanned for Bluetooth devices");
    } catch (const std::exception& e) {
        std::string error_message = "Failed to scan devices: " + std::string(e.what());
        log_error(error_message);
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", error_message.c_str());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void BluetoothManager::connect_device() {
    GtkListBoxRow* row = gtk_list_box_get_selected_row(GTK_LIST_BOX(listbox));
    if (row) {
        GtkWidget* label = gtk_bin_get_child(GTK_BIN(row));
        const char* device = gtk_label_get_text(GTK_LABEL(label));
        try {
            std::string command = "sudo hccontrol -n ubt0hci create_connection " + std::string(device);
            std::string output = exec(command.c_str());
            GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Connected to %s", device);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            log_action("Connected to " + std::string(device));
        } catch (const std::exception& e) {
            std::string error_message = "Failed to connect to device: " + std::string(e.what());
            log_error(error_message);
            GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", error_message.c_str());
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    } else {
        GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "No device selected");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        log_action("No device selected for connection");
    }
}

void scan_devices(GtkWidget* widget, gpointer data) {
    BluetoothManager* manager = static_cast<BluetoothManager*>(data);
    manager->scan_devices();
}

void connect_device(GtkWidget* widget, gpointer data) {
    BluetoothManager* manager = static_cast<BluetoothManager*>(data);
    manager->connect_device();
}

int main(int argc, char* argv[]) {
    log_action("Bluetooth Manager started");
    gtk_init(&argc, &argv);
    BluetoothManager manager;
    manager.run();
    return 0;
}