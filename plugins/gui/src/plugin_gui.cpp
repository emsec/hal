#include "gui/plugin_gui.h"

#include "gui/content_manager/content_manager.h"
#include "gui/file_manager/file_manager.h"
#include "gui/file_status_manager/file_status_manager.h"
#include "gui/focus_logger/focus_logger.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/gui_api/gui_api.h"
#include "gui/main_window/main_window.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/plugin_relay/plugin_relay.h"
#include "gui/python/python_context.h"
#include "gui/selection_relay/selection_relay.h"
#include "gui/thread_pool/thread_pool.h"
#include "gui/user_action/user_action_manager.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/style/style.h"
#include "gui/style/style_manager.h"

#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"

#include <QDir>
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QMetaType>
#include <QResource>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QDebug>

#include <signal.h>

namespace hal
{
    extern std::unique_ptr<BasePluginInterface> create_plugin_instance()
    {
        return std::make_unique<PluginGui>();
    }

    QSettings* mGSettings                         = nullptr;
    QSettings* gGuiState                          = nullptr;
    ContentManager* gContentManager               = nullptr;
    std::shared_ptr<Netlist> gNetlistOwner        = nullptr;
    Netlist* gNetlist                             = nullptr;
    NetlistRelay* gNetlistRelay                   = nullptr;
    PluginRelay* gPluginRelay                     = nullptr;
    SelectionRelay* gSelectionRelay               = nullptr;
    FileStatusManager* gFileStatusManager         = nullptr;
    ThreadPool* gThreadPool                       = nullptr;
    GraphContextManager* gGraphContextManager     = nullptr;
    GuiApi* gGuiApi                               = nullptr;
    std::unique_ptr<PythonContext> gPythonContext = nullptr;

    static void handleProgramArguments(const ProgramArguments& args)
    {
        if (args.is_option_set("--input-file"))
        {
            auto fileName = std::filesystem::path(args.get_parameter("--input-file"));
            log_info("gui", "GUI started with file {}.", fileName.string());
            FileManager::get_instance()->openFile(QString::fromStdString(fileName.string()));
        }
    }

    static void cleanup()
    {
        delete mGSettings;
        delete gGuiState;
        delete gFileStatusManager;
        delete gGraphContextManager;
        delete gNetlistRelay;
        delete gPluginRelay;
        delete gSelectionRelay;
    }

    static void mCleanup(int sig)
    {
        if (sig == SIGINT)
        {
            log_info("gui", "Detected Ctrl+C in terminal");
            QApplication::exit(0);
        }
    }

    static void mCrashHandler(int sig)
    {
        log_info("gui", "Emergency dump of executed actions on signal {}", sig);
        UserActionManager::instance()->crashDump(sig);
        signal(sig,SIG_DFL);
        return;
    }

    bool PluginGui::exec(ProgramArguments& args)
    {
        int argc;
        const char** argv;
        args.get_original_arguments(&argc, &argv);
        QApplication a(argc, const_cast<char**>(argv));
        //FocusLogger focusLogger(&a);

        QObject::connect(&a, &QApplication::aboutToQuit, cleanup);

        QApplication::setApplicationName("HAL Qt Interface");
        QApplication::setOrganizationName("Chair for Embedded Security - Ruhr University Bochum");
        QApplication::setOrganizationDomain("emsec.rub.de");

// Non native dialogs do not work on macOS. Therefore do net set AA_DontUseNativeDialogs!
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

        gate_library_manager::load_all();

        //TEMPORARY CODE TO CHANGE BETWEEN THE 2 STYLESHEETS WITH SETTINGS (NOT FINAL)
        //this settingsobject is currently neccessary to read from the settings from here, because the mGSettings are not yet initialized(?)
        QString styleSheetToOpen;

        MainWindow::sSettingStyle = new SettingsItemDropdown(
            "Theme",
            "main_style/theme",
            MainWindow::StyleSheetOption::Dark,
            "Appearance:Style",
            "Specifies which theme should be used. Light style is designed to print screenshots and not recommended for regular use."
        );
        MainWindow::sSettingStyle->setValueNames<MainWindow::StyleSheetOption>();
        MainWindow::StyleSheetOption theme = static_cast<MainWindow::StyleSheetOption>(MainWindow::sSettingStyle->value().toInt());

        switch(theme)
        {
            case MainWindow::StyleSheetOption::Dark : styleSheetToOpen = ":/style/dark"; break;
            case MainWindow::StyleSheetOption::Light : styleSheetToOpen = ":/style/light"; break;

            default: styleSheetToOpen = ":/style/dark";
        }

        QFile stylesheet(styleSheetToOpen);
        stylesheet.open(QFile::ReadOnly);
        a.setStyleSheet(QString(stylesheet.readAll()));
        stylesheet.close();
        //##############END OF TEMPORARY TESTING TO SWITCH BETWEEN STYLESHEETS

        StyleManager::get_instance();
        style::debugUpdate();

        qRegisterMetaType<spdlog::level::level_enum>("spdlog::level::level_enum");

        mGSettings           = new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guisettings.ini").string()), QSettings::IniFormat);
        gGuiState            = new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guistate.ini").string()), QSettings::IniFormat);
        gNetlistRelay        = new NetlistRelay();
        gPluginRelay         = new PluginRelay();
        gSelectionRelay      = new SelectionRelay();
        gFileStatusManager   = new FileStatusManager();
        gGraphContextManager = new GraphContextManager();

        gThreadPool = new ThreadPool();

        gGuiApi = new GuiApi();

        const int handleSignals[] = { SIGTERM, SIGSEGV, SIGILL, SIGABRT, SIGFPE, 0 };
        for (int isignal=0; handleSignals[isignal]; isignal++)
            signal(handleSignals[isignal], mCrashHandler);
        signal(SIGINT, mCleanup);

        MainWindow w;
        handleProgramArguments(args);
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
        return std::string("2.0");
    }

    void PluginGui::initialize_logging()
    {
        LogManager& l = LogManager::get_instance();
        l.add_channel("user", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        l.add_channel("gui", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        l.add_channel("python", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
        l.add_channel("UserStudy", {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

    ProgramOptions PluginGui::get_cli_options() const
    {
        ProgramOptions mDescription;

        mDescription.add({"--gui", "-g"}, "start graphical user interface");

        return mDescription;
    }

}    // namespace hal
