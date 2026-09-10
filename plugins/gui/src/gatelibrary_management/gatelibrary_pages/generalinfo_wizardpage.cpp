#include "gui/gatelibrary_management/gatelibrary_pages/generalinfo_wizardpage.h"

#include "gui/gatelibrary_management/gatelibrary_wizard.h"
#include "gui/gui_utils/graphics.h"

namespace hal
{
    ListPropertyModel::ListPropertyModel(QObject *parent)
        : QAbstractTableModel(parent)
    {
        for (int i=(int)GateTypeProperty::combinational; i<=(int)GateTypeProperty::c_lut; i++)
            mList.append({(GateTypeProperty)i,false});
    }

    QVariant ListPropertyModel::data(const QModelIndex& index, int role) const
    {
        if (role != Qt::DisplayRole || index.row() >= mList.size()) return QVariant();

        GateTypeProperty gtp = mList.at(index.row()).property;
        return QString::fromStdString(enum_to_string<GateTypeProperty>(gtp));
    }

    void ListPropertyModel::setSelected(GateTypeProperty gtp, bool select)
    {
        for (int irow=0; irow < mList.size(); irow++)
        {
            ListPropertyEntry& lpe = mList[irow];
            if (lpe.property == gtp)
            {
                if (lpe.isSelected == select) return; // nothing to do
                lpe.isSelected = select;
                QModelIndex inx = index(irow,0);
                Q_EMIT dataChanged(inx,inx);
                return;
            }
        }
    }

    ListPropertyProxy::ListPropertyProxy(bool showSel, QObject* parent)
        : QSortFilterProxyModel(parent), mShowSelected(showSel)
    {;}

    bool ListPropertyProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        Q_UNUSED(sourceParent);
        const ListPropertyModel* model = static_cast<const ListPropertyModel*>(sourceModel());

        return (model->isSelected(sourceRow) == mShowSelected);
    }

    GeneralInfoWizardPage::GeneralInfoWizardPage(const GateLibrary* gt, QWidget* parent) : QWizardPage(parent)
    {
        setTitle("General Information");
        setSubTitle("Add general information about the gate, such as name and properties");
        mLayout = new QGridLayout(this);
        mName = new QLineEdit(this);
        mPropertyModel = new ListPropertyModel(this);

        mPropertiesSelected = new QListView(this);
        ListPropertyProxy* selectedProxy = new ListPropertyProxy(true, this);
        selectedProxy->setSourceModel(mPropertyModel);
        mPropertiesSelected->setModel(selectedProxy);

        mPropertiesAvailable = new QListView(this);
        ListPropertyProxy* availableProxy = new ListPropertyProxy(false, this);
        availableProxy->setSourceModel(mPropertyModel);
        mPropertiesAvailable->setModel(availableProxy);

        mGateLibrary = gt;

        QLabel* labName = new QLabel("Name *", this);
        QLabel* labProperties = new QLabel("Properties:", this);
        labProperties->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
        labProperties->setObjectName("labProperties");
        QLabel* labPropertiesSelected = new QLabel(" Selected properties *", this);
        QLabel* labPropertiesAvailable = new QLabel(" Available properties", this);
        labPropertiesSelected->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        labPropertiesAvailable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        mAddBtn = new QPushButton(this);
        mAddBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mAddBtn->setObjectName("arrowButton");
        mDelBtn = new QPushButton(this);
        mDelBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mDelBtn->setObjectName("arrowButton");

        QStyle* s = style();

        s->unpolish(this);
        s->polish(this);

        mLeftArrowIcon = gui_utility::getStyledSvgIcon(mEnabledIconStyle, mLeftArrowIconPath, mDisabledIconStyle);
        mRightArrowIcon = gui_utility::getStyledSvgIcon(mEnabledIconStyle, mRightArrowIconPath, mDisabledIconStyle);
        mAddBtn->setIcon(mLeftArrowIcon);
        mDelBtn->setIcon(mRightArrowIcon);
        mAddBtn->setIconSize(QSize(24,24));
        mDelBtn->setIconSize(QSize(24,24));

        mLayout->addWidget(labName, 0, 0);
        mLayout->addWidget(mName, 0, 1, 1, 2);
        mLayout->addWidget(labProperties, 1, 0, 1, 3);
        mLayout->addWidget(labPropertiesSelected, 2, 0);
        mLayout->addWidget(labPropertiesAvailable, 2, 2);
        mLayout->addWidget(mPropertiesSelected, 3, 0, 3, 1);
        mLayout->addWidget(mAddBtn, 3, 1);
        mLayout->addWidget(mDelBtn, 4, 1);
        mLayout->addWidget(mPropertiesAvailable, 3, 2, 3, 1);

        setLayout(mLayout);

        connect(mAddBtn, &QPushButton::clicked, this, &GeneralInfoWizardPage::addProperty);
        connect(mDelBtn, &QPushButton::clicked, this, &GeneralInfoWizardPage::deleteProperty);
        connect(mName, &QLineEdit::textChanged, this, &GeneralInfoWizardPage::handleNameChanged);

        QRegularExpression rx("^[A-z]([A-z]|\\d|_)*$");
        mValidator = new QRegularExpressionValidator(rx, this);
        mName->setValidator(mValidator);
    }

    void GeneralInfoWizardPage::initializePage()
    {
        mWizard = static_cast<GateLibraryWizard*>(wizard());
        mWizard->mEditMode = true;
    }

    void GeneralInfoWizardPage::setData(QString name, const std::vector<GateTypeProperty>& properties)
    {
        mName->setText(name);

        mNameInit = name;

        for (GateTypeProperty gtp : properties)
        {
            mPropertyModel->setSelected(gtp, true);
        }

        Q_EMIT completeChanged();
    }

    QString GeneralInfoWizardPage::getName()
    {
        return mName->text();
    }

    QList<GateTypeProperty> GeneralInfoWizardPage::getProperties() const
    {
        QList<GateTypeProperty> retval;
        for (int irow = 0; irow < mPropertyModel->rowCount(); irow++)
            if (mPropertyModel->isSelected(irow))
                retval.append(mPropertyModel->property(irow));

        return retval;
    }

    void GeneralInfoWizardPage::addProperty()
    {
        QModelIndex inx = mPropertiesAvailable->currentIndex();
        if (!inx.isValid()) return;
        GateTypeProperty gtp = enum_from_string<GateTypeProperty>(mPropertiesAvailable->model()->data(inx).toString().toStdString(),GateTypeProperty::combinational);
        mPropertyModel->setSelected(gtp,true);
        mAddBtn->setEnabled(mPropertiesAvailable->model()->rowCount());
        mDelBtn->setEnabled(mPropertiesSelected->model()->rowCount());
        Q_EMIT completeChanged();
    }

    void GeneralInfoWizardPage::deleteProperty()
    {
        QModelIndex inx = mPropertiesSelected->currentIndex();
        if (!inx.isValid()) return;
        GateTypeProperty gtp = enum_from_string<GateTypeProperty>(mPropertiesSelected->model()->data(inx).toString().toStdString(),GateTypeProperty::combinational);
        mPropertyModel->setSelected(gtp,false);
        mAddBtn->setEnabled(mPropertiesAvailable->model()->rowCount());
        mDelBtn->setEnabled(mPropertiesSelected->model()->rowCount());
        Q_EMIT completeChanged();
    }

    void GeneralInfoWizardPage::handleNameChanged(const QString &txt)
    {
        Q_UNUSED (txt);
        Q_EMIT completeChanged();
    }

    bool GeneralInfoWizardPage::isComplete() const
    {
        if (getProperties().isEmpty() || mName->text().isEmpty()) return false;

        if (mName->text() == mNameInit) return true; // name of existing type unchanged

        for (auto it : mGateLibrary->get_gate_types())
        {
            if (QString::fromStdString(it.first) == mName->text())
                return false;
        }
        mWizard->mEditMode = false;
        return true;
    }
}
