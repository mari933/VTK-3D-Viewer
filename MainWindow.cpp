#include "MainWindow.h"
#include <cmath>
#include <QDebug>
#include <vtkPointData.h>
#include <vtkDataArray.h>

MainWindow::MainWindow(const QString& filename, QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("VTU Viewer - " + filename);
    resize(1200, 800);
    
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    mainLayout = new QHBoxLayout(centralWidget);
    
    leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    vtkWidget = new QVTKOpenGLNativeWidget();
    leftLayout->addWidget(vtkWidget);
    
    rightPanel = new QWidget();
    rightPanel->setMaximumWidth(250);
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setAlignment(Qt::AlignTop);
    
    arrayLabel = new QLabel("Coloring Array:");
    rightLayout->addWidget(arrayLabel);
    arrayCombo = new QComboBox();
    rightLayout->addWidget(arrayCombo);
    
    componentLabel = new QLabel("Component:");
    rightLayout->addWidget(componentLabel);
    componentCombo = new QComboBox();
    rightLayout->addWidget(componentCombo);
    
    rightLayout->addStretch();
    
    mainLayout->addWidget(leftPanel, 3);
    mainLayout->addWidget(rightPanel, 1);
    
    connect(arrayCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onArrayChanged);
    connect(componentCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onComponentChanged);
    
    loadVTUFile(filename);
}

MainWindow::~MainWindow() {}

void MainWindow::loadVTUFile(const QString& filename)
{
    reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    reader->SetFileName(filename.toStdString().c_str());
    reader->Update();
    
    mapper = vtkSmartPointer<vtkDataSetMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());
    
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderer->AddActor(actor);
    renderer->SetBackground(0.1, 0.1, 0.15);
    
    renderWindow = vtkWidget->renderWindow();
    renderWindow->AddRenderer(renderer);
    
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    vtkWidget->interactor()->SetInteractorStyle(style);
    
    vtkPointData* pointData = reader->GetOutput()->GetPointData();
    for (int i = 0; i < pointData->GetNumberOfArrays(); i++) {
        vtkDataArray* arr = pointData->GetArray(i);
        if (arr) {
            arrayCombo->addItem(arr->GetName());
        }
    }
    
    if (arrayCombo->count() > 0) {
        arrayCombo->setCurrentIndex(0);
        onArrayChanged(0);
    }
    
    renderWindow->Render();
}

void MainWindow::onArrayChanged(int index)
{
    if (index < 0) return;
    
    QString name = arrayCombo->itemText(index);
    vtkDataArray* arr = reader->GetOutput()->GetPointData()->GetArray(name.toStdString().c_str());
    if (!arr) return;
    
    componentCombo->clear();
    int comps = arr->GetNumberOfComponents();
    
    if (comps == 1) {
        componentCombo->addItem("Value");
    } else if (comps == 3) {
        componentCombo->addItem("Magnitude");
        componentCombo->addItem("X");
        componentCombo->addItem("Y");
        componentCombo->addItem("Z");
    } else if (comps == 6) {
        componentCombo->addItem("Magnitude");
        componentCombo->addItem("XX");
        componentCombo->addItem("YY");
        componentCombo->addItem("ZZ");
        componentCombo->addItem("XY");
        componentCombo->addItem("YZ");
        componentCombo->addItem("XZ");
    } else {
        componentCombo->addItem("Magnitude");
        for (int i = 0; i < comps; i++) {
            componentCombo->addItem(QString::number(i));
        }
    }
    
    currentArrayName = name;
    currentArrayComponents = comps;
    updateColoring();
}

void MainWindow::onComponentChanged(int)
{
    updateColoring();
}

void MainWindow::updateColoring()
{
    if (currentArrayName.isEmpty()) return;
    
    vtkDataArray* arr = reader->GetOutput()->GetPointData()->GetArray(currentArrayName.toStdString().c_str());
    if (!arr) return;
    
    int compIndex = componentCombo->currentIndex();
    int numComp = arr->GetNumberOfComponents();
    
    lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfColors(256);
    lut->Build();
    
    double range[2];
    
    if (compIndex == 0 && numComp > 1) {
        vtkIdType n = arr->GetNumberOfTuples();
        double minMag = 1e10, maxMag = -1e10;
        for (vtkIdType i = 0; i < n; i++) {
            double mag = 0;
            for (int j = 0; j < numComp; j++) {
                double v = arr->GetComponent(i, j);
                mag += v * v;
            }
            mag = sqrt(mag);
            if (mag < minMag) minMag = mag;
            if (mag > maxMag) maxMag = mag;
        }
        range[0] = minMag;
        range[1] = maxMag;
        
        vtkSmartPointer<vtkDoubleArray> magArr = vtkSmartPointer<vtkDoubleArray>::New();
        magArr->SetName("Magnitude");
        magArr->SetNumberOfTuples(n);
        for (vtkIdType i = 0; i < n; i++) {
            double mag = 0;
            for (int j = 0; j < numComp; j++) {
                double v = arr->GetComponent(i, j);
                mag += v * v;
            }
            mag = sqrt(mag);
            magArr->SetValue(i, mag);
        }
        
        mapper->SetScalarVisibility(true);
        mapper->SetScalarRange(range);
        mapper->SetScalarModeToUsePointFieldData();
        mapper->SelectColorArray("Magnitude");
        mapper->SetLookupTable(lut);
        reader->GetOutput()->GetPointData()->AddArray(magArr);
    } else {
        int comp = compIndex;
        if (numComp == 1) comp = 0;
        else if (compIndex > 0) comp = compIndex - 1;
        
        arr->GetRange(range, comp);
        mapper->SetScalarVisibility(true);
        mapper->SetScalarRange(range);
        mapper->SetScalarModeToUsePointFieldData();
        mapper->SelectColorArray(currentArrayName.toStdString().c_str());
        mapper->SetArrayId(comp);
        mapper->SetLookupTable(lut);
    }
    
    if (scalarBar) renderer->RemoveActor(scalarBar);
    scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->SetLookupTable(lut);
    scalarBar->SetTitle(currentArrayName.toStdString().c_str());
    renderer->AddActor(scalarBar);
    
    renderWindow->Render();
}
