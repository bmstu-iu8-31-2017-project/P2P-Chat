#include "clientwindow.h"
#include "ui_clientwindow.h"

//<=converters=>
std::string hex_to_string(const std::string& in)
{
    std::string output;
    if ((in.length() % 2) != 0) {
        throw std::runtime_error("String is not valid length ...");
    }
    size_t cnt = in.length() / 2;
    for (size_t i = 0; cnt > i; ++i) {
        uint32_t s = 0;
        std::stringstream ss;
        ss << std::hex << in.substr(i * 2, 2);
        ss >> s;
        output.push_back(static_cast<unsigned char>(s));
    }
    return output;
}
std::string string_to_hex(const std::string& in)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; in.length() > i; ++i)
    {
        ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(in[i]));
    }
    return ss.str();
}
QString IntegerToString(CryptoPP::Integer Numb)
{
    std::stringstream ss;
    ss << Numb;
    QString RetStr = QString::fromStdString(string_to_hex(ss.str()));
    return RetStr;
}
QString SecBBToString(CryptoPP::SecByteBlock Block)
{   
    std::string buff(Block.begin(), Block.end());
    QString RetStr = QString::fromStdString(string_to_hex(buff));
    return RetStr;
}

void BlockAddition(std::string &Message)
{
   while(Message.length()%32 != 0)
   {
       Message += '00';
   }
}

//<=methods=>
ClientWindow::ClientWindow(int Port, QString address, QWidget *parent)
    :QMainWindow(parent)
    ,ServerSocket (new QTcpSocket(this))
    ,ThisListenSocket(new QTcpServer(this))
    ,ServerIP(address)
    ,ServerPort(Port)
    ,ui(new Ui::ClientWindow)
{
    ui->setupUi(this);

    if(!ThisListenSocket.get()->listen())
    {
        ui->DebugWindow->append("ListenSocket unavailable");
    }
    GenKeyParams();
    connect(ThisListenSocket.get(), SIGNAL(newConnection()), this, SLOT(ConnDetector()));
}

ClientWindow::~ClientWindow()
{
    delete ui;
}
Peer* ClientWindow::SearchPeerByName(QString Name)
{
    for(int i = 0; i < Peers.size(); ++i )
    {
        if(Peers[i].PeerName == Name)
        {
            return &Peers[i];
        }
    }
    return nullptr;
}
int ClientWindow::Resolver(QString Data)
{
    if(Data.startsWith("!S!"))   //server response (success search)
    {
        return 0;
    }
    else if(Data.startsWith("!C!"))  //new connection request (from peer)
    {
        return 1;
    }
    else if(Data.startsWith("!M!"))  //message
    {
        return 2;
    }
    else if(Data.startsWith("!A!")) //for key exchange(answer)
    {
        return 3;
    }
    else if(Data.startsWith("!CR!"))   //connect request (private user->public user)
    {
        return 4;
    }
    else if(Data.startsWith("!SMESS!")) //server message (errors etc.)
    {
        return 9;
    }
    return -1;
    //add more flags
}
void ClientWindow::SendMessageToPeer(QString PeerName)
{
    if(!Destination.isEmpty() && !SearchPeerByName(PeerName)->SessionKey.isEmpty())
    {
        QDataStream PeerStream(SearchPeerByName(PeerName)->PeerSocket.get());
        QString Message = ui->MsgInput->text();
        Message = Encrypt(Message, SearchPeerByName(PeerName)->SessionKey);
        Message = "!M!" + QString::fromStdString(string_to_hex(NickName.toStdString())) + ':' + Message;
        PeerStream << Message;
    }
}
void ClientWindow::SendConnectRequest(QString PeerName)
{
    QString ConnectReq("!CR!" + ThisListenSocket.get()->serverAddress().toString() + ':'
                       + QString::number(ThisListenSocket.get()->serverPort()) + ':'
                       + NickName);
    QDataStream Stream(SearchPeerByName(PeerName)->PeerSocket.get());
    Stream << ConnectReq;
}
void ClientWindow::SendGeneratedPublicKey(QString PeerName, CryptoPP::SecByteBlock publicKey)
{
        QDataStream PeerStream(SearchPeerByName(PeerName)->PeerSocket.get());
        QString Message = "!A!" + NickName + ':' + SecBBToString(publicKey);
        PeerStream << Message;
}
void ClientWindow::ConnDetector()
{
    QTcpSocket* LSocket(ThisListenSocket.get()->nextPendingConnection());
    connect(LSocket, SIGNAL(readyRead()), this, SLOT(onRead()));
}
void ClientWindow::ConnectToPeer(QString IP, int Port, QString UserName)
{
    if(SearchPeerByName(UserName) == nullptr)
    {
        QHostAddress addr(IP);
        std::shared_ptr<QTcpSocket>NewSocket(new QTcpSocket());
        NewSocket.get()->connectToHost(addr,Port);
        QDataStream Stream (NewSocket.get());
        QString KeyExcReq(':' + IntegerToString(Prime) + ':' + IntegerToString(Generator) + ':' + SecBBToString(PublicNumb));
        QString ConnectReq("!C!" + ThisListenSocket.get()->serverAddress().toString() + ':'
                                 + QString::number(ThisListenSocket.get()->serverPort()) + ':'
                                 + NickName + KeyExcReq);

        Stream << ConnectReq;
        connect(NewSocket.get(),  SIGNAL(readyRead()), this, SLOT(onRead()));
        Peer NewPeer(UserName,NewSocket);
        Peers.push_back(NewPeer);
    }
}
void ClientWindow::ParseAllUsersData(QString Response)
{
   int PeersNumber = Response.count('|');
   QVector<QString> Users;
   for(int i = 0; i < PeersNumber; ++i)
   {
       Users.push_back(Response.split('|')[i]);
       if(Users[i].split(':')[2]!= NickName)
       {
            QString IP(Users[i].split(':')[0]);
            int Port = Users[i].split(':')[1].toInt();
            QString Name(Users[i].split(':')[2]);
            ui->FriendList->addItem(Name);
            if(Private == false)
            {
                ConnectToPeer(IP, Port, Name);
            }
            else
            {
                std::shared_ptr<QTcpSocket>NewSocket(new QTcpSocket());
                NewSocket.get()->connectToHost(IP,Port);
                Peer NewPeer(Name, NewSocket);
                Peers.push_back(NewPeer);
            }
       }
    }
}
void ClientWindow::on_SearchLine_returnPressed()
{
    if(ConnectedToServer)
    {
        QString Request = ui->SearchLine->text();

        if(Request != NickName && SearchPeerByName(Request) == nullptr)
        {
            Request = "!2!" + Request; //search request
            QDataStream ServStream(ServerSocket.get());
            ServStream << Request;
        }
    }
    else
    {
        ui->DebugWindow->append("Can't connect to server!");
    }
}
void ClientWindow::on_NameInput_returnPressed()
{
    NickName = ui->NameInput->text();
    //hiding controls
    ui->NameInput->hide();
    ui->checkBox->hide();
    //connecting to server
    ServerSocket.get()->connectToHost(ServerIP, ServerPort);
    QString Status("!PUB!");
    if(Private == true)
    {
        Status = "!PR!";
    }
    QString RegStr = "!0!" + NickName + ',' + ThisListenSocket.get()->serverAddress().toString() +':' + QString::number(ThisListenSocket.get()->serverPort()) + ',' + Status; // +address
    QDataStream ServStream(ServerSocket.get());
    ServStream << RegStr;
    connect(ServerSocket.get(), SIGNAL(readyRead()), this, SLOT(onRead()));
    ui->NameLabel->setText("Logged as " + NickName);
    ConnectedToServer = true;
    //generating key

}
void ClientWindow::onRead()
{
    QTcpSocket* ListenSocket((QTcpSocket*)sender());
    QDataStream LStream(ListenSocket);
    QString Response;
    LStream >> Response;
    if(Resolver(Response) == 0) //data from server
    {
        Response = Response.mid(3);
        ParseAllUsersData(Response);
    }
    else if(Resolver(Response) == 1) //new connect
    {      
        if(Response.split(':')[2] != NickName)
        {
            Response = Response.mid(3);
            ui->DebugWindow->append("New peer connected! " + Response.split(':')[2]);

            //parsing
            QHostAddress IP(Response.split(':')[0]);
            int Port = Response.split(':')[1].toInt();

            QString PeerName = Response.split(':')[2];

            std::string buffstr = hex_to_string(Response.split(':')[3].toStdString());
            CryptoPP::Integer prime(buffstr.c_str());

            buffstr = hex_to_string(Response.split(':')[4].toStdString());
            CryptoPP::Integer generator(buffstr.c_str());

            buffstr = hex_to_string(Response.split(':')[5].toStdString());
            CryptoPP::SecByteBlock pubNumb(buffstr.length());
            for(size_t i = 0; i < buffstr.length(); ++i)
            {
                pubNumb[i] = buffstr[i];
            }

            std::shared_ptr<QTcpSocket>NewSocket(new QTcpSocket());
            NewSocket.get()->connectToHost(IP,Port);

            if(SearchPeerByName(Response.split(':')[2])== nullptr)
            {
                ui->FriendList->addItem(Response.split(':')[2]);
                Peer NewPeer(PeerName, NewSocket);
                Peers.push_back(NewPeer);
            }
            CryptoPP::SecByteBlock PublicKey = IncomingSessionKeyGen(PeerName, prime, generator, pubNumb);
            SendGeneratedPublicKey(PeerName, PublicKey);
        }
    }
    else if(Resolver(Response) == 2) //message
    {
        Response = Response.mid(3);
        QString Name = QString(hex_to_string(Response.split(':')[0].toStdString()).c_str());
        Response = Response.split(':')[1];
        Response = Decrypt(Response, SearchPeerByName(Name)->SessionKey);
        ui->MsgBrowser->append(Name + ": " + Response);
    }
    else if(Resolver(Response) == 3)
    {
        Response = Response.mid(3);
        QString Name = Response.split(':')[0];
        std::string buffStr = hex_to_string(Response.split(':')[1].toStdString());
        CryptoPP::SecByteBlock SecBB(buffStr.length());
        for(int i = 0; i < buffStr.length(); ++i)
        {
            SecBB[i] = buffStr[i];
        }
        GettingAgreement(Name, SecBB);
    }
    else if(Resolver(Response) == 4)
    {
        Response = Response.mid(4);
        ui->FriendList->addItem(Response.split(':')[2]);
        ConnectToPeer(Response.split(':')[0], Response.split(':')[1].toInt(), Response.split(':')[2]);
    }
    else if(Resolver(Response) == 9) //server messages
    {
        Response = Response.mid(7);
        ui->DebugWindow->append("Server : " + Response);
    }
    else if(Resolver(Response) == -1) //errors
    {
        ui->DebugWindow->append("Error! " + Response);
    }
}
void ClientWindow::on_SendMsg_clicked()
{
    if(Private == true && SearchPeerByName(Destination)->SessionKey.isEmpty())
    {
         ui->MsgBrowser->append("\n --- Your Message can not be sended now. It just used for key exchange with destination peer because u are a private user. Resend it --- \n");
         SendConnectRequest(Destination);
    }
    else
    {
        QString Message = ui->MsgInput->text();
        ui->MsgBrowser->append ("Me: " + Message);
        SendMessageToPeer(Destination);
        ui->MsgInput->clear();
    }
}
void ClientWindow::on_FriendList_itemDoubleClicked(QListWidgetItem *item)
{
    Destination = item->text();
    ui->DestName->setText(Destination);
}
void ClientWindow::on_MsgInput_returnPressed()
{
    on_SendMsg_clicked();
}
void ClientWindow::on_UpdateListButton_clicked()
{
    QString UpdReq = "!UPD!";
    ui->FriendList->clear();
    Peers.clear();
    QDataStream ServStream(ServerSocket.get());
    ServStream << UpdReq;
}
//for Diffie-Hellman key exchange
void ClientWindow::GenKeyParams()
{
    CryptoPP::AutoSeededRandomPool rnd;
    dh.AccessGroupParameters().GenerateRandomWithKeySize(rnd, 256);
    Prime = dh.GetGroupParameters().GetModulus();
    Generator = dh.GetGroupParameters().GetSubgroupGenerator();
    PrivNumb = CryptoPP::SecByteBlock(dh.PrivateKeyLength());
    PublicNumb = CryptoPP::SecByteBlock(dh.PublicKeyLength());
    MySecretKey = CryptoPP::SecByteBlock(dh.AgreedValueLength());
    dh.GenerateKeyPair(rnd, PrivNumb, PublicNumb);
}
CryptoPP::SecByteBlock ClientWindow::IncomingSessionKeyGen(QString Username, CryptoPP::Integer prime, CryptoPP::Integer generator, CryptoPP::SecByteBlock publicNumb)
{
    CryptoPP::AutoSeededRandomPool rngB;
    CryptoPP::DH dhB(prime, generator);
    CryptoPP::SecByteBlock privB(dhB.PrivateKeyLength());
    CryptoPP::SecByteBlock pubB(dhB.PublicKeyLength());
    CryptoPP::SecByteBlock secretKeyB(dhB.AgreedValueLength());

    dhB.GenerateKeyPair(rngB, privB, pubB);

    if (dhB.Agree(secretKeyB, privB, publicNumb))
    {
        ui->DebugWindow->append("Success key exchange with: " + Username);
        SearchPeerByName(Username)->SetSessionKey(SecBBToString(secretKeyB));
    }
    return pubB;
}
void ClientWindow::GettingAgreement(QString Username, CryptoPP::SecByteBlock NewKey)
{
    if (dh.Agree(MySecretKey, PrivNumb, NewKey))
    {
        ui->DebugWindow->append("Success key exchange with: " + Username);
        SearchPeerByName(Username)->SetSessionKey(SecBBToString(MySecretKey));
    }

}

QString ClientWindow::Encrypt(QString &Message, QString Key)
{
    const ByteBlock key = hex_to_bytes(Key.toStdString());
    std::string HexString (string_to_hex(Message.toStdString()));
    if(HexString.length()%32 != 0) //==16 bytes
        {
            BlockAddition(HexString);
        }
    ByteBlock ByteMessage = hex_to_bytes(HexString);
    Kuznyechik Encryptor(key);
    std::vector<ByteBlock> Vect16BytesBlocks = split_blocks(ByteMessage, 16);
    HexString.clear();
    for(size_t i = 0; i < Vect16BytesBlocks.size(); ++i)
        {
            Encryptor.encrypt(Vect16BytesBlocks[i], Vect16BytesBlocks[i]);
            HexString += hex_representation(Vect16BytesBlocks[i]);
        }
    Message = QString::fromStdString(HexString);
    return Message;
}
QString ClientWindow::Decrypt(QString &Message, QString Key)
{
    const ByteBlock BBKEY = hex_to_bytes(Key.toStdString());
    std::string BufString = (Message.toStdString());
    ByteBlock ByteMessage = hex_to_bytes(BufString);
    Kuznyechik decryptor(BBKEY);
    std::vector<ByteBlock> Vect16BytesMessages = split_blocks(ByteMessage, 16); //16-bytes blocks
    BufString.clear(); //using for write encrypted data in hex
    for(size_t i = 0; i < Vect16BytesMessages.size(); ++i)
    {
        decryptor.decrypt(Vect16BytesMessages[i], Vect16BytesMessages[i]);
        BufString += hex_representation(Vect16BytesMessages[i]);
    }
    Message = QString::fromStdString(hex_to_string(BufString));
    return Message;
}
void ClientWindow::on_checkBox_toggled(bool checked)
{
    Private = checked;
}
