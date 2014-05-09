// Copyright (c) 2011-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINPRIVKEYVALIDATOR_H
#define BITCOINPRIVKEYVALIDATOR_H

#include <QValidator>

/** Bitcoin address widget validator, checks for a valid dogecoin private keys.
 */
class BitcoinPrivKeyCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit BitcoinPrivKeyCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const;
};

#endif // BITCOINPRIVKEYVALIDATOR_H
