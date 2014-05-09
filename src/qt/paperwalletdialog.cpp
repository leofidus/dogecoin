// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "paperwalletdialog.h"
#include "ui_paperwalletdialog.h"

#include "walletmodel.h"
#include "bitcoinunits.h"
#include "addressbookpage.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "receiverequestdialog.h"
#include "addresstablemodel.h"
#include "recentrequeststablemodel.h"
#include "wallet.h"

#include <QAction>
#include <QCursor>
#include <QMessageBox>
#include <QTextDocument>
#include <QScrollBar>

PaperWalletDialog::PaperWalletDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PaperWalletDialog),
    model(0)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC // Icons on push buttons are very uncommon on Mac
    ui->withdrawButton->setIcon(QIcon());
    ui->clearButton->setIcon(QIcon());
    //ui->showRequestButton->setIcon(QIcon());
    //ui->removeRequestButton->setIcon(QIcon());
#endif

    importWallet = new CWallet();
    
    GUIUtil::setupPrivKeyWidget(ui->reqKey, this);
    
    connect(ui->withdrawButton, SIGNAL(clicked()), this, SLOT(withdraw()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    connect(ui->reqKey, SIGNAL(editingFinished()), this, SLOT(privKeyEntered()));
}

void PaperWalletDialog::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
    {
        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
        updateDisplayUnit();
    }
}

PaperWalletDialog::~PaperWalletDialog()
{
    delete ui;
    delete importWallet;
}

void PaperWalletDialog::clear()
{
    ui->reqKey->clear();
    clearAllButKey();
}

void PaperWalletDialog::clearAllButKey()
{
    ui->balanceLabel->clear();
    ui->feeLabel->clear();
    ui->reqAmount->clear();
    updateDisplayUnit();
}

void PaperWalletDialog::withdraw()
{
    //clear();
}

void PaperWalletDialog::privKeyEntered()
{
    const std::string skey = ui->reqKey->text().toStdString();
    
    if(skey == lastPrivKey)
        return;
    
    delete importWallet;
    importWallet = new CWallet();
    lastPrivKey = skey;
    
    if(!ui->reqKey->isValid()) {
        clearAllButKey();
        return;
    }
    
    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(skey);

    if (!fGood) return;

    CKey key = vchSecret.GetKey();
    if (!key.IsValid()) return;

    CPubKey pubkey = key.GetPubKey();
    CKeyID vchAddress = pubkey.GetID();
    {
        LOCK2(cs_main, importWallet->cs_wallet);

        importWallet->MarkDirty();

        importWallet->mapKeyMetadata[vchAddress].nCreateTime = 1;

        if (!importWallet->AddKeyPubKey(key, pubkey))
            return; //TODO: show error?

        // whenever a key is imported, we need to scan the whole chain
        importWallet->nTimeFirstKey = 1; // 0 would be considered 'no value'

        importWallet->ScanForWalletTransactions(chainActive.Genesis(), true);
        
        ui->balanceLabel->setValue(importWallet->GetBalance());
    }
}

void PaperWalletDialog::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        ui->balanceLabel->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->feeLabel->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
        ui->reqAmount->setDisplayUnit(model->getOptionsModel()->getDisplayUnit());
    }
}

void PaperWalletDialog::on_withdrawButton_clicked()
{
    if(!model || !model->getOptionsModel() || !model->getAddressTableModel())
        return;
        
    if(ui->reqKey->text().toStdString() != lastPrivKey)
        privKeyEntered();
    
    bool fValid = true;
    
    if(importWallet->GetBalance() == 0 || ui->reqAmount->value(&fValid) > importWallet->GetBalance())
        return;
        
    if(!fValid)
        return;
    
    const QString address = model->getAddressTableModel()->addRow(AddressTableModel::Receive, QString(tr("Paper Wallet")), "");
    
    
    
    
    SendCoinsRecipient recipient(address, "", ui->reqAmount->value(NULL), "");
    
#if 0
    // Format confirmation message
    QStringList formatted;
    foreach(const SendCoinsRecipient &rcp, recipients)
    {
        // generate bold amount string
        QString amount = "<b>" + BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), rcp.amount);
        amount.append("</b>");
        // generate monospace address string
        QString address = "<span style='font-family: monospace;'>" + rcp.address;
        address.append("</span>");

        QString recipientElement;

        if (!rcp.paymentRequest.IsInitialized()) // normal payment
        {
            if(rcp.label.length() > 0) // label with address
            {
                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
                recipientElement.append(QString(" (%1)").arg(address));
            }
            else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
        }
        else if(!rcp.authenticatedMerchant.isEmpty()) // secure payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
        }
        else // insecure payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, address);
        }

        formatted.append(recipientElement);
    }

    fNewRecipientAllowed = false;


    WalletModel::UnlockContext ctx(model->requestUnlock());
    if(!ctx.isValid())
    {
        // Unlock wallet was cancelled
        fNewRecipientAllowed = true;
        return;
    }

    // prepare transaction for getting txFee earlier
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;
    if (model->getOptionsModel()->getCoinControlFeatures()) // coin control enabled
        prepareStatus = model->prepareTransaction(currentTransaction, CoinControlDialog::coinControl);
    else
        prepareStatus = model->prepareTransaction(currentTransaction);

    // process prepareStatus and on error generate message shown to user
    processSendCoinsReturn(prepareStatus,
        BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), currentTransaction.getTransactionFee()));

    if(prepareStatus.status != WalletModel::OK) {
        fNewRecipientAllowed = true;
        return;
    }

    qint64 txFee = currentTransaction.getTransactionFee();
    QString questionString = tr("Are you sure you want to send?");
    questionString.append("<br /><br />%1");

    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), txFee));
        questionString.append("</span> ");
        questionString.append(tr("added as transaction fee"));
    }

    // add total amount in all subdivision units
    questionString.append("<hr />");
    qint64 totalAmount = currentTransaction.getTotalTransactionAmount() + txFee;
    QStringList alternativeUnits;
    foreach(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
    {
        if(u != model->getOptionsModel()->getDisplayUnit())
            alternativeUnits.append(BitcoinUnits::formatWithUnit(u, totalAmount));
    }
    questionString.append(tr("Total Amount %1 (= %2)")
        .arg(BitcoinUnits::formatWithUnit(model->getOptionsModel()->getDisplayUnit(), totalAmount))
        .arg(alternativeUnits.join(" " + tr("or") + " ")));

    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm send coins"),
        questionString.arg(formatted.join("<br />")),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        fNewRecipientAllowed = true;
        return;
    }

    // now send the prepared transaction
    WalletModel::SendCoinsReturn sendStatus = model->sendCoins(currentTransaction);
    // process sendStatus and on error generate message shown to user
    processSendCoinsReturn(sendStatus);

    if (sendStatus.status == WalletModel::OK)
    {
        accept();
        CoinControlDialog::coinControl->UnSelectAll();
        coinControlUpdateLabels();
    }
    fNewRecipientAllowed = true;
#endif
}


void PaperWalletDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return)
    {
        // press return -> submit form
        if (ui->reqKey->hasFocus() || ui->reqAmount->hasFocus())
        {
            event->ignore();
            on_withdrawButton_clicked();
            return;
        }
    }

    this->QDialog::keyPressEvent(event);
}
