#ifndef ISSUECREATOR_H
#define ISSUECREATOR_H

#include <QDialog>
#include <QProcess>
#include <QNetworkConfigurationManager>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
namespace Ui {
class issuecreator;
}
//! Класс, предоставляющий интерфейc для создания Github Issues
class issuecreator : public QDialog
{
    Q_OBJECT

public:
  /*!
    \brief Конструктор по умолчанию
  */
    explicit issuecreator(QWidget *parent = 0);
    /*!
      \brief Конструктор для случая с выделенным текстом.

      Весь выделенный текст в окне сообщений будет перенесен в панель ввода Description.
    */
    issuecreator(const QString& selectedText, QWidget* parent = 0);
    /*!
     \brief Проверка интернет-соединения.
     Проверка соединения с помощью стандартных средств Qt.
     \return False если отсутствует интернет-соединение. Иначе - true.
    */
    bool IsInternetConnected();
    /*!
     \brief Запись в config-файл токена.
     \param[in] Token  Токен для записи в файл.
    */
    void WriteTokenToFile(const std::string& Token);
    /*!
     \brief Чтение токена из config-файла.
     \return Строка - токен.
    */
    QString ReadTokenFromFile();
    /*!
     \brief Парсинг полученных данных от Github и получение токена из них.
     \param[in] Data Данные от Github.
     \return Строка - токен.
    */
    QString ParseToken(QString Data);
    /*!
     \brief Отправка запроса на получение Github токена.
     \param[in] Login Логин Github.
     \param[in] Pass Пароль от аккаунта Github.
    */
    QString GetGithubToken(const QString& Login, const QString& Pass);

      ~issuecreator();

private slots:
/*!
 \brief Слот, отвечающий за отправку запроса на Github с целью создания Issue
*/
    void on_Send_clicked();
private:

    bool isTokenWrited = false; //!< Флаг, сигнализирующий о наличии токена в файле config
    QString Token; //!< Токен
    QString IssueDescription; //!< Описание проблемы
    Ui::issuecreator *ui;
};

#endif // ISSUECREATOR_H
