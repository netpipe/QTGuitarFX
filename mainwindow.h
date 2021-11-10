#include <QMutex>
#include <QMainWindow>
#include <QThread>
#include <QMessageBox>

#ifndef MAINWINDOW_H
#define MAINWINDOW_H



namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    bool loaded;
    QString fileName;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_DGainDial_valueChanged(int value);

    void on_dial_2_valueChanged(int value);

    void on_DClipFDial_valueChanged(int value);

    void on_DGainLN_editingFinished();

    void on_DClipLN_editingFinished();

    void on_DCFactLN_editingFinished();

    void on_TDepthDial_valueChanged(int value);

    void on_TPeriodDial_valueChanged(int value);

    void on_TDepthLN_editingFinished();

    void on_TPeriodLN_editingFinished();

    void on_DGainLN_textChanged(const QString &arg1);

    void on_DClipLN_textChanged(const QString &arg1);

    void on_DCFactLN_textChanged(const QString &arg1);

    void on_TDepthLN_textChanged(const QString &arg1);

    void on_TPeriodLN_textChanged(const QString &arg1);

    void on_checkBox_toggled(bool checked);

    void on_Check_Tr_toggled(bool checked);

    void on_Check_D_toggled(bool checked);

    void on_DTime_Dial_valueChanged(int value);

    void on_DFeedBack_Dial_valueChanged(int value);

    void on_DMix_Dial_valueChanged(int value);

    void on_Dtime_LE_textChanged(const QString &arg1);

    void on_DFeedBack_LE_textChanged(const QString &arg1);

    void on_DMix_LE_textChanged(const QString &arg1);

    void on_verticalSlider_valueChanged(int value);

    void updateVolume();

    void on_checkBox_7_toggled(bool checked);

    void on_checkBox_8_toggled(bool checked);

    void on_VibSpeedDial_valueChanged(int value);

    void on_VibDepthDial_valueChanged(int value);

    void on_VibFeedbackDial_valueChanged(int value);

    void on_VibWetDial_valueChanged(int value);

    void on_VibDryDial_valueChanged(int value);

    void on_VibSpeedLE_textChanged(const QString &arg1);

    void on_VibDepthLE_textChanged(const QString &arg1);

    void on_VibFeedBackLE_textChanged(const QString &arg1);

    void on_VibWetLE_textChanged(const QString &arg1);

    void on_VibDryLE_textChanged(const QString &arg1);

    void on_checkBox_4_toggled(bool checked);

    void on_ChSpeedDial_valueChanged(int value);

    void on_ChDepthDial_valueChanged(int value);

    void on_ChFeedbackDial_valueChanged(int value);

    void on_ChWetDial_valueChanged(int value);

    void on_ChDryDial_valueChanged(int value);

    void on_ChSpeedLE_textChanged(const QString &arg1);

    void on_ChDepthLE_textChanged(const QString &arg1);

    void on_ChFeedbackLE_textChanged(const QString &arg1);

    void on_ChWetLE_textChanged(const QString &arg1);

    void on_ChDryLE_textChanged(const QString &arg1);

    void on_VibDepthLE_textEdited(const QString &arg1);

    void on_ShShiftDial_valueChanged(int value);

    void on_ShFeedbackDial_valueChanged(int value);

    void on_ShWetDial_valueChanged(int value);

    void on_ShDryDial_valueChanged(int value);

    void on_ShShiftLE_textChanged(const QString &arg1);

    void on_ShFeedbackLE_textChanged(const QString &arg1);

    void on_ShWetLE_textChanged(const QString &arg1);

    void on_ShDryLE_textChanged(const QString &arg1);

    void on_checkBox_10_toggled(bool checked);

    void on_verticalSlider_2_valueChanged(int value);

    void on_checkBox_6_toggled(bool checked);

    void on_checkBox_3_toggled(bool checked);

    void on_CompAttDial_valueChanged(int value);

    void on_CompGateThDial_valueChanged(int value);

    void on_CompRelDial_valueChanged(int value);

    void on_CompGateFDial_valueChanged(int value);

    void on_CompLimThDial_valueChanged(int value);

    void on_CompCompFDial_valueChanged(int value);

    void on_CompLimFDial_valueChanged(int value);

    void on_CompAttLE_textChanged(const QString &arg1);

    void on_CompRelLE_textChanged(const QString &arg1);

    void on_CompGateThLE_textChanged(const QString &arg1);

    void on_CompGateFLE_textChanged(const QString &arg1);

    void on_CompLimThLE_textChanged(const QString &arg1);

    void on_CompCompFLE_textChanged(const QString &arg1);

    void on_CompLimFLE_textChanged(const QString &arg1);

    void on_checkBox_5_toggled(bool checked);

    void on_PhBaseDial_valueChanged(int value);

    void on_PhDepthDial_valueChanged(int value);

    void on_PhRateDial_valueChanged(int value);

    void on_PhFeedBackDial_valueChanged(int value);

    void on_PhDryDial_valueChanged(int value);

    void on_PhWetDial_valueChanged(int value);

    void on_PhBaseLE_textChanged(const QString &arg1);

    void on_PhDepthLE_textChanged(const QString &arg1);

    void on_PhRateLE_textChanged(const QString &arg1);

    void on_PhFeedBackLE_textChanged(const QString &arg1);

    void on_PhDryLE_textChanged(const QString &arg1);

    void on_PhWetLE_textChanged(const QString &arg1);

    void on_checkBox_2_toggled(bool checked);

    void on_WahBaseDial_valueChanged(int value);

    void on_WahDepthDial_valueChanged(int value);

    void on_WahRateDial_valueChanged(int value);

    void on_WahFeedBackDial_valueChanged(int value);

    void on_WahDryDial_valueChanged(int value);

    void on_WahWetDial_valueChanged(int value);

    void on_WahRDial_valueChanged(int value);

    void on_WahBaseLE_textChanged(const QString &arg1);

    void on_WahDepthLE_textChanged(const QString &arg1);

    void on_WahRateLE_textChanged(const QString &arg1);

    void on_WahFeedBackLE_textChanged(const QString &arg1);

    void on_WahDryLE_textChanged(const QString &arg1);

    void on_WahWetLE_textChanged(const QString &arg1);

    void on_WahRLE_textChanged(const QString &arg1);

    void on_checkBox_9_toggled(bool checked);

    void on_EQ63SL_valueChanged(int value);

    void on_EQ250SL_valueChanged(int value);

    void on_EQ1000SL_valueChanged(int value);

    void on_EQ4000SL_valueChanged(int value);

    void on_EQ8000SL_valueChanged(int value);

    void on_EQ212000SL_valueChanged(int value);

    void on_EQ16000SL_valueChanged(int value);

    void on_checkBox_11_toggled(bool checked);

    void on_RVDSL1_valueChanged(int value);

    void on_RVDSL2_valueChanged(int value);

    void on_RVDSL3_valueChanged(int value);

    void on_RVDSL4_valueChanged(int value);

    void on_RVDSL5_valueChanged(int value);

    void on_RVDSL6_valueChanged(int value);

    void on_fiSL1_valueChanged(int value);

    void on_fiSL2_valueChanged(int value);

    void on_fiSL3_valueChanged(int value);

    void on_fiSL4_valueChanged(int value);

    void on_fiSL5_valueChanged(int value);

    void on_fiSL6_valueChanged(int value);

    void on_RVDSLFB1_valueChanged(int value);

    void on_RVDSLFB2_valueChanged(int value);

    void on_RVDSLFB3_valueChanged(int value);

    void on_FBthSL_valueChanged(int value);

    void on_FBfiSL_valueChanged(int value);

    void on_FBpsSL_destroyed();

    void on_FBpsSL_valueChanged(int value);

    void on_FBG1_valueChanged(int value);

    void on_FBG2_valueChanged(int value);

    void on_FBG3_valueChanged(int value);

    void on_saveBTN_clicked();

    void on_cmbTheme_currentIndexChanged(const QString &arg1);

    void on_LoadBTN_clicked();

private:
    Ui::MainWindow *ui;
};

class SoundThread : public QThread
{
    Q_OBJECT
private:
    void run();
};

#endif // MAINWINDOW_H
