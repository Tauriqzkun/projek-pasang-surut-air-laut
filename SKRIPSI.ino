#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>


#include "variabel.h"

byte interval = 10; // Menit

#define trig 26
#define echo 27
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP (interval * 60)    /* Time ESP32 will go to sleep (in seconds) */
#define CLOCK_INTERRUPT_PIN 4 //PIN SQW
#define LED 12
#define ONE_WIRE_BUS 15

HardwareSerial & SIM7000 = Serial2;

////SD CARD
File file;
String filename;

//variabel
unsigned int kode;

boolean koneksi = 0;
boolean awal = 1;
boolean kirim = 0;
String operators, network;
char karakter;
byte a, b;
unsigned int nilai, i, j;

RTC_DS3231 rtc;
DateTime waktu, waktunanti;
byte detik;


const int chipSelect = 5; //sesuai dengan yang di ESP32
unsigned int indeks = 1;

float durasi, jarak;
float distance[5]={0,0,0,0,0};

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorSuhu(&oneWire);
DeviceAddress insideThermometer;// TAMBAHAN DARI GITHUB
float temperatureC;

//EEPROM
RTC_DATA_ATTR unsigned int nomor = 0;
RTC_DATA_ATTR char namaFile[14] = "/LOG0001.txt";


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  SIM7000.begin(9600);// TAMBAHAN DARI GITHUB

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(chipSelect, OUTPUT);
  Serial.println ("Alat Pasang Surut");

  //INIT SENSOR SUHU DS18B20
  Serial.print("Initializing DS18B20...");
  sensorSuhu.begin();
  Serial.println(" ");

  //INIT RTC
  Serial.print("Initializing RTC...");

  if (!rtc.begin()) {
    Serial.println("RTC Tidak Terbaca");
    digitalWrite(LED, HIGH); // LED RTC ERROR
    Serial.flush();
    while (1);
  }
  Serial.println("RTC Terbaca");

  //INIT MICRO SD
  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("Micro SD Tidak Terbaca");
    digitalWrite(LED, HIGH); // LED SDCARD ERROR
    while (1);
  }
  Serial.println("Micro SD Terbaca");

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == 2) {
    loop();
  }

  digitalWrite(LED, LOW);

  //CEK CONFIG.TXT
  Serial.println ("Cek Config.txt");
  Serial.print(" cek file = ");
  boolean ada = 1;
  while (ada == 1) {
    sprintf (namaFile, "/LOG%04d.txt", nomor);
    ada = SD.exists (namaFile);
    if (ada == 1) nomor ++;
    if (ada == 0) {
      //Membuat File
      Serial.println(namaFile);
      File dataFile = SD.open(namaFile, FILE_WRITE);
      dataFile.println ("Tanggal (yyyy/mm/dd hh:mm:ss), Jarak (cm), Suhu (ºC), Dates (yyyy/mm/dd hh:mm:ss)");
      dataFile.close();
    }
  }


  for (byte i = 0; i < 7; i++) {
    Serial.println(6 - i);
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
  }


  //INIT GSM
  Serial.println("Initialization GSM");
  delay(2000);

  Serial.println("\n> Check GSM");
  for (i = 1; i < 5; i++) {
    Serial.print(i);
    Serial.print(" ");
    j = ConnectAT(200);
    if (j == 8) break;
  }
  if ( j == 6) {
    Serial.println("GSM Error!!!");
    while (1);
  }

  Serial.println("GSM OK!");
  delay(1000);

  Serial.println("GSM Ready to Use!");
  delay(1000);
  initGSM();

  //operator
  Serial.println("Check Operator");
  gsmOperator();
  delay(1000);


  //kualitas sinyal
  Serial.println("Check Signal Quality");
  gsmSinyal();
  delay(1000);

  Serial.println("Check Internet Connection");
  //Cek GPRS
  cekGPRS();
  Serial.println("done");
  delay(1000);

  //HTTP TERMINATE
  Serial.flush();
  Serial1.flush();
  Serial.println(F("\r\n - TUTUP TCP IP - "));
  TCPclose(500);
  koneksi = 1;
  Serial.flush();
  Serial1.flush();

  //tunggu hingga detiknya nol
  detik = 50;
  while (detik != 0) {
    delay(200);
    ambilWaktu();
    detik = waktu.second();
    Serial.println(detik);
  }
}


void loop() {
  Serial.println(F("\r\n - ambil data - "));
  ambilWaktu();
  Serial.print(waktu.timestamp(DateTime::TIMESTAMP_DATE));//TAHUN, BULAN, HARI
  Serial.print(" ");
  Serial.println(waktu.timestamp(DateTime::TIMESTAMP_TIME));//JAM, MENIT, DETIK


  ambilSuhu();
  for (i = 0; i <= 4; i++) {
    ambilJarak();
    distance[i]=jarak;
  }

  bubbleSort(distance,5);
  printArray(distance,5);
  jarak=distance[2];
  simpanData();
  Serial.flush();
  Serial1.flush();

  gprsComm();
  kirimData();

  //simpan waktu setelah data berhasil diterima oleh server
  filename = String(namaFile);
  Serial.println(filename);
  DateTime dates = rtc.now();
  File dataFile = SD.open(filename, FILE_APPEND);
  dataFile.print(",");
  dataFile.print(dates.timestamp(DateTime::TIMESTAMP_DATE));//TAHUN, BULAN, HARI
  dataFile.print(" ");
  dataFile.println(dates.timestamp(DateTime::TIMESTAMP_TIME));//JAM, MENIT, DETIK
  dataFile.close();

  //MENCETAK KE SERIAL MONITOR
  Serial.print(waktu.timestamp(DateTime::TIMESTAMP_DATE));//TAHUN, BULAN, HARI
  Serial.print(" ");
  Serial.print(waktu.timestamp(DateTime::TIMESTAMP_TIME));//JAM, MENIT, DETIK
  Serial.print(" , ");
  Serial.print (jarak);
  Serial.print (" , ");
  Serial.print(temperatureC);
  Serial.println("ºC");

  // atur dulu alaram RTC nya
  aturAlarm();
  sleepmode();
}

void onAlarm() {
  Serial.println("Alarm occured!");
}

void aturAlarm() {
  rtc.disable32K();

  pinMode(CLOCK_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INTERRUPT_PIN), onAlarm, FALLING);

  // set alarm 1, 2 flag to false (so alarm 1, 2 didn't happen so far)
  // if not done, this easily leads to problems, as both register aren't reset on reboot/recompile
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  // stop oscillating signals at SQW Pin
  // otherwise setAlarm1 will fail
  rtc.writeSqwPinMode(DS3231_OFF);

  // turn off alarm 2 (in case it isn't off already)
  // again, this isn't done at reboot, so a previously set alarm could easily go overlooked
  rtc.disableAlarm(2);

  //bangun setiap interval dalam menit
  waktunanti = waktu + interval * 60;
  waktu = DateTime(waktunanti.year(), waktunanti.month(), waktunanti.day(), waktunanti.hour(), waktunanti.minute(), 0);
  rtc.setAlarm1(
    waktu,
    DS3231_A1_Hour // this mode triggers the alarm when the seconds match. See Doxygen for other options
  );
}

void sleepmode() {
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
  //ATUR SLEEP TIME
  //  Serial.println("tidur");
  Serial.flush();
  esp_deep_sleep_start();
}
