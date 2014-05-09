// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PAPERWALLETDIALOG_H
#define PAPERWALLETDIALOG_H

#include <string>

#include <QDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPoint>
#include <QVariant>

#include "wallet.h"

namespace Ui {
    class PaperWalletDialog;
}
class WalletModel;
class OptionsModel;

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Dialog for requesting payment of bitcoins */
class PaperWalletDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaperWalletDialog(QWidget *parent = 0);
    ~PaperWalletDialog();

    void setModel(WalletModel *model);

public slots:
    void withdraw();
    void clear();
    void privKeyEntered();

protected:
    virtual void keyPressEvent(QKeyEvent *event);

private:
    Ui::PaperWalletDialog *ui;
    WalletModel *model;
    std::string lastPrivKey;
    CWallet* importWallet;

private slots:
    void clearAllButKey();
    void on_withdrawButton_clicked();
    void updateDisplayUnit();
};

#endif // PAPERWALLETDIALOG_H
