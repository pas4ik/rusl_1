#include <Arduino.h>
// ATTENTION!!! millis() 50 days!!! - need to fix
#define CAMERA_WAIT_TIMEOUT  (uint16_t)2000 // ms
#define CAM_NO_ERR    (uint8_t)1
#define CAM_ERR_SIG   (uint8_t)2

#define OUT_TIMEOUT   (uint16_t)500   // ms
#define BUFF_OK         (0)
#define BUFF_ERR_NULL   (-1)
#define BUFF_ERR_EMPTY  (-2)
#define BUFF_ERR_FULL   (-3)

int to_buffer(unsigned char * val);
int from_buffer(unsigned char * val);
void update_in(void);

int           buf_res;
unsigned int  sens1_ON, sens1_ON_old, sens1_old, sens1_cnt,
              sens2_ON, sens2_ON_old, sens2_old, sens2_cnt,
              sens_cam_ON, sens_cam_ON_old, sens_cam_old, sens_cam_cnt;
unsigned char camera_wait = 0, temp,
              camera_fix = 0, out_on,
              buff[5], buff_ind_write, buff_ind_read; // ring buffer
unsigned long camera_wait_time, out_time;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, INPUT_PULLUP);   // photo 1
  pinMode(3, INPUT_PULLUP);   // photo 2
  pinMode(4, INPUT_PULLUP);   // cam error signal
  pinMode(5, OUTPUT);         // out rele
  digitalWrite(5, HIGH);      // invers, active low
  // init ring buffer
  buff_ind_write = 1;
  buff_ind_read = 1;
}

int to_buffer(unsigned char * val)
{
  unsigned char tmp = buff_ind_write + 1;
  if (tmp >= sizeof(buff)) tmp = 1;
  if (tmp == buff_ind_read)
    return BUFF_ERR_FULL;
  
  buff[buff_ind_write] = (*val);

  unsigned char temp = buff_ind_write + 1; 
  if (temp >= sizeof(buff)) temp = 1;
  buff_ind_write = temp;

  return BUFF_OK;
}

int from_buffer(unsigned char * val)
{
  if (buff_ind_read == buff_ind_write)
    return BUFF_ERR_EMPTY;
  
  (*val) = buff[buff_ind_read];
  buff[buff_ind_read] = 0;

  unsigned char temp = buff_ind_read + 1; 
  if (temp >= sizeof(buff)) temp = 1;
  buff_ind_read = temp;

  return BUFF_OK;
}

void update_in(void)
{
  static unsigned long debouns_millis;

  if (millis() > debouns_millis + 10) // 10 ms step
  {
    if (digitalRead(2) == 0)  // debounce photo sens1
    {
        if (sens1_old == 1)
        {
            if (sens1_cnt < 200) sens1_cnt++;
            if (sens1_cnt > 30) sens1_ON = 1; // 300 ms
        } else sens1_old = 1;
    } else
    {
      sens1_old = 0;
      sens1_ON_old = 0;
      sens1_cnt = 0;
      sens1_ON = 0;
    }

    if (digitalRead(3) == 0)  // debounce photo sens2
    {
        if (sens2_old == 1)
        {
            if (sens2_cnt < 200) sens2_cnt++;
            if (sens2_cnt > 30) sens2_ON = 1; // 300 ms
        } else sens2_old = 1;
    } else
    {
      sens2_old = 0;
      sens2_ON_old = 0;
      sens2_cnt = 0;
      sens2_ON = 0;
    }

    if (digitalRead(4) == 0)  // debounce cam error signal
    {
        if (sens_cam_old == 1)
        {
            if (sens_cam_cnt < 200) sens_cam_cnt++;
            if (sens_cam_cnt > 30) sens_cam_ON = 1; // 300 ms
        } else sens_cam_old = 1;
    } else
    {
      sens_cam_old = 0;
      sens_cam_ON_old = 0;
      sens_cam_cnt = 0;
      sens_cam_ON = 0;
    }
  }
}

void loop() {
// test buffer
/*
if (Serial.available() > 0)
  {
    unsigned char temp = (unsigned char)Serial.read();
    unsigned char temp2 = 0;
    Serial.println("");
    if (temp == '9')
    {
      Serial.print(buff_ind_read); Serial.print(":");
      Serial.println(from_buffer(&temp2));
      Serial.println(temp2);
      Serial.println(buff_ind_read);
    } else
    {
      Serial.print(buff_ind_write); Serial.print(":");
      Serial.println(to_buffer(&temp));
      Serial.println(buff_ind_write);
    }
    for (uint8_t i = 0; i < sizeof(buff); i++)
    {
      Serial.print(buff[i]);
      Serial.print(" ");
    }
    Serial.println("");
  }
*/    

  update_in();
  
  if ((sens1_ON) && (sens1_ON_old == 0))
  {
    sens1_ON_old = 1;
    camera_wait = 1;
    camera_fix = 0;
    camera_wait_time = millis();
  } else
  if (sens1_ON == 0)
  {
  }

  if (camera_wait)
  {
    if ((sens_cam_ON) && (sens_cam_ON_old == 0))
    {
      sens_cam_ON_old = sens_cam_ON;

      temp = CAM_ERR_SIG;
      buf_res = to_buffer(&temp); // if cam error
      if (buf_res == BUFF_OK)
      {
        for (uint8_t i = 0; i < sizeof(buff); i++)
        {
          Serial.print(buff[i]);
          Serial.print(" ");
        }
        Serial.println("");
      } else
      {
        Serial.println(buf_res);
      }
      camera_fix = 1;
      camera_wait = 0;
    } else
    if (millis() > camera_wait_time + CAMERA_WAIT_TIMEOUT)
    {// stop waiting error signal from camera
      temp = CAM_NO_ERR;
      if (!camera_fix)
      {
        buf_res = to_buffer(&temp);  // no cam error
        if (buf_res == BUFF_OK)
        {
          for (uint8_t i = 0; i < sizeof(buff); i++)
          {
            Serial.print(buff[i]);
            Serial.print(" ");
          }
          Serial.println("");
        } else
        {
          Serial.println(buf_res);
        }
        camera_fix = 1;
      }
      camera_wait = 0;
    }
  }

  if ((sens2_ON) && (sens2_ON_old == 0))
  {
    sens2_ON_old = 1;
    buf_res = from_buffer(&temp);
    if (buf_res == BUFF_OK)
    {
      if (temp == CAM_NO_ERR)
      {
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(5, LOW);   // rele on
        out_time = millis();
        out_on = 1;
      } else
      {
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(5, HIGH);   // rele off
        out_on = 1;
      }

      for (uint8_t i = 0; i < sizeof(buff); i++)
      {
        Serial.print(buff[i]);
        Serial.print(" ");
      }
      Serial.println("");
    } else
    {
      Serial.println(buf_res);
    }
  } else
  if (sens2_ON == 0)
  {
  }

  if (out_on)
  {
    if (millis() > out_time + OUT_TIMEOUT)
    {
      out_on = 0;
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(5, HIGH);
    }
  }

}