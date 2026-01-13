from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QPushButton, QTextEdit, QGroupBox)
from PyQt5.QtCore import Qt

class RealtimeTab(QWidget):
    """
    Onglet 1: Affichage des données en temps réel via MQTT
    - Connexion au broker MQTT
    - Réception des données du serveur C
    - Affichage en temps réel
    """
    def __init__(self):
        super().__init__()
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        self.setLayout(layout)
        
        # Titre
        title = QLabel("Données en Temps Réel (MQTT)")
        title.setObjectName("pageTitle")
        layout.addWidget(title)
        
        # Zone de statut de connexion
        status_group = QGroupBox("Statut de connexion")
        status_layout = QHBoxLayout()
        status_group.setLayout(status_layout)
        
        self.status_label = QLabel("Déconnecté")
        self.status_label.setObjectName("statusLabel")
        status_layout.addWidget(self.status_label)
        
        self.connect_btn = QPushButton("Connecter")
        self.connect_btn.setObjectName("actionButton")
        status_layout.addWidget(self.connect_btn)
        
        status_layout.addStretch()
        layout.addWidget(status_group)
        
        # Zone d'affichage des données
        data_group = QGroupBox("Données reçues")
        data_layout = QVBoxLayout()
        data_group.setLayout(data_layout)
        
        self.data_display = QTextEdit()
        self.data_display.setReadOnly(True)
        self.data_display.setPlaceholderText("En attente de données MQTT...")
        data_layout.addWidget(self.data_display)
        
        layout.addWidget(data_group)
        
        # Bouton pour effacer l'affichage
        clear_btn = QPushButton("Effacer")
        clear_btn.setObjectName("secondaryButton")
        layout.addWidget(clear_btn, alignment=Qt.AlignRight)