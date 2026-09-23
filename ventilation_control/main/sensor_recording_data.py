import serial

# Replace 'COM3' with your ESP32 port (e.g., '/dev/ttyUSB0' on Linux/Mac)
ser = serial.Serial('COM3', 115200)

with open('sensor_log.csv', 'a') as file:
    print("Logging started. Press Ctrl+C to stop.")
    while True:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            # Filter out non-CSV system logs from ESP-IDF
            if line and not line.startswith("I (") and not line.startswith("E ("):
                file.write(line + '\n')
                file.flush()  # Ensures data is written immediately
                print(f"Logged: {line}")
        except KeyboardInterrupt:
            break