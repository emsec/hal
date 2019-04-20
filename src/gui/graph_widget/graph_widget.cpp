#include "graph_widget/graph_widget.h"
#include "graph_widget/graph_layouter_manager.h"
#include "graph_widget/graph_layouter_selection_widget.h"
#include "graph_widget/graph_scene.h"
#include "gui_globals.h"
#include "gui_utility.h"
#include "toolbar/toolbar.h"
#include <QGraphicsScene>
#include <QOpenGLWidget>
#include <QtWidgets>
#include <qmath.h>

graph_widget::graph_widget(QWidget* parent) : content_widget("Graph", parent)
{
    m_stacked_widget = new QStackedWidget(this);
    m_content_layout->addWidget(m_stacked_widget);
    m_selection_widget = new graph_layouter_selection_widget(this, nullptr);
    m_selection_widget->show();
    m_stacked_widget->addWidget(m_selection_widget);
    m_view = new graph_graphics_view(this);
    m_view->setFrameStyle(QFrame::NoFrame);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);

    m_zoom_slider = new QSlider(this);
    m_zoom_slider->setMinimum(0);
    m_zoom_slider->setMaximum(500);
    //m_zoom_slider->setValue(250);

    //#ifndef QT_NO_OPENGL
    //    m_openGlButton->setEnabled(QGLFormat::hasOpenGL());
    //#else
    //m_openGlButton->setEnabled(false);
    //#endif

    //    QButtonGroup *pointerModeGroup = new QButtonGroup(this);
    //    pointerModeGroup->setExclusive(true);
    //    pointerModeGroup->addButton(m_select_mode_button);
    //    pointerModeGroup->addButton(m_drag_mode_button);

    QHBoxLayout* topLayout = new QHBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(m_view);
    topLayout->addWidget(m_zoom_slider);
    //m_content_layout.addLayout(topLayout);
    m_view_container = new QWidget(this);
    m_view_container->setLayout(topLayout);
    m_stacked_widget->addWidget(m_view_container);

    m_opengl_viewport = false;

    connect(m_zoom_slider, &QSlider::valueChanged, this, &graph_widget::setup_matrix);

    setup_matrix();
    //m_stacked_widget->setCurrentWidget(m_selection_widget);
}

void graph_widget::setup_toolbar(toolbar* toolbarp)
{
    Q_UNUSED(toolbarp)

    m_layout_button = new QToolButton(this);
    m_layout_button->setText("Layout");

    //    m_select_mode_button = new QToolButton;
    //    m_select_mode_button->setText("Select");
    //    m_select_mode_button->setCheckable(true);

    //    m_drag_mode_button = new QToolButton;
    //    m_drag_mode_button->setText("Drag");
    //    m_drag_mode_button->setCheckable(true);

    m_antialias_button = new QToolButton(this);
    m_antialias_button->setText("Antialiasing");
    m_antialias_button->setCheckable(true);

    //    m_opengl_button = new QToolButton(this);
    //    m_opengl_button->setText("OpenGL");
    //    m_opengl_button->setCheckable(true);

    //    if (m_view->dragMode() == QGraphicsView::RubberBandDrag)
    //    {
    //        m_select_mode_button->setChecked(true);
    //        m_drag_mode_button->setChecked(false);
    //    }
    //    else
    //    {
    //        m_select_mode_button->setChecked(false);
    //        m_drag_mode_button->setChecked(true);
    //    }

    m_antialias_button->setChecked(m_view->renderHints() & QPainter::Antialiasing);
    //    m_opengl_button->setChecked(m_opengl_viewport);

    //TODO HACK change parent
    //    QButtonGroup* pointerModeGroup = new QButtonGroup(m_select_mode_button);
    //    pointerModeGroup->setExclusive(true);
    //    pointerModeGroup->addButton(m_select_mode_button);
    //    pointerModeGroup->addButton(m_drag_mode_button);

    connect(m_layout_button, &QToolButton::clicked, this, &graph_widget::show_layout_selection);
    //    connect(m_select_mode_button, &QToolButton::toggled, this, &graph_widget::toggle_pointer_mode);
    //    connect(m_drag_mode_button, &QToolButton::toggled, this, &graph_widget::toggle_pointer_mode);
    connect(m_antialias_button, &QToolButton::toggled, this, &graph_widget::toggle_antialiasing);
    //    connect(m_opengl_button, &QToolButton::toggled, this, &graph_widget::toggle_opengl);

    toolbarp->addSeparator();
    toolbarp->addWidget(m_layout_button);
    toolbarp->addSeparator();
    toolbarp->addWidget(m_antialias_button);
    //    toolbarp->addWidget(m_opengl_button);
    toolbarp->addSeparator();

    m_layouter = "";
}

void graph_widget::subscribe(QString layouter)
{
    if (m_layouter == layouter)
        return;

    graph_scene* scene = graph_layouter_manager::get_instance().subscribe(layouter);

    if (scene)
        m_view->setScene(scene);

    if (m_layouter != "")
        graph_layouter_manager::get_instance().unsubscribe(m_layouter);

    m_layouter = layouter;
}

graph_graphics_view* graph_widget::view() const
{
    return m_view;
}

QString graph_widget::get_layouter()
{
    return m_layouter;
}

void graph_widget::show_view()
{
    m_stacked_widget->setCurrentWidget(m_view_container);
}

void graph_widget::setup_matrix()
{
    qreal scale = qPow(qreal(2), (m_zoom_slider->value() - 250) / qreal(50));

    QMatrix matrix;
    matrix.scale(scale, scale);
    m_view->setMatrix(matrix);
}

void graph_widget::show_layout_selection(bool checked)
{
    Q_UNUSED(checked)

    m_stacked_widget->setCurrentWidget(m_selection_widget);
}

//void graph_widget::toggle_pointer_mode()
//{
//    m_view->setDragMode(m_select_mode_button->isChecked() ? QGraphicsView::RubberBandDrag : QGraphicsView::ScrollHandDrag);
//    m_view->setInteractive(m_select_mode_button->isChecked());
//}

void graph_widget::toggle_opengl()
{
    if (m_opengl_button->isChecked())
    {
        QOpenGLWidget* view_port = new QOpenGLWidget();
        QSurfaceFormat format;
        format.setSamples(16);
        view_port->setFormat(format);
        m_view->setViewport(view_port);
        m_opengl_viewport = true;
    }
    m_view->setViewport(new QWidget());
    m_opengl_viewport = false;
}

void graph_widget::toggle_antialiasing()
{
    m_view->setRenderHint(QPainter::Antialiasing, m_antialias_button->isChecked());
}

void graph_widget::zoom_in(int level)
{
    m_zoom_slider->setValue(m_zoom_slider->value() + level);
}

void graph_widget::zoom_out(int level)
{
    m_zoom_slider->setValue(m_zoom_slider->value() - level);
}
