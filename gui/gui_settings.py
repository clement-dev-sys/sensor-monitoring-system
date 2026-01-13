from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QPushButton, QLineEdit, QGroupBox, QDateTimeEdit, 
                             QSpinBox, QFormLayout)
from PyQt5.QtCore import Qt, QDateTime

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
        title.setObjectName("pageTitle")
        layout.addWidget(title)
        
        # Paramètres de période
        period_group = QGroupBox("Période de consultation (pour l'historique)")
        period_layout = QFormLayout()
        period_group.setLayout(period_layout)
        
        self.start_datetime = QDateTimeEdit()
        self.start_datetime.setDateTime(QDateTime.currentDateTime().addDays(-7))
        self.start_datetime.setCalendarPopup(True)
        period_layout.addRow("Date de début:", self.start_datetime)
        
        self.end_datetime = QDateTimeEdit()
        self.end_datetime.setDateTime(QDateTime.currentDateTime())
        self.end_datetime.setCalendarPopup(True)
        period_layout.addRow("Date de fin:", self.end_datetime)
        
        layout.addWidget(period_group)
        
        # Paramètres MQTT
        mqtt_group = QGroupBox("Configuration MQTT")
        mqtt_layout = QFormLayout()
        mqtt_group.setLayout(mqtt_layout)
        
        self.mqtt_host = QLineEdit()
        self.mqtt_host.setPlaceholderText("localhost")
        mqtt_layout.addRow("Serveur:", self.mqtt_host)
        
        self.mqtt_port = QSpinBox()
        self.mqtt_port.setRange(1, 65535)
        self.mqtt_port.setValue(1883)
        mqtt_layout.addRow("Port:", self.mqtt_port)
        
        self.mqtt_topic = QLineEdit()
        self.mqtt_topic.setPlaceholderText("sensors/data")
        mqtt_layout.addRow("Topic:", self.mqtt_topic)
        
        layout.addWidget(mqtt_group)
        
        # Paramètres SQLite
        sqlite_group = QGroupBox("Configuration SQLite")
        sqlite_layout = QFormLayout()
        sqlite_group.setLayout(sqlite_layout)
        
        self.db_path = QLineEdit()
        self.db_path.setPlaceholderText("/chemin/vers/database.db")
        sqlite_layout.addRow("Chemin BDD:", self.db_path)
        
        browse_btn = QPushButton("Parcourir...")
        browse_btn.setObjectName("secondaryButton")
        sqlite_layout.addRow("", browse_btn)
        
        layout.addWidget(sqlite_group)
        
        # Boutons de sauvegarde
        layout.addStretch()
        
        buttons_layout = QHBoxLayout()
        buttons_layout.addStretch()
        
        cancel_btn = QPushButton("Annuler")
        cancel_btn.setObjectName("secondaryButton")
        buttons_layout.addWidget(cancel_btn)
        
        save_btn = QPushButton("Enregistrer")
        save_btn.setObjectName("actionButton")
        buttons_layout.addWidget(save_btn)
        
        layout.addLayout(buttons_layout)