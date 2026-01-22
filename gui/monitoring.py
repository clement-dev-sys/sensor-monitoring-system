import sys
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QStackedWidget)
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
        self.setWindowTitle("Monitoring")
        
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)
        
        tabs_layout = QHBoxLayout()
        tabs_layout.setContentsMargins(0, 0, 0, 0)
        tabs_layout.setSpacing(0)
        
        self.tab_buttons = []
        tab_names = ["Temps Réel", "Historique", "Paramètres", "À propos"]
        
        for i, name in enumerate(tab_names):
            btn = QPushButton(name)
            btn.setProperty("active", i == 0)
            btn.clicked.connect(lambda checked, index=i: self.switch_tab(index))
            self.tab_buttons.append(btn)
            tabs_layout.addWidget(btn)
        
        tabs_layout.addStretch()
        
        tabs_widget = QWidget()
        tabs_widget.setLayout(tabs_layout)
        main_layout.addWidget(tabs_widget)
        
        self.stacked_widget = QStackedWidget()
        main_layout.addWidget(self.stacked_widget)
        
        self.create_tabs()
    
    def create_tabs(self):
        self.realtime_tab = RealtimeTab()
        self.stacked_widget.addWidget(self.realtime_tab)
        
        self.history_tab = HistoryTab()
        self.stacked_widget.addWidget(self.history_tab)
        
        self.settings_tab = SettingsTab()
        self.stacked_widget.addWidget(self.settings_tab)
        
        self.about_tab = AboutTab()
        self.stacked_widget.addWidget(self.about_tab)
        
        self.settings_tab.connect_btn.clicked.connect(self.handle_connection_toggle)
        self.realtime_tab.status_changed.connect(self.update_settings_connection_status)
    
    def switch_tab(self, index):
        self.stacked_widget.setCurrentIndex(index)
        
        for i, btn in enumerate(self.tab_buttons):
            btn.setProperty("active", i == index)
            btn.style().unpolish(btn)
            btn.style().polish(btn)
            
    def handle_connection_toggle(self):
        host = self.settings_tab.mqtt_host.text().strip()
        port = int(self.settings_tab.mqtt_port.text().strip())
        topic = self.settings_tab.mqtt_topic.text().strip()

        if not host:
            self.settings_tab.status_label.setText("Erreur : Hôte vide")
            return
        if not topic:
            self.settings_tab.status_label.setText("Erreur : Topic vide")
            return

        self.realtime_tab.update_mqtt_config(host, port, topic)
        self.realtime_tab.toggle_connection()
        
    def update_settings_connection_status(self, connected, message):
        self.settings_tab.status_label.setText(message)
        self.realtime_tab.status_label.setText("Status : " + message)

        if connected:
            self.settings_tab.connect_btn.setText("Déconnecter")
        else:
            self.settings_tab.connect_btn.setText("Connecter")


    def load_stylesheet(self):
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