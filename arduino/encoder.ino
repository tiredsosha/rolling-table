#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define CLOCK_PIN 7
#define DATA_PIN 2

#define ZERO 5
#define ONEEIGHTY 3

#define RECEIVE_UDP 8888
#define SEND_UDP 8000

#define ABS_TURN 15
#define ABS_ANG 167.0

IPAddress IP_UDP(10, 104, 0, 87);
//
short int last_turn = 0;
short int last_angle = 0;
short int index = 0;
//
short int zero_status = 1;
unsigned int zero_res = 0;
//
short int dest_angle = 100;
short int dest_turns = 95;
// static ip data
byte mac[] = {0xDE, 0xAD, 0xEE, 0xEF, 0xFE, 0xED};
byte ip[] = {10, 104, 0, 88};
byte gateway[] = {10, 104, 0, 1};
byte subnet[] = {255, 255, 254, 0};
byte dns[] = {10, 10, 0, 41};
// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

unsigned long _shiftIn(const int data_pin, const int clock_pin, const int bit_count)
{
    unsigned long data = 0;

    for (int i = 0; i < bit_count; i++)
    {
        data <<= 1;
        digitalWrite(clock_pin, LOW);
        delayMicroseconds(1);

        digitalWrite(clock_pin, HIGH);
        delayMicroseconds(1);

        data |= digitalRead(data_pin);
    }
    return data;
}

static uint32_t grey_to_binary(unsigned long v)
{
    uint32_t mask = v;
    while (mask)
    {
        mask >>= 1;
        v ^= mask;
    }
    return v;
}

void check_dest(short int turns, short int angle)
{
    unsigned int temp_res = (turns * 4095) + angle;
    unsigned int temp_dif = abs(temp_res - zero_res);
    short int temp_turns = temp_dif / 4095;
    float temp_angle = (float(temp_dif) / 4095.0 - float(temp_turns)) * 4095.0;
    float no_period_angle = (float(temp_turns) * 4095.0 + temp_angle) / ABS_ANG;
    String current_angle = String(no_period_angle);

    if (abs(last_turn - turns) < 3 && abs(last_angle - no_period_angle) < 150)
    {
        udp(current_angle);
    }
    last_turn = turns;
    last_angle = no_period_angle;
}

void udp(String current_angle)
{
    Serial.print("Current angle is ");
    Serial.println(current_angle);
    Udp.beginPacket(IP_UDP, SEND_UDP);
    Udp.print(current_angle);
    Udp.endPacket();
}

void read_pin()
{
    if (digitalRead(ZERO) == 0)
    {
        zero_status = 1;
    }
    else
    {
        zero_status = 0;
    }
    if (digitalRead(ONEEIGHTY) == 0)
    {
        zero_status = 2;
    }
    else
    {
        zero_status = 0;
    }
}
void encoder()
{
    static uint32_t s3 = 0;
    delay(1);

    uint32_t s1 = _shiftIn(DATA_PIN, CLOCK_PIN, 24);
    delayMicroseconds(25);
    uint32_t s2 = _shiftIn(DATA_PIN, CLOCK_PIN, 24);
    delayMicroseconds(100);

    if (s1 != s2)
    {
        return;
    }
    if (s1 == s3)
    {
        return;
    }

    s3 = s1;
    s1 = grey_to_binary(s1);

    uint16_t bin_turns = s1 >> 12, bin_angle = s1 & 0xFFF;
    short int int_turns = bin_turns;
    short int int_angle = bin_angle;
    Serial.print("Total: ");
    Serial.print(bin_turns);
    Serial.print(", Current position: ");
    Serial.println(bin_angle);
    if (index == 0)
    {
        last_turn = int_turns;
        last_angle = int_angle;
        index = 1;
    }
    if (zero_status == 1)
    {
        zero_res = (int_turns * 4095) + int_angle;
        zero_status = 0;
    }
    check_dest(int_turns, int_angle);
}

void setup()
{
    pinMode(DATA_PIN, INPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    digitalWrite(CLOCK_PIN, HIGH);
    pinMode(ZERO, INPUT);
    pinMode(ONEEIGHTY, INPUT);
    Ethernet.begin(mac, ip, dns, gateway, subnet);
    Udp.begin(RECEIVE_UDP);

    Serial.begin(9600);
    Serial.println("Setup is complite");
}

void loop()
{
    // read_pin();
    encoder();
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
        zero_status = 1;
    }
}
