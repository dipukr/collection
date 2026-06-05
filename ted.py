"""
Simple Multi-Tabbed File Editor with trapezoidal (Sublime-style) tabs.

Run:
    pip install PyQt5
    python ted.py
"""

import sys
import os
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QTabWidget, QPlainTextEdit, QFileDialog,
    QAction, QMessageBox, QTabBar, QWidget
)
from PyQt5.QtGui import (
    QPainter, QPainterPath, QColor, QFont, QFontMetrics, QPen, QBrush
)
from PyQt5.QtCore import Qt, QRect, QSize, QPoint, pyqtSignal


# ---------------------------------------------------------------------------
# Custom TabBar that paints trapezoidal (Sublime Text style) tabs
# ---------------------------------------------------------------------------
class TrapezoidTabBar(QTabBar):
    """A QTabBar whose tabs are drawn as slanted trapezoids."""

    SLANT = 17          # how far the slanted edge extends horizontally
    TAB_HEIGHT = 48     # fixed tab height
    CLOSE_BTN_SIZE = 16 # size of the small 'x' close button
    OVERLAP = 13        # how much each tab overlaps its neighbour
    MIN_TEXT_WIDTH = 150  # minimum width for the text portion of a tab

    tabCloseRequested2 = pyqtSignal(int)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setExpanding(False)
        self.setDrawBase(False)
        self.setMouseTracking(True)
        self._hover_close = -1   # index of tab whose close button is hovered

    # ---------- sizing ----------
    def tabSizeHint(self, index):
        text = self.tabText(index)
        fm = QFontMetrics(self.font())
        text_w = max(fm.horizontalAdvance(text), self.MIN_TEXT_WIDTH)
        # text width + slant on both sides + room for close button + padding
        w = text_w + 2 * self.SLANT + self.CLOSE_BTN_SIZE + 28
        return QSize(w, self.TAB_HEIGHT)

    def tabRect(self, index):
        """Shift each tab leftward by index * OVERLAP so the slanted edges
        overlap their neighbours (Sublime / Chrome style)."""
        r = super().tabRect(index)
        if r.isNull():
            return r
        return r.translated(-index * self.OVERLAP, 0)

    # ---------- helpers ----------
    def _tab_path(self, rect):
        """Build a trapezoidal path for a tab rectangle."""
        path = QPainterPath()
        x, y, w, h = rect.x(), rect.y(), rect.width(), rect.height()
        s = self.SLANT
        # Start bottom-left, go up the slanted left edge, across the top,
        # down the slanted right edge, back along the bottom.
        path.moveTo(x, y + h)
        path.lineTo(x + s, y + 4)
        path.quadTo(x + s + 2, y + 2, x + s + 6, y + 2)
        path.lineTo(x + w - s - 6, y + 2)
        path.quadTo(x + w - s - 2, y + 2, x + w - s, y + 4)
        path.lineTo(x + w, y + h)
        path.closeSubpath()
        return path

    def _close_btn_rect(self, tab_rect):
        """Rectangle for the 'x' close button inside a tab."""
        size = self.CLOSE_BTN_SIZE
        # place towards the right side, vertically centered, before the slant
        x = tab_rect.right() - self.SLANT - size - 6
        y = tab_rect.top() + (tab_rect.height() - size) // 2
        return QRect(x, y, size, size)

    # ---------- painting ----------
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing, True)

        # Background bar colour (matches Sublime's header strip)
        painter.fillRect(self.rect(), QColor("#cfc7c0"))

        current = self.currentIndex()

        # Paint inactive tabs first, then the active one on top.
        order = [i for i in range(self.count()) if i != current]
        if current != -1:
            order.append(current)

        for i in order:
            rect = self.tabRect(i)
            is_active = (i == current)

            # Extend the active tab a few pixels below the bar so its white
            # fill blends seamlessly into the editor pane (no visible seam).
            if is_active:
                rect = QRect(rect.x(), rect.y(),
                             rect.width(), rect.height() + 3)

            path = self._tab_path(rect)

            if is_active:
                fill = QColor("#ffffff")
                border = QColor("#a89f97")
                text_color = QColor("#2b2b2b")
            else:
                fill = QColor("#bdb4ab")
                border = QColor("#9c9389")
                text_color = QColor("#3a3a3a")

            # Fill with no stroke first
            painter.setPen(Qt.NoPen)
            painter.setBrush(QBrush(fill))
            painter.drawPath(path)

            # Draw the border WITHOUT the bottom edge so the active tab
            # merges cleanly with the editor below.
            painter.setPen(QPen(border, 1))
            x, y, w, h = rect.x(), rect.y(), rect.width(), rect.height()
            s = self.SLANT
            border_path = QPainterPath()
            border_path.moveTo(x, y + h)
            border_path.lineTo(x + s, y + 4)
            border_path.quadTo(x + s + 2, y + 2, x + s + 6, y + 2)
            border_path.lineTo(x + w - s - 6, y + 2)
            border_path.quadTo(x + w - s - 2, y + 2, x + w - s, y + 4)
            border_path.lineTo(x + w, y + h)
            painter.strokePath(border_path, QPen(border, 1))

            # Tab label
            painter.setPen(text_color)
            painter.setFont(self.font())
            text_rect = QRect(
                rect.left() + self.SLANT + 4,
                rect.top(),
                rect.width() - 2 * self.SLANT - self.CLOSE_BTN_SIZE - 12,
                rect.height(),
            )
            fm = QFontMetrics(self.font())
            elided = fm.elidedText(self.tabText(i), Qt.ElideRight, text_rect.width())
            painter.drawText(text_rect, Qt.AlignVCenter | Qt.AlignLeft, elided)

            # Close button ('x')
            cb = self._close_btn_rect(rect)
            if i == self._hover_close:
                painter.setBrush(QColor("#d9534f"))
                painter.setPen(Qt.NoPen)
                painter.drawEllipse(cb)
                pen_color = QColor("#ffffff")
            else:
                pen_color = QColor("#7a7a7a") if not is_active else QColor("#666666")

            painter.setPen(QPen(pen_color, 1.5))
            pad = 4
            painter.drawLine(cb.left() + pad, cb.top() + pad,
                             cb.right() - pad, cb.bottom() - pad)
            painter.drawLine(cb.right() - pad, cb.top() + pad,
                             cb.left() + pad, cb.bottom() - pad)

        painter.end()

    # ---------- mouse interaction ----------
    def _tab_at(self, pos):
        """Return the index of the topmost tab whose trapezoidal path contains
        pos, or -1. Paint order is: inactive in ascending order, then active
        on top — so we search in reverse paint order to find the top-most."""
        current = self.currentIndex()
        # Build hit-test order: current first (since it's painted last),
        # then remaining tabs from highest index down to lowest.
        order = []
        if current != -1:
            order.append(current)
        for i in range(self.count() - 1, -1, -1):
            if i != current:
                order.append(i)
        for i in order:
            rect = self.tabRect(i)
            if i == current:
                rect = QRect(rect.x(), rect.y(),
                             rect.width(), rect.height() + 3)
            if self._tab_path(rect).contains(pos):
                return i
        return -1

    def mouseMoveEvent(self, event):
        new_hover = -1
        # Close-button hit: same top-most-first rule.
        hit = self._tab_at(event.pos())
        if hit != -1 and self._close_btn_rect(self.tabRect(hit)).contains(event.pos()):
            new_hover = hit
        if new_hover != self._hover_close:
            self._hover_close = new_hover
            self.update()
        super().mouseMoveEvent(event)

    def leaveEvent(self, event):
        if self._hover_close != -1:
            self._hover_close = -1
            self.update()
        super().leaveEvent(event)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            hit = self._tab_at(event.pos())
            if hit != -1:
                # Close button takes priority over tab activation.
                if self._close_btn_rect(self.tabRect(hit)).contains(event.pos()):
                    self.tabCloseRequested2.emit(hit)
                    return
                # Activate the top-most tab under the cursor and swallow the
                # event so QTabBar's own (rect-based) selection doesn't pick a
                # different one in the overlap region.
                self.setCurrentIndex(hit)
                return
        super().mousePressEvent(event)


# ---------------------------------------------------------------------------
# Main editor window
# ---------------------------------------------------------------------------
class TabbedEditor(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("untitled • - Simple Editor")
        self.resize(1200, 900)

        self.tabs = QTabWidget()
        self.tab_bar = TrapezoidTabBar(self.tabs)
        self.tabs.setTabBar(self.tab_bar)
        self.tabs.setDocumentMode(True)
        self.tab_bar.tabCloseRequested2.connect(self.close_tab)
        self.setCentralWidget(self.tabs)

        # Style the editor area + tab strip
        self.setStyleSheet("""
            QMainWindow { background: #cfc7c0; }
            QPlainTextEdit {
                background: #ffffff;
                color: #2b2b2b;
                border: none;
                padding: 8px;
                font-family: Consolas, 'Courier New', monospace;
                font-size: 12pt;
            }
            QTabWidget::pane {
                border: none;
                background: #ffffff;
                top: -2px;
                margin: 0px;
            }
            QTabWidget::tab-bar {
                left: 0px;
            }
        """)

        self._build_menu()

        # Start with one empty tab
        self.new_tab()

    # ---------- menu ----------
    def _build_menu(self):
        mb = self.menuBar()
        file_menu = mb.addMenu("&File")

        new_act = QAction("&New", self, shortcut="Ctrl+N",
                          triggered=lambda: self.new_tab())
        open_act = QAction("&Open...", self, shortcut="Ctrl+O", triggered=self.open_file)
        save_act = QAction("&Save", self, shortcut="Ctrl+S", triggered=self.save_file)
        save_as_act = QAction("Save &As...", self, shortcut="Ctrl+Shift+S",
                              triggered=self.save_file_as)
        close_act = QAction("&Close Tab", self, shortcut="Ctrl+W",
                            triggered=lambda: self.close_tab(self.tabs.currentIndex()))
        quit_act = QAction("&Quit", self, shortcut="Ctrl+Q", triggered=self.close)

        for a in (new_act, open_act, save_act, save_as_act):
            file_menu.addAction(a)
        file_menu.addSeparator()
        file_menu.addAction(close_act)
        file_menu.addSeparator()
        file_menu.addAction(quit_act)

    # ---------- tab operations ----------
    def new_tab(self, title="untitled", content=""):
        editor = QPlainTextEdit()
        editor.setPlainText(content)
        editor.setProperty("file_path", None)
        idx = self.tabs.addTab(editor, title)
        self.tabs.setCurrentIndex(idx)
        return editor

    def close_tab(self, index):
        if index < 0 or index >= self.tabs.count():
            return
        widget = self.tabs.widget(index)
        if widget and widget.document().isModified():
            ans = QMessageBox.question(
                self, "Unsaved changes",
                f"'{self.tabs.tabText(index)}' has unsaved changes. Close anyway?",
                QMessageBox.Yes | QMessageBox.No
            )
            if ans != QMessageBox.Yes:
                return
        self.tabs.removeTab(index)
        if self.tabs.count() == 0:
            self.new_tab()

    # ---------- file operations ----------
    def open_file(self):
        path, _ = QFileDialog.getOpenFileName(self, "Open File", "", "All Files (*.*)")
        if not path:
            return
        try:
            with open(path, "r", encoding="utf-8", errors="replace") as f:
                content = f.read()
        except OSError as e:
            QMessageBox.warning(self, "Open failed", str(e))
            return
        editor = self.new_tab(os.path.basename(path), content)
        editor.setProperty("file_path", path)
        editor.document().setModified(False)

    def save_file(self):
        editor = self.tabs.currentWidget()
        if editor is None:
            return
        path = editor.property("file_path")
        if not path:
            return self.save_file_as()
        self._write(editor, path)

    def save_file_as(self):
        editor = self.tabs.currentWidget()
        if editor is None:
            return
        path, _ = QFileDialog.getSaveFileName(self, "Save As", "", "All Files (*.*)")
        if not path:
            return
        self._write(editor, path)
        editor.setProperty("file_path", path)
        self.tabs.setTabText(self.tabs.currentIndex(), os.path.basename(path))

    def _write(self, editor, path):
        try:
            with open(path, "w", encoding="utf-8") as f:
                f.write(editor.toPlainText())
            editor.document().setModified(False)
        except OSError as e:
            QMessageBox.warning(self, "Save failed", str(e))


def main():
    app = QApplication(sys.argv)
    app.setFont(QFont("Segoe UI", 9))
    w = TabbedEditor()
    w.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
