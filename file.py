#!/usr/bin/env python3
"""
Simple File Explorer in PyQt5
Look based on the provided HTML/CSS reference and screenshot.
"""

import os
import sys
import subprocess

from PyQt5.QtCore import Qt, QSize, QStandardPaths
from PyQt5.QtGui import QPainter, QColor, QPixmap, QIcon, QPen, QPainterPath, QFont
from PyQt5.QtWidgets import (
    QApplication, QWidget, QHBoxLayout, QVBoxLayout, QLabel, QPushButton,
    QFrame, QScrollArea, QGridLayout, QLineEdit, QSizePolicy, QStyle,
    QFileIconProvider,
)
from PyQt5.QtCore import QFileInfo


# ----------------------------------------------------------------------------
# Colors / constants (from the CSS reference)
# ----------------------------------------------------------------------------
BG_WINDOW      = "#f0f0f0"
BG_CARD        = "#ffffff"
BG_SIDEBAR     = "#f5f5f5"
BORDER         = "#e0e0e0"
HOVER          = "#e8e8e8"
ACTIVE         = "#e0e0e0"
TEXT           = "#333333"
FOLDER_BLUE    = "#5DADE2"
FOLDER_BLUE_LT = "#85C1E9"


# ----------------------------------------------------------------------------
# SVG-style stroke icon rendering (mirrors the lucide-style icons in the HTML)
# ----------------------------------------------------------------------------
def _stroke_icon(draw_fn, size=20, color="#333333", width=2.0):
    """Create a QIcon by drawing on a 24x24 canvas scaled to `size`."""
    pm = QPixmap(size, size)
    pm.fill(Qt.transparent)
    p = QPainter(pm)
    p.setRenderHint(QPainter.Antialiasing)
    p.scale(size / 24.0, size / 24.0)
    pen = QPen(QColor(color))
    pen.setWidthF(width)
    pen.setCapStyle(Qt.RoundCap)
    pen.setJoinStyle(Qt.RoundJoin)
    p.setPen(pen)
    p.setBrush(Qt.NoBrush)
    draw_fn(p)
    p.end()
    return QIcon(pm)


def icon_search(p):
    p.drawEllipse(3, 3, 16, 16)
    p.drawLine(21, 21, 16, 16)

def icon_menu(p):
    for y in (6, 12, 18):
        p.drawLine(3, y, 21, y)

def icon_home(p):
    path = QPainterPath()
    path.moveTo(3, 9); path.lineTo(12, 2); path.lineTo(21, 9)
    path.lineTo(21, 20); path.lineTo(3, 20); path.closeSubpath()
    p.drawPath(path)

def icon_clock(p):
    p.drawEllipse(2, 2, 20, 20)
    p.drawLine(12, 7, 12, 12)
    p.drawLine(12, 12, 16, 14)

def icon_star(p):
    pts = [(12,2),(15,9),(22,9),(17,14),(18,21),(12,17),(6,21),(7,14),(2,9),(9,9)]
    path = QPainterPath()
    path.moveTo(*pts[0])
    for x, y in pts[1:]:
        path.lineTo(x, y)
    path.closeSubpath()
    p.drawPath(path)

def icon_network(p):
    p.drawEllipse(10, 10, 4, 4)
    p.drawLine(12, 2, 12, 6); p.drawLine(12, 18, 12, 22)
    p.drawLine(2, 12, 6, 12);  p.drawLine(18, 12, 22, 12)

def icon_trash(p):
    p.drawLine(3, 6, 21, 6)
    path = QPainterPath()
    path.moveTo(5, 6); path.lineTo(5, 20); path.lineTo(19, 20); path.lineTo(19, 6)
    p.drawPath(path)
    p.drawLine(10, 4, 14, 4)

def icon_doc(p):
    path = QPainterPath()
    path.moveTo(13, 2); path.lineTo(6, 2); path.lineTo(6, 22)
    path.lineTo(18, 22); path.lineTo(18, 9); path.closeSubpath()
    p.drawPath(path)
    p.drawLine(13, 2, 13, 9); p.drawLine(13, 9, 18, 9)

def icon_music(p):
    path = QPainterPath()
    path.moveTo(9, 18); path.lineTo(9, 5); path.lineTo(21, 3); path.lineTo(21, 16)
    p.drawPath(path)
    p.drawEllipse(3, 15, 6, 6)
    p.drawEllipse(15, 13, 6, 6)

def icon_pictures(p):
    p.drawRoundedRect(3, 3, 18, 18, 2, 2)
    p.drawEllipse(7, 7, 3, 3)
    path = QPainterPath()
    path.moveTo(21, 15); path.lineTo(16, 10); path.lineTo(5, 21)
    p.drawPath(path)

def icon_videos(p):
    path = QPainterPath()
    path.moveTo(23, 7); path.lineTo(16, 12); path.lineTo(23, 17); path.closeSubpath()
    p.drawPath(path)
    p.drawRoundedRect(1, 5, 15, 14, 2, 2)

def icon_downloads(p):
    path = QPainterPath()
    path.moveTo(3, 15); path.lineTo(3, 21); path.lineTo(21, 21); path.lineTo(21, 15)
    p.drawPath(path)
    p.drawLine(7, 10, 12, 15); p.drawLine(12, 15, 17, 10)
    p.drawLine(12, 15, 12, 3)

def icon_chevron_left(p):
    path = QPainterPath()
    path.moveTo(15, 18); path.lineTo(9, 12); path.lineTo(15, 6)
    p.drawPath(path)

def icon_chevron_right(p):
    path = QPainterPath()
    path.moveTo(9, 18); path.lineTo(15, 12); path.lineTo(9, 6)
    p.drawPath(path)

def icon_dots(p):
    for cy in (5, 12, 19):
        p.drawEllipse(11, cy - 1, 2, 2)

def icon_grid(p):
    for x, y in ((3,3),(14,3),(14,14),(3,14)):
        p.drawRect(x, y, 7, 7)

def icon_close(p):
    p.drawLine(18, 6, 6, 18)
    p.drawLine(6, 6, 18, 18)


# ----------------------------------------------------------------------------
# Folder icon (the filled blue folder with a glyph, from createFolderIcon)
# ----------------------------------------------------------------------------
def make_folder_pixmap(glyph_fn, color_hex, size=80):
    pm = QPixmap(size, size)
    pm.fill(Qt.transparent)
    p = QPainter(pm)
    p.setRenderHint(QPainter.Antialiasing)
    # 100x80 viewBox -> scale into square
    p.scale(size / 100.0, size / 80.0)
    col = QColor(color_hex)

    # back tab part (slightly transparent)
    back = QPainterPath()
    back.moveTo(10, 10); back.lineTo(45, 10); back.lineTo(55, 20)
    back.lineTo(90, 20)
    back.arcTo(80, 20, 20, 20, 90, -90)
    back.lineTo(100, 60)
    back.arcTo(80, 60, 20, 20, 0, -90)
    back.lineTo(10, 70)
    back.arcTo(0, 60, 20, 20, -90, -90)
    back.lineTo(0, 20)
    back.arcTo(0, 10, 20, 20, 180, -90)
    back.closeSubpath()
    c2 = QColor(col); c2.setAlphaF(0.9)
    p.fillPath(back, c2)

    # front body
    front = QPainterPath()
    front.moveTo(10, 20); front.lineTo(90, 20); front.lineTo(90, 65)
    front.arcTo(85, 60, 10, 10, 90, -90)
    front.lineTo(15, 70)
    front.arcTo(10, 60, 10, 10, -90, -90)
    front.closeSubpath()
    p.fillPath(front, col)

    # white glyph centered, drawn in 24x24 local space
    p.save()
    p.translate(38, 28)
    p.scale(24 / 24.0, 24 / 24.0)
    # scale glyph to ~24px region
    p.scale(1.0, 1.0)
    pen = QPen(QColor("white"))
    pen.setWidthF(2.2)
    pen.setCapStyle(Qt.RoundCap)
    pen.setJoinStyle(Qt.RoundJoin)
    p.setPen(pen)
    p.setBrush(Qt.NoBrush)
    glyph_fn(p)
    p.restore()

    p.end()
    return pm


# glyph drawings (white inner icons) in 24x24 space
def g_desktop(p):
    p.drawEllipse(8, 3, 8, 8)
    path = QPainterPath()
    path.moveTo(4, 21); path.lineTo(4, 19)
    p.drawPath(path)
    p.drawRoundedRect(6, 9, 12, 12, 1, 1)

def g_doc(p):
    path = QPainterPath()
    path.moveTo(13, 2); path.lineTo(6, 2); path.lineTo(6, 22)
    path.lineTo(18, 22); path.lineTo(18, 9); path.closeSubpath()
    p.drawPath(path)
    p.drawLine(13, 2, 13, 9); p.drawLine(13, 9, 18, 9)

def g_downloads(p):
    path = QPainterPath()
    path.moveTo(3, 15); path.lineTo(3, 21); path.lineTo(21, 21); path.lineTo(21, 15)
    p.drawPath(path)
    p.drawLine(7, 10, 12, 15); p.drawLine(12, 15, 17, 10)
    p.drawLine(12, 15, 12, 3)

def g_music(p):
    path = QPainterPath()
    path.moveTo(9, 18); path.lineTo(9, 5); path.lineTo(21, 3); path.lineTo(21, 16)
    p.drawPath(path)
    p.drawEllipse(3, 15, 6, 6)
    p.drawEllipse(15, 13, 6, 6)

def g_pictures(p):
    p.drawRect(3, 3, 18, 18)
    p.drawEllipse(7, 7, 3, 3)
    path = QPainterPath()
    path.moveTo(21, 15); path.lineTo(16, 10); path.lineTo(5, 21)
    p.drawPath(path)

def g_public(p):
    path = QPainterPath()
    path.moveTo(4, 22); path.lineTo(4, 20)
    p.drawPath(path)
    p.drawEllipse(5, 4, 8, 8)
    p.drawRoundedRect(4, 18, 12, 4, 1, 1)
    p.drawLine(19, 8, 19, 14)
    path2 = QPainterPath()
    path2.moveTo(16, 11); path2.lineTo(19, 14); path2.lineTo(22, 11)
    p.drawPath(path2)

def g_videos(p):
    p.drawRect(1, 5, 15, 14)
    path = QPainterPath()
    path.moveTo(23, 7); path.lineTo(16, 12); path.lineTo(23, 17); path.closeSubpath()
    p.drawPath(path)


FOLDER_GLYPHS = {
    "Desktop":   (g_desktop,   FOLDER_BLUE),
    "Documents": (g_doc,       FOLDER_BLUE),
    "Downloads": (g_downloads, FOLDER_BLUE_LT),
    "Music":     (g_music,     FOLDER_BLUE_LT),
    "Pictures":  (g_pictures,  FOLDER_BLUE),
    "Public":    (g_public,    FOLDER_BLUE),
    "Templates": (g_doc,       FOLDER_BLUE),
    "Videos":    (g_videos,    FOLDER_BLUE_LT),
}


# ----------------------------------------------------------------------------
# Widgets
# ----------------------------------------------------------------------------
class IconButton(QPushButton):
    def __init__(self, draw_fn, tip="", icon_size=20, parent=None):
        super().__init__(parent)
        self.setIcon(_stroke_icon(draw_fn, size=icon_size, color=TEXT))
        self.setIconSize(QSize(icon_size, icon_size))
        self.setFixedSize(36, 36)
        self.setCursor(Qt.PointingHandCursor)
        if tip:
            self.setToolTip(tip)
        self.setStyleSheet("""
            QPushButton { border: none; background: transparent; border-radius: 6px; }
            QPushButton:hover { background: %s; }
        """ % ACTIVE)


class NavItem(QPushButton):
    def __init__(self, draw_fn, text, parent=None):
        super().__init__(text, parent)
        self.setIcon(_stroke_icon(draw_fn, size=18, color=TEXT))
        self.setIconSize(QSize(18, 18))
        self.setCheckable(True)
        self.setCursor(Qt.PointingHandCursor)
        self.setFixedHeight(38)
        self.setStyleSheet("""
            QPushButton {
                border: none; background: transparent; text-align: left;
                padding: 0 20px; font-size: 14px; color: %s;
            }
            QPushButton:hover { background: %s; }
            QPushButton:checked { background: %s; }
        """ % (TEXT, HOVER, ACTIVE))


class FolderItem(QFrame):
    def __init__(self, name, on_open, parent=None):
        super().__init__(parent)
        self.name = name
        self.on_open = on_open
        self.setCursor(Qt.PointingHandCursor)
        self.setFixedSize(130, 130)
        self.setStyleSheet("""
            QFrame { border-radius: 8px; background: transparent; }
            QFrame:hover { background: %s; }
        """ % BG_SIDEBAR)

        lay = QVBoxLayout(self)
        lay.setContentsMargins(15, 15, 15, 15)
        lay.setSpacing(8)
        lay.setAlignment(Qt.AlignCenter)

        glyph_fn, color = FOLDER_GLYPHS.get(name, (g_doc, FOLDER_BLUE))
        icon = QLabel()
        icon.setPixmap(make_folder_pixmap(glyph_fn, color, size=72))
        icon.setAlignment(Qt.AlignCenter)
        lay.addWidget(icon)

        lbl = QLabel(name)
        lbl.setAlignment(Qt.AlignCenter)
        lbl.setStyleSheet("font-size: 13px; color: %s; background: transparent;" % TEXT)
        lay.addWidget(lbl)

    def mouseDoubleClickEvent(self, e):
        if self.on_open:
            self.on_open(self.name)


# ----------------------------------------------------------------------------
# Main window
# ----------------------------------------------------------------------------
class DragBar(QFrame):
    """A frame that drags the parent window when clicked, and double-click
    toggles maximize/restore."""
    def __init__(self, window, parent=None):
        super().__init__(parent)
        self._win = window
        self._press = None

    def mousePressEvent(self, e):
        if e.button() == Qt.LeftButton:
            self._press = e.globalPos() - self._win.frameGeometry().topLeft()
            e.accept()

    def mouseMoveEvent(self, e):
        if self._press is not None and e.buttons() & Qt.LeftButton:
            if self._win.isMaximized():
                self._win.showNormal()
            self._win.move(e.globalPos() - self._press)
            e.accept()

    def mouseReleaseEvent(self, e):
        self._press = None

    def mouseDoubleClickEvent(self, e):
        if self._win.isMaximized():
            self._win.showNormal()
        else:
            self._win.showMaximized()


class FileExplorer(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("File Explorer")
        self.resize(1000, 700)
        self.setMinimumSize(640, 460)

        # Frameless: remove the native OS title bar / window chrome.
        self.setWindowFlags(Qt.FramelessWindowHint | Qt.Window)
        self.setAttribute(Qt.WA_TranslucentBackground)

        self.icon_provider = QFileIconProvider()
        self.setMouseTracking(True)

        # drag / resize state
        self._drag_pos = None
        self._resize_edge = None
        self._resize_margin = 6

        outer = QVBoxLayout(self)
        outer.setContentsMargins(20, 20, 20, 20)

        card = QFrame()
        card.setObjectName("card")
        card.setStyleSheet(
            "QFrame#card { background: %s; border-radius: 12px;"
            "border: 1px solid %s; }" % (BG_CARD, BORDER)
        )
        # subtle shadow like the reference
        try:
            from PyQt5.QtWidgets import QGraphicsDropShadowEffect
            shadow = QGraphicsDropShadowEffect(self)
            shadow.setBlurRadius(40)
            shadow.setXOffset(0)
            shadow.setYOffset(10)
            shadow.setColor(QColor(0, 0, 0, 40))
            card.setGraphicsEffect(shadow)
        except Exception:
            pass
        outer.addWidget(card)

        root = QHBoxLayout(card)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)

        root.addWidget(self._build_sidebar())
        root.addWidget(self._build_main(), 1)

        self.navigate_home()

    # --- sidebar ---------------------------------------------------------
    def _build_sidebar(self):
        sidebar = QFrame()
        sidebar.setFixedWidth(250)
        sidebar.setStyleSheet(
            "QFrame { background: %s; border-right: 1px solid %s;"
            "border-top-left-radius: 12px; border-bottom-left-radius: 12px; }"
            % (BG_SIDEBAR, BORDER)
        )
        v = QVBoxLayout(sidebar)
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)

        # header (also draggable as part of the custom title bar)
        header = DragBar(self)
        header.setStyleSheet(
            "background: %s; border-bottom: 1px solid %s;"
            "border-top-left-radius: 12px;" % (BG_SIDEBAR, BORDER)
        )
        h = QHBoxLayout(header)
        h.setContentsMargins(20, 18, 20, 18)
        h.setSpacing(15)
        h.addWidget(IconButton(icon_search, "Search"))
        title = QLabel("Files")
        title.setStyleSheet("font-weight: 600; font-size: 15px; color: %s;" % TEXT)
        h.addWidget(title)
        h.addStretch(1)
        h.addWidget(IconButton(icon_menu, "Menu"))
        v.addWidget(header)

        # nav
        nav = QVBoxLayout()
        nav.setContentsMargins(0, 10, 0, 10)
        nav.setSpacing(2)

        self.nav_items = []

        top = [
            (icon_home, "Home"), (icon_clock, "Recent"),
            (icon_star, "Starred"), (icon_network, "Network"),
            (icon_trash, "Trash"),
        ]
        bottom = [
            (icon_doc, "Documents"), (icon_music, "Music"),
            (icon_pictures, "Pictures"), (icon_videos, "Videos"),
            (icon_downloads, "Downloads"),
        ]

        for fn, name in top:
            item = NavItem(fn, name)
            item.clicked.connect(lambda _, n=name, it=item: self._nav_clicked(n, it))
            nav.addWidget(item)
            self.nav_items.append(item)

        divider = QFrame()
        divider.setFixedHeight(1)
        divider.setStyleSheet("background: %s; margin: 10px 0;" % BORDER)
        nav.addWidget(divider)

        for fn, name in bottom:
            item = NavItem(fn, name)
            item.clicked.connect(lambda _, n=name, it=item: self._nav_clicked(n, it))
            nav.addWidget(item)
            self.nav_items.append(item)

        nav.addStretch(1)
        v.addLayout(nav)

        # default active = Home
        self.nav_items[0].setChecked(True)
        return sidebar

    # --- main ------------------------------------------------------------
    def _build_main(self):
        main = QFrame()
        main.setStyleSheet("background: %s; border-top-right-radius: 12px;"
                           "border-bottom-right-radius: 12px;" % BG_CARD)
        v = QVBoxLayout(main)
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)

        # toolbar (doubles as the custom title bar — draggable)
        toolbar = DragBar(self)
        toolbar.setStyleSheet(
            "background: %s; border-bottom: 1px solid %s;"
            "border-top-right-radius: 12px;" % (BG_CARD, BORDER)
        )
        t = QHBoxLayout(toolbar)
        t.setContentsMargins(20, 15, 20, 15)
        t.setSpacing(10)

        self.back_btn = IconButton(icon_chevron_left, "Back")
        self.fwd_btn = IconButton(icon_chevron_right, "Forward")
        self.back_btn.clicked.connect(self.go_back)
        self.fwd_btn.clicked.connect(self.go_forward)
        t.addWidget(self.back_btn)
        t.addWidget(self.fwd_btn)

        breadcrumb = QFrame()
        breadcrumb.setStyleSheet("background: %s; border-radius: 6px;" % BG_SIDEBAR)
        b = QHBoxLayout(breadcrumb)
        b.setContentsMargins(12, 8, 12, 8)
        b.setSpacing(8)
        crumb_icon = QLabel()
        crumb_icon.setPixmap(_stroke_icon(icon_home, 18, TEXT).pixmap(18, 18))
        b.addWidget(crumb_icon)
        self.crumb_label = QLabel("Home")
        self.crumb_label.setStyleSheet("font-size: 14px; color: %s; background: transparent;" % TEXT)
        b.addWidget(self.crumb_label)
        b.addStretch(1)
        t.addWidget(breadcrumb, 1)

        t.addWidget(IconButton(icon_dots, "More"))
        t.addWidget(IconButton(icon_grid, "Grid view"))
        t.addWidget(IconButton(icon_menu, "List view"))
        close_btn = IconButton(icon_close, "Close")
        close_btn.clicked.connect(self.close)
        t.addWidget(close_btn)

        v.addWidget(toolbar)

        # content (scrollable grid)
        self.scroll = QScrollArea()
        self.scroll.setWidgetResizable(True)
        self.scroll.setFrameShape(QFrame.NoFrame)
        self.scroll.setStyleSheet("QScrollArea { background: %s; }" % BG_CARD)

        self.grid_host = QWidget()
        self.grid_host.setStyleSheet("background: %s;" % BG_CARD)
        self.grid = QGridLayout(self.grid_host)
        self.grid.setContentsMargins(30, 30, 30, 30)
        self.grid.setHorizontalSpacing(15)
        self.grid.setVerticalSpacing(15)
        self.grid.setAlignment(Qt.AlignTop | Qt.AlignLeft)

        self.scroll.setWidget(self.grid_host)
        v.addWidget(self.scroll, 1)
        return main

    # --- navigation logic -----------------------------------------------
    def _nav_clicked(self, name, item):
        for it in self.nav_items:
            it.setChecked(False)
        item.setChecked(True)

        home = os.path.expanduser("~")
        mapping = {
            "Home": home,
            "Documents": os.path.join(home, "Documents"),
            "Music": os.path.join(home, "Music"),
            "Pictures": os.path.join(home, "Pictures"),
            "Videos": os.path.join(home, "Videos"),
            "Downloads": os.path.join(home, "Downloads"),
            "Trash": home,  # placeholder
        }
        path = mapping.get(name, home)
        if not os.path.isdir(path):
            path = home
        self.crumb_label.setText(name)
        self.load_directory(path, crumb=name)

    def navigate_home(self):
        self._home_folders = [
            "Desktop", "Documents", "Downloads", "Music",
            "Pictures", "Public", "Templates", "Videos",
        ]
        self.crumb_label.setText("Home")
        self.populate_home()

    def populate_home(self):
        self._clear_grid()
        cols = 4
        for i, name in enumerate(self._home_folders):
            item = FolderItem(name, self.open_named_folder)
            self.grid.addWidget(item, i // cols, i % cols)

    def open_named_folder(self, name):
        home = os.path.expanduser("~")
        path = os.path.join(home, name)
        if os.path.isdir(path):
            self.load_directory(path, crumb=name)
        else:
            # fall back: just update breadcrumb
            self.crumb_label.setText(name)

    def load_directory(self, path, crumb=None):
        self._clear_grid()
        self.crumb_label.setText(crumb or os.path.basename(path) or path)
        try:
            entries = sorted(
                os.listdir(path),
                key=lambda n: (not os.path.isdir(os.path.join(path, n)), n.lower())
            )
        except OSError:
            entries = []

        cols = 4
        count = 0
        for name in entries:
            if name.startswith("."):
                continue
            full = os.path.join(path, name)
            w = self._make_entry_widget(full, name)
            self.grid.addWidget(w, count // cols, count % cols)
            count += 1

        if count == 0:
            empty = QLabel("This folder is empty")
            empty.setStyleSheet("color: #999; font-size: 14px; background: transparent;")
            self.grid.addWidget(empty, 0, 0)

    def _make_entry_widget(self, full, name):
        is_dir = os.path.isdir(full)
        if is_dir:
            return FolderItem(
                name if name in FOLDER_GLYPHS else "Documents",
                lambda _n: self.load_directory(full, crumb=name)
            ) if name in FOLDER_GLYPHS else self._generic_folder(full, name)
        return self._file_widget(full, name)

    def _generic_folder(self, full, name):
        frame = QFrame()
        frame.setFixedSize(130, 130)
        frame.setCursor(Qt.PointingHandCursor)
        frame.setStyleSheet(
            "QFrame { border-radius: 8px; background: transparent; }"
            "QFrame:hover { background: %s; }" % BG_SIDEBAR
        )
        lay = QVBoxLayout(frame)
        lay.setContentsMargins(15, 15, 15, 15)
        lay.setSpacing(8)
        lay.setAlignment(Qt.AlignCenter)
        icon = QLabel()
        icon.setPixmap(make_folder_pixmap(g_doc, FOLDER_BLUE, size=72))
        icon.setAlignment(Qt.AlignCenter)
        lay.addWidget(icon)
        lbl = QLabel(name)
        lbl.setAlignment(Qt.AlignCenter)
        lbl.setWordWrap(True)
        lbl.setStyleSheet("font-size: 13px; color: %s; background: transparent;" % TEXT)
        lay.addWidget(lbl)
        frame.mouseDoubleClickEvent = lambda e: self.load_directory(full, crumb=name)
        return frame

    def _file_widget(self, full, name):
        frame = QFrame()
        frame.setFixedSize(130, 130)
        frame.setCursor(Qt.PointingHandCursor)
        frame.setStyleSheet(
            "QFrame { border-radius: 8px; background: transparent; }"
            "QFrame:hover { background: %s; }" % BG_SIDEBAR
        )
        lay = QVBoxLayout(frame)
        lay.setContentsMargins(15, 15, 15, 15)
        lay.setSpacing(8)
        lay.setAlignment(Qt.AlignCenter)
        icon = QLabel()
        qicon = self.icon_provider.icon(QFileInfo(full))
        icon.setPixmap(qicon.pixmap(56, 56))
        icon.setAlignment(Qt.AlignCenter)
        lay.addWidget(icon)
        lbl = QLabel(name)
        lbl.setAlignment(Qt.AlignCenter)
        lbl.setWordWrap(True)
        lbl.setStyleSheet("font-size: 13px; color: %s; background: transparent;" % TEXT)
        lay.addWidget(lbl)
        frame.mouseDoubleClickEvent = lambda e: self._open_file(full)
        return frame

    def _open_file(self, full):
        try:
            if sys.platform.startswith("darwin"):
                subprocess.Popen(["open", full])
            elif os.name == "nt":
                os.startfile(full)  # type: ignore
            else:
                subprocess.Popen(["xdg-open", full])
        except Exception:
            pass

    def _clear_grid(self):
        while self.grid.count():
            item = self.grid.takeAt(0)
            w = item.widget()
            if w:
                w.deleteLater()

    # --- back/forward history -------------------------------------------
    def go_back(self):
        # Simple: return to Home view
        for it in self.nav_items:
            it.setChecked(False)
        self.nav_items[0].setChecked(True)
        self.navigate_home()

    def go_forward(self):
        pass

    # --- frameless window edge resizing ---------------------------------
    def _edge_at(self, pos):
        m = self._resize_margin + 14  # +card margin tolerance
        r = self.rect()
        left = pos.x() <= m
        right = pos.x() >= r.width() - m
        top = pos.y() <= m
        bottom = pos.y() >= r.height() - m
        return left, top, right, bottom

    def _cursor_for_edge(self, edge):
        l, t, r, b = edge
        if (l and t) or (r and b):
            return Qt.SizeFDiagCursor
        if (r and t) or (l and b):
            return Qt.SizeBDiagCursor
        if l or r:
            return Qt.SizeHorCursor
        if t or b:
            return Qt.SizeVerCursor
        return Qt.ArrowCursor

    def mousePressEvent(self, e):
        if e.button() == Qt.LeftButton:
            edge = self._edge_at(e.pos())
            if any(edge):
                self._resize_edge = edge
                self._resize_start = e.globalPos()
                self._start_geom = self.geometry()
                e.accept()

    def mouseMoveEvent(self, e):
        if self._resize_edge and (e.buttons() & Qt.LeftButton):
            self._do_resize(e.globalPos())
            e.accept()
            return
        edge = self._edge_at(e.pos())
        self.setCursor(self._cursor_for_edge(edge))

    def mouseReleaseEvent(self, e):
        self._resize_edge = None
        self.setCursor(Qt.ArrowCursor)

    def _do_resize(self, gpos):
        l, t, r, b = self._resize_edge
        d = gpos - self._resize_start
        g = self.geometry()
        sg = self._start_geom
        minw, minh = self.minimumWidth(), self.minimumHeight()
        x, y, w, h = sg.x(), sg.y(), sg.width(), sg.height()
        if r:
            w = max(minw, sg.width() + d.x())
        if b:
            h = max(minh, sg.height() + d.y())
        if l:
            neww = max(minw, sg.width() - d.x())
            x = sg.x() + (sg.width() - neww)
            w = neww
        if t:
            newh = max(minh, sg.height() - d.y())
            y = sg.y() + (sg.height() - newh)
            h = newh
        self.setGeometry(x, y, w, h)


def main():
    app = QApplication(sys.argv)
    app.setFont(QFont("Segoe UI", 10))
    win = FileExplorer()
    win.show()
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()