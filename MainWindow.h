#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QWidget>

#include <vtkSmartPointer.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkColorTransferFunction.h>

#include <QVTKOpenGLNativeWidget.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString& filename, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onArrayChanged(int index);
    void onComponentChanged(int index);

private:
    void loadVTUFile(const QString& filename);
    void extractArrayInfo();
    void updateColoring();
    
    QWidget* centralWidget;
    QHBoxLayout* mainLayout;
    QWidget* leftPanel;
    QWidget* rightPanel;
    
    QVBoxLayout* rightLayout;
    QLabel* arrayLabel;
    QComboBox* arrayCombo;
    QLabel* componentLabel;
    QComboBox* componentCombo;
    
    QVTKOpenGLNativeWidget* vtkWidget;
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader;
    vtkSmartPointer<vtkDataSetMapper> mapper;
    vtkSmartPointer<vtkActor> actor;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> renderWindow;
    vtkSmartPointer<vtkLookupTable> lut;
    vtkSmartPointer<vtkScalarBarActor> scalarBar;
    
    QStringList availableArrays;
    int currentArrayComponents;
    QString currentArrayName;
};
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
#endif
