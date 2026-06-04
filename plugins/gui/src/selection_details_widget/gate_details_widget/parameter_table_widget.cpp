#include "gui/selection_details_widget/gate_details_widget/parameter_table_widget.h"

#include "hal_core/netlist/gate.h"

#include <QHeaderView>

namespace hal
{
    ParameterTableWidget::ParameterTableWidget(QWidget* parent) : QTableView(parent), mParameterTableModel(new ParameterTableModel(this))
    {
        setModel(mParameterTableModel);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setFocusPolicy(Qt::NoFocus);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        verticalHeader()->setVisible(false);
        horizontalHeader()->setVisible(true);
        horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        horizontalHeader()->setStretchLastSection(true);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setFrameStyle(QFrame::NoFrame);
    }

    void ParameterTableWidget::setGate(Gate* gate)
    {
        if (gate == nullptr)
            return;

        mParameterTableModel->updateData(gate->get_parameters());
        clearSelection();
    }
}

