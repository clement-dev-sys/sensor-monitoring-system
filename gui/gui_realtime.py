from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLabel, 
                             QPushButton, QGroupBox)
from PyQt5.QtCore import Qt, QThread, pyqtSignal
import paho.mqtt.client as mqtt
import json

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
    status_changed = pyqtSignal(bool, str)
    
    def __init__(self):
        super().__init__()
        self.mqtt_thread = None
        self.is_connected = False
        self.mqtt_config = {
            'host': None,
            'port': None,
            'topic': None
        }
        self.data_history = {
            'temperature': [],
            'pression': [],
            'humidite': []
        }
        self.max_history = 2160
        self.init_ui()
    
    def init_ui(self):
        layout = QVBoxLayout()
        self.setLayout(layout)
        
        # Titre
        title = QLabel("Données en Temps Réel")
        layout.addWidget(title)
        
        # Affichage des données
        data_group = QGroupBox("Données reçues")
        data_layout = QVBoxLayout()
        data_group.setLayout(data_layout)
               
        self.data_labels = []
        for i in range(4):
            row_layout = QHBoxLayout()

            key_label = QLabel("")
            if i == 0: key_label.setText("Heure UTC : ")
            elif i == 1: key_label.setText("Température : ")
            elif i == 2: key_label.setText("Pression : ")
            elif i == 3: key_label.setText("Humidité : ")
            key_label.setMinimumWidth(150)
            row_layout.addWidget(key_label)

            value_label = QLabel("-")
            row_layout.addWidget(value_label)
            
            row_layout.addStretch()

            self.data_labels.append((key_label, value_label))
            data_layout.addLayout(row_layout)
      
        layout.addWidget(data_group)
        
        # Affichage des statistiques
        stats_group = QGroupBox("Statistiques")
        stats_layout = QVBoxLayout()
        stats_group.setLayout(stats_layout)
               
        self.stats_labels = []
        for i in range(4):
            row_layout = QHBoxLayout()

            key_label = QLabel("")
            if i == 1: key_label.setText("Température : ")
            elif i == 2: key_label.setText("Pression : ")
            elif i == 3: key_label.setText("Humidité : ")
            key_label.setMinimumWidth(150)
            row_layout.addWidget(key_label)
            
            moy_label = QLabel("-")
            if i == 0: moy_label.setText("Moyenne")
            moy_label.setMinimumWidth(150)
            row_layout.addWidget(moy_label)
            
            max_label = QLabel("-")
            if i == 0: max_label.setText("Max")
            max_label.setMinimumWidth(150)
            row_layout.addWidget(max_label)
            
            min_label = QLabel("-")
            if i == 0: min_label.setText("Min")
            row_layout.addWidget(min_label)

            row_layout.addStretch()

            self.stats_labels.append((key_label, moy_label, max_label, min_label))
            stats_layout.addLayout(row_layout)
      
        layout.addWidget(stats_group)
        
        # Effacer
        clear_btn = QPushButton("Effacer")
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
            self.status_changed.emit(False, f"Erreur : {str(e)}")
    
    def disconnect_mqtt(self):
        if self.mqtt_thread:
            self.mqtt_thread.stop()
            self.mqtt_thread.wait()
            self.mqtt_thread = None
            
    def update_mqtt_config(self, host, port, topic):
        self.mqtt_config['host'] = host
        self.mqtt_config['port'] = port
        self.mqtt_config['topic'] = topic
    
    def update_connection_status(self, connected, message):
        self.is_connected = connected
        self.status_changed.emit(connected, message)
    
    def display_message(self, message):
        try:
            data = json.loads(message)

            key_mapping = {
                0: None,
                1: 'temperature',
                2: 'pression',
                3: 'humidite'
            }

            for i, (key, value) in enumerate(data.items()):
                if i < len(self.data_labels):
                    self.data_labels[i][1].setText(str(value))

                    if i != 0 and key_mapping[i]:
                        data_key = key_mapping[i]
                        try:
                            num_value = float(value)

                            self.data_history[data_key].append(num_value)
                            if len(self.data_history[data_key]) > self.max_history:
                                self.data_history[data_key].pop(0)

                            values = self.data_history[data_key]
                            if len(values) > 0:
                                moy = sum(values) / len(values)
                                max_val = max(values)
                                min_val = min(values)

                                self.stats_labels[i][1].setText(f"{moy:.2f}")
                                self.stats_labels[i][2].setText(f"{max_val:.2f}")
                                self.stats_labels[i][3].setText(f"{min_val:.2f}")
                        except ValueError:
                            self.stats_labels[i][2].setText("-")
                            self.stats_labels[i][3].setText("-")
                            self.stats_labels[i][4].setText("-")
                        
        except json.JSONDecodeError as e:
            self.data_labels[0][1].setText(f"Erreur JSON : {str(e)}")
    
    def clear_display(self):
        for i in range(4):
            self.data_labels[i][1].setText("-")
        
        self.data_history = {
            'temperature': [],
            'pression': [],
            'humidite': []
        }
        for i in range(1, 4):
            for j in range(1, 4):
                self.stats_labels[i][j].setText("-")
   
    def closeEvent(self, event):
        if self.is_connected:
            self.disconnect_mqtt()
        event.accept()