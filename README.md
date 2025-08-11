# 🌦️ Vejrstation – Gruppe 10

## Oversigt
Dette projekt er en prototype af en vejrstation, der indsamler målinger af temperatur, luftfugtighed og tryk, gemmer dem i en database og gør dem tilgængelige via API, webgrænseflade og dashboard.

Systemet er bygget som et distribueret setup med både hardware- og softwarekomponenter, der kommunikerer via MQTT.

---

## Arkitektur

**Komponenter:**
- **Arduino UNO R4 WiFi** med BME280-sensor til måling af vejrdata (I2C).
- **MQTT broker** (Mosquitto) på en Linux-server til at distribuere målinger.
- **Python MQTT-subscriber** til at modtage målinger, filtrere dem via deadband/heartbeat og indsætte dem i en MySQL-database.
- **MySQL** til lagring af historiske målinger.
- **.NET Web API** til at udstille data fra databasen.
- **React-frontend** til at præsentere data for brugeren.
- **Grafana** til visuel overvågning og analyse af vejrdata.

**Dataflow:**
1. Arduino måler temperatur, luftfugtighed og tryk via BME280-sensor.
2. Målinger sendes via MQTT til broker.
3. Python-script abonnerer på brokerens topic, filtrerer data og indsætter i MySQL.
4. Grafana henter data direkte fra databasen og viser dem på dashboards.
5. .NET API henter data fra databasen og eksponerer dem til frontend.
6. React-frontend viser API-data i browseren.

---

## Funktionalitet

### Arduino
- Måler temperatur, luftfugtighed og tryk.
- Sender målinger som JSON via MQTT.

### Python MQTT-subscriber
- Modtager målinger fra MQTT broker.
- Bruger deadband for at undgå unødvendige databaseinserts.
- Indsætter data i MySQL med timestamp.

### Database (MySQL)
- Tabel `readings` gemmer alle målinger.
- Indeholder felter for tidspunkt, temperatur, luftfugtighed og tryk.

### .NET Web API
- Endpoint for seneste måling: `/api/Målinger/seneste`
- Endpoint for målinger i interval: `/api/Målinger/interval?from=...&to=...`
- Returnerer data som JSON.

### React-frontend
- Henter data fra API’et og viser dem i et brugervenligt interface.
- Kan vise både seneste måling og historiske data.

### Grafana
- Dashboard med grafer for temperatur, luftfugtighed og tryk.
- Direkte forbindelse til MySQL for realtidsvisning.

---

## Tekniske detaljer
- **Hardware:** Arduino UNO R4 WiFi, BME280-sensor.
- **Protokoller:** I2C (sensor), MQTT (dataoverførsel).
- **Backend:** Python, .NET 8 Web API, MySQL.
- **Frontend:** React med Vite.
- **Visualisering:** Grafana.
- **Broker:** Mosquitto MQTT.

---
