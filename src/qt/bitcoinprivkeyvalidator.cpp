// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bitcoinprivkeyvalidator.h"

#include "base58.h"
#include "key.h"

BitcoinPrivKeyCheckValidator::BitcoinPrivKeyCheckValidator(QObject *parent) :
    QValidator(parent)
{
}

QValidator::State BitcoinPrivKeyCheckValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos);
    // Validate the passed Dogecoin private key
    CBitcoinSecret vchSecret;
    bool fGood = vchSecret.SetString(input.toStdString());

    if (!fGood) return QValidator::Invalid;
    
    CKey pkey = vchSecret.GetKey();
    if (!pkey.IsValid()) return QValidator::Invalid;
    
    return QValidator::Acceptable;
}
