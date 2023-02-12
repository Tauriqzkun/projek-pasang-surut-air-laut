byte ConnectAT(int wait) {
  byte indeks = 6;
  karakter = -2;;
  unsigned long mulai = millis();
  Serial.println("AT");
  SIM7000.println("AT");
  while (millis() - mulai <= wait)  {
    while (SIM7000.available() > 0)    {
      karakter = SIM7000.read();

      Serial.write(karakter);
      if (karakter == 'K') {
        indeks = 8;
        break;
      }
    }
  }
  return indeks;
}

void initGSM() {
  // Check pin and functionality
  Serial.println(F("\r\n- CEK FUNGSI GSM -"));
  gsmCheckPin(1000);

  // REGISTRASI KARTU KE NETWORK PROVIDER
  Serial.println(F("\r\n\r\n- REGISTRASI KE NETWORK -"));
  gsmRegister(500);

}

void readSerial(unsigned int wait) {
  char karakter = -2;
  unsigned long mulai = millis();
  while (millis() - mulai <= wait)  {
    while (SIM7000.available() > 0)    {
      karakter = SIM7000.read();
      Serial.write(karakter);
    }
  }
}

void gsmCheckPin(byte waktu) {
  SIM7000.println(F("AT+CPIN?;+CFUN=1"));
  readSerial(waktu);
}

//Registrasi Network
void gsmRegister(byte waktu) {
  SIM7000.println(F("AT+CREG=1"));
  readSerial(waktu);
}

void gsmOperator() {
  Serial.println("\n> Check GSM Operators");
  byte  indeks = 0;
  filename = "";
  while (indeks < 15) {
    char karakter = '0';
    Serial.print("\r\n");
    Serial.print(indeks + 1);
    Serial.print(F(". "));

    //Pengecekan AT+COPS?
    unsigned long mulai = millis();
    SIM7000.println(F("AT+COPS?"));
    while (millis() - mulai <= 500) {
      while (SIM7000.available() > 0) {
        karakter = SIM7000.read();
        filename += karakter;
        Serial.write(karakter);
      }
    }

    Serial.print("Hasil COPS:");
    Serial.println(filename);
    i = filename.indexOf('"');
    j = filename.indexOf('"', i + 1);
    operators = filename.substring(i + 1, j);
    Serial.println(operators);
    Serial.flush();

    if (operators.length() > 0) break;
    indeks = indeks + 1;
  }

  if ( operators == "") {
    Serial.println("No GSM Operator Found!!!");
  }
  
  delay(1000);
}

void gsmSinyal() {
  Serial.println("\n> Check GSM Signal");

  karakter = '\0';
  i = 99;
  Serial.println("\n> Go");
  filename = "";
  while (i == 99) {
    unsigned long mulai = millis();
    SIM7000.println(F("AT+CSQ"));
    delay(200);
    while (millis() - mulai <= 1000) {
      while (SIM7000.available() > 0) {
        karakter = SIM7000.read();
        filename += karakter;
        Serial.write(karakter);
      }
    }

    i = filename.indexOf(':');
    j = filename.indexOf(',', i + 1);
    //    operators = filename.substring(i + 1, j);

    i = filename.substring(i + 1, j).toInt();
    Serial.print(i);
    Serial.print(" ");

    if (i < 10) {
      network = "SIGNAL POOR";
    }
    if (i > 9 && i < 15) {
      network = "SIGNAL FAIR";
    }
    if (i > 14 && i < 20) {
      network = "SIGNAL GOOD";
    }
    if (i > 19 && i <= 31) {
      network = "SIGNAL EXCELLENT";
    }
    if (i == 99) {
      network = "UNKNOWN";
    }
  }

  Serial.println(network);
}
