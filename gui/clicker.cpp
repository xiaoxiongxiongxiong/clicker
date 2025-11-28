#include "clicker.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QDateTime>
#include <QIntValidator>
#include <QRandomGenerator>
#include <windows.h>

clicker::clicker(QWidget * parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    ui.m_dateEffect->setDateTime(QDateTime::currentDateTime());

    QIntValidator * validator = new QIntValidator(-2147483647, 2147483647, this);
    ui.m_edtIntervalMin->setValidator(validator);
    ui.m_edtIntervalMax->setValidator(validator);

    QImage * m_imgLogin = new QImage;
    if (m_imgLogin->load(":/clicker/res/clicker.ico"))
    {
        QImage c = m_imgLogin->scaled(ui.m_labLogo->size(), Qt::KeepAspectRatio);
        ui.m_labLogo->setPixmap(QPixmap::fromImage(c));
    }

    connect(ui.m_btnMinimize, SIGNAL(clicked()), this, SLOT(onBtnClickedMinimize()));
    connect(ui.m_btnClose, SIGNAL(clicked()), this, SLOT(onBtnClickedClose()));
    connect(ui.m_btnCtrl, SIGNAL(clicked()), this, SLOT(onBtnClickedCtrl()));
}

clicker::~clicker()
{}

void clicker::mousePressEvent(QMouseEvent * event)
{
    m_blPressed = true; // 当前鼠标按下的即是QWidget而非界面上布局的其它控件
    m_ptPos = event->globalPos();
}

void clicker::mouseMoveEvent(QMouseEvent * event)
{
    if (m_blPressed)
    {
        int dx = event->globalX() - m_ptPos.x();
        int dy = event->globalY() - m_ptPos.y();
        m_ptPos = event->globalPos();
        move(x() + dx, y() + dy);
    }
}

void clicker::mouseReleaseEvent(QMouseEvent * event)
{
    int dx = event->globalX() - m_ptPos.x();
    int dy = event->globalY() - m_ptPos.y();
    move(x() + dx, y() + dy);
    m_blPressed = false; // 鼠标松开时，置为false
}

void clicker::onBtnClickedMinimize()
{
    if (Qt::WindowMinimized == this->windowState())
        this->showNormal();
    else
        this->showMinimized();
}

void clicker::onBtnClickedClose()
{
    if (_running.load())
    {
        _running.store(false);
        if (_thr.joinable())
            _thr.join();
        //ui.m_btnCtrl->setText(QStringLiteral("启 动"));
        ui.m_btnCtrl->setIcon(QIcon(":/clicker/res/pause.ico"));
        ui.m_labMessage->setText(QStringLiteral("倒计时0秒，累计点击次数0"));
    }

    QApplication * app;
    app->quit();
}

void clicker::onBtnClickedCtrl()
{
    if (_running.load())
    {
        _running.store(false);
        if (_thr.joinable())
            _thr.join();
        //ui.m_btnCtrl->setText(QStringLiteral("启 动"));
        ui.m_btnCtrl->setIcon(QIcon(":/clicker/res/pause.ico"));
        ui.m_labMessage->setText(QStringLiteral("倒计时0秒，累计点击次数0"));
        return;
    }

    if (m_ptDst.x() <= 0 && m_ptDst.y() <= 0)
    {
        QMessageBox::critical(this, QStringLiteral("警告"), QStringLiteral("未设定鼠标位置！"));
        ui.m_labPos->setStyleSheet(R"(color: rgb(255, 0, 0);)");
        ui.m_labPos->setText(QStringLiteral("位置未设定"));
        return;
    }

    if (ui.m_edtIntervalMin->text().isEmpty())
    {
        QMessageBox::critical(this, QStringLiteral("警告"), QStringLiteral("请输入最小间隔！"));
        return;
    }

    if (ui.m_edtIntervalMax->text().isEmpty())
    {
        QMessageBox::critical(this, QStringLiteral("警告"), QStringLiteral("请输入最大间隔！"));
        return;
    }

    if (ui.m_edtIntervalMin->text().toInt() > ui.m_edtIntervalMax->text().toInt())
    {
        QMessageBox::critical(this, QStringLiteral("警告"), QStringLiteral("无效的间隔范围！"));
        return;
    }

    try
    {
        _running.store(true);
        srand(static_cast<uint32_t>(time(nullptr)));
        _thr = std::thread{ &clicker::processThr, this };
    }
    catch (const std::exception & e)
    {
        _running.store(false);
        QMessageBox::critical(this, QStringLiteral("警告"), QStringLiteral("%1！").arg(e.what()));
        return;
    }
    catch (...)
    {
        _running.store(false);
        QMessageBox::critical(this, QStringLiteral("警告"), QStringLiteral("未知错误！"));
        return;
    }
    ui.m_btnCtrl->setIcon(QIcon(":/clicker/res/play.ico"));
    //ui.m_btnCtrl->setText(QStringLiteral("停 止"));
}

void clicker::keyPressEvent(QKeyEvent * event)
{
    QWidget::keyPressEvent(event);
}

void clicker::keyReleaseEvent(QKeyEvent * event)
{
    if (Qt::Key_A == event->key())
    {
        m_ptDst = QCursor::pos();
        ui.m_labPos->setStyleSheet(R"(color: rgb(0, 255, 0);)");
        ui.m_labPos->setText(QStringLiteral("位置已设定"));
        //ui.m_edtHorizontal->setText(QString::number(pt.x()));
        //ui.m_edtVertical->setText(QString::number(pt.y()));
    }

    QWidget::keyReleaseEvent(event);
}

void clicker::processThr()
{
    int times = 0;
    int x = m_ptDst.x();
    int y = m_ptDst.y();
    int min_interval = ui.m_edtIntervalMin->text().toInt();
    int max_interval = ui.m_edtIntervalMax->text().toInt();
    int interval = QRandomGenerator::global()->bounded(min_interval, max_interval + 1) * 60;

    int64_t last_ts = 0;
    QDateTime tmp = {};
    if (ui.m_chkEnable->isChecked())
        tmp = ui.m_dateEffect->dateTime();
    else
        tmp = QDateTime::currentDateTime();
    last_ts = static_cast<int64_t>(tmp.toSecsSinceEpoch());

    while (_running.load())
    {
        const auto cur_time = QDateTime::currentDateTime();
        const auto cur_ts = cur_time.toSecsSinceEpoch();
        const auto diff = cur_ts - last_ts;

        if (diff < interval)
        {
            ui.m_labMessage->setText(QStringLiteral("倒计时%1秒，累计点击次数%2").arg(interval - diff).arg(times));
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }

        interval = QRandomGenerator::global()->bounded(min_interval, max_interval + 1) * 60;
        last_ts = cur_ts;

        QCursor::setPos(x, y);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        mouse_event(MOUSEEVENTF_LEFTDOWN, x, y, 0, GetMessageExtraInfo());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        mouse_event(MOUSEEVENTF_LEFTUP, x, y, 0, GetMessageExtraInfo());
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        times++;
    }
}
