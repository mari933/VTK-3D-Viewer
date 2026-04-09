# 🖥️ VTK 3D Viewer

Интерактивное приложение для визуализации неструктурированных сеток из файлов `.vtu`.

## Функциональность

- Загрузка `.vtu` файлов (аргумент командной строки или диалог)
- Вращение, масштабирование, панорамирование мышью
- Выбор массива для раскраски (Displacement, Stress и др.)
- Выбор компоненты (Magnitude, X, Y, Z для векторов)
- Автоматическая цветовая шкала

## Сборка и запуск (Ubuntu)

```bash
sudo apt install qtbase5-dev libvtk9-dev libvtk9-qt-dev cmake
git clone https://github.com/mari933/VTK-3D-Viewer.git
cd VTK-3D-Viewer
mkdir build && cd build
cmake ..
make -j$(nproc)
./VTUViewer /путь/к/файлу.vtu
## Шаг 3: Сохраняем файл

В редакторе nano:

1. Нажми `Ctrl + X` (появится запрос внизу экрана)
2. Нажми `Y` (подтвердить сохранение)
3. Нажми `Enter` (подтвердить имя файла)

---

## Шаг 4: Добавляем README в git

```bash
git add README.md
