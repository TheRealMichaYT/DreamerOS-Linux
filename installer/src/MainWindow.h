// MainWindow.h
// Owns the QStackedWidget of every installer page, the shared InstallerState,
// and the NEXT/BACK buttons. This replaces Calamares's ViewManager: there's
// no plugin system here, every page is just a QWidget subclass added to the
// stack in the constructor, in the fixed order defined by pageOrder().

#pragma once

#include <QMainWindow>
#include <QVector>

#include "InstallerState.h"

class QStackedWidget;
class QPushButton;
class QLabel;
class InstallerPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow( QWidget* parent = nullptr );

private Q_SLOTS:
    void goNext();
    void goBack();
    void updateNavButtons();

private:
    void buildPages();
    void showAboutModal();

    QStackedWidget* m_stack;
    QPushButton* m_nextButton;
    QPushButton* m_backButton;
    QPushButton* m_aboutButton;
    QLabel* m_logoLabel;

    InstallerState m_state;
    QVector<InstallerPage*> m_pages;
    int m_currentPageIndex = 0;
};
