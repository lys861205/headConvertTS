#ifndef CONVERTUI_H
#define CONVERTUI_H

#include <QDialog>

namespace Ui {
    class ConvertUI;
}

class ConvertUI : public QDialog
{
    Q_OBJECT

public:
    explicit ConvertUI(QWidget *parent = 0);
    ~ConvertUI();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::ConvertUI *ui;

private slots:
    void on_addBtn_clicked();
    void on_ConvertBtn_clicked();
};

#endif // CONVERTUI_H
