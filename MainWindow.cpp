#include "MainWindow.h"
#include <QDebug>
#include <QMessageBox>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <cmath>

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
    rightPanel->setMinimumWidth(200);
    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setAlignment(Qt::AlignTop);
    
    rightLayout->addSpacing(20);
    
    arrayLabel = new QLabel("Coloring Array:");
    arrayLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    rightLayout->addWidget(arrayLabel);
    
    arrayCombo = new QComboBox();
    arrayCombo->setMinimumHeight(30);
    rightLayout->addWidget(arrayCombo);
    
    rightLayout->addSpacing(20);
    
    componentLabel = new QLabel("Component:");
    componentLabel->setStyleSheet("font-weight: bold; font-size: 12px;");
    rightLayout->addWidget(componentLabel);
    
    componentCombo = new QComboBox();
    componentCombo->setMinimumHeight(30);
    rightLayout->addWidget(componentCombo);
    
    rightLayout->addStretch();
    
    mainLayout->addWidget(leftPanel, 3);
    mainLayout->addWidget(rightPanel, 1);
    
    connect(arrayCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onArrayChanged);
    connect(componentCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &MainWindow::onComponentChanged);
    
    loadVTUFile(filename);
}

MainWindow::~MainWindow()
{
}

void MainWindow::loadVTUFile(const QString& filename)
{
    try
    {
        reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
        reader->SetFileName(filename.toStdString().c_str());
        reader->Update();
        
        if (reader->GetOutput()->GetNumberOfPoints() == 0)
        {
            QMessageBox::warning(this, "Error", "No points found in the file!");
            return;
        }
        
        mapper = vtkSmartPointer<vtkDataSetMapper>::New();
        mapper->SetInputConnection(reader->GetOutputPort());
        
        actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        
        renderer = vtkSmartPointer<vtkRenderer>::New();
        renderer->AddActor(actor);
        renderer->SetBackground(0.1, 0.1, 0.15);
        
        renderWindow = vtkWidget->GetRenderWindow();
        renderWindow->AddRenderer(renderer);
        
        vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = 
            vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
        vtkWidget->interactor()->SetInteractorStyle(style);
        
        extractArrayInfo();
        
        if (availableArrays.size() > 0)
        {
            arrayCombo->setCurrentIndex(0);
            updateColoring();
        }
        
        renderWindow->Render();
    }
    catch (...)
    {
        QMessageBox::warning(this, "Error", "Failed to load VTU file!");
    }
}

void MainWindow::extractArrayInfo()
{
    availableArrays.clear();
    arrayCombo->clear();
    
    if (!reader->GetOutput())
        return;
    
    vtkPointData* pointData = reader->GetOutput()->GetPointData();
    if (!pointData)
        return;
    
    int numArrays = pointData->GetNumberOfArrays();
    
    for (int i = 0; i < numArrays; i++)
    {
        vtkDataArray* array = pointData->GetArray(i);
        if (array)
        {
            QString arrayName = QString::fromStdString(array->GetName());
            availableArrays.append(arrayName);
            arrayCombo->addItem(arrayName);
        }
    }
    
    qDebug() << "Found arrays:" << availableArrays;
}

void MainWindow::onArrayChanged(int index)
{
    if (index < 0 || index >= availableArrays.size())
        return;
    
    currentArrayName = availableArrays[index];
    
    if (reader->GetOutput())
    {
        vtkDataArray* array = reader->GetOutput()->GetPointData()->GetArray(
            currentArrayName.toStdString().c_str());
        if (array)
        {
            currentArrayComponents = array->GetNumberOfComponents();
            
            componentCombo->clear();
            if (currentArrayComponents == 1)
            {
                componentCombo->addItem("Magnitude");
            }
            else if (currentArrayComponents == 3)
            {
                componentCombo->addItem("Magnitude");
                componentCombo->addItem("X");
                componentCombo->addItem("Y");
                componentCombo->addItem("Z");
            }
            else if (currentArrayComponents == 6)
            {
                componentCombo->addItem("Magnitude");
                componentCombo->addItem("XX");
                componentCombo->addItem("YY");
                componentCombo->addItem("ZZ");
                componentCombo->addItem("XY");
                componentCombo->addItem("YZ");
                componentCombo->addItem("XZ");
            }
        }
    }
    
    updateColoring();
}

void MainWindow::onComponentChanged(int index)
{
    updateColoring();
}

void MainWindow::updateColoring()
{
    if (!reader->GetOutput() || currentArrayName.isEmpty())
        return;
    
    vtkDataArray* array = reader->GetOutput()->GetPointData()->GetArray(
        currentArrayName.toStdString().c_str());
    
    if (!array)
        return;
    
    int componentIndex = componentCombo->currentIndex();
    int numComponents = array->GetNumberOfComponents();
    
    lut = vtkSmartPointer<vtkLookupTable>::New();
    lut->SetNumberOfColors(256);
    lut->Build();
    
    double range[2];
    
    if (componentIndex == 0)
    {
        double minMag = 1e10, maxMag = -1e10;
        vtkIdType numTuples = array->GetNumberOfTuples();
        
        for (vtkIdType i = 0; i < numTuples; i++)
        {
            double mag = 0;
            for (int j = 0; j < numComponents; j++)
            {
                double val = array->GetComponent(i, j);
                mag += val * val;
            }
            mag = sqrt(mag);
            if (mag < minMag) minMag = mag;
            if (mag > maxMag) maxMag = mag;
        }
        
        range[0] = minMag;
        range[1] = maxMag;
        
        vtkSmartPointer<vtkDoubleArray> magArray = 
            vtkSmartPointer<vtkDoubleArray>::New();
        magArray->SetName("Magnitude");
        magArray->SetNumberOfTuples(numTuples);
        
        for (vtkIdType i = 0; i < numTuples; i++)
        {
            double mag = 0;
            for (int j = 0; j < numComponents; j++)
            {
                double val = array->GetComponent(i, j);
                mag += val * val;
            }
            mag = sqrt(mag);
            magArray->SetValue(i, mag);
        }
        
        mapper->SetScalarVisibility(true);
        mapper->SetScalarRange(range);
        mapper->SetScalarModeToUsePointFieldData();
        mapper->SelectColorArray("Magnitude");
        mapper->SetLookupTable(lut);
        
        reader->GetOutput()->GetPointData()->AddArray(magArray);
    }
    else
    {
        int comp = componentIndex - 1;
        if (comp < numComponents)
        {
            array->GetRange(range, comp);
            mapper->SetScalarVisibility(true);
            mapper->SetScalarRange(range);
            mapper->SetScalarModeToUsePointFieldData();
            mapper->SelectColorArray(currentArrayName.toStdString().c_str());
            mapper->SetArrayId(comp);
            mapper->SetLookupTable(lut);
        }
    }
    
    if (scalarBar)
    {
        renderer->RemoveActor(scalarBar);
    }
    
    scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
    scalarBar->SetLookupTable(lut);
    scalarBar->SetTitle(currentArrayName.toStdString().c_str());
    scalarBar->SetNumberOfLabels(8);
    renderer->AddActor(scalarBar);
    
    renderWindow->Render();
}
