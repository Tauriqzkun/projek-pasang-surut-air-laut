void ambilWaktu() {
  //AMBIL WAKTU
  waktu = rtc.now();
}

void ambilSuhu() {
  sensorSuhu.requestTemperatures();
  temperatureC = sensorSuhu.getTempCByIndex(0);
}

void ambilJarak() {
  //pakai while agar nol nya hilang
  jarak = 0; //Reset nilai jarak
  while (jarak == 0 || jarak >= 800) {
    //AMBIL JARAK
    digitalWrite(trig, LOW); delayMicroseconds(2);
    digitalWrite(trig, HIGH); delayMicroseconds(10);
    digitalWrite(trig, LOW);
    durasi = pulseIn(echo, HIGH);
    jarak = durasi / 58.2; //mengubah waktu menjadi jarak
  }
}

void simpanData() {
  //MENYIMPAN FILE DATA TAHUN, BULAN, HARI, JAM, MENIT, DETIK, JARAK, DAN SUHU
  filename = String(namaFile);
  //  Serial.println(namaFile);
  //  Serial.println(filename);
  File dataFile = SD.open(filename, FILE_APPEND);
  if (dataFile) {
    dataFile.print(waktu.timestamp(DateTime::TIMESTAMP_DATE));//TAHUN, BULAN, HARI
    dataFile.print(" ");
    dataFile.print(waktu.timestamp(DateTime::TIMESTAMP_TIME));//JAM, MENIT, DETIK
    dataFile.print(","); dataFile.print(jarak); //JARAK
    dataFile.print(","); dataFile.print(temperatureC);
    dataFile.close();
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
  }
  else {
    Serial.println("error opening datalog.txt");
    digitalWrite(LED, HIGH); // LED SDCARD ERROR
  }
}


void dataJSON() {
  json = "";
  json = "{\"suhu\":" + String(temperatureC, 2);
  json += ",\"jarak\":" + String(jarak, 0);
  json += "}";
  Serial.println(json);
}

void kirimData() {
  boolean nilai = TCPstart(5000, 1);
  dataJSON();
  //SET HTTP PARAMETERS VALUE
  Serial.flush();
  Serial1.flush();
  Serial.println(F("\r\n - KIRIM DATA - "));
  nilai = TCPsend();
  Serial.println(F("\r\n"));
  TCPclose(500);
  readSerial(500);
  Serial.flush();
  Serial1.flush();
}

void swap(float* xp, float* yp) {
  float temp = *xp;
  *xp = *yp;
  *yp = temp;
}

void bubbleSort(float arr[], int n) {
  int i, j;
  for (i = 0; i < n - 1; i++) {
    // Last i elements are already in place
    for (j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1])    swap(&arr[j], &arr[j + 1]);
    }
  }
}

void printArray(float arr[], int sizeN) {
  int i;
  for (i = 0; i < sizeN; i++)  {
    Serial.print(arr[i]);
    Serial.print(",");
  }
  Serial.println("\n");
}
