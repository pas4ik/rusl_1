#include <Arduino.h>

#define BUFF_OK         (0)
#define BUFF_ERR_NULL   (-1)
#define BUFF_ERR_EMPTY  (-2)
#define BUFF_ERR_FULL   (-3)

int to_buffer(unsigned char * val);
int from_buffer(unsigned char * val);
void update_in(void);

unsigned int  sens1_ON, sens1_ON_old, sens1_old, sens1_cnt,
              sens2_ON, sens2_ON_old, sens2_old, sens2_cnt,
              sens_cam_ON, sens_cam_ON_old, sens_cam_old, sens_cam_cnt;
unsigned char camera_wait = 0,
              camera_fix = 0,
              buff[5], buff_ind_write, buff_ind_read;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  buff_ind_write = 1;
  buff_ind_read = 1;

  Serial.print("w "); Serial.print(buff_ind_write);
  Serial.print("r "); Serial.print(buff_ind_read);
  Serial.println("");
}

int to_buffer(unsigned char * val)
{
  unsigned char tmp = buff_ind_write + 1;
  if (tmp > sizeof(buff)) tmp = 1;
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
    if (digitalRead(2) == 0)  // debounce sens1
    {
        if (sens1_old == 1)
        {
            if (sens1_cnt < 200) sens1_cnt++;
            if (sens1_cnt > 10) sens1_ON = 1; // 100 ms
        } else sens1_old = 1;
    } else
    {
      sens1_old = 0;
      sens1_ON_old = 0;
      sens1_cnt = 0;
      sens1_ON = 0;
    }

    if (digitalRead(3) == 0)  // debounce sens2
    {
        if (sens2_old == 1)
        {
            if (sens2_cnt < 200) sens2_cnt++;
            if (sens2_cnt > 10) sens2_ON = 1; // 100 ms
        } else sens2_old = 1;
    } else
    {
      sens2_old = 0;
      sens2_ON_old = 0;
      sens2_cnt = 0;
      sens2_ON = 0;
    }

    if (digitalRead(4) == 0)  // debounce cam
    {
        if (sens_cam_old == 1)
        {
            if (sens_cam_cnt < 200) sens_cam_cnt++;
            if (sens_cam_cnt > 10) sens_cam_ON = 1; // 100 ms
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
  
//  update_in();  

  if (Serial.available() > 0)
  {
    unsigned char temp = (unsigned char)Serial.read();
    unsigned char temp2 = 0;
    if (temp == 99)
    {
      Serial.println(from_buffer(&temp2));
      Serial.println(temp2);
    } else
    {
      Serial.println(to_buffer(&temp));
    }
  }

/*  
  if ((sens1_ON) && (sens1_ON_old == 0))
  {
    sens1_ON_old = 1;
    camera_wait = 1;
    camera_fix = 0;
  } else
  if (sens1_ON == 0)
  {
    if (camera_wait == 1)
    {
      camera_wait = 0;
      if (!camera_fix) to_buffer(0);  // no cam error
    }
  }

  if (camera_wait)
  {
    if (sens_cam_ON)
    {
      to_buffer(1); // 1 if cam error
      camera_fix = 1;
      camera_wait = 0;
    }
  }
*/
}