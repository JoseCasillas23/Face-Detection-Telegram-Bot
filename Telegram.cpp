#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "telegrambot.h"
#include "mat2qimage.h"
#include<QDebug>
#include <QDirIterator>
#include<QDebug>
#include<QFile>
#include <QTextStream>
#include<QtNetwork/QtNetwork>
#include<QTimer>
#include<QSqlDatabase>
#include<QtSql>
#include<QSqlQuery>
#include<QSqlError>



TelegramBot interfacesbot("5323553233:AAEUYuhUn2DUecZZWmHUdS3hAB7n9Y1Egm8");

bool bandCS = false;
bool bandUP = false;

QSqlDatabase baseDatos = QSqlDatabase::addDatabase("QMYSQL");
QString claveS = "";
QString idUP = "";
QVariant idChatID;
QString usuarioP = "";
bool guardarUsuario = false;
TelegramBotMessage msgSent;

QString GetRandomString()
{ //Generar valor de la clave para el primer registro de forma aleatoria

  const QString possibleCharacters("5323553233:AAEUYuhUn2DUecZZWmHUdS3hAB7n9Y1Egm8");
  const int randomStringLength = 10; // assuming you want random strings of 12 characters

  QString randomString;
  for(int i=0; i<randomStringLength; ++i)
  {
      int index = QRandomGenerator::global()->generate() % possibleCharacters.length();
      QChar nextChar = possibleCharacters.at(index);
      randomString.append(nextChar);
  }
  return randomString;
}



void MainWindow::Temporizador(){
    //En caso de registro de primer usuario
    if(guardarUsuario){

        //guardar info del usuario
        if(baseDatos.open()){
              qDebug()<<"Base de Datos abierta correctamente";

              //Paso # 1. Crear el comando SQL para insertar los datos.
              QString comandoTexto = "INSERT INTO usuario7(chatID, Usuario) VALUES(?,?)";
              QSqlQuery comandoSQL;
              comandoSQL.prepare(comandoTexto);
              comandoSQL.addBindValue(idUP);
              comandoSQL.addBindValue(usuarioP);

              if(comandoSQL.exec()){
                  guardarUsuario = false;
                  qDebug() << "El dato se inserto correctamente";

              }
        }
    }

}

void MainWindow::enviarMensajeTelegram(){
    //Corroborar que ya existe un usuario registrado
    if(bandUP){

    if(baseDatos.open()){
    QString comandoTexto = "SELECT * FROM actividad7 WHERE enviarImagen = 'N' ORDER BY id DESC LIMIT 1";
    QSqlQuery comandoSQL;
    comandoSQL.prepare(comandoTexto);

    if(comandoSQL.exec()){
        while(comandoSQL.next()){
             QString nombreImagen = comandoSQL.value(1).toString();
             int id =  comandoSQL.value(0).toInt();
            QString hora = nombreImagen;
                    hora.replace(".jpg","");
                    uint epoch = hora.toInt();
                    QDateTime Fecha;
                    Fecha=Fecha.fromTime_t(epoch);
                    QString dia = Fecha.date().toString("yyyy-MM-dd")+"T"+Fecha.time().toString()+"Z";

             qDebug() << "Imagen: " <<nombreImagen;
             QString mensaje = "Se detecto un intruso a las "+dia;

             QString directorioMasImagen = "/home/victor/Descargas/"+nombreImagen;
             interfacesbot.sendPhoto(idChatID, directorioMasImagen, mensaje);

             QString comando2 = "UPDATE actividad7 SET enviarImagen = 'S' WHERE id = ?";
             QSqlQuery comandoSQL2;
             comandoSQL2.prepare(comando2);
             comandoSQL2.addBindValue(id);
             if(comandoSQL2.exec()){

             }
              }


    }
    else{
               qDebug() << "Error al ejectuar el comando";
           }
 }
    else{
        qDebug()<<"Error al abrir la base de datos";
    }}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    baseDatos.setHostName("localhost");
    baseDatos.setPort(3306);
    baseDatos.setDatabaseName("Interfaces");
    baseDatos.setUserName("admin");
    baseDatos.setPassword("hola1234");

   if(baseDatos.open()){
   QString comandoTexto = "SELECT * FROM usuario7 WHERE 1";
   QSqlQuery comandoSQL;
   comandoSQL.prepare(comandoTexto);
       if(comandoSQL.exec()){

           while(comandoSQL.next()){
                idUP = comandoSQL.value(1).toString();
                qDebug() << "UP: " <<idUP;
               idChatID = QVariant::fromValue(idUP);
                   bandUP=true;
           }

       }
       else{
       qDebug() << "Error al ejectuar el comando";
       }
    }   
    else{
           qDebug()<<"Error al abrir la base de datos";
        }


     if(idUP != ""){
         interfacesbot.sendMessage(idChatID,
                         "Bienvenido",
                         0,
                         TelegramBot::NoFlag,
                         TelegramKeyboardRequest(),
                         &msgSent);
     }


    QTimer *cronometro = new QTimer(this);
    connect(cronometro, SIGNAL(timeout()), this, SLOT(Temporizador()));
    cronometro->start(50);

    QTimer *cronometro2 = new QTimer(this);
    connect(cronometro2, SIGNAL(timeout()), this, SLOT(enviarMensajeTelegram()));
    cronometro2->start(10000);

        QObject::connect(&interfacesbot, &TelegramBot::newMessage, [&interfacesbot](TelegramBotUpdate update) {
            // only handle Messages
            if(update->type != TelegramBotMessageType::Message) return;
            // simplify message access
            TelegramBotMessage& message = *update->message;
            // send message (Format: Normal)

            QString mensajeRecibido = message.text;
            QVariant identificador = QVariant::fromValue(message.chat.id);
            QString identificadorUsuario = identificador.toString();
            qDebug() << "Identificador:" << identificadorUsuario;

            if(!bandUP){
                QString nombreU = "";
                QString claveU = "";
                //Averiguar primero si el usuario escribio su nombre y clave
                //nombre:RUBEN ESTRADA MARMOLEJO clave:abde123456
                int existeNombre = mensajeRecibido.indexOf("nombre:");
                int existeClave = mensajeRecibido.indexOf("clave:");

                if(existeNombre >=0 && existeClave >= 0){
                    claveU = mensajeRecibido.mid(existeClave+6);
                    qDebug() << "Contraseña:" << claveU;
                    nombreU = mensajeRecibido.mid(existeNombre+7, existeClave-7);
                    qDebug() << "Usuario:" << nombreU;
                }

                if(nombreU == "" && claveU == ""){
                    if(!bandCS){
                        claveS = GetRandomString();
                        bandCS = true;
                    }
                    QString claveR = "Contraseña: "+claveS;
                    interfacesbot.sendMessage(message.chat.id,
                                    claveR,
                                    0,
                                    TelegramBot::NoFlag,
                                    TelegramKeyboardRequest(),
                                    &msgSent);
                    interfacesbot.sendMessage(message.chat.id,
                                    "Ingresa la informacion con el siguiente formato: nombre:(Usuario) clave:(resivida)",
                                    0,
                                    TelegramBot::NoFlag,
                                    TelegramKeyboardRequest(),
                                    &msgSent);

                }
                else{
                    if(claveU == claveS){
                        guardarUsuario = true;
                        bandUP = true;
                        idUP = identificadorUsuario;
                        usuarioP = nombreU;
                    }else{
                        interfacesbot.sendMessage(message.chat.id,
                                        "Contraseña Incorrecta",
                                        0,
                                        TelegramBot::NoFlag,
                                        TelegramKeyboardRequest(),
                                        &msgSent);
                    }

                }
            }
            else{
                if(identificadorUsuario == idUP ){

                        interfacesbot.sendMessage(message.chat.id,
                                        "Usuario Correcto",
                                        0,
                                        TelegramBot::NoFlag,
                                        TelegramKeyboardRequest(),
                                        &msgSent);

                }
                else{
                    QString claveR = "Acceso no autorizado";
                    interfacesbot.sendMessage(message.chat.id,
                                    claveR,
                                    0,
                                    TelegramBot::NoFlag,
                                    TelegramKeyboardRequest(),
                                    &msgSent);
                }
            }

        });
        interfacesbot.startMessagePulling();

}

MainWindow::~MainWindow()
{
    delete ui;
}

