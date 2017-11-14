#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QTcpServer>
#include <QListWidgetItem>
#include "Kuznyechik.hpp"
#include <iomanip>
#include <sstream>
#include <dh.h>
#include <dh2.h>
#include <nbtheory.h>
#include <iostream>
#include <osrng.h>
#include <peer.h>
namespace Ui {
class ClientWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    ClientWindow(int Port, QString address, QWidget *parent = 0);
    ~ClientWindow();
private slots:
    void on_SearchLine_returnPressed();
    void onRead();
    void on_NameInput_returnPressed();   
    void on_SendMsg_clicked();
    void on_FriendList_itemDoubleClicked(QListWidgetItem *item);
    void on_MsgInput_returnPressed();
    void ConnDetector();
    void on_UpdateListButton_clicked();
    void on_checkBox_toggled(bool checked);

private:
    //<=methods=>
    int Resolver(QString Data);
    void SendMessageToPeer(QString PeerName);
    void ConnectToPeer(QString IP, int Port, QString UserName);
    void ParseAllUsersData(QString Response);
    QString Encrypt(QString &Message, QString Key);
    QString Decrypt(QString &Message, QString Key);
    void GenKeyParams();
    CryptoPP::SecByteBlock IncomingSessionKeyGen(QString Username, CryptoPP::Integer prime, CryptoPP::Integer generator, CryptoPP::SecByteBlock publicNumb);
    void GettingAgreement(QString Username, CryptoPP::SecByteBlock NewKey);
    void SendGeneratedPublicKey(QString UserName, CryptoPP::SecByteBlock publicKey);
    Peer* SearchPeerByName(QString Name);
    //<=fields=>
        //crypto
    CryptoPP::DH dh;
    CryptoPP::Integer Prime;
    CryptoPP::Integer Generator;
    CryptoPP::SecByteBlock PrivNumb;
    CryptoPP::SecByteBlock PublicNumb;
    CryptoPP::SecByteBlock MySecretKey;
        //sockets
    QVector<Peer> Peers;
    std::unique_ptr<QTcpSocket> ServerSocket;
    std::unique_ptr<QTcpServer> ThisListenSocket;
       //vars
    QString Destination;
    QString NickName;
    QString ServerIP;
    int ServerPort;
    bool ConnectedToServer = false;
    bool Private = false;
    Ui::ClientWindow *ui;
};

#endif // CLIENTWINDOW_H
