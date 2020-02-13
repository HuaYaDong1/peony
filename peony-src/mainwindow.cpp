#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QToolButton>
#include <QIcon>

#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>

#include "controls/navigation-bar/advanced-location-bar.h"

#include <QtWidgets/private/qwidgetresizehandler_p.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /*!
      \todo
      make window be recognized as csd window in all window managers.
      .
      In kwin, it also detect _GTK_FRAME_EXTENT atom, and asume it as csd window.
      However it seems that there is no effect for qt window, and maybe
      I have to try implementing a KDecorator for client side decorations in kwin.

      make window can be maximumed/half-maximumed like other windows have titlebar.
      */
    //setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_Hover);
    //setContentsMargins(4, 4, 4, 4);
    //use qt private api resize the widget.
    auto handler = new QWidgetResizeHandler(this);
    handler->setActive(QWidgetResizeHandler::Resize, false);

    //title bar
    ui->setupUi(this);
    ui->mainToolBar->setFixedHeight(50);
    auto a = ui->mainToolBar->addAction(QIcon::fromTheme("folder-new-symbolic"), tr("Create New Folder"));
    auto b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedSize(40, 40);

    a = ui->mainToolBar->addAction(QIcon::fromTheme("terminal"), tr("Open in Terminal"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedSize(40, 40);

    ui->mainToolBar->addSeparator();

    auto hlayout = new QHBoxLayout();
    auto tmp = new QWidget(this);
    tmp->setLayout(hlayout);
    hlayout->setMargin(0);
    hlayout->setSpacing(0);

    auto p = new QPushButton(QIcon::fromTheme("go-previous"), nullptr, this);
    p->setFixedSize(QSize(36, 28));
    p->setToolTip(tr("Go Back"));
    p->setIconSize(QSize(16, 16));
    hlayout->addWidget(p);

    p = new QPushButton(QIcon::fromTheme("go-next"), nullptr, this);
    p->setFixedSize(QSize(36, 28));
    p->setToolTip(tr("Go Forward"));
    p->setIconSize(QSize(16, 16));
    hlayout->addWidget(p);

    ui->mainToolBar->addWidget(tmp);

    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addWidget(new Peony::AdvancedLocationBar(this));
    ui->mainToolBar->addSeparator();

    a = ui->mainToolBar->addAction(QIcon::fromTheme("search-symbolic"), tr("Find"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedSize(40, 40);

    ui->mainToolBar->addSeparator();

    a = ui->mainToolBar->addAction(QIcon::fromTheme("view-grid-symbolic"), tr("View"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedHeight(40);
    b->setPopupMode(QToolButton::MenuButtonPopup);

    a = ui->mainToolBar->addAction(QIcon::fromTheme("view-sort-ascending-symbolic"), tr("View"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedHeight(40);
    b->setPopupMode(QToolButton::MenuButtonPopup);

    a = ui->mainToolBar->addAction(QIcon::fromTheme("open-menu-symbolic"), tr("Menu"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedSize(40, 40);

    a = ui->mainToolBar->addAction(QIcon::fromTheme("window-minimize-symbolic"), tr("Menu"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedSize(40, 40);

    a = ui->mainToolBar->addAction(QIcon::fromTheme("window-maximize-symbolic"), tr("Menu"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedSize(40, 40);

    a = ui->mainToolBar->addAction(QIcon::fromTheme("window-close-symbolic"), tr("Menu"));
    b = qobject_cast<QToolButton*>(ui->mainToolBar->widgetForAction(a));
    b->setAutoRaise(false);
    b->setFixedSize(40, 40);
    //title bar
}

MainWindow::~MainWindow()
{
    delete ui;
}