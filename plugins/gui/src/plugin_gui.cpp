#include "gui/plugin_gui.h"

#include "gui/content_manager/content_manager.h"
#include "gui/file_manager/file_manager.h"
#include "gui/file_status_manager/file_status_manager.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/gui_api/gui_api.h"
#include "gui/main_window/main_window.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/notifications/notification_manager.h"
#include "gui/plugin_management/plugin_relay.h"
#include "gui/python/python_context.h"
#include "gui/selection_relay/selection_relay.h"
#include "gui/settings/keybind_manager.h"
#include "gui/settings/settings_manager.h"
#include "gui/settings/settings_relay.h"
#include "gui/style/style.h"
#include "gui/thread_pool/thread_pool.h"
#include "gui/window_manager/window_manager.h"
#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QMetaType>
#include <QResource>
#include <QSettings>
#include <QString>
#include <gui/focus_logger/focus_logger.h>
#include <signal.h>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<PluginGui>();
    }

    SettingsManager* g_settings_manager             = nullptr;    // this relay MUST be initialized before everything else since other components need to connect() to it when initializing
    QSettings* g_settings                           = nullptr;
    QSettings* g_gui_state                          = nullptr;
    SettingsRelay* g_settings_relay                 = nullptr;
    KeybindManager* g_keybind_manager               = nullptr;
    WindowManager* g_window_manager                 = nullptr;
    NotificationManager* g_notification_manager     = nullptr;
    ContentManager* g_content_manager               = nullptr;
    std::unique_ptr<Netlist> g_netlist_owner        = nullptr;
    Netlist* g_netlist                              = nullptr;
    NetlistRelay* g_netlist_relay                   = nullptr;
    PluginRelay* g_plugin_relay                     = nullptr;
    SelectionRelay* g_selection_relay               = nullptr;
    FileStatusManager* g_file_status_manager        = nullptr;
    ThreadPool* g_thread_pool                       = nullptr;
    GraphContextManager* g_graph_context_manager    = nullptr;
    GuiApi* g_gui_api                               = nullptr;
    std::unique_ptr<PythonContext> g_python_context = nullptr;

    // NOTE
    // ORDER = LOGGER -> SETTINGS -> (STYLE / RELAYS / OTHER STUFF) -> MAINWINDOW (= EVERYTHING ELSE & DATA)
    // USE POINTERS FOR EVERYTHING ?

    static void handle_program_arguments(const ProgramArguments& args)
    {
        if (args.is_option_set("--input-file"))
        {
            auto file_name = std::filesystem::path(args.get_parameter("--input-file"));
            log_info("gui", "GUI started with file {}.", file_name.string());
            FileManager::get_instance()->open_file(QString::fromStdString(file_name.string()));
        }
    }

    static void cleanup()
    {
        delete g_settings_manager;
        delete g_settings;
        delete g_gui_state;
        delete g_keybind_manager;
        delete g_file_status_manager;
        delete g_graph_context_manager;
        delete g_netlist_relay;
        delete g_plugin_relay;
        delete g_selection_relay;
        delete g_settings_relay;
        delete g_notification_manager;
        //    delete g_window_manager;
    }

    static void m_cleanup(int sig)
    {
        if (sig == SIGINT)
        {
            log_info("gui", "Detected Ctrl+C in terminal");
            QApplication::exit(0);
        }
    }

    bool PluginGui::exec(ProgramArguments& args)
    {
        int argc;
        const char** argv;
        args.get_original_arguments(&argc, &argv);
        QApplication a(argc, const_cast<char**>(argv));
        FocusLogger focusLogger(&a);

        QObject::connect(&a, &QApplication::aboutToQuit, cleanup);

        QApplication::setApplicationName("HAL Qt Interface");
        QApplication::setOrganizationName("Chair for Embedded Security - Ruhr University Bochum");
        QApplication::setOrganizationDomain("emsec.rub.de");

// Non native dialogs does not work on macOS. Therefore do net set AA_DontUseNativeDialogs!
#ifdef __linux__
        a.setAttribute(Qt::AA_DontUseNativeDialogs, true);
#endif
        a.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

        QResource::registerResource("gui_resources.rcc");

        QFontDatabase::addApplicationFont(":/fonts/Cabin-Bold");
        QFontDatabase::addApplicationFont(":/fonts/Cabin-BoldItalic");
        QFontDatabase::addApplicationFont(":/fonts/Cabin-Italic");
        QFontDatabase::addApplicationFont(":/fonts/Cabin-Medium");
        QFontDatabase::addApplicationFont(":/fonts/Cabin-MediumItalic");
        QFontDatabase::addApplicationFont(":/fonts/Cabin-Regular");
        QFontDatabase::addApplicationFont(":/fonts/Cabin-SemiBold");
        QFontDatabase::addApplicationFont(":/fonts/Cabin-SemiBoldItalic");
        QFontDatabase::addApplicationFont(":/fonts/Hack-Bold");
        QFontDatabase::addApplicationFont(":/fonts/Hack-BoldItalic");
        QFontDatabase::addApplicationFont(":/fonts/Hack-Regular");
        QFontDatabase::addApplicationFont(":/fonts/Hack-RegularItalic");
        QFontDatabase::addApplicationFont(":/fonts/Hack-RegularOblique");
        QFontDatabase::addApplicationFont(":/fonts/iosevka-bold");
        QFontDatabase::addApplicationFont(":/fonts/iosevka-bolditalic");
        QFontDatabase::addApplicationFont(":/fonts/iosevka-italic");
        QFontDatabase::addApplicationFont(":/fonts/iosevka-oblique");
        QFontDatabase::addApplicationFont(":/fonts/iosevka-regular");
        QFontDatabase::addApplicationFont(":/fonts/Droid Sans Mono/DroidSansMono");
        QFontDatabase::addApplicationFont(":/fonts/Montserrat/Montserrat-Black");
        QFontDatabase::addApplicationFont(":/fonts/Source Code Pro/SourceCodePro-Black");

        // LOGGER HERE
        gate_library_manager::load_all();

        // TEST
        //    g_settings->setValue("stylesheet/base", ":/style/test base");
        //    g_settings->setValue("stylesheet/definitions", ":/style/test definitions2");
        //    a.setStyleSheet(style::get_stylesheet());

        //TEMPORARY CODE TO CHANGE BETWEEN THE 2 STYLESHEETS WITH SETTINGS (NOT FINAL)
        //this settingsobject is currently neccessary to read from the settings from here, because the g_settings are not yet initialized(?)
        QSettings tempsettings_to_read_from(QString::fromStdString((utils::get_user_config_directory() / "guisettings.ini").string()), QSettings::IniFormat);
        QString stylesheet_to_open = ":/style/darcula";    //default style

        if (tempsettings_to_read_from.value("main_style/theme", "") == "" || tempsettings_to_read_from.value("main_style/theme", "") == "darcula")
            stylesheet_to_open = ":/style/darcula";
        else if (tempsettings_to_read_from.value("main_style/theme", "") == "sunny")
            stylesheet_to_open = ":/style/sunny";

        QFile stylesheet(stylesheet_to_open);
        stylesheet.open(QFile::ReadOnly);
        a.setStyleSheet(QString(stylesheet.readAll()));
        stylesheet.close();
        //##############END OF TEMPORARY TESTING TO SWITCH BETWEEN STYLESHEETS

        style::debug_update();

        qRegisterMetaType<spdlog::level::level_enum>("spdlog::level::level_enum");

        g_settings_manager      = new SettingsManager();
        g_settings              = new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guisettings.ini").string()), QSettings::IniFormat);
        g_gui_state             = new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guistate.ini").string()), QSettings::IniFormat);
        g_netlist_relay         = new NetlistRelay();
        g_plugin_relay          = new PluginRelay();
        g_selection_relay       = new SelectionRelay();
        g_settings_relay        = new SettingsRelay();
        g_keybind_manager       = new KeybindManager();
        g_file_status_manager   = new FileStatusManager();
        g_graph_context_manager = new GraphContextManager();

        //g_window_manager       = new WindowManager();
        g_notification_manager = new NotificationManager();

        g_thread_pool = new ThreadPool();

        g_gui_api = new GuiApi();

        signal(SIGINT, m_cleanup);

        MainWindow w;
        handle_program_arguments(args);
        w.show();
        auto ret = a.exec();
        return ret;
    }

    std::string PluginGui::get_name() const
    {
        return std::string("hal_gui");
    }

    std::string PluginGui::get_version() const
    {
        return std::string("0.1");
    }

    void PluginGui::initialize_logging()
    {
        LogManager& l = LogManager::get_instance();
        l.add_channel("user", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        l.add_channel("gui", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        l.add_channel("python", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

    ProgramOptions PluginGui::get_cli_options() const
    {
        ProgramOptions description;

        description.add({"--gui", "-g"}, "start graphical user interface");

        return description;
    }

}    // namespace hal
