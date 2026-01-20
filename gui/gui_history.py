from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QPushButton, QTableWidget, QGroupBox)
from PyQt5.QtCore import Qt

class HistoryTab(QWidget):
    """
    Onglet 2: Consultation de l'historique via SQLite
    - Connexion à la base SQLite
    - Récupération des données selon la période définie dans l'onglet Paramètres
    - Affichage sous forme de tableau
    """
    def __init__(self):
        super().__init__()
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        self.setLayout(layout)
        
        # Titre
        title = QLabel("Historique des Données")
        title.setObjectName("pageTitle")
        layout.addWidget(title)
        
#        # Zone d'information sur la période
#        period_group = QGroupBox("Période de consultation")
#        period_layout = QHBoxLayout()
#        period_group.setLayout(period_layout)
#        
#        self.period_label = QLabel("Période non définie")
#        self.period_label.setObjectName("infoLabel")
#        period_layout.addWidget(self.period_label)
#        
#        period_layout.addStretch()
#        
#        self.load_btn = QPushButton("Charger les données")
#        self.load_btn.setObjectName("actionButton")
#        period_layout.addWidget(self.load_btn)
#        
#        layout.addWidget(period_group)
#        
#        # Zone d'affichage des données
#        data_group = QGroupBox("Données historiques")
#        data_layout = QVBoxLayout()
#        data_group.setLayout(data_layout)
#        
#        self.data_table = QTableWidget()
#        self.data_table.setColumnCount(4)
#        self.data_table.setHorizontalHeaderLabels(["Date/Heure", "Donnée 1", "Donnée 2", "Donnée 3"])
#        data_layout.addWidget(self.data_table)
#        
#        layout.addWidget(data_group)
#        
#        # Boutons d'actions
#        buttons_layout = QHBoxLayout()
#        
#        export_btn = QPushButton("Exporter (CSV)")
#        export_btn.setObjectName("secondaryButton")
#        buttons_layout.addWidget(export_btn)
#        
#        buttons_layout.addStretch()
#        
#        refresh_btn = QPushButton("Actualiser")
#        refresh_btn.setObjectName("actionButton")
#        buttons_layout.addWidget(refresh_btn)
#        
#        layout.addLayout(buttons_layout)