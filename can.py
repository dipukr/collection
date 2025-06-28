import sys
from PyQt6.QtWidgets import *
from PyQt6.QtGui import *
from PyQt6.QtCore import Qt, QPoint

class DrawingCanvas(QWidget):
    def __init__(self):
        super().__init__()
        self.setFixedSize(800, 600)
        self.setMouseTracking(True)
        self.drawing = False
        self.last_point = QPoint()
        self.paths = []
        # Create a pixmap to draw on (off-screen buffer)
        self.canvas = QPixmap(self.size())
        self.canvas.fill(Qt.GlobalColor.yellow)

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self.drawing = True
            self.last_point = event.position().toPoint()
            self.paths.append([self.last_point])

    def mouseMoveEvent(self, event):
        if self.drawing and event.buttons() & Qt.MouseButton.LeftButton:
            current_point = event.position().toPoint()
            self.paths[-1].append(current_point)
            self.update()  # Repaint canvas

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self.drawing = False

    def paintEvent(self, event):
        painter = QPainter(self)
        pen = QPen(Qt.GlobalColor.blue, 5, Qt.PenStyle.SolidLine)
        painter.setPen(pen)

        for path in self.paths:
            for i in range(len(path) - 1):
                painter.drawLine(path[i], path[i + 1])

    def save_image(self, filename):
        self.canvas.save(filename, "PNG")           

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.centralWid = DrawingCanvas()
        self.setWindowTitle("Mouse Drawing Canvas (PyQt6)")
        self.setCentralWidget(self.centralWid)
        menuBar = self.menuBar()
        file_menu = menuBar.addMenu("File")
        save_action = QAction("Save", self)
        save_action.triggered.connect(self.save_file)
        file_menu.addAction(save_action)

    def save_file(self):
        print("Saving")
        self.centralWid.save_image("img.png")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec())