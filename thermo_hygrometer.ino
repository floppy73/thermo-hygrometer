#include <DHT.h>
#define DHT_PIN 8
#define DHT_MODEL DHT11
#define interval 3600000
#define HUMIDITY_PIN 5
#define MEMORY_PIN 12
float volt_r = 0;
unsigned long lud = 0;

DHT dht(DHT_PIN, DHT_MODEL);
int pin[] = {6, 9, 10, 11}; //温度表示用のピン
int memory[24][3] = {};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);         // シリアル通信の開始
  dht.begin();
  for (int i = 0; i < sizeof(pin); i++) {
    pinMode(pin[i], OUTPUT);
  }
  pinMode(HUMIDITY_PIN, OUTPUT);
  pinMode(2, OUTPUT); //2,3,4は不快指数表示用のピン
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(MEMORY_PIN, OUTPUT);
}

void loop() {
  volt_r = analogRead(A0);
  int hour = int(map(volt_r, 0, 1023, 0, 24));

  if (hour > 0) 
  {
    digitalWrite(MEMORY_PIN, HIGH);
    delay(5);
    
    int t_memory = memory[24 - hour][0]; //配列から温湿度を取得
    int h_memory = memory[24 - hour][1];
    int di_memory = memory[24 - hour][2];

    // LED点灯処理
    float m_h_duty = map(h_memory, 0, 100, 0, 255);
    analogWrite(HUMIDITY_PIN, (int)m_h_duty);
    on_t_led(t_memory);
    if(t_memory == 0 && h_memory == 0) {
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      digitalWrite(4, LOW);
    } else {
      on_di_led(di_memory);
    }
  }
  else //可変抵抗がいじられていないとき
  {    
    digitalWrite(MEMORY_PIN, LOW);
    int humidity = round(dht.readHumidity());
    int temperature = round(dht.readTemperature());

    if (isnan(humidity)) {
      humidity = 0;
    } 
    if(isnan(temperature)) {
      temperature = 0;
    }

    temperature = min(temperature, 40);

    float di_f = 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;
    int di = round(di_f);

    // LED点灯処理
    float humidity_duty = map(humidity, 0, 100, 0, 255);
    analogWrite(HUMIDITY_PIN, (int)humidity_duty); 
    on_t_led(temperature);
    if(temperature == 0 && humidity == 0) {
      digitalWrite(2, LOW);
      digitalWrite(3, LOW);
      digitalWrite(4, LOW);
    } else {
      on_di_led(di);
    }

    //intervalごとに実行
    unsigned long now = millis();
    if(now - lud >= interval || lud == 0) {
      update_memory(temperature, humidity, di);
      lud = now;
    }
    
    // シリアルモニタに表示
    Serial.print("温度: ");
    Serial.print(temperature);
    Serial.print("[℃] ");

    Serial.print("湿度: ");
    Serial.print(humidity);
    Serial.print("[%] ");

    Serial.print("不快指数: ");
    Serial.println(di);

    delay(2000);
  }
}

//温度に応じてLEDを光らせる関数
void on_t_led(int t){
  for (int i = 0; i < 4; i++) {
      digitalWrite(pin[i], LOW);
  }
  for (int i = 0; i < t/10; i++) {
      digitalWrite(pin[i], HIGH);
  }
  int fraction_duty = (t % 10) * 256/10;
  analogWrite(pin[t/10], fraction_duty);
}

//不快指数に応じてLEDを光らせる関数
void on_di_led(int di) {
  if(di <= 60) {
    digitalWrite(2, HIGH);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
  } else if(di > 60 && di < 75) {
    digitalWrite(2, LOW);
    digitalWrite(3, HIGH);
    digitalWrite(4, LOW);
  } else {
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, HIGH);
  }
}

//メモリー用配列に値を登録する関数
void update_memory(int t, int h, int di) {
  for (int i = 0; i < 23; i++) {
      memory[i][0] = memory[i + 1][0];
      memory[i][1] = memory[i + 1][1];
      memory[i][2] = memory[i + 1][2];
    }
    memory[23][0] = t;
    memory[23][1] = h;
    memory[23][2] = di;
}
