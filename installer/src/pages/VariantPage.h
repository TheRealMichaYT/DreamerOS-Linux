// VariantPage.h — page03 (Game/Develop/Scratch cards) + page03b (software list)

#pragma once

#include "../InstallerPage.h"
#include <QVariantMap>

class QStackedWidget;
class QButtonGroup;
class QAbstractButton;
class QVBoxLayout;
class QLabel;

class VariantPage : public InstallerPage
{
    Q_OBJECT
public:
    enum SubPage
    {
        Cards,
        Software
    };

    explicit VariantPage( InstallerState* state, QWidget* parent = nullptr );

    QString pageTitle() const override { return tr( "Choose Your DreamerOS" ); }
    bool isNextEnabled() const override;
    bool handleNext() override;
    bool handleBack() override;

private Q_SLOTS:
    void onCardClicked( QAbstractButton* button );
    void onOptionalToggled( bool checked );

private:
    void rebuildSoftwarePage();
    QVariantMap variantConfig( const QString& id ) const;

    SubPage m_subPage = Cards;
    QStackedWidget* m_stack;
    QWidget* m_cardsPage;
    QWidget* m_softwarePage;
    QVBoxLayout* m_softwareLayout;
    QButtonGroup* m_cardGroup;
};
