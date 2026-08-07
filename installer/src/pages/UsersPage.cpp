// UsersPage.cpp

#include "UsersPage.h"
#include "../InstallerState.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>

UsersPage::UsersPage( InstallerState* state, QWidget* parent )
    : InstallerPage( state, parent )
{
    auto* root = new QVBoxLayout( this );
    auto* title = new QLabel( pageTitle(), this );
    title->setObjectName( "dreamerPageTitle" );
    root->addWidget( title );

    m_stack = new QStackedWidget( this );
    root->addWidget( m_stack );

    // --- page04a / 04c: the user list (empty until someone is added) ---
    m_listView = new QWidget( m_stack );
    auto* listLayout = new QVBoxLayout( m_listView );
    m_userList = new QListWidget( m_listView );
    m_userList->setObjectName( "dreamerUserList" );
    listLayout->addWidget( m_userList );

    m_addUserButton = new QPushButton( tr( "+ Add User" ), m_listView );
    m_addUserButton->setObjectName( "dreamerAddUserButton" );
    connect( m_addUserButton, &QPushButton::clicked, this, &UsersPage::onAddUserClicked );
    listLayout->addWidget( m_addUserButton );

    // --- root password, shown under the list too (page04d) ---
    auto* rootLabel = new QLabel( tr( "Root password" ), m_listView );
    rootLabel->setObjectName( "dreamerFieldLabel" );
    m_rootPasswordField = new QLineEdit( m_listView );
    m_rootPasswordField->setEchoMode( QLineEdit::Password );
    m_rootConfirmField = new QLineEdit( m_listView );
    m_rootConfirmField->setEchoMode( QLineEdit::Password );
    m_rootConfirmField->setPlaceholderText( tr( "Confirm root password" ) );
    connect( m_rootPasswordField, &QLineEdit::textChanged, this, [this]( const QString& text ) {
        m_state->rootPassword = text;
        emit validityChanged();
    } );
    listLayout->addWidget( rootLabel );
    listLayout->addWidget( m_rootPasswordField );
    listLayout->addWidget( m_rootConfirmField );

    m_stack->addWidget( m_listView );

    // --- page04b: add-user form ---
    m_addUserView = new QWidget( m_stack );
    auto* addLayout = new QVBoxLayout( m_addUserView );

    m_fullNameField = new QLineEdit( m_addUserView );
    m_fullNameField->setPlaceholderText( tr( "Full name" ) );
    m_usernameField = new QLineEdit( m_addUserView );
    m_usernameField->setPlaceholderText( tr( "Username" ) );
    m_passwordField = new QLineEdit( m_addUserView );
    m_passwordField->setEchoMode( QLineEdit::Password );
    m_passwordField->setPlaceholderText( tr( "Password" ) );
    m_confirmPasswordField = new QLineEdit( m_addUserView );
    m_confirmPasswordField->setEchoMode( QLineEdit::Password );
    m_confirmPasswordField->setPlaceholderText( tr( "Confirm password" ) );

    m_sudoerCheck = new QCheckBox( tr( "Make this user an administrator" ), m_addUserView );
    m_sudoerCheck->setChecked( true );
    m_autoLoginCheck = new QCheckBox( tr( "Log in automatically" ), m_addUserView );
    // "login without password" is locked whenever sudo is on, per the original spec.
    connect( m_sudoerCheck, &QCheckBox::toggled, this, [this]( bool checked ) {
        if ( checked )
        {
            m_autoLoginCheck->setChecked( false );
        }
        m_autoLoginCheck->setEnabled( !checked );
    } );

    addLayout->addWidget( m_fullNameField );
    addLayout->addWidget( m_usernameField );
    addLayout->addWidget( m_passwordField );
    addLayout->addWidget( m_confirmPasswordField );
    addLayout->addWidget( m_sudoerCheck );
    addLayout->addWidget( m_autoLoginCheck );

    auto* addButtonsRow = new QHBoxLayout();
    auto* cancelButton = new QPushButton( tr( "Cancel" ), m_addUserView );
    auto* confirmButton = new QPushButton( tr( "Add" ), m_addUserView );
    connect( cancelButton, &QPushButton::clicked, this, &UsersPage::onCancelAddUser );
    connect( confirmButton, &QPushButton::clicked, this, &UsersPage::onConfirmAddUser );
    addButtonsRow->addWidget( cancelButton );
    addButtonsRow->addWidget( confirmButton );
    addLayout->addLayout( addButtonsRow );

    m_stack->addWidget( m_addUserView );
    m_stack->setCurrentWidget( m_listView );
}

void
UsersPage::onAddUserClicked()
{
    m_fullNameField->clear();
    m_usernameField->clear();
    m_passwordField->clear();
    m_confirmPasswordField->clear();
    m_stack->setCurrentWidget( m_addUserView );
}

void
UsersPage::onConfirmAddUser()
{
    if ( m_usernameField->text().isEmpty() || m_passwordField->text().isEmpty()
         || m_passwordField->text() != m_confirmPasswordField->text() )
    {
        return; // real build shows an inline validation message here
    }

    UserAccount account;
    account.fullName = m_fullNameField->text();
    account.username = m_usernameField->text();
    account.password = m_passwordField->text();
    account.isSudoer = m_sudoerCheck->isChecked();
    account.autoLogin = m_autoLoginCheck->isChecked();
    m_state->users.append( account );

    auto* item = new QListWidgetItem(
        QStringLiteral( "%1 (%2)%3" )
            .arg( account.fullName.isEmpty() ? account.username : account.fullName, account.username,
                  account.isSudoer ? tr( " \u2014 administrator" ) : QString() ),
        m_userList );
    Q_UNUSED( item )

    m_stack->setCurrentWidget( m_listView );
    emit validityChanged();
}

void
UsersPage::onCancelAddUser()
{
    m_stack->setCurrentWidget( m_listView );
}

bool
UsersPage::isNextEnabled() const
{
    // NEXT blocked until a user exists — matches the original review's
    // "real logic already baked in" note about the original mockup package.
    return !m_state->users.isEmpty() && !m_state->rootPassword.isEmpty();
}
