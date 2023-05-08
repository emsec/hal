#include "gui/plugin_gui.h"

#include "gui/content_manager/content_manager.h"
#include "gui/file_manager/file_manager.h"
#include "gui/file_status_manager/file_status_manager.h"
#include "gui/graph_widget/graph_context_manager.h"
#include "gui/gui_api/gui_api.h"
#include "gui/main_window/main_window.h"
#include "gui/netlist_relay/netlist_relay.h"
#include "gui/plugin_relay/plugin_relay.h"
#include "gui/plugin_relay/gui_plugin_manager.h"
#include "gui/python/python_context.h"
#include "gui/selection_relay/selection_relay.h"
#include "gui/user_action/user_action_manager.h"
#include "gui/settings/settings_items/settings_item_dropdown.h"
#include "gui/style/style.h"
#include "gui/graph_widget/layout_locker.h"
#include "gui/comment_system/comment_manager.h"

#include "hal_core/netlist/gate_library/gate_library_manager.h"
#include "hal_core/netlist/netlist.h"
#include "hal_core/plugin_system/plugin_manager.h"
#include "hal_core/utilities/log.h"
#include "hal_core/utilities/utils.h"
#include "hal_core/netlist/project_manager.h"
#include "hal_core/utilities/project_directory.h"

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
    void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        const char *file = context.file ? context.file : "";
        const char *function = context.function ? context.function : "";
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtInfoMsg:
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        }

        if (localMsg.startsWith("QFSFileEngine::open"))
            fprintf(stderr, "***break***\n");
    }

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
    GraphContextManager* gGraphContextManager     = nullptr;
    GuiApi* gGuiApi                               = nullptr;
    CommentManager* gCommentManager               = nullptr;
    PythonContext* gPythonContext                 = nullptr;

    static void handleProgramArguments(const ProgramArguments& args)
    {
        enum OpenArgs { None = 0, OpenProject = 1, DefaultImport = 2, ImportToProject = 3 } openArgs = None;
        ProjectManager* pm = ProjectManager::instance();
        std::filesystem::path fileName;
        ProjectDirectory projDir;
        QString gateLibraryPath;
        if (args.is_option_set("--project-dir"))
        {
            openArgs = (OpenArgs) (openArgs|OpenProject);
            projDir = ProjectDirectory(args.get_parameter("--project-dir"));
            log_info("gui", "GUI started with project {}.", projDir.string());
        }
        if (args.is_option_set("--import-netlist"))
        {
            openArgs = (OpenArgs) (openArgs|DefaultImport);
            fileName = std::filesystem::path(args.get_parameter("--import-netlist"));
            log_info("gui", "GUI started with file {}.", fileName.string());
            if (args.is_option_set("--gate-library"))
                gateLibraryPath = QString::fromStdString(args.get_parameter("--gate-library"));
        }
        switch (openArgs)
        {
        case None:
            return;
        case OpenProject:
            FileManager::get_instance()->openProject(QString::fromStdString(projDir.string()));
            break;
        case DefaultImport:
            projDir = ProjectDirectory(fileName);
            [[fallthrough]];
            // continue with Import
        case ImportToProject:
            if (!pm->create_project_directory(projDir.string()))
            {
                log_error("gui", "Cannot create project directory {}.", projDir.string());
                return;
            }
            else
            {
                std::filesystem::path lpath = pm->get_project_directory().get_default_filename(".log");
                LogManager::get_instance()->set_file_name(lpath);
                if (!FileManager::get_instance()->deprecatedOpenFile(QString::fromStdString(fileName.string()),gateLibraryPath))
                    log_error("gui", "Failed to open netlist '{}'.", fileName.string());
            }
            break;
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

        qInstallMessageHandler(myMessageOutput);

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

//        gate_library_manager::load_all();

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

        style::debugUpdate();

        qRegisterMetaType<u32>("u32");
        qRegisterMetaType<spdlog::level::level_enum>("spdlog::level::level_enum");

        mGSettings           = new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guisettings.ini").string()), QSettings::IniFormat);
        gGuiState            = new QSettings(QString::fromStdString((utils::get_user_config_directory() / "guistate.ini").string()), QSettings::IniFormat);
        gNetlistRelay        = new NetlistRelay();
        gPluginRelay         = new PluginRelay();
        gSelectionRelay      = new SelectionRelay();
        gFileStatusManager   = new FileStatusManager();
        gGraphContextManager = new GraphContextManager();

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

    std::string PluginGui::get_description() const
    {
        return std::string("GUI control for HAL app (can't be unloaded)");
    }

    void PluginGui::initialize_logging()
    {
        const char* gui_info_channel[] = {"user", "gui", "python", "UserStudy", nullptr };
        for (int i=0; gui_info_channel[i]; i++)
            LogManager::get_instance()->add_channel(gui_info_channel[i], {LogManager::create_stdout_sink(), LogManager::create_file_sink(), LogManager::create_gui_sink()}, "info");
    }

    ProgramOptions CliExtensionsGui::get_cli_options() const
    {
        ProgramOptions mDescription;

        mDescription.add({"--gui", "-g"}, "start graphical user interface");

        return mDescription;
    }

    void PluginGui::set_layout_locker(bool enable)
    {
        if (enable)
            mLayoutLockerList.append(new LayoutLocker);
        else
        {
            if (mLayoutLockerList.isEmpty()) return;
            LayoutLocker* ll = mLayoutLockerList.takeLast();
            delete ll;
        }
    }
}    // namespace hal
