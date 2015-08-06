#pragma once
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QRadioButton>

class FindDialog: public QDialog{
    Q_OBJECT

    QLineEdit * indexEdit;

    QRadioButton *t1;
    QRadioButton *t2;
    QRadioButton *t3;

    int nv,nf;

public:
    FindDialog (int num_vert, int num_face, QWidget * parent = 0, Qt::WindowFlags f = 0 ) : QDialog(parent, f)
    {
        index = 0;

        nv = num_vert;
        nf = num_face;

        QGridLayout *layout = new QGridLayout;

        // Item types
        t1 = new QRadioButton("Vertex");
        t2 = new QRadioButton("Face");
        t3 = new QRadioButton("Edge");

        t1->setChecked(true);

        layout->addWidget(t1);
        layout->addWidget(t2);
        layout->addWidget(t3);

        // Item index
        indexEdit = new QLineEdit;
        indexEdit->setText("0");
        layout->addWidget(indexEdit);

        // Find Button
        QPushButton *findButton = new QPushButton("Find");
        this->connect(findButton, SIGNAL(clicked()), SLOT(doFind()));
        layout->addWidget(findButton);

        this->setLayout(layout);
    }

    int index;

public slots:
    void doFind()
    {
        index = indexEdit->text().toInt();

        if(t2->isChecked()) index += (nv);
        if(t3->isChecked()) index += (nv + nf);

        this->close();
    }
};
