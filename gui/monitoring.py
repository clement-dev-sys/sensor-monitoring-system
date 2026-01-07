#!/usr/bin/env python3
"""
GUI PyQt5 pour monitoring temps réel des données MQTT
Sensor Monitoring System - Client GUI
"""

import sys
import json
from datetime import datetime
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QLineEdit, QPushButton, QGroupBox, QGridLayout, QStatusBar
)
from PyQt5.QtCore import QThread, pyqtSignal, Qt
from PyQt5.QtGui import QFont, QPalette, QColor
import paho.mqtt.client as mqtt


class MQTTWorker(QThread):
    """Thread pour gérer la connexion MQTT sans bloquer l'interface"""
    
    # Signaux pour communiquer avec l'interface principale
    data_received = pyqtSignal(str, float, float, int)  # timestamp, temp, press, hum
    connection_status = pyqtSignal(bool, str)  # connected, message
    
    def __init__(self, broker_address, broker_port, topic):
        super().__init__()
        self.broker_address = broker_address
        self.broker_port = broker_port
        self.topic = topic
        self.client = None
        self.running = True
        
    def on_connect(self, client, userdata, flags, rc):
        """Callback lors de la connexion au broker"""
        if rc == 0:
            self.connection_status.emit(True, f"Connecté au broker {self.broker_address}")
            client.subscribe(self.topic)
        else:
            error_msg = f"Échec connexion (code {rc})"
            self.connection_status.emit(False, error_msg)
    
    def on_disconnect(self, client, userdata, rc):
        """Callback lors de la déconnexion"""
        self.connection_status.emit(False, "Déconnecté du broker")
    
    def on_message(self, client, userdata, msg):
        """Callback lors de la réception d'un message"""
        try:
            # Parser le JSON
            payload = json.loads(msg.payload.decode())
            
            # Extraire les données
            timestamp = payload.get('timestamp', datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
            temperature = float(payload.get('temperature', 0))
            pression = float(payload.get('pression', 0))
            humidite = int(payload.get('humidite', 0))
                        
            # Émettre le signal avec les données
            self.data_received.emit(timestamp, temperature, pression, humidite)
            
        except (json.JSONDecodeError, ValueError, KeyError) as e:
            print(f"Erreur parsing message: {e}")
    
    def run(self):
        """Boucle principale du thread MQTT"""
        try:
            # Créer le client MQTT
            self.client = mqtt.Client(client_id="GUI_Monitor")
            self.client.on_connect = self.on_connect
            self.client.on_disconnect = self.on_disconnect
            self.client.on_message = self.on_message
            
            # Connexion au broker
            self.client.connect(self.broker_address, self.broker_port, 60)
            
            # Boucle MQTT
            while self.running:
                self.client.loop(timeout=1.0)
                
        except Exception as e:
            self.connection_status.emit(False, f"Erreur: {str(e)}")
    
    def stop(self):
        """Arrêter proprement le thread"""
        self.running = False
        if self.client:
            self.client.disconnect()
        self.wait()


class SensorMonitorGUI(QMainWindow):
    """Interface principale de monitoring"""
    
    def __init__(self):
        super().__init__()
        
        # Variables
        self.mqtt_worker = None
        self.is_connected = False
        
        # Configuration par défaut
        # IMPORTANT: Utiliser l'IP WiFi du serveur (192.168.1.X), pas 192.168.69.1
        # 192.168.69.1 est l'IP du réseau privé ESP32↔Serveur
        self.default_broker = "192.168.1.93"  # À ADAPTER selon votre serveur
        self.default_port = "1883"
        self.default_topic = "esp32/env"
        
        # Initialiser l'interface
        self.init_ui()
        
    def init_ui(self):
        """Créer l'interface utilisateur"""
        self.setWindowTitle("Sensor Monitor - GUI")
        self.setGeometry(100, 100, 600, 500)
        
        # Widget central
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Layout principal
        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)
        
        # === SECTION CONNEXION ===
        connection_group = QGroupBox("Configuration MQTT")
        connection_layout = QGridLayout()
        
        # Broker address
        connection_layout.addWidget(QLabel("Broker:"), 0, 0)
        self.broker_input = QLineEdit(self.default_broker)
        connection_layout.addWidget(self.broker_input, 0, 1)
        
        # Port
        connection_layout.addWidget(QLabel("Port:"), 0, 2)
        self.port_input = QLineEdit(self.default_port)
        self.port_input.setMaximumWidth(80)
        connection_layout.addWidget(self.port_input, 0, 3)
        
        # Topic
        connection_layout.addWidget(QLabel("Topic:"), 1, 0)
        self.topic_input = QLineEdit(self.default_topic)
        connection_layout.addWidget(self.topic_input, 1, 1, 1, 3)
        
        # Bouton connexion
        self.connect_btn = QPushButton("Connecter")
        self.connect_btn.clicked.connect(self.toggle_connection)
        connection_layout.addWidget(self.connect_btn, 2, 0, 1, 4)
        
        connection_group.setLayout(connection_layout)
        main_layout.addWidget(connection_group)
        
        # === SECTION DONNÉES ===
        data_group = QGroupBox("Données en temps réel")
        data_layout = QGridLayout()
        
        # Style pour les labels
        label_font = QFont("Monospace", 12, QFont.Bold)
        value_font = QFont("Monospace", 16, QFont.Bold)
        
        # Timestamp
        data_layout.addWidget(QLabel("Timestamp:"), 0, 0)
        self.timestamp_label = QLabel("---")
        self.timestamp_label.setFont(label_font)
        data_layout.addWidget(self.timestamp_label, 0, 1)
        
        # Température
        temp_label = QLabel("Température:")
        temp_label.setFont(label_font)
        data_layout.addWidget(temp_label, 1, 0)
        
        self.temp_value = QLabel("-- °C")
        self.temp_value.setFont(value_font)
        self.temp_value.setStyleSheet("color: #FF6B6B; background-color: #2C2C2C; padding: 10px; border-radius: 5px;")
        self.temp_value.setAlignment(Qt.AlignCenter)
        data_layout.addWidget(self.temp_value, 1, 1)
        
        # Pression
        press_label = QLabel("Pression:")
        press_label.setFont(label_font)
        data_layout.addWidget(press_label, 2, 0)
        
        self.press_value = QLabel("-- hPa")
        self.press_value.setFont(value_font)
        self.press_value.setStyleSheet("color: #4ECDC4; background-color: #2C2C2C; padding: 10px; border-radius: 5px;")
        self.press_value.setAlignment(Qt.AlignCenter)
        data_layout.addWidget(self.press_value, 2, 1)
        
        # Humidité
        hum_label = QLabel("Humidité:")
        hum_label.setFont(label_font)
        data_layout.addWidget(hum_label, 3, 0)
        
        self.hum_value = QLabel("-- %")
        self.hum_value.setFont(value_font)
        self.hum_value.setStyleSheet("color: #95E1D3; background-color: #2C2C2C; padding: 10px; border-radius: 5px;")
        self.hum_value.setAlignment(Qt.AlignCenter)
        data_layout.addWidget(self.hum_value, 3, 1)
        
        data_group.setLayout(data_layout)
        main_layout.addWidget(data_group)
        
        # Espaceur
        main_layout.addStretch()
        
        # === BARRE DE STATUS ===
        self.status_bar = QStatusBar()
        self.setStatusBar(self.status_bar)
        self.status_bar.showMessage("Déconnecté")
        
    def toggle_connection(self):
        """Connecter ou déconnecter du broker MQTT"""
        if not self.is_connected:
            self.connect_to_broker()
        else:
            self.disconnect_from_broker()
    
    def connect_to_broker(self):
        """Établir la connexion au broker MQTT"""
        broker = self.broker_input.text().strip()
        port = int(self.port_input.text().strip())
        topic = self.topic_input.text().strip()
        
        if not broker or not topic:
            self.status_bar.showMessage("Erreur: Broker et Topic requis")
            return
        
        # Désactiver les inputs pendant la connexion
        self.broker_input.setEnabled(False)
        self.port_input.setEnabled(False)
        self.topic_input.setEnabled(False)
        self.connect_btn.setText("Connexion...")
        self.connect_btn.setEnabled(False)
        
        # Créer et démarrer le worker MQTT
        self.mqtt_worker = MQTTWorker(broker, port, topic)
        self.mqtt_worker.data_received.connect(self.update_data)
        self.mqtt_worker.connection_status.connect(self.handle_connection_status)
        self.mqtt_worker.start()
    
    def disconnect_from_broker(self):
        """Se déconnecter du broker MQTT"""
        if self.mqtt_worker:
            self.mqtt_worker.stop()
            self.mqtt_worker = None
        
        self.is_connected = False
        self.connect_btn.setText("Connecter")
        self.broker_input.setEnabled(True)
        self.port_input.setEnabled(True)
        self.topic_input.setEnabled(True)
        self.status_bar.showMessage("Déconnecté")
        
        # Réinitialiser l'affichage
        self.timestamp_label.setText("---")
        self.temp_value.setText("-- °C")
        self.press_value.setText("-- hPa")
        self.hum_value.setText("-- %")
    
    def handle_connection_status(self, connected, message):
        """Gérer les changements de statut de connexion"""
        self.status_bar.showMessage(message)
        
        if connected:
            self.is_connected = True
            self.connect_btn.setText("Déconnecter")
            self.connect_btn.setEnabled(True)
            self.connect_btn.setStyleSheet("background-color: #FF6B6B;")
        else:
            if self.mqtt_worker:
                self.disconnect_from_broker()
    
    def update_data(self, timestamp, temperature, pression, humidite):
        """Mettre à jour l'affichage avec les nouvelles données"""
        self.timestamp_label.setText(timestamp)
        self.temp_value.setText(f"{temperature:.1f} °C")
        self.press_value.setText(f"{pression:.1f} hPa")
        self.hum_value.setText(f"{humidite} %")
    
    def closeEvent(self, event):
        """Gérer la fermeture de l'application"""
        if self.mqtt_worker:
            self.mqtt_worker.stop()
        event.accept()


def main():
    """Point d'entrée de l'application"""
    app = QApplication(sys.argv)
    
    # Style sombre (optionnel)
    app.setStyle("Fusion")
    palette = QPalette()
    palette.setColor(QPalette.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.WindowText, Qt.white)
    palette.setColor(QPalette.Base, QColor(35, 35, 35))
    palette.setColor(QPalette.AlternateBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ToolTipBase, Qt.white)
    palette.setColor(QPalette.ToolTipText, Qt.white)
    palette.setColor(QPalette.Text, Qt.white)
    palette.setColor(QPalette.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ButtonText, Qt.white)
    palette.setColor(QPalette.BrightText, Qt.red)
    palette.setColor(QPalette.Link, QColor(42, 130, 218))
    palette.setColor(QPalette.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.HighlightedText, Qt.black)
    app.setPalette(palette)
    
    # Créer et afficher la fenêtre
    window = SensorMonitorGUI()
    window.show()
    
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()