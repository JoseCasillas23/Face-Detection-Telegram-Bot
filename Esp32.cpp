#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mat2qimage.h"
#include<opencv4/opencv2/cvconfig.h>
#include<opencv2/core/core.hpp>
#include<opencv2/ml/ml.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/video/background_segm.hpp>
#include<opencv2/videoio.hpp>
#include<opencv2/imgcodecs.hpp>
#include<QTimer>
#include<QDebug>
#include<opencv2/objdetect.hpp>
#include<QDateTime>
#include<QSqlDatabase>
#include<QtSql>
#include<QSqlQuery>
#include<QSqlError>
#include<QtNetwork/QtNetwork>

using namespace cv;
VideoCapture camara(0);
QSqlDatabase baseDatos = QSqlDatabase::addDatabase("QMYSQL");
QString nombreArchivo = "../CaraFrontal.xml";
CascadeClassifier detector_caras;
int tiempoEspera = 30;
int tiempoEsperaMensaje = 10;
int contCaras = 0;
bool bandCara = false;
bool bandMensaje = false;
long int tiempoD = 0;
long int tiempoP = 0;
long int tiempoM = 0;

QString MainWindow::conexionWeb(QString url){
    QString respuesta = "error 1";

    //Paso # 1 - Verificar que la red, por donde se enviará el
    //mensaje, sea un red funcional.
    QNetworkInterface redConectada = QNetworkInterface::interfaceFromName("wlo1");
    QList<QNetworkAddressEntry> lista = redConectada.addressEntries();

    //Paso # 2 - Verificar que la red este activa
    if(!lista.empty()){
        QNetworkAddressEntry IP = lista.first();
        qDebug() << "Red activa: " << IP.ip();

        //Crear el mensaje HTML/HTTP

        QNetworkRequest solicitud;
        QNetworkAccessManager *clienteWeb = new QNetworkAccessManager();
        QUrl servidor(url.toUtf8().constData());

    //Paso # 3 - Verificar que la url sea valida
    if(servidor.isValid()){
        qDebug() << "Servidor valido ";

    //Paso # 4 - Formar el mensaje HTTP
        solicitud.setUrl(servidor);
        solicitud.setRawHeader(QByteArray("User-Agent"),QByteArray("bot"));
        solicitud.setRawHeader(QByteArray("Connection"),QByteArray("close"));

     //Paso # 5 - Realizar la conexion
        QNetworkReply *conexionServidor = clienteWeb->get(QNetworkRequest(servidor));


             }
        else{
            respuesta = "error 3" ;
        }
    }
    else{
        respuesta = "Error 2";

    }

    return respuesta;
}

void MainWindow::cronometro(){
    qDebug() <<"CI: " << contCaras << " encontre cara:" << bandCara;

    QDateTime tiempoActual = QDateTime::currentDateTime();
    tiempoP = tiempoActual.toSecsSinceEpoch();

    if((tiempoP >= (tiempoD+tiempoEspera)) && bandCara){
        bandCara = false;

    }
    Mat imagen;
    camara >> imagen;
    Mat imagenChica;
    if(!imagen.empty()){

        cv::resize(imagen, imagenChica, Size(640,360),0,0,0);

        if((tiempoP >= (tiempoM+tiempoEsperaMensaje)) && bandMensaje){
            bandMensaje = false;
            conexionWeb("http://192.168.0.16/DESACTIVAR");
        }
        if(bandMensaje){
            cv::putText(imagenChica, "Intruso Detectado!", Point(30,30),0,1, Scalar(255,0,0),1,8,false);

        }


        Mat GRIS;
        cv::cvtColor( imagenChica, GRIS, COLOR_BGR2GRAY );
        cv::equalizeHist(GRIS,GRIS);

        std::vector<Rect> carasEncontradas;
        detector_caras.detectMultiScale( GRIS, carasEncontradas, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );
        int numeroCaras = (int)carasEncontradas.size();

        ui->lcdNumber->display(numeroCaras);
        if(carasEncontradas.size() > 0 && !bandCara){
            contCaras++;

            if(contCaras == 3){
                contCaras = 0;
                //encontre una cara
                bandCara = true;
                bandMensaje = true;

                QDateTime tiempo = QDateTime::currentDateTime();
                tiempoD = tiempo.toSecsSinceEpoch();
                tiempoM = tiempo.toSecsSinceEpoch();

                QString nombreImagen = QString::number(tiempoD)+".jpg";
                QString ruta = "/home/victor/Descargas/"+nombreImagen;
               imwrite(ruta.toUtf8().constData(),imagen);

                        if(baseDatos.open()){
                              qDebug()<<"Base de Datos Abiera";

                              //Paso # 1. Crear el comando SQL para insertar los datos.
                              QString comandoTexto = "INSERT INTO actividad7(imagen) VALUES(?)";
                              QSqlQuery comandoSQL;
                              comandoSQL.prepare(comandoTexto);
                              comandoSQL.addBindValue(nombreImagen);

                              if(comandoSQL.exec()){
                                  qDebug() << "El dato se inserto correctamente";
                              }
                conexionWeb("http://192.168.0.16/ACTIVAR");
                        }



            }
        }
        else{
            contCaras--;
            if(contCaras < 0){
                contCaras = 0;
            }
        }
        for ( size_t i = 0; i < carasEncontradas.size(); i++ ){
                   Point center( carasEncontradas[i].x + carasEncontradas[i].width/2, carasEncontradas[i].y + carasEncontradas[i].height/2 );
                   ellipse( imagenChica, center, Size( carasEncontradas[i].width/2, carasEncontradas[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

         }

    }


    QImage img = Mat2QImage(imagenChica);
    QPixmap img2 = QPixmap::fromImage(img);
    ui->label->clear();
    ui->label->setPixmap(img2);

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

    QTimer *cronos = new QTimer(this);
    connect(cronos, SIGNAL(timeout()),this,SLOT(cronometro()));
    cronos->start(50);

    if(!detector_caras.load(nombreArchivo.toUtf8().constData())){
        qDebug() << "Error al cargar el archivo";
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

