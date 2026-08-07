// UsersPage.h — page04a (empty list) / 04b (add user) / 04c (list) / 04d (root password)

#pragma once

#include "../InstallerPage.h"

class QListWidget;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QStackedWidget;

class UsersPage : public InstallerPage
{
    Q_OBJECT
public:
    explicit UsersPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Create Your Account" ); }
    bool isNextEnabled() const override;

private Q_SLOTS:
    void onAddUserClicked();
    void onConfirmAddUser();
    void onCancelAddUser();

private:
    QStackedWidget* m_stack;
    QWidget* m_listView;
    QWidget* m_addUserView;

    QListWidget* m_userList;
    QPushButton* m_addUserButton;

    QLineEdit* m_fullNameField;
    QLineEdit* m_usernameField;
    QLineEdit* m_passwordField;
    QLineEdit* m_confirmPasswordField;
    QCheckBox* m_sudoerCheck;
    QCheckBox* m_autoLoginCheck;

    QLineEdit* m_rootPasswordField;
    QLineEdit* m_rootConfirmField;
};
