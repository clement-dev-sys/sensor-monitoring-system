from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QLabel, QGroupBox, 
                             QHBoxLayout)
from PyQt5.QtCore import Qt

class AboutTab(QWidget):
    """
    Onglet 4: Informations et statuts
    - Informations sur l'application
    - Statut des connexions (MQTT et SQLite)
    - Version, etc.
    """
    def __init__(self):
        super().__init__()
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        self.setLayout(layout)
        
        # Titre
        title = QLabel("À propos")
        title.setObjectName("pageTitle")
        layout.addWidget(title)
        
#        # Informations sur l'application
#        info_group = QGroupBox("Application")
#        info_layout = QVBoxLayout()
#        info_group.setLayout(info_layout)
#        
#        app_name = QLabel("Monitoring MQTT & SQLite")
#        app_name.setObjectName("appTitle")
#        info_layout.addWidget(app_name)
#        
#        version = QLabel("Version 1.0.0")
#        version.setObjectName("infoLabel")
#        info_layout.addWidget(version)
#        
#        description = QLabel(
#            "Application de monitoring temps réel via MQTT et "
#            "consultation d'historique via SQLite."
#        )
#        description.setWordWrap(True)
#        description.setObjectName("descriptionLabel")
#        info_layout.addWidget(description)
#        
#        layout.addWidget(info_group)
#        
#        # Statut des connexions
#        status_group = QGroupBox("État des connexions")
#        status_layout = QVBoxLayout()
#        status_group.setLayout(status_layout)
#        
#        # Statut MQTT
#        mqtt_status_layout = QHBoxLayout()
#        mqtt_label = QLabel("MQTT:")
#        mqtt_label.setObjectName("statusTitle")
#        mqtt_status_layout.addWidget(mqtt_label)
#        
#        self.mqtt_status = QLabel("Déconnecté")
#        self.mqtt_status.setObjectName("statusValue")
#        mqtt_status_layout.addWidget(self.mqtt_status)
#        mqtt_status_layout.addStretch()
#        
#        status_layout.addLayout(mqtt_status_layout)
#        
#        # Statut SQLite
#        db_status_layout = QHBoxLayout()
#        db_label = QLabel("Base de données:")
#        db_label.setObjectName("statusTitle")
#        db_status_layout.addWidget(db_label)
#        
#        self.db_status = QLabel("Non connecté")
#        self.db_status.setObjectName("statusValue")
#        db_status_layout.addWidget(self.db_status)
#        db_status_layout.addStretch()
#        
#        status_layout.addLayout(db_status_layout)
#        
#        layout.addWidget(status_group)
#        
#        layout.addStretch()
#        
#        # Footer
#        footer = QLabel("© 2026 - Développé avec PyQt5")
#        footer.setAlignment(Qt.AlignCenter)
#        footer.setObjectName("footerLabel")
#        layout.addWidget(footer)