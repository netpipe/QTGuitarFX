#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QVariant"
#include <QTimer>
#include "FX.cpp"
#include "myAlsa.cpp"
//#include "myJack.cpp"
#include <QFile>
#include <QDirIterator>
#include <QDebug>

QTimer *timer;
bool useJack=true;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateVolume()));


    QDirIterator it("./Resource/themes/", QStringList() << "*.qss", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()){
      //  QFileInfo fileInfo(f.fileName());
        ui->cmbTheme->addItem(it.next().toLatin1());
    }


    QFile MyFile("themes.txt");
    if(MyFile.exists()){
        MyFile.open(QIODevice::ReadWrite);
        QTextStream in (&MyFile);
        QString line;
        QStringList list;
         //   QList<QString> nums;
        QStringList nums;
        QRegExp rx("[:]");
        line = in.readLine();
  QString stylesheet;
        if (line.contains(":")) {
            list = line.split(rx);
                qDebug() << "theme" <<  list.at(1).toLatin1();
                stylesheet =  list.at(1).toLatin1();

                MyFile.close();
        }

  fileName=stylesheet;
        QFile file(stylesheet);
        file.open(QIODevice::Text | QIODevice::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());

        qApp->setStyleSheet(styleSheet);

    }
  loaded=true;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateVolume()
{
    mutex.lock();
    float v=outVolume;
    mutex.unlock();
    ui->VolumeProgr->setValue(v);

    mutex.lock();
    float iv=inVolume;
    mutex.unlock();
    ui->VolumeProgr_2->setValue(iv);
}
void SoundThread::run()
{
    //use alsa API
    myAlsa ma;
    this->setPriority(HighestPriority);
    int i = ma.Mic_to_Spk();
    //int i = ma.Play_sine();


}
SoundThread sound_thread;

void MainWindow::on_pushButton_clicked()
{

    //use JACK API
    myAlsa ma ;

    if (ui->pushButton->text()!="Start")
    {
        if (useJack)
        {
            // tell JACK to stop processing the client
            jack_deactivate(client);

            // close the client
            jack_client_close(client);
            ui->pushButton->setText("Start");
            return;
        }

        // mutex protect
        mutex.lock();
        STOP=true;
        mutex.unlock();
        ////////////////

        sound_thread.wait();
        if (timer->isActive())
        {
            timer->stop();
            ui->VolumeProgr->setValue(0);
        }
        ui->pushButton->setText("Start");

    }
    else
    {
        if (useJack)
        {
            myJack();
            ui->pushButton->setText("Stop");
            return;
        }
        First_Run=true;
        //mutex protect
        mutex.lock();
        STOP=false;
        mutex.unlock();
        ///////////////
        sound_thread.start();
        if (!timer->isActive())
        {
            timer->start(20);
            ui->VolumeProgr->setValue(0);
        }

        ui->pushButton->setText("Stop");
    }

}

void MainWindow::on_pushButton_2_clicked()
{
    // JACK
    if (useJack)
    {
        if (ui->pushButton->text()=="Stop")
        {
            // tell JACK to stop processing the client
            jack_deactivate(client);

            // close the client
            jack_client_close(client);
        }
        this->close();
        return;
    }
    //alsa
    mutex.lock();
    STOP=true;
    mutex.unlock();
    sound_thread.wait();
    if (timer->isActive())
    {
        timer->stop();
        ui->VolumeProgr->setValue(0);
    }
    this->close();
}

bool lock = false;
void MainWindow::on_DGainDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->DGainDial->value();
    ui->DGainLN->setText(v.toString());
}

void MainWindow::on_dial_2_valueChanged(int value)
{

    if (lock) return ;
    QVariant v = (float)ui->dial_2->value();
    ui->DClipLN->setText(v.toString());
}

void MainWindow::on_DClipFDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->DClipFDial->value();
    ui->DCFactLN->setText(v.toString());
}

void MainWindow::on_DGainLN_editingFinished()
{
}

void MainWindow::on_DClipLN_editingFinished()
{
}

void MainWindow::on_DCFactLN_editingFinished()
{
}

void MainWindow::on_TDepthDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->TDepthDial->value();
    ui->TDepthLN->setText(v.toString());
}

void MainWindow::on_TPeriodDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->TPeriodDial->value();
    ui->TPeriodLN->setText(v.toString());
}

void MainWindow::on_TDepthLN_editingFinished()
{
}

void MainWindow::on_TPeriodLN_editingFinished()
{
}

void MainWindow::on_DGainLN_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->DGainLN->text();
    if (v.toInt()>ui->DGainDial->maximum())
    {
        v=ui->DGainDial->maximum();
        ui->DGainLN->setText(v.toString());
    }
    if (v.toInt()<ui->DGainDial->minimum())
    {
        v=ui->DGainDial->minimum();
        ui->DGainLN->setText(v.toString());
    }
    ui->DGainDial->setValue(v.toInt(0));
    float g=ui->DGainDial->value();
    float f = (float)(1+(g/100));
    lock = false;
    mutex.lock();
    GainFactor_Dist=f;
    mutex.unlock();

}

void MainWindow::on_DClipLN_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->DClipLN->text();
    if (v.toInt()>ui->dial_2->maximum())
    {
        v=ui->dial_2->maximum();
        ui->DClipLN->setText(v.toString());
    }
    if (v.toInt()<ui->dial_2->minimum())
    {
        v=ui->dial_2->minimum();
        ui->DClipLN->setText(v.toString());
    }
    ui->dial_2->setValue(v.toInt(0));
    lock = false;
    int i =ui->dial_2->value();
    mutex.lock();
    Threshold_Dist=i;
    mutex.unlock();
}

void MainWindow::on_DCFactLN_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->DCFactLN->text();
    if (v.toInt()>ui->DClipFDial->maximum())
    {
        v=ui->DClipFDial->maximum();
        ui->DCFactLN->setText(v.toString());
    }
    if (v.toInt()<ui->DClipFDial->minimum())
    {
        v=ui->DClipFDial->minimum();
        ui->DCFactLN->setText(v.toString());
    }
    ui->DClipFDial->setValue(v.toInt(0));
    lock= false;
    float i = (float) ui->DClipFDial->value();
    mutex.lock();
    ClippingFactor_Dist=i/10;
    mutex.unlock();
}

void MainWindow::on_TDepthLN_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->TDepthLN->text();
    if (v.toInt()>ui->TDepthDial->maximum())
    {
        v=ui->TDepthDial->maximum();
        ui->TDepthLN->setText(v.toString());
    }
    if (v.toInt()<ui->TDepthDial->minimum())
    {
        v=ui->TDepthDial->minimum();
        ui->TDepthLN->setText(v.toString());
    }
    ui->TDepthDial->setValue(v.toInt());
    lock = false;
    mutex.lock();
    float f = ui->TDepthDial->value();
    Depth_Tr=f/100;
    mutex.unlock();
}

void MainWindow::on_TPeriodLN_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->TPeriodLN->text();
    if (v.toInt()>ui->TPeriodDial->maximum())
    {
        v=ui->TPeriodDial->maximum();
        ui->TPeriodLN->setText(v.toString());
    }
    if (v.toInt()<ui->TPeriodDial->minimum())
    {
        v=ui->TPeriodDial->minimum();
        ui->TPeriodLN->setText(v.toString());
    }
    ui->TPeriodDial->setValue(v.toInt());
    lock = false;
    mutex.lock();
    float f = ui->TPeriodDial->value();
    Period_Tr=f/10000;
    mutex.unlock();
}

void MainWindow::on_checkBox_toggled(bool checked)
{
    mutex.lock();
    Enabled_Dist=checked;
    mutex.unlock();
}

void MainWindow::on_Check_Tr_toggled(bool checked)
{
    mutex.lock();
    Enabled_Tr=checked;
    mutex.unlock();
}

void MainWindow::on_Check_D_toggled(bool checked)
{
    if (checked)
    {
        // Delay Buffer - Max_Delay_Time seconds long//
        Delay_Buffer = (float *) malloc((Max_Delay_Time)*SamplingRate*2/*2 channels*/*sizeof(float) /*4 bytes per floating point number*/);
        for (int i=0;i<(Max_Delay_Time)*SamplingRate*2;i++)
        {
            Delay_Buffer[i]=0;
        }
        // Fill Delay_Buffer with zeros
    }
    mutex.lock();
    Enabled_D=checked;
    mutex.unlock();
    if (!checked)
    {
        usleep(100);
        mutex.lock();
        free(Delay_Buffer);
        mutex.unlock();
    }
}

void MainWindow::on_DTime_Dial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->DTime_Dial->value();
    ui->Dtime_LE->setText(v.toString());
}

void MainWindow::on_DFeedBack_Dial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->DFeedBack_Dial->value();
    ui->DFeedBack_LE->setText(v.toString());
}

void MainWindow::on_DMix_Dial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->DMix_Dial->value();
    ui->DMix_LE->setText(v.toString());
}

void MainWindow::on_Dtime_LE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->Dtime_LE->text();
    if (v.toInt()>ui->DTime_Dial->maximum())
    {
        v=ui->DTime_Dial->maximum();
        ui->Dtime_LE->setText(v.toString());
    }
    if (v.toInt()<ui->DTime_Dial->minimum())
    {
        v=ui->DTime_Dial->minimum();
        ui->Dtime_LE->setText(v.toString());
    }
    ui->DTime_Dial->setValue(v.toInt());
    lock = false;
    float f = ui->DTime_Dial->value();
    mutex.lock();
    DTime_D=f/1000;
    mutex.unlock();
}

void MainWindow::on_DFeedBack_LE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->DFeedBack_LE->text();
    if (v.toInt()>ui->DFeedBack_Dial->maximum())
    {
        v=ui->DFeedBack_Dial->maximum();
        ui->DFeedBack_LE->setText(v.toString());
    }
    if (v.toInt()<ui->DFeedBack_Dial->minimum())
    {
        v=ui->DFeedBack_Dial->minimum();
        ui->DFeedBack_LE->setText(v.toString());
    }
    ui->DFeedBack_Dial->setValue(v.toInt());
    float f = ui->DFeedBack_Dial->value();
    lock = false;
    mutex.lock();
    FeedBack_D=(f/100);
    mutex.unlock();
}

void MainWindow::on_DMix_LE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->DMix_LE->text();
    if (v.toInt()>ui->DMix_Dial->maximum())
    {
        v=ui->DMix_Dial->maximum();
        ui->DMix_LE->setText(v.toString());
    }
    if (v.toInt()<ui->DMix_Dial->minimum())
    {
        v=ui->DMix_Dial->minimum();
        ui->DMix_LE->setText(v.toString());
    }
    ui->DMix_Dial->setValue(v.toInt());
    float f = ui->DMix_Dial->value();
    lock = false;
    mutex.lock();
    Mix_D=(f/100);
    mutex.unlock();
}

void MainWindow::on_verticalSlider_valueChanged(int value)
{
    mutex.lock();
    Volume=value*2;
    mutex.unlock();
}

void MainWindow::on_checkBox_7_toggled(bool checked)
{
    if (checked)
    {
        // Vibrato Buffer - 5 seconds long - for now //
        Vibrato_Buffer = (float *) malloc(5*SamplingRate*2/*2 channels*/*sizeof(float) /*4 bytes per floating point number*/);
        for (int i=0;i<5*SamplingRate*2;i++)
        {
            Vibrato_Buffer[i]=0;
        }
        // Fill Delay_Buffer with zeros
    }
    mutex.lock();
    Enabled_Vib=checked;
    mutex.unlock();
    if (!checked)
    {
        usleep(100);
        mutex.lock();
        free(Vibrato_Buffer);
        mutex.unlock();
    }
}

void MainWindow::on_checkBox_8_toggled(bool checked)
{
    if (checked)
    {
        // Shifter Buffer - 5 seconds long - for now //
        Shifter_Buffer = (float *) malloc(5*SamplingRate*2/*2 channels*/*sizeof(float) /*4 bytes per floating point number*/);
        for (int i=0;i<5*SamplingRate*2;i++)
        {
            Shifter_Buffer[i]=0;
        }
        // Fill Delay_Buffer with zeros
    }
    mutex.lock();
    Enabled_Sh=checked;
    mutex.unlock();
    if (!checked)
    {
        usleep(100);
        mutex.lock();
        free(Shifter_Buffer);
        mutex.unlock();
    }
}

void MainWindow::on_VibSpeedDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->VibSpeedDial->value();
    ui->VibSpeedLE->setText(v.toString());
}

void MainWindow::on_VibDepthDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->VibDepthDial->value();
    ui->VibDepthLE->setText(v.toString());
}

void MainWindow::on_VibFeedbackDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->VibFeedbackDial->value();
    ui->VibFeedBackLE->setText(v.toString());
}

void MainWindow::on_VibWetDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->VibWetDial->value();
    ui->VibWetLE->setText(v.toString());
}

void MainWindow::on_VibDryDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->VibDryDial->value();
    ui->VibDryLE->setText(v.toString());
}

void MainWindow::on_VibSpeedLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->VibSpeedLE->text();
    if (v.toInt()>ui->VibSpeedDial->maximum())
    {
        v=ui->VibSpeedDial->maximum();
        ui->VibSpeedLE->setText(v.toString());
    }
    if (v.toInt()<ui->VibSpeedDial->minimum())
    {
        v=ui->VibSpeedDial->minimum();
        ui->VibSpeedLE->setText(v.toString());
    }
    ui->VibSpeedDial->setValue(v.toInt());
    float f = ui->VibSpeedDial->value();
    lock = false;
    mutex.lock();
    Speed_Vib=(f/10);
    mutex.unlock();
}

void MainWindow::on_VibDepthLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->VibDepthLE->text();
    if (v.toInt()>ui->VibDepthDial->maximum())
    {
        v=ui->VibDepthDial->maximum();
        ui->VibDepthLE->setText(v.toString());
    }
    if (v.toInt()<ui->VibDepthDial->minimum())
    {
        v=ui->VibDepthDial->minimum();
        ui->VibDepthLE->setText(v.toString());
    }
    ui->VibDepthDial->setValue(v.toInt());
    float f = ui->VibDepthDial->value();
    lock = false;
    mutex.lock();
    Depth_Vib=(f/100000);
    mutex.unlock();
}

void MainWindow::on_VibFeedBackLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->VibFeedBackLE->text();
    if (v.toInt()>ui->VibFeedbackDial->maximum())
    {
        v=ui->VibFeedbackDial->maximum();
        ui->VibFeedBackLE->setText(v.toString());
    }
    if (v.toInt()<ui->VibFeedbackDial->minimum())
    {
        v=ui->VibFeedbackDial->minimum();
        ui->VibFeedBackLE->setText(v.toString());
    }
    ui->VibFeedbackDial->setValue(v.toInt());
    float f = ui->VibFeedbackDial->value();
    lock = false;
    mutex.lock();
    FeedBack_Vib=(f/100);
    mutex.unlock();
}

void MainWindow::on_VibWetLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->VibWetLE->text();
    if (v.toInt()>ui->VibWetDial->maximum())
    {
        v=ui->VibWetDial->maximum();
        ui->VibWetLE->setText(v.toString());
    }
    if (v.toInt()<ui->VibWetDial->minimum())
    {
        v=ui->VibWetDial->minimum();
        ui->VibWetLE->setText(v.toString());
    }
    ui->VibWetDial->setValue(v.toInt());
    float f = ui->VibWetDial->value();
    lock = false;
    mutex.lock();
    Wet_Vib=(f/100);
    mutex.unlock();
}

void MainWindow::on_VibDryLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->VibDryLE->text();
    if (v.toInt()>ui->VibDryDial->maximum())
    {
        v=ui->VibDryDial->maximum();
        ui->VibDryLE->setText(v.toString());
    }
    if (v.toInt()<ui->VibDryDial->minimum())
    {
        v=ui->VibDryDial->minimum();
        ui->VibDryLE->setText(v.toString());
    }
    ui->VibDryDial->setValue(v.toInt());
    float f = ui->VibDryDial->value();
    lock = false;
    mutex.lock();
    Dry_Vib=(f/100);
    mutex.unlock();
}

void MainWindow::on_checkBox_4_toggled(bool checked)
{
    if (checked)
    {
        // Shifter Buffer - 5 seconds long - for now //
        Chorus_Buffer = (float *) malloc(5*SamplingRate*2/*2 channels*/*sizeof(float) /*4 bytes per floating point number*/);
        for (int i=0;i<5*SamplingRate*2;i++)
        {
            Chorus_Buffer[i]=0;
        }
        // Fill Delay_Buffer with zeros
    }
    mutex.lock();
    Enabled_Ch=checked;
    mutex.unlock();
    if (!checked)
    {
        usleep(100);
        mutex.lock();
        free(Chorus_Buffer);
        mutex.unlock();
    }
}

void MainWindow::on_ChSpeedDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ChSpeedDial->value();
    ui->ChSpeedLE->setText(v.toString());
}

void MainWindow::on_ChDepthDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ChDepthDial->value();
    ui->ChDepthLE->setText(v.toString());
}

void MainWindow::on_ChFeedbackDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ChFeedbackDial->value();
    ui->ChFeedbackLE->setText(v.toString());
}

void MainWindow::on_ChWetDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ChWetDial->value();
    ui->ChWetLE->setText(v.toString());
}

void MainWindow::on_ChDryDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ChDryDial->value();
    ui->ChDryLE->setText(v.toString());
}

void MainWindow::on_ChSpeedLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ChSpeedLE->text();
    if (v.toInt()>ui->ChSpeedDial->maximum())
    {
        v=ui->ChSpeedDial->maximum();
        ui->ChSpeedLE->setText(v.toString());
    }
    if (v.toInt()<ui->ChSpeedDial->minimum())
    {
        v=ui->ChSpeedDial->minimum();
        ui->ChSpeedLE->setText(v.toString());
    }
    ui->ChSpeedDial->setValue(v.toInt());
    float f = ui->ChSpeedDial->value();
    lock = false;
    mutex.lock();
    Speed_Ch=(f/10);
    mutex.unlock();
}

void MainWindow::on_ChDepthLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ChDepthLE->text();
    if (v.toInt()>ui->ChDepthDial->maximum())
    {
        v=ui->ChDepthDial->maximum();
        ui->ChDepthLE->setText(v.toString());
    }
    if (v.toInt()<ui->ChDepthDial->minimum())
    {
        v=ui->ChDepthDial->minimum();
        ui->ChDepthLE->setText(v.toString());
    }
    ui->ChDepthDial->setValue(v.toInt());
    float f = ui->ChDepthDial->value();
    lock = false;
    mutex.lock();
    Depth_Ch=(f);
    mutex.unlock();
}

void MainWindow::on_ChFeedbackLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ChFeedbackLE->text();
    if (v.toInt()>ui->ChFeedbackDial->maximum())
    {
        v=ui->ChFeedbackDial->maximum();
        ui->ChFeedbackLE->setText(v.toString());
    }
    if (v.toInt()<ui->ChFeedbackDial->minimum())
    {
        v=ui->ChFeedbackDial->minimum();
        ui->ChFeedbackLE->setText(v.toString());
    }
    ui->ChFeedbackDial->setValue(v.toInt());
    float f = ui->ChFeedbackDial->value();
    lock = false;
    mutex.lock();
    FeedBack_Ch=(f/100);
    mutex.unlock();
}

void MainWindow::on_ChWetLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ChWetLE->text();
    if (v.toInt()>ui->ChWetDial->maximum())
    {
        v=ui->ChWetDial->maximum();
        ui->ChWetLE->setText(v.toString());
    }
    if (v.toInt()<ui->ChWetDial->minimum())
    {
        v=ui->ChWetDial->minimum();
        ui->ChWetLE->setText(v.toString());
    }
    ui->ChWetDial->setValue(v.toInt());
    float f = ui->ChWetDial->value();
    lock = false;
    mutex.lock();
    Wet_Ch=(f/100);
    mutex.unlock();
}

void MainWindow::on_ChDryLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ChDryLE->text();
    if (v.toInt()>ui->ChDryDial->maximum())
    {
        v=ui->ChDryDial->maximum();
        ui->ChDryLE->setText(v.toString());
    }
    if (v.toInt()<ui->ChDryDial->minimum())
    {
        v=ui->ChDryDial->minimum();
        ui->ChDryLE->setText(v.toString());
    }
    ui->ChDryDial->setValue(v.toInt());
    float f = ui->ChDryDial->value();
    lock = false;
    mutex.lock();
    Dry_Ch=(f/100);
    mutex.unlock();
}

void MainWindow::on_VibDepthLE_textEdited(const QString &arg1)
{

}

void MainWindow::on_ShShiftDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ShShiftDial->value();
    ui->ShShiftLE->setText(v.toString());
}

void MainWindow::on_ShFeedbackDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ShFeedbackDial->value();
    ui->ShFeedbackLE->setText(v.toString());
}

void MainWindow::on_ShWetDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ShWetDial->value();
    ui->ShWetLE->setText(v.toString());
}

void MainWindow::on_ShDryDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->ShDryDial->value();
    ui->ShDryLE->setText(v.toString());
}

void MainWindow::on_ShShiftLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ShShiftLE->text();
    if (v.toInt()>ui->ShShiftDial->maximum())
    {
        v=ui->ShShiftDial->maximum();
        ui->ShShiftLE->setText(v.toString());
    }
    if (v.toInt()<ui->ShShiftDial->minimum())
    {
        v=ui->ShShiftDial->minimum();
        ui->ShShiftLE->setText(v.toString());
    }
    ui->ShShiftDial->setValue(v.toInt());
    float f = ui->ShShiftDial->value();
    lock = false;
    mutex.lock();
    Shift_Sh=(f/10000);
    mutex.unlock();
}

void MainWindow::on_ShFeedbackLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ShFeedbackLE->text();
    if (v.toInt()>ui->ShFeedbackDial->maximum())
    {
        v=ui->ShFeedbackDial->maximum();
        ui->ShFeedbackLE->setText(v.toString());
    }
    if (v.toInt()<ui->ShFeedbackDial->minimum())
    {
        v=ui->ShFeedbackDial->minimum();
        ui->ShFeedbackLE->setText(v.toString());
    }
    ui->ShFeedbackDial->setValue(v.toInt());
    float f = ui->ShFeedbackDial->value();
    lock = false;
    mutex.lock();
    FeedBack_Sh=(f/100);
    mutex.unlock();
}

void MainWindow::on_ShWetLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ShWetLE->text();
    if (v.toInt()>ui->ShWetDial->maximum())
    {
        v=ui->ShWetDial->maximum();
        ui->ShWetLE->setText(v.toString());
    }
    if (v.toInt()<ui->ShWetDial->minimum())
    {
        v=ui->ShWetDial->minimum();
        ui->ShWetLE->setText(v.toString());
    }
    ui->ShWetDial->setValue(v.toInt());
    float f = ui->ShWetDial->value();
    lock = false;
    mutex.lock();
    Wet_Sh=(f/100);
    mutex.unlock();
}

void MainWindow::on_ShDryLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->ShDryLE->text();
    if (v.toInt()>ui->ShDryDial->maximum())
    {
        v=ui->ShDryDial->maximum();
        ui->ShDryLE->setText(v.toString());
    }
    if (v.toInt()<ui->ShDryDial->minimum())
    {
        v=ui->ShDryDial->minimum();
        ui->ShDryLE->setText(v.toString());
    }
    ui->ShDryDial->setValue(v.toInt());
    float f = ui->ShDryDial->value();
    lock = false;
    mutex.lock();
    Dry_Sh=(f/100);
    mutex.unlock();
}

void MainWindow::on_checkBox_10_toggled(bool checked)
{
    mutex.lock();
    asymmetrical_Dist=checked;
    mutex.unlock();
}

void MainWindow::on_verticalSlider_2_valueChanged(int value)
{
    mutex.lock();
    iVolume=value*2;
    mutex.unlock();
}

void MainWindow::on_checkBox_6_toggled(bool checked)
{
    mutex.lock();
    Enable_Filter=checked;
    mutex.unlock();
}

void MainWindow::on_checkBox_3_toggled(bool checked)
{
    FirstRun_Comp = true;
    RunCount_Comp = 0;
    sum_previous_Comp=0;
    x_previous_Comp=0;
    env=0;
    att=0;
    rel=0;
    mutex.lock();
    Enabled_Comp=checked;
    mutex.unlock();
}

void MainWindow::on_CompAttDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->CompAttDial->value();
    ui->CompAttLE->setText(v.toString());
}

void MainWindow::on_CompGateThDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->CompGateThDial->value();
    ui->CompGateThLE->setText(v.toString());
}

void MainWindow::on_CompRelDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->CompRelDial->value();
    ui->CompRelLE->setText(v.toString());
}

void MainWindow::on_CompGateFDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->CompGateFDial->value();
    ui->CompGateFLE->setText(v.toString());
}

void MainWindow::on_CompLimThDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->CompLimThDial->value();
    ui->CompLimThLE->setText(v.toString());
}

void MainWindow::on_CompCompFDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->CompCompFDial->value();
    ui->CompCompFLE->setText(v.toString());
}

void MainWindow::on_CompLimFDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->CompLimFDial->value();
    ui->CompLimFLE->setText(v.toString());
}

void MainWindow::on_CompAttLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->CompAttLE->text();
    if (v.toInt()>ui->CompAttDial->maximum())
    {
        v=ui->CompAttDial->maximum();
        ui->CompAttLE->setText(v.toString());
    }
    if (v.toInt()<ui->CompAttDial->minimum())
    {
        v=ui->CompAttDial->minimum();
        ui->CompAttLE->setText(v.toString());
    }
    ui->CompAttDial->setValue(v.toInt());
    float f = ui->CompAttDial->value();
    lock = false;
    mutex.lock();
    tatt=(f/10000);
    mutex.unlock();
}

void MainWindow::on_CompRelLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->CompRelLE->text();
    if (v.toInt()>ui->CompRelDial->maximum())
    {
        v=ui->CompRelDial->maximum();
        ui->CompRelLE->setText(v.toString());
    }
    if (v.toInt()<ui->CompRelDial->minimum())
    {
        v=ui->CompRelDial->minimum();
        ui->CompRelLE->setText(v.toString());
    }
    ui->CompRelDial->setValue(v.toInt());
    float f = ui->CompRelDial->value();
    lock = false;
    mutex.lock();
    trel=(f/10000);
    mutex.unlock();
}

void MainWindow::on_CompGateThLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->CompGateThLE->text();
    if (v.toInt()>ui->CompGateThDial->maximum())
    {
        v=ui->CompGateThDial->maximum();
        ui->CompGateThLE->setText(v.toString());
    }
    if (v.toInt()<ui->CompGateThDial->minimum())
    {
        v=ui->CompGateThDial->minimum();
        ui->CompGateThLE->setText(v.toString());
    }
    ui->CompGateThDial->setValue(v.toInt());
    float f = ui->CompGateThDial->value();
    lock = false;
    mutex.lock();
    GateThresh_Comp=(f/100);
    mutex.unlock();
}

void MainWindow::on_CompGateFLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->CompGateFLE->text();
    if (v.toInt()>ui->CompGateFDial->maximum())
    {
        v=ui->CompGateFDial->maximum();
        ui->CompGateFLE->setText(v.toString());
    }
    if (v.toInt()<ui->CompGateFDial->minimum())
    {
        v=ui->CompGateFDial->minimum();
        ui->CompGateFLE->setText(v.toString());
    }
    ui->CompGateFDial->setValue(v.toInt());
    float f = ui->CompGateFDial->value();
    lock = false;
    mutex.lock();
    GateFactor_Comp=(f/100);
    mutex.unlock();
}

void MainWindow::on_CompLimThLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->CompLimThLE->text();
    if (v.toInt()>ui->CompLimThDial->maximum())
    {
        v=ui->CompLimThDial->maximum();
        ui->CompLimThLE->setText(v.toString());
    }
    if (v.toInt()<ui->CompLimThDial->minimum())
    {
        v=ui->CompLimThDial->minimum();
        ui->CompLimThLE->setText(v.toString());
    }
    ui->CompLimThDial->setValue(v.toInt());
    float f = ui->CompLimThDial->value();
    lock = false;
    mutex.lock();
    LimiterThresh_Comp=(f/100);
    mutex.unlock();
}

void MainWindow::on_CompCompFLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->CompCompFLE->text();
    if (v.toInt()>ui->CompCompFDial->maximum())
    {
        v=ui->CompCompFDial->maximum();
        ui->CompCompFLE->setText(v.toString());
    }
    if (v.toInt()<ui->CompCompFDial->minimum())
    {
        v=ui->CompCompFDial->minimum();
        ui->CompCompFLE->setText(v.toString());
    }
    ui->CompCompFDial->setValue(v.toInt());
    float f = ui->CompCompFDial->value();
    lock = false;
    mutex.lock();
    Compression_Comp=(f/100);
    mutex.unlock();
}

void MainWindow::on_CompLimFLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->CompLimFLE->text();
    if (v.toInt()>ui->CompLimFDial->maximum())
    {
        v=ui->CompLimFDial->maximum();
        ui->CompLimFLE->setText(v.toString());
    }
    if (v.toInt()<ui->CompLimFDial->minimum())
    {
        v=ui->CompLimFDial->minimum();
        ui->CompLimFLE->setText(v.toString());
    }
    ui->CompLimFDial->setValue(v.toInt());
    float f = ui->CompLimFDial->value();
    lock = false;
    mutex.lock();
    LimiterFactor_Comp=(f/100);
    mutex.unlock();
}

void MainWindow::on_checkBox_5_toggled(bool checked)
{
    if (checked)
    {
    }
    mutex.lock();
    Enabled_Ph=checked;
    mutex.unlock();
}

void MainWindow::on_PhBaseDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->PhBaseDial->value();
    ui->PhBaseLE->setText(v.toString());
}

void MainWindow::on_PhDepthDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->PhDepthDial->value();
    ui->PhDepthLE->setText(v.toString());
}

void MainWindow::on_PhRateDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->PhRateDial->value();
    ui->PhRateLE->setText(v.toString());
}

void MainWindow::on_PhFeedBackDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->PhFeedBackDial->value();
    ui->PhFeedBackLE->setText(v.toString());
}

void MainWindow::on_PhDryDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->PhDryDial->value();
    ui->PhDryLE->setText(v.toString());
}

void MainWindow::on_PhWetDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->PhWetDial->value();
    ui->PhWetLE->setText(v.toString());
}

void MainWindow::on_PhBaseLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->PhBaseLE->text();
    if (v.toInt()>ui->PhBaseDial->maximum())
    {
        v=ui->PhBaseDial->maximum();
        ui->PhBaseLE->setText(v.toString());
    }
    if (v.toInt()<ui->PhBaseDial->minimum())
    {
        v=ui->PhBaseDial->minimum();
        ui->PhBaseLE->setText(v.toString());
    }
    ui->PhBaseDial->setValue(v.toInt());
    float f = ui->PhBaseDial->value();
    lock = false;
    mutex.lock();
    BaseFreq_Ph=(f);
    mutex.unlock();
}

void MainWindow::on_PhDepthLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->PhDepthLE->text();
    if (v.toInt()>ui->PhDepthDial->maximum())
    {
        v=ui->PhDepthDial->maximum();
        ui->PhDepthLE->setText(v.toString());
    }
    if (v.toInt()<ui->PhDepthDial->minimum())
    {
        v=ui->PhDepthDial->minimum();
        ui->PhDepthLE->setText(v.toString());
    }
    ui->PhDepthDial->setValue(v.toInt());
    float f = ui->PhDepthDial->value();
    lock = false;
    mutex.lock();
    OffsetFreq_Ph=BaseFreq_Ph+f;
    mutex.unlock();
}

void MainWindow::on_PhRateLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->PhRateLE->text();
    if (v.toInt()>ui->PhRateDial->maximum())
    {
        v=ui->PhRateDial->maximum();
        ui->PhRateLE->setText(v.toString());
    }
    if (v.toInt()<ui->PhRateDial->minimum())
    {
        v=ui->PhRateDial->minimum();
        ui->PhRateLE->setText(v.toString());
    }
    ui->PhRateDial->setValue(v.toInt());
    float f = ui->PhRateDial->value();
    lock = false;
    mutex.lock();
    SweepRate_Ph=(f/10);
    mutex.unlock();
}

void MainWindow::on_PhFeedBackLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->PhFeedBackLE->text();
    if (v.toInt()>ui->PhFeedBackDial->maximum())
    {
        v=ui->PhFeedBackDial->maximum();
        ui->PhFeedBackLE->setText(v.toString());
    }
    if (v.toInt()<ui->PhFeedBackDial->minimum())
    {
        v=ui->PhFeedBackDial->minimum();
        ui->PhFeedBackLE->setText(v.toString());
    }
    ui->PhFeedBackDial->setValue(v.toInt());
    float f = ui->PhFeedBackDial->value();
    lock = false;
    mutex.lock();
    FeedBack_Ph=(f/100);
    mutex.unlock();
}

void MainWindow::on_PhDryLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->PhDryLE->text();
    if (v.toInt()>ui->PhDryDial->maximum())
    {
        v=ui->PhDryDial->maximum();
        ui->PhDryLE->setText(v.toString());
    }
    if (v.toInt()<ui->PhDryDial->minimum())
    {
        v=ui->PhDryDial->minimum();
        ui->PhDryLE->setText(v.toString());
    }
    ui->PhDryDial->setValue(v.toInt());
    float f = ui->PhDryDial->value();
    lock = false;
    mutex.lock();
    Dry_Ph=(f/100);
    mutex.unlock();
}

void MainWindow::on_PhWetLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->PhWetLE->text();
    if (v.toInt()>ui->PhWetDial->maximum())
    {
        v=ui->PhWetDial->maximum();
        ui->PhWetLE->setText(v.toString());
    }
    if (v.toInt()<ui->PhWetDial->minimum())
    {
        v=ui->PhWetDial->minimum();
        ui->PhWetLE->setText(v.toString());
    }
    ui->PhWetDial->setValue(v.toInt());
    float f = ui->PhWetDial->value();
    lock = false;
    mutex.lock();
    Wet_Ph=(f/100);
    mutex.unlock();
}

void MainWindow::on_checkBox_2_toggled(bool checked)
{
    RunCount_Wah=0;
    mutex.lock();
    Enabled_Wah=checked;
    mutex.unlock();
}

void MainWindow::on_WahBaseDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->WahBaseDial->value();
    ui->WahBaseLE->setText(v.toString());
}

void MainWindow::on_WahDepthDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->WahDepthDial->value();
    ui->WahDepthLE->setText(v.toString());
}

void MainWindow::on_WahRateDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->WahRateDial->value();
    ui->WahRateLE->setText(v.toString());
}

void MainWindow::on_WahFeedBackDial_valueChanged(int value)
{
}

void MainWindow::on_WahDryDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->WahDryDial->value();
    ui->WahDryLE->setText(v.toString());
}

void MainWindow::on_WahWetDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->WahWetDial->value();
    ui->WahWetLE->setText(v.toString());
}

void MainWindow::on_WahRDial_valueChanged(int value)
{
    if (lock) return ;
    QVariant v = (float)ui->WahRDial->value();
    ui->WahRLE->setText(v.toString());
}

void MainWindow::on_WahBaseLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->WahBaseLE->text();
    if (v.toInt()>ui->WahBaseDial->maximum())
    {
        v=ui->WahBaseDial->maximum();
        ui->WahBaseLE->setText(v.toString());
    }
    if (v.toInt()<ui->WahBaseDial->minimum())
    {
        v=ui->WahBaseDial->minimum();
        ui->WahBaseLE->setText(v.toString());
    }
    ui->WahBaseDial->setValue(v.toInt());
    float f = ui->WahBaseDial->value();
    lock = false;
    mutex.lock();
    BaseFreq_Wah=(f);
    mutex.unlock();
}

void MainWindow::on_WahDepthLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->WahDepthLE->text();
    if (v.toInt()>ui->WahDepthDial->maximum())
    {
        v=ui->WahDepthDial->maximum();
        ui->WahDepthLE->setText(v.toString());
    }
    if (v.toInt()<ui->WahDepthDial->minimum())
    {
        v=ui->WahDepthDial->minimum();
        ui->WahDepthLE->setText(v.toString());
    }
    ui->WahDepthDial->setValue(v.toInt());
    float f = ui->WahDepthDial->value();
    lock = false;
    mutex.lock();
    OffsetFreq_Wah=(BaseFreq_Wah+f);
    mutex.unlock();
}

void MainWindow::on_WahRateLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->WahRateLE->text();
    if (v.toInt()>ui->WahRateDial->maximum())
    {
        v=ui->WahRateDial->maximum();
        ui->WahRateLE->setText(v.toString());
    }
    if (v.toInt()<ui->WahRateDial->minimum())
    {
        v=ui->WahRateDial->minimum();
        ui->WahRateLE->setText(v.toString());
    }
    ui->WahRateDial->setValue(v.toInt());
    float f = ui->WahRateDial->value();
    lock = false;
    mutex.lock();
    SweepRate_Wah=(f/10);
    mutex.unlock();
}

void MainWindow::on_WahFeedBackLE_textChanged(const QString &arg1)
{

}

void MainWindow::on_WahDryLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->WahDryLE->text();
    if (v.toInt()>ui->WahDryDial->maximum())
    {
        v=ui->WahDryDial->maximum();
        ui->WahDryLE->setText(v.toString());
    }
    if (v.toInt()<ui->WahDryDial->minimum())
    {
        v=ui->WahDryDial->minimum();
        ui->WahDryLE->setText(v.toString());
    }
    ui->WahDryDial->setValue(v.toInt());
    float f = ui->WahDryDial->value();
    lock = false;
    mutex.lock();
    Dry_Wah=(f/330);
    mutex.unlock();
}

void MainWindow::on_WahWetLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->WahWetLE->text();
    if (v.toInt()>ui->WahWetDial->maximum())
    {
        v=ui->WahWetDial->maximum();
        ui->WahWetLE->setText(v.toString());
    }
    if (v.toInt()<ui->WahWetDial->minimum())
    {
        v=ui->WahWetDial->minimum();
        ui->WahWetLE->setText(v.toString());
    }
    ui->WahWetDial->setValue(v.toInt());
    float f = ui->WahWetDial->value();
    lock = false;
    mutex.lock();
    Wet_Wah=(f/330);
    mutex.unlock();
}

void MainWindow::on_WahRLE_textChanged(const QString &arg1)
{
    lock = true;
    QVariant v = ui->WahRLE->text();
    if (v.toInt()>ui->WahRDial->maximum())
    {
        v=ui->WahRDial->maximum();
        ui->WahRLE->setText(v.toString());
    }
    if (v.toInt()<ui->WahRDial->minimum())
    {
        v=ui->WahRDial->minimum();
        ui->WahRLE->setText(v.toString());
    }
    ui->WahRDial->setValue(v.toInt());
    float f = ui->WahRDial->value();
    lock = false;
    mutex.lock();
    R_Wah=(f/1000);
    mutex.unlock();
}

void MainWindow::on_checkBox_9_toggled(bool checked)
{
    mutex.lock();
    Enabled_EQ=checked;
    mutex.unlock();
}

void MainWindow::on_EQ63SL_valueChanged(int value)
{
    mutex.lock();
    LP_Gain=value/1000;
    mutex.unlock();
}

void MainWindow::on_EQ250SL_valueChanged(int value)
{
    mutex.lock();
    BP1_Gain=value/1000;
    mutex.unlock();
}

void MainWindow::on_EQ1000SL_valueChanged(int value)
{
    mutex.lock();
    BP2_Gain=value/1000;
    mutex.unlock();
}

void MainWindow::on_EQ4000SL_valueChanged(int value)
{
    mutex.lock();
    BP3_Gain=value/1000;
    mutex.unlock();
}

void MainWindow::on_EQ8000SL_valueChanged(int value)
{
    mutex.lock();
    BP4_Gain=value/1000;
    mutex.unlock();
}

void MainWindow::on_EQ212000SL_valueChanged(int value)
{
    mutex.lock();
    BP5_Gain=value/1000;
    mutex.unlock();
}

void MainWindow::on_EQ16000SL_valueChanged(int value)
{
    mutex.lock();
    HP_Gain=value/1000;
    mutex.unlock();
}

void MainWindow::on_checkBox_11_toggled(bool checked)
{
    if (checked)
    {
        // Reverb Buffers init//
        // Fill Buffers with zeros
        Rv1_Buffer = (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            Rv1_Buffer[i]=0;
        }
        Rv2_Buffer = (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            Rv2_Buffer[i]=0;
        }
        Rv3_Buffer = (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            Rv3_Buffer[i]=0;
        }
        Rv4_Buffer = (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            Rv4_Buffer[i]=0;
        }
        Rv5_Buffer = (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            Rv5_Buffer[i]=0;
        }
        Rv6_Buffer = (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            Rv6_Buffer[i]=0;
        }
        RvFb1_Buffer= (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            RvFb1_Buffer[i]=0;
        }
        RvFb2_Buffer= (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            RvFb2_Buffer[i]=0;
        }
        RvFb3_Buffer= (float *) malloc(Rv_BufferSize*2*sizeof(float));
        for (int i=0;i<(Rv_BufferSize*2);i++)
        {
            RvFb3_Buffer[i]=0;
        }
    }
    mutex.lock();
    Enabled_Rv=checked;
    mutex.unlock();
    if (!checked)
    {
        usleep(100);
        mutex.lock();
        free(Rv1_Buffer);
        free(Rv2_Buffer);
        free(Rv3_Buffer);
        free(Rv4_Buffer);
        free(Rv5_Buffer);
        free(Rv6_Buffer);
        free(RvFb1_Buffer);
        free(RvFb2_Buffer);
        free(RvFb3_Buffer);
        mutex.unlock();
    }
}

void MainWindow::on_RVDSL1_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    d1_Rv=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_RVDSL2_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    d2_Rv=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_RVDSL3_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    d3_Rv=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_RVDSL4_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    d4_Rv=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_RVDSL5_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    d5_Rv=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_RVDSL6_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    d6_Rv=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_fiSL1_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    fi1=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_fiSL2_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    fi2=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_fiSL3_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    fi3=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_fiSL4_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    fi4=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_fiSL5_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    fi5=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_fiSL6_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    fi6=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_RVDSLFB1_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    dFb1=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_RVDSLFB2_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    dFb2=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_RVDSLFB3_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    dFb3=(f/1000.0f)*SamplingRate;
    mutex.unlock();
}

void MainWindow::on_FBthSL_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    th3d=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_FBfiSL_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    fi3d=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_FBpsSL_destroyed()
{

}

void MainWindow::on_FBpsSL_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    ps3d=(f/314.0f);
    mutex.unlock();
}

void MainWindow::on_FBG1_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    g1=(f/100.0f);
    mutex.unlock();
}

void MainWindow::on_FBG2_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    g2=(f/100.0f);
    mutex.unlock();
}

void MainWindow::on_FBG3_valueChanged(int value)
{
    float f =value;
    mutex.lock();
    g3=(f/100.0f);
    mutex.unlock();
}

void MainWindow::on_saveBTN_clicked()
{

QFile file2("settings.txt");
    if(file2.open(QIODevice::ReadWrite | QIODevice::Text))// QIODevice::Append |
    {
            QTextStream stream(&file2);
       //     file2.seek(0);

               stream << "CompAttLE:" << ui->CompAttLE->text().toLatin1()<< endl;
               stream << "CompGateThLE:" << ui->CompGateThLE->text().toLatin1()<< endl;
               stream << "CompRelLE:" << ui->CompRelLE->text().toLatin1()<< endl;
               stream << "CompGateFLE:" << ui->CompGateFLE->text().toLatin1()<< endl;
               stream << "CompLimThLE:" << ui->CompLimThLE->text().toLatin1()<< endl;
               stream << "CompCompFLE:" << ui->CompCompFLE->text().toLatin1()<< endl;
               stream << "CompLimFLE:" << ui->CompLimFLE->text().toLatin1()<< endl;
               stream << "checkBox_3:" << ui->checkBox_3->isChecked()<< endl;
               ////////////
               stream << "DGainLN:" << ui->DGainLN->text().toLatin1() << endl;
               stream << "DClipLN:" << ui->DClipLN->text().toLatin1()<< endl;
               stream << "DCFactLN:" << ui->DCFactLN->text().toLatin1()<< endl;
               stream << "checkBox_10:" << ui->checkBox_10->text().toLatin1()<< endl;
               stream << "checkBox:" << ui->checkBox->isChecked()<< endl;
               //////////////////////////////////////////////////////////
               stream << "TDepthLN:" << ui->TDepthLN->text().toLatin1()<< endl;
               stream << "TPeriodLN:" << ui->TPeriodLN->text().toLatin1()<< endl;
               stream << "Check_Tr:" << ui->Check_Tr->isChecked()<< endl;
               ////////////////////////////
               stream << "ChSpeedLE:" << ui->ChSpeedLE->text().toLatin1()<< endl;
               stream << "ChDepthLE:" << ui->ChDepthLE->text().toLatin1()<< endl;
               stream << "ChFeedbackLE:" << ui->ChFeedbackLE->text().toLatin1()<< endl;
               stream <<  "ChWetLE:" <<ui->ChWetLE->text().toLatin1()<< endl;
               stream << "ChDryLE:" << ui->ChDryLE->text().toLatin1()<< endl;
               stream << "checkBox_4:" << ui->checkBox_4->isChecked()<< endl;
               ///////////////////////////////
               stream << "WahBaseLE:" << ui->WahBaseLE->text().toLatin1()<< endl;
               stream << "WahDepthLE:" << ui->WahDepthLE->text().toLatin1()<< endl;
               stream << "WahRateLE:" << ui->WahRateLE->text().toLatin1()<< endl;
               stream << "WahDryLE:" << ui->WahDryLE->text().toLatin1()<< endl;
               stream << "WahWetLE:" << ui->WahWetLE->text().toLatin1()<< endl;
               stream << "WahRLE:" << ui->WahRLE->text().toLatin1()<< endl;
               stream << "checkBox_2:" << ui->checkBox_2->isChecked()<< endl;
               ///////////////////////////////
               stream << "VibSpeedLE:" << ui->VibSpeedLE->text().toLatin1()<< endl;
               stream << "VibDepthLE:" << ui->VibDepthLE->text().toLatin1()<< endl;
               stream << "VibFeedBackLE:" << ui->VibFeedBackLE->text().toLatin1()<< endl;
               stream << "VibWetLE:" << ui->VibWetLE->text().toLatin1()<< endl;
               stream << "VibDryLE:" << ui->VibDryLE->text().toLatin1()<< endl;
               stream << "checkBox_7:" << ui->checkBox_7->isChecked()<< endl;
               ///////////////////////////////
               stream << "ShShiftLE:" << ui->ShShiftLE->text().toLatin1()<< endl;
               stream << "ShFeedbackLE:" << ui->ShFeedbackLE->text().toLatin1()<< endl;
               stream << "ShWetLE:" << ui->ShWetLE->text().toLatin1()<< endl;
               stream << "ShDryLE:" << ui->ShDryLE->text().toLatin1()<< endl;
               stream << "checkBox_8:" << ui->checkBox_8->isChecked()<< endl;
               ///////////////////////////////
               stream << "PhBaseLE:" << ui->PhBaseLE->text().toLatin1()<< endl;
               stream << "PhDepthLE:" << ui->PhDepthLE->text().toLatin1()<< endl;
               stream << "PhRateLE:" << ui->PhRateLE->text().toLatin1()<< endl;
               stream << "PhFeedBackLE:" << ui->PhFeedBackLE->text().toLatin1()<< endl;
               stream << "PhDryLE:" << ui->PhDryLE->text().toLatin1()<< endl;
               stream << "PhWetLE:" << ui->PhWetLE->text().toLatin1()<< endl;
               stream << "checkBox_5:" << ui->checkBox_5->isChecked()<< endl;
               ///////////////////////////////

               stream << "Dtime_LE:" << ui->Dtime_LE->text().toLatin1()<< endl;
               stream << "DFeedBack_LE:" << ui->DFeedBack_LE->text().toLatin1()<< endl;
               stream << "DMix_LE:" << ui->DMix_LE->text().toLatin1()<< endl;
               stream << "Check_D:" << ui->Check_D->isChecked()<< endl;

               ///////////////////////////////
               //sliders
               stream << "RVDSL1:" << ui->RVDSL1->value()<< endl;
               stream << "RVDSL2:" << ui->RVDSL2->value()<< endl;
               stream << "RVDSL3:" << ui->RVDSL3->value()<< endl;
               stream << "RVDSL4:" << ui->RVDSL4->value()<< endl;
               stream << "RVDSL5:" << ui->RVDSL5->value()<< endl;
               stream << "RVDSL6:" << ui->RVDSL6->value()<< endl;
               stream << "RVDSLFB1:" << ui->RVDSLFB1->value()<< endl;
               stream << "RVDSLFB2:" << ui->RVDSLFB2->value()<< endl;
               stream << "RVDSLFB3:" << ui->RVDSLFB3->value()<< endl;
               stream << "FBthSL:" << ui->FBthSL->value()<< endl;
               stream << "FBfiSL:" << ui->FBfiSL->value()<< endl;
               stream << "FBpsSL:" << ui->FBpsSL->value()<< endl;

               stream << "fiSL1:" << ui->fiSL1->value()<< endl;
               stream << "fiSL2:" << ui->fiSL2->value()<< endl;
               stream << "fiSL3:" << ui->fiSL3->value()<< endl;
               stream << "fiSL4:" << ui->fiSL4->value()<< endl;
               stream << "fiSL5:" << ui->fiSL5->value()<< endl;
               stream << "fiSL6:" << ui->fiSL6->value()<< endl;

               stream << "FBG1:" << ui->FBG1->value()<< endl;
               stream << "FBG2:" << ui-> FBG2->value()<< endl;
               stream << "FBG3:" << ui->FBG3->value()<< endl;
               stream << "checkBox_11:" << ui->checkBox_11->isChecked()<< endl;
               ///////////////////////////////
               stream << "EQ63SL:" << ui->EQ63SL->value()<< endl;
               stream << "EQ250SL:" << ui->EQ250SL->value()<< endl;
               stream << "EQ1000SL:" << ui->EQ1000SL->value()<< endl;
               stream << "EQ4000SL:" << ui->EQ4000SL->value()<< endl;
               stream << "EQ8000SL:" << ui->EQ8000SL->value()<< endl;
               stream << "EQ212000SL:" << ui->EQ212000SL->value()<< endl;
               stream << "EQ16000SL:" << ui->EQ16000SL->value()<< endl;
               stream << "checkBox_9:" << ui->checkBox_9->isChecked()<< endl;
               ////noise filter
               stream << "verticalSlider_2:" << ui->verticalSlider_2->value()<< endl;
               stream << "verticalSlider:" << ui->verticalSlider->value()<< endl;
               stream << "checkBox_6:" << ui->checkBox_6->isChecked()<< endl;

        //                file.write("\n");
           file2.close();
    }
}

void MainWindow::on_cmbTheme_currentIndexChanged(const QString &arg1)
{
    if (loaded==true)
    {
    fileName=ui->cmbTheme->currentText();
    QFile file(fileName);

 //   qDebug("testing");

    file.open(QIODevice::Text | QIODevice::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);
    file.close();

    QFile file2("themes.txt");
        if(file2.open(QIODevice::ReadWrite | QIODevice::Text))// QIODevice::Append |
        {
                QTextStream stream(&file2);
                file2.seek(0);
                stream << "theme:" << ui->cmbTheme->currentText().toLatin1()<< endl;
                for (int i = 0; i < ui->cmbTheme->count(); i++)
                {
                 stream << "theme:" << ui->cmbTheme->itemText(i) << endl;
                }
            //                file.write("\n");
               file2.close();
        }

    if (ui->cmbTheme->currentText().toLatin1() != ""){
      //   ui->cmbTheme->currentText().toLatin1();
    }
}
}

void MainWindow::on_LoadBTN_clicked()
{

//    QDirIterator it("./presets/", QStringList() << "*.presets", QDir::Files, QDirIterator::Subdirectories);
//    while (it.hasNext()){
//      //  QFileInfo fileInfo(f.fileName());
//        ui->cmbTheme->addItem(it.next().toLatin1());
//    }

    //https://stackoverflow.com/questions/31384273/how-to-search-for-a-string-in-a-text-file-using-qt
        QString searchString(":");
        QFile MyFile("settings.txt");
        MyFile.open(QIODevice::ReadWrite);
        QTextStream in (&MyFile);
        QString line;
      //  int ii=0;
        QStringList list;
         //   QList<QString> nums;
        QStringList nums;


        do {
            line = in.readLine();
            searchString=":";
            if (line.contains(searchString)) { //, Qt::CaseSensitive
                // do something
                QRegExp rx("[:]");// match a comma or a space
                list = line.split(rx);
                nums.append(list.at(1).toLatin1());
            }
        } while (!line.isNull());


            ui->CompAttLE->setText(nums.at(0));
            ui->CompGateThLE->setText(nums.at(1));
            ui->CompRelLE->setText(nums.at(2));
            ui->CompGateFLE->setText(nums.at(3));
            ui->CompLimThLE->setText(nums.at(4));
            ui->CompCompFLE->setText(nums.at(5));
            ui->CompLimFLE->setText(nums.at(6));
            ui->checkBox_3->setChecked(nums.at(7).toInt());
          ////////////
            ui->DGainLN->setText(nums.at(8));
            ui->DClipLN->setText(nums.at(9));
            ui->DCFactLN->setText(nums.at(10));
            ui->checkBox_10->setText(nums.at(11));
            ui->checkBox->setChecked(nums.at(12).toInt());
          //////////////////////////////////////////////////////////
            ui->TDepthLN->setText(nums.at(12));
            ui->TPeriodLN->setText(nums.at(14));
            ui->Check_Tr->setChecked(nums.at(15).toInt());
          ////////////////////////////
            ui->ChSpeedLE->setText(nums.at(16));
            ui->ChDepthLE->setText(nums.at(17));
            ui->ChFeedbackLE->setText(nums.at(18));
            ui->ChWetLE->setText(nums.at(19));
            ui->ChDryLE->setText(nums.at(20));
            ui->checkBox_4->setChecked(nums.at(21).toInt());
          ///////////////////////////////
            ui->WahBaseLE->setText(nums.at(22));
            ui->WahDepthLE->setText(nums.at(23));
            ui->WahRateLE->setText(nums.at(24));
            ui->WahDryLE->setText(nums.at(25));
            ui->WahWetLE->setText(nums.at(26));
            ui->WahRLE->setText(nums.at(27));
            ui->checkBox_2->setChecked(nums.at(28).toInt());
          ///////////////////////////////
            ui->VibSpeedLE->setText(nums.at(29));
            ui->VibDepthLE->setText(nums.at(30));
            ui->VibFeedBackLE->setText(nums.at(31));
            ui->VibWetLE->setText(nums.at(32));
            ui->VibDryLE->setText(nums.at(33));
            ui->checkBox_7->setChecked(nums.at(34).toInt());
          ///////////////////////////////
            ui->ShShiftLE->setText(nums.at(35));
            ui->ShFeedbackLE->setText(nums.at(36));
            ui->ShWetLE->setText(nums.at(37));
            ui->ShDryLE->setText(nums.at(38));
            ui->checkBox_8->setChecked(nums.at(39).toInt());
          ///////////////////////////////
            ui->PhBaseLE->setText(nums.at(40));
            ui->PhDepthLE->setText(nums.at(41));
            ui->PhRateLE->setText(nums.at(42));
            ui->PhFeedBackLE->setText(nums.at(43));
            ui->PhDryLE->setText(nums.at(44));
            ui->PhWetLE->setText(nums.at(45));
            ui->checkBox_5->setChecked(nums.at(46).toInt());
          ///////////////////////////////

            ui->Dtime_LE->setText(nums.at(47));
            ui->DFeedBack_LE->setText(nums.at(48));
            ui->DMix_LE->setText(nums.at(49));
     ui->Check_D->setChecked(nums.at(50).toInt());

    ///////////////////////////////
    //sliders
    ui->RVDSL1->setValue(nums.at(51).toInt());
            ui->RVDSL2->setValue(nums.at(52).toInt());
            ui->RVDSL3->setValue(nums.at(53).toInt());
            ui->RVDSL4->setValue(nums.at(54).toInt());
            ui->RVDSL5->setValue(nums.at(55).toInt());
            ui->RVDSL6->setValue(nums.at(56).toInt());
            ui->RVDSLFB1->setValue(nums.at(57).toInt());
            ui->RVDSLFB2->setValue(nums.at(58).toInt());
            ui->RVDSLFB3->setValue(nums.at(59).toInt());
            ui->FBthSL->setValue(nums.at(60).toInt());
            ui->FBfiSL->setValue(nums.at(61).toInt());
            ui->FBpsSL->setValue(nums.at(62).toInt());

            ui->fiSL1->setValue(nums.at(63).toInt());
            ui->fiSL2->setValue(nums.at(64).toInt());
            ui->fiSL3->setValue(nums.at(65).toInt());
            ui->fiSL4->setValue(nums.at(66).toInt());
            ui->fiSL5->setValue(nums.at(67).toInt());
            ui->fiSL6->setValue(nums.at(68).toInt());

            ui->FBG1->setValue(nums.at(69).toInt());
            ui-> FBG2->setValue(nums.at(70).toInt());
            ui->FBG3->setValue(nums.at(71).toInt());
             ui->checkBox_11->setChecked(nums.at(72).toInt());
             ///////////////////////////////
            ui->EQ63SL->setValue(nums.at(73).toInt());
            ui->EQ250SL->setValue(nums.at(74).toInt());
            ui->EQ1000SL->setValue(nums.at(75).toInt());
            ui->EQ4000SL->setValue(nums.at(76).toInt());
            ui->EQ8000SL->setValue(nums.at(77).toInt());
            ui->EQ212000SL->setValue(nums.at(78).toInt());
            ui->EQ16000SL->setValue(nums.at(79).toInt());
            ui->checkBox_9->setChecked(nums.at(80).toInt());
             ////noise filter
            ui->verticalSlider_2->setValue(nums.at(81).toInt());
            ui->verticalSlider->setValue(nums.at(82).toInt());
            ui->checkBox_6->setChecked(nums.at(83).toInt());
}
