from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QPushButton, QTextEdit, QGroupBox)
from PyQt5.QtCore import Qt, QThread, pyqtSignal
import paho.mqtt.client as mqtt
import json
from datetime import datetime

class MQTTThread(QThread):
    """Thread pour gérer la connexion MQTT sans bloquer l'interface"""
    message_received = pyqtSignal(str)
    connection_status = pyqtSignal(bool, str)
    
    def __init__(self, host, port, topic):
        super().__init__()
        self.host = host
        self.port = port
        self.topic = topic
        self.client = None
        self.running = False
    
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            self.connection_status.emit(True, "Connecté")
            client.subscribe(self.topic)
        else:
            self.connection_status.emit(False, f"Erreur de connexion ({rc})")
    
    def on_message(self, client, userdata, msg):
        try:
            message = msg.payload.decode('utf-8')
            data = json.loads(message)
            self.message_received.emit(json.dumps(data))
        except json.JSONDecodeError as e:
            self.message_received.emit(f"Erreur JSON : {str(e)}")
        except Exception as e:
            self.message_received.emit(f"Erreur décodage : {str(e)}")
    
    def on_disconnect(self, client, userdata, rc):
        self.connection_status.emit(False, "Déconnecté")
    
    def run(self):
        try:
            self.client = mqtt.Client()
            self.client.on_connect = self.on_connect
            self.client.on_message = self.on_message
            self.client.on_disconnect = self.on_disconnect
            
            self.client.connect(self.host, self.port, 60)
            self.running = True
            self.client.loop_forever()
        except Exception as e:
            self.connection_status.emit(False, f"Erreur : {str(e)}")
    
    def stop(self):
        self.running = False
        if self.client:
            self.client.disconnect()
            self.client.loop_stop()

class RealtimeTab(QWidget):
    """
    Onglet 1: Affichage des données en temps réel via MQTT
    - Connexion au broker MQTT
    - Réception des données du serveur C
    - Affichage en temps réel
    """
    def __init__(self):
        super().__init__()
        self.mqtt_thread = None
        self.is_connected = False
        self.mqtt_config = {
            'host': '192.168.1.100',
            'port': 1883,
            'topic': 'server/data'
        }
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        self.setLayout(layout)
        
        # Titre
        title = QLabel("Données en Temps Réel (MQTT)")
        title.setObjectName("pageTitle")
        layout.addWidget(title)
        
        # Statut de connexion
        status_group = QGroupBox("Statut de connexion")
        status_layout = QHBoxLayout()
        status_group.setLayout(status_layout)
        
        self.status_label = QLabel("Déconnecté")
        self.status_label.setObjectName("statusLabel")
        status_layout.addWidget(self.status_label)
        
        self.connect_btn = QPushButton("Connecter")
        self.connect_btn.setObjectName("actionButton")
        self.connect_btn.clicked.connect(self.toggle_connection)
        status_layout.addWidget(self.connect_btn)
        
        status_layout.addStretch()
        layout.addWidget(status_group)
        
        # Affichage des données
        data_group = QGroupBox("Données reçues")
        data_layout = QVBoxLayout()
        data_group.setLayout(data_layout)
        
        timestamp_layout = QHBoxLayout()
        self.timestamp_label = []
        timestamp_label = QLabel("Heure UTC : ")
        timestamp_label.setMinimumWidth(150)
        timestamp_layout.addWidget(timestamp_label)
        self.value_timestamp = QLabel("-")
        self.value_timestamp.setMinimumWidth(550)
        timestamp_layout.addWidget(self.value_timestamp)
        data_layout.addLayout(timestamp_layout)
        
        self.data_labels = []
        for i in range(3):
            row_layout = QHBoxLayout()

            key_label = QLabel("")
            if i == 0: key_label.setText("Température : ")
            elif i == 1: key_label.setText("Pression : ")
            elif i == 2: key_label.setText("Humidité : ")
            key_label.setMinimumWidth(150)
            row_layout.addWidget(key_label)

            value_label = QLabel("-")
            value_label.setMinimumWidth(150)
            row_layout.addWidget(value_label)
            
            moy_label = QLabel("-")
            moy_label.setMinimumWidth(150)
            row_layout.addWidget(moy_label)
            
            max_label = QLabel("-")
            max_label.setMinimumWidth(150)
            row_layout.addWidget(max_label)
            
            min_label = QLabel("-")
            row_layout.addWidget(min_label)

            row_layout.addStretch()

            self.data_labels.append((key_label, value_label, moy_label, max_label, min_label))
            data_layout.addLayout(row_layout)
      
        layout.addWidget(data_group)
        
        # Effacer
        clear_btn = QPushButton("Effacer")
        clear_btn.setObjectName("secondaryButton")
        clear_btn.clicked.connect(self.clear_display)
        layout.addWidget(clear_btn, alignment=Qt.AlignRight)
       
    def toggle_connection(self):
        if self.is_connected:
            self.disconnect_mqtt()
        else:
            self.connect_mqtt()
    
    def connect_mqtt(self):
        try:
            self.mqtt_thread = MQTTThread(
                self.mqtt_config['host'],
                self.mqtt_config['port'],
                self.mqtt_config['topic']
            )
            self.mqtt_thread.message_received.connect(self.display_message)
            self.mqtt_thread.connection_status.connect(self.update_connection_status)
            self.mqtt_thread.start()
        except Exception as e:
            self.status_label.setText(f"Erreur : {str(e)}")
    
    def disconnect_mqtt(self):
        if self.mqtt_thread:
            self.mqtt_thread.stop()
            self.mqtt_thread.wait()
            self.mqtt_thread = None
    
    def update_connection_status(self, connected, message):
        self.is_connected = connected
        self.status_label.setText(message)
        
        if connected:
            self.connect_btn.setText("Déconnecter")
        else:
            self.connect_btn.setText("Connecter")
    
    def display_message(self, message):
        try:
            data = json.loads(message)
            
            for i, (key, value) in enumerate(data.items()):
                if i == 0 :
                    self.value_timestamp.setText(str(value))
                elif i < len(self.data_labels):
                    self.data_labels[i-1][1].setText(str(value))
        except json.JSONDecodeError as e:
            self.data_labels[0][1].setText(f"Erreur JSON : {str(e)}")
    
    def clear_display(self):
        self.value_timestamp.setText("-")
        for i in range(3):
            self.data_labels[i][1].setText("-")
    
    def closeEvent(self, event):
        if self.is_connected:
            self.disconnect_mqtt()
        event.accept()