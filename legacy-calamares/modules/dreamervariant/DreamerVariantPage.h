// DreamerVariantPage.h
// page03 (three cards: Game / Develop / Scratch) and page03b (software list
// for whichever was picked) as two stacked sub-pages of one widget.

#pragma once

#include <QWidget>
#include <QVariantList>
#include <QStringList>

class QStackedWidget;
class QLabel;
class QVBoxLayout;
class QButtonGroup;
class QAbstractButton;

class DreamerVariantPage : public QWidget
{
    Q_OBJECT
public:
    enum SubPage
    {
        VariantCards,
        SoftwareSelection
    };

    explicit DreamerVariantPage( QWidget* parent = nullptr );

    void setTitle( const QString& title );
    void loadVariants( const QVariantList& variants );

    void showVariantCards();
    void showSoftwareSelection();
    SubPage currentSubPage() const { return m_currentSubPage; }

    QString selectedVariantId() const { return m_selectedVariantId; }
    QStringList selectedOptionalPackageIds() const;

Q_SIGNALS:
    void selectionChanged();

private Q_SLOTS:
    void onCardClicked( QAbstractButton* button );
    void onOptionalPackageToggled( bool checked );

private:
    void rebuildSoftwarePage();

    QVariantList m_variants;
    QString m_selectedVariantId;
    QStringList m_selectedOptionalPackageIds;

    SubPage m_currentSubPage = VariantCards;

    QLabel* m_titleLabel;
    QStackedWidget* m_stack;
    QWidget* m_cardsPage;
    QWidget* m_softwarePage;
    QVBoxLayout* m_softwareListLayout;
    QButtonGroup* m_cardGroup;
};
