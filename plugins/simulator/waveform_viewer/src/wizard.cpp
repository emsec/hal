#include "waveform_viewer/wizard.h"

Wizard::Wizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Empty Wizard"));

    addPage(createIntroPage());
    addPage(createPage1());
    addPage(createPage2());
    addPage(createPage3());
    addPage(createPage4());
    addPage(createConclusionPage());
}

QWizardPage *Wizard::createIntroPage()
{
    QWizardPage *page = new IntroPage;
    page->setTitle(tr("Introduction"));
    page->setSubTitle(tr("Introduction about Wizard"));
    return page;
}

QWizardPage *Wizard::createPage1()
{
    QWizardPage *page = new Page1;
    page->setTitle(tr("Step 1"));
    page->setSubTitle(tr("Select Gates"));
    return page;
}

QWizardPage *Wizard::createPage2()
{
    QWizardPage *page = new Page2;
    page->setTitle(tr("Step 2"));
    page->setSubTitle(tr("Clock settings"));
    return page;
}

QWizardPage *Wizard::createPage3()
{
    QWizardPage *page = new Page3;
    page->setTitle(tr("Step 3"));
    page->setSubTitle(tr("Engine settings"));
    return page;
}

QWizardPage *Wizard::createPage4()
{
    QWizardPage *page = new Page4;
    page->setTitle(tr("Step 4"));
    page->setSubTitle(tr("Load input Data"));
    return page;
}

QWizardPage *Wizard::createConclusionPage()
{
    QWizardPage *page = new ConclusionPage;
    page->setTitle(tr("Conclusion"));
    page->setSubTitle(tr("Run Simulation"));
    return page;
}

IntroPage::IntroPage(QWidget *parent) : QWizardPage(parent)
{
    label = new QLabel(tr("What is the Waveform Simulation"));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

Page1::Page1(QWidget *parent): QWizardPage(parent)
{
    label = new QLabel(tr("Select Gate:"));
    lineEdit = new QLineEdit;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    setLayout(layout);
}

Page2::Page2(QWidget *parent): QWizardPage(parent)
{
    label = new QLabel(tr("Clock:"));
    lineEdit = new QLineEdit;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    setLayout(layout);
}

Page3::Page3(QWidget *parent): QWizardPage(parent)
{
    label = new QLabel(tr("Engine:"));
    lineEdit = new QLineEdit;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    setLayout(layout);
}

Page4::Page4(QWidget *parent): QWizardPage(parent)
{
    label = new QLabel(tr("Input Data:"));
    lineEdit = new QLineEdit;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    setLayout(layout);
}

ConclusionPage::ConclusionPage(QWidget *parent): QWizardPage(parent)
{
    label = new QLabel(tr("Run Simulation"));
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}
