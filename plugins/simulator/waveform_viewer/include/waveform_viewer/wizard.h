#ifndef WIZARD_H
#define WIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>

class Wizard : public QWizard {
    Q_OBJECT

public:
    Wizard(QWidget *parent = 0);

private:
    QWizardPage *createIntroPage();
    QWizardPage *createPage1();
    QWizardPage *createPage2();
    QWizardPage *createPage3();
    QWizardPage *createPage4();
    QWizardPage *createConclusionPage();
};

class IntroPage : public QWizardPage {
    Q_OBJECT

public:
    IntroPage(QWidget *parent = 0);

private:
    QLabel *label;
};

class Page1 : public QWizardPage {
    Q_OBJECT

public:
    Page1(QWidget *parent = 0);

private:
    QLabel *label;
    QLineEdit *lineEdit;
};

class Page2 : public QWizardPage {
    Q_OBJECT

public:
    Page2(QWidget *parent = 0);

private:
    QLabel *label;
    QLineEdit *lineEdit;
};

class Page3 : public QWizardPage {
    Q_OBJECT

public:
    Page3(QWidget *parent = 0);

private:
    QLabel *label;
    QLineEdit *lineEdit;
};

class Page4 : public QWizardPage {
    Q_OBJECT

public:
    Page4(QWidget *parent = 0);

private:
    QLabel *label;
    QLineEdit *lineEdit;
};

class ConclusionPage : public QWizardPage {
    Q_OBJECT

public:
    ConclusionPage(QWidget *parent = 0);

private:
    QLabel *label;
};

#endif // WIZARD_H
