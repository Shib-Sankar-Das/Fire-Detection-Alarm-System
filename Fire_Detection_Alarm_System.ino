#include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>

// Firebase settings
#define FIREBASE_HOST "https://weather-app-9b23b-default-rtdb.firebaseio.com"  // Firebase host without "https://"
#define FIREBASE_AUTH "hu50iiS4OVTdG4OzBqoxf5ri2IQSTEtfRZS1e2Sa"       // Firebase authentication token

// WiFi credentials
#define WIFI_SSID "B_Cable_Network"          // Your WiFi SSID
#define WIFI_PASSWORD "9903585313"           // Your WiFi password

// Sensor and alarm pin definitions
#define DHTPIN 13            // DHT11 data pin connected to GPIO 13
#define DHTTYPE DHT11        // Define DHT type
#define MQ135_PIN 34         // MQ-135 analog pin connected to GPIO 34
#define ALARM_PIN 14         // Alarm module pin connected to GPIO 14

DHT dht(DHTPIN, DHTTYPE);
FirebaseData firebaseData;
FirebaseAuth firebaseAuth;
FirebaseConfig firebaseConfig;

// Fire detection threshold values
const int smokeThreshold = 1200;  // Adjust based on sensitivity of MQ-135
const int tempThreshold = 50;      // Temperature threshold for fire detection

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(MQ135_PIN, INPUT);
  pinMode(ALARM_PIN, OUTPUT);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize Firebase
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH; // Use `token.token` if you have a custom token
  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  // Check Firebase connection status
  if (!Firebase.ready()) {
    Serial.println("Firebase initialization failed. Check configuration and credentials.");
  } else {
    Serial.println("Connected to Firebase");
  }
  
  // Enable debug messages for Firebase
}

void loop() {
  // Reading temperature and smoke levels
  float temperature = dht.readTemperature();
  int smokeLevel = analogRead(MQ135_PIN);

  // Check if data is valid
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Determine if fire is detected
  bool fireDetected = (smokeLevel > smokeThreshold) || (temperature > tempThreshold);

  // Trigger alarm if fire is detected
  if (fireDetected) {
    digitalWrite(ALARM_PIN, LOW);  // Turn alarm ON
  } else {
    digitalWrite(ALARM_PIN, HIGH);   // Turn alarm OFF
  }

  // Update Firebase
  if (Firebase.ready()) {
    if (Firebase.setFloat(firebaseData, "/Temperature", temperature)) {
      Serial.println("Temperature data sent to Firebase");
    } else {
      Serial.print("Failed to send Temperature data: ");
      Serial.println(firebaseData.errorReason());
    }

    if (Firebase.setInt(firebaseData, "/SmokeLevel", smokeLevel)) {
      Serial.println("Smoke Level data sent to Firebase");
    } else {
      Serial.print("Failed to send Smoke Level data: ");
      Serial.println(firebaseData.errorReason());
    }

    if (Firebase.setBool(firebaseData, "/FireDetected", fireDetected)) {
      Serial.println("Fire Detection status sent to Firebase");
    } else {
      Serial.print("Failed to send Fire Detection status: ");
      Serial.println(firebaseData.errorReason());
    }
  } else {
    Serial.print("Firebase is not ready: ");
    Serial.println(firebaseData.errorReason());
  }

  // Debugging output
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Smoke Level: ");
  Serial.println(smokeLevel);
  Serial.print("Fire Detected: ");
  Serial.println(fireDetected);

  // Delay for stability
  delay(2000);
}
