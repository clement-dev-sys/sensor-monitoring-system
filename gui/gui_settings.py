from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QPushButton, QLineEdit, QGroupBox, QFormLayout)

class SettingsTab(QWidget):
    """
    Onglet 3: Configuration des paramètres
    - Définition de la période pour la consultation historique
    - Paramètres de connexion MQTT
    - Paramètres de connexion SQLite
    """
    def __init__(self):
        super().__init__()
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        self.setLayout(layout)
        
        # Titre
        title = QLabel("Paramètres de l'Application")
        layout.addWidget(title)
                
        # Paramètres MQTT
        mqtt_group = QGroupBox("Configuration Temps réel")
        mqtt_layout = QFormLayout()
        mqtt_group.setLayout(mqtt_layout)
        
        self.mqtt_host = QLineEdit()
        self.mqtt_host.setPlaceholderText("192.168.1.100")
        self.mqtt_host.setText("192.168.1.100")
        mqtt_layout.addRow("Serveur :", self.mqtt_host)
        
        self.mqtt_port = QLineEdit()
        self.mqtt_port.setPlaceholderText("1883")
        self.mqtt_port.setText("1883")
        mqtt_layout.addRow("Port :", self.mqtt_port)
        
        self.mqtt_topic = QLineEdit()
        self.mqtt_topic.setPlaceholderText("server/data")
        self.mqtt_topic.setText("server/data")
        mqtt_layout.addRow("Topic :", self.mqtt_topic)
        
        # Bouton de connection
        status_container = QWidget()
        status_layout = QHBoxLayout()
        status_layout.setContentsMargins(0, 0, 0, 0)
        status_container.setLayout(status_layout)
        self.status_label = QLabel("Déconnecté")
        status_layout.addWidget(self.status_label)
        status_layout.addStretch()
        self.connect_btn = QPushButton("Connecter")
        status_layout.addWidget(self.connect_btn)
        mqtt_layout.addRow("Status :", status_container)
        
        layout.addWidget(mqtt_group)
        layout.addStretch()