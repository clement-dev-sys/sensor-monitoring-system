import sys
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QLabel, QPushButton, QStackedWidget)
from PyQt5.QtCore import Qt

# Import des classes d'onglets
from gui_realtime import RealtimeTab
from gui_history import HistoryTab
from gui_settings import SettingsTab
from gui_about import AboutTab

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.init_ui()
        # self.load_stylesheet()
    
    def init_ui(self):
        # Configuration de la fenêtre principale
        self.setWindowTitle("Monitoring MQTT & SQLite")
        
        # Widget central
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Layout principal vertical
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)
        
        # Zone des onglets (boutons en haut)
        tabs_layout = QHBoxLayout()
        tabs_layout.setContentsMargins(0, 0, 0, 0)
        tabs_layout.setSpacing(0)
        
        # Création des boutons d'onglets
        self.tab_buttons = []
        tab_names = ["Temps Réel", "Historique", "Paramètres", "À propos"]
        
        for i, name in enumerate(tab_names):
            btn = QPushButton(name)
            btn.setObjectName(f"tabButton")
            btn.setProperty("active", i == 0)
            btn.clicked.connect(lambda checked, index=i: self.switch_tab(index))
            self.tab_buttons.append(btn)
            tabs_layout.addWidget(btn)
        
        # Espacement pour pousser les onglets à gauche
        tabs_layout.addStretch()
        
        # Widget conteneur pour les onglets
        tabs_widget = QWidget()
        tabs_widget.setObjectName("tabsContainer")
        tabs_widget.setLayout(tabs_layout)
        main_layout.addWidget(tabs_widget)
        
        # Zone de contenu avec QStackedWidget
        self.stacked_widget = QStackedWidget()
        main_layout.addWidget(self.stacked_widget)
        
        # Création des onglets
        self.create_tabs()
    
    def create_tabs(self):
        # Onglet 1: Temps réel (MQTT)
        self.realtime_tab = RealtimeTab()
        self.stacked_widget.addWidget(self.realtime_tab)
        
        # Onglet 2: Historique (SQLite)
        self.history_tab = HistoryTab()
        self.stacked_widget.addWidget(self.history_tab)
        
        # Onglet 3: Paramètres
        self.settings_tab = SettingsTab()
        self.stacked_widget.addWidget(self.settings_tab)
        
        # Onglet 4: À propos
        self.about_tab = AboutTab()
        self.stacked_widget.addWidget(self.about_tab)
    
    def switch_tab(self, index):
        # Changer l'onglet affiché
        self.stacked_widget.setCurrentIndex(index)
        
        # Mettre à jour l'état actif des boutons
        for i, btn in enumerate(self.tab_buttons):
            btn.setProperty("active", i == index)
            btn.style().unpolish(btn)
            btn.style().polish(btn)
    
    def load_stylesheet(self):
        # Chargement de la feuille de style externe
        try:
            with open('style.qss', 'r', encoding='utf-8') as f:
                self.setStyleSheet(f.read())
        except FileNotFoundError:
            print("Fichier style.qss non trouvé")

def main():
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()