/*
	BY8x0116Pv2.h - Library for the BY8x01-6P module(s)
	First Release Created by /u/afreeland (Forked from Critters/MP3FLASH16P)
	Updated naming and removed defined serial ports by r0ndL
	Released into the public domain
*/

#include "Arduino.h"
#include "BY8x0116Pv2.h"

BY8x0116Pv2::BY8x0116Pv2()
  : _port(Serial)
{
  
}

BY8x0116Pv2::BY8x0116Pv2(Stream& port)
  : _port(port)
{
  
}

void BY8x0116Pv2::init(int PIN_BUSY, int VOLUME)
{
    _PIN_BUSY = PIN_BUSY;
    _VOLUME = (uint8_t)VOLUME;
    pinMode(_PIN_BUSY, INPUT);
    digitalWrite(_PIN_BUSY, LOW);
}

void BY8x0116Pv2::playFile(int FILE_NUMBER, int VOLUME)
{   
    if(VOLUME != -1)
    {
        _VOLUME = (uint8_t)VOLUME;
    }

    setVolume(_VOLUME);

    delay(100);
    uint8_t filenumber = (uint8_t)FILE_NUMBER;
    // PDF example of how to play first track
    //uint8_t play1[7] = { 0X7E, 0X05, 0X41, 0X00, 0X01, 0X45, 0XEF };

    // param[1] Op Code - Determines what action is caused
    // param[2] High Track - No clue
    // param[3] Low Track - The file that we want to play
    uint8_t play_params[3] = { 0X41, 0X00, filenumber };
    int arrSize = sizeof(play_params)/sizeof(uint8_t);
    uint8_t * _params = buildParams(play_params, arrSize);

    makeRequest(_params, arrSize);

}

void BY8x0116Pv2::playFileAndWait(int FILE_NUMBER, int VOLUME)
{
    playFile(FILE_NUMBER, VOLUME);
    delay(100);
    while(isBusy()){
    }
    delay(250);
}

bool BY8x0116Pv2::isBusy()
{
  // The BUSY pin will be HIGH while playing and LOW when stopped
  // So we need to check if our busy pin is HIGH, if so it is playing, therefor, is busy
    return digitalRead(_PIN_BUSY) == HIGH;
}

uint8_t * BY8x0116Pv2::buildParams(uint8_t *params, int len)
{
  // Since we cant easily set dynamic array length, and even if we
  // did it can cause the 'heap' to become fragmented...we will simply default to a max of 10
  // http://arduino.stackexchange.com/questions/3774/how-can-i-declare-an-array-of-variable-size-globally
  // http://arduino.stackexchange.com/questions/682/is-using-malloc-and-free-a-really-bad-idea-on-arduino
  //uint8_t payload[10] = { (uint8_t)len };
  uint8_t* payload = new uint8_t[10];
  
  // Set the Length to user params + 2
  // params.length + Length arg + Checksum arg
  payload[0] = (uint8_t)(len + 2);

  // Create our the beginning of our calculated checksum..the first part is our length argument
  uint8_t checksum = payload[0];
  
  // Build out our full payload while also
  // calculating out our checksum
  for (int i=0; i<len; i++) 
  {
      // We need to XOR the current parameter with our current checksum
      checksum = (checksum ^ params[i]);
      
      payload[(i + 1)] = params[i];
  }
  
  payload[len+1] = checksum;

  for(int z = 0; z < 10; z++){
    if(z > len + 1){
      // our current iteration is larger than the params passed in, lets start zeroing out
      payload[z] = 0x99;
    }else{
      //Serial.println(payload[z], HEX);
    }
  }
  
  return (uint8_t *)payload;
}

void BY8x0116Pv2::setVolume(int VOLUME)
{
    _VOLUME = constrain(VOLUME, 0, 30);
    
    //uint8_t volume_op[6]  = {0X7E, 0X04, 0X31, 0X19, 0X2C, 0XEF };
    uint8_t volume_op[2]  = {0X31, _VOLUME};
    int arrSize = sizeof(volume_op)/sizeof(volume_op[0]);
    uint8_t * _params = buildParams(volume_op, arrSize);

    makeRequest(_params, arrSize);
    
}

void BY8x0116Pv2::makeRequest(uint8_t * params, int len)
{
  // Write our start code
    _port.write((uint8_t)0X7E);
    
    // Write parameters while building checksum
    for (int i=0; i < len + 2; i++) 
    {
      if(params[i] == 0x99)
        break;
        
      //Serial.println(params[i], HEX);
        // Write our current parameter
        _port.write( (uint8_t)params[i] ); 
    }

    // Write our end code
    _port.write((uint8_t)0XEF);
}

void BY8x0116Pv2::volumeUp()
{
    _VOLUME = min(_VOLUME+5, 30);
}

void BY8x0116Pv2::volumeDown()
{
    _VOLUME = max(_VOLUME-5, 0);
}

void BY8x0116Pv2::stopPlay()
{   
    uint8_t play1[8] = { 0X7E, 0XFF, 0X06, 0X16, 0X00, 0x00, 0x00, 0XEF };
    for (int i=0; i<8; i++) 
    {
        _port.write( play1[i] ); 
    }
}



