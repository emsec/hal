#pragma once
#include <QDialog>

#include <vector>
#include "hal_core/netlist/gate.h"

class QDialogButtonBox;
class QPushButton;
class QTableView;
namespace hal {

    class GateSelectionDialog : public QDialog
    {
        Q_OBJECT
    public:
        GateSelectionDialog(QWidget* parent=nullptr);

        std::vector<Gate*> selectedGates() const;

    private Q_SLOTS:
        void handleSelectAll();
        void handleCurrentGuiSelection();
        void handleClearSelection();

    private:
        QDialogButtonBox* mButtonBox;
        QPushButton* mButAll;
        QPushButton* mButNone;
        QPushButton* mButSel;
        QTableView* mTableView;
    };

}
