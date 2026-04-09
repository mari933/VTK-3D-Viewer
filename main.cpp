#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    QString filename;
    
    if (argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        filename = QFileDialog::getOpenFileName(
            nullptr, 
            "Open VTU File", 
            "", 
            "VTU Files (*.vtu);;All Files (*)"
        );
    }
    
    if (filename.isEmpty())
    {
        QMessageBox::warning(nullptr, "Error", "No file selected!");
        return 1;
    }
    
    QFile file(filename);
    if (!file.exists())
    {
        QMessageBox::warning(nullptr, "Error", "File does not exist!");
        return 1;
    }
    
    MainWindow window(filename);
    window.show();
    
    return app.exec();
}
