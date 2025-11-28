#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_clicker.h"
#include <atomic>
#include <thread>

class clicker : public QMainWindow
{
    Q_OBJECT

public:
    clicker(QWidget * parent = nullptr);
    ~clicker();

    void mousePressEvent(QMouseEvent * event);//鼠标点击
    void mouseMoveEvent(QMouseEvent * event);//鼠标移动
    void mouseReleaseEvent(QMouseEvent * event);//鼠标释放

public slots:
    void onBtnClickedMinimize();
    void onBtnClickedClose();
    void onBtnClickedCtrl();

protected:
    void keyPressEvent(QKeyEvent * event) override;
    void keyReleaseEvent(QKeyEvent * event) override;

private:
    // 处理线程
    void processThr();

private:
    Ui::clickerClass ui;

    bool m_blPressed = false;
    QPoint m_ptPos;
    QPoint m_ptDst{};

    std::thread _thr;
    std::atomic_bool _running = { false };
};
