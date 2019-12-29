#include <SoftwareSerial.h>

SoftwareSerial SSerial(SCL, SDA);//I2Cポートでソフトウエアシリアルを使用
typedef uint8_t U8;

void setup()
{
  Serial.begin(9600);

  delay(200);
  GroveMP3V3_begin();
  Serial.print("VolumeControl=>");
  GroveMP3V3_VolumeControl(3);
  Serial.println(GroveMP3V3_QueryCurrentVolume());

  Serial.print("Total Number of MusicFiles=>");
  Serial.println(GroveMP3V3_QueryTotalMusicFiles());

  GroveMP3V3_FilePlay("01", 2);
  delay(10000);

  GroveMP3V3_DirIndexPlay("MUSIC", 5, 1);
  delay(10000);

  Serial.println("Play");
  GroveMP3V3_IndexPlay(3);
  //while (GroveMP3V3_QueryCurrentOperationState() == 1) delay(3000);//再生終了は左記のループでチェック
  delay(10000);

  Serial.println("Pause");
  GroveMP3V3_Pause();
  delay(3000);
  GroveMP3V3_Pause();
  delay(3000);

  Serial.println("FastForward");
  GroveMP3V3_FastForward();
  delay(10000);

  Serial.println("Rewind");
  GroveMP3V3_Rewind();
  delay(10000);

  Serial.println("Stop");
  GroveMP3V3_Stop();
}

void loop()
{
}

void GroveMP3V3_begin()
{
  SSerial.begin(9600);
}

void GroveMP3V3_IndexPlay(int index)
{
  char parameter[2];
  parameter[0] = index / 256;
  parameter[1] = index % 256;
  GroveMP3V3_WriteCommand(0xa2, parameter, sizeof(parameter));

  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

void GroveMP3V3_FilePlay(U8 *parameter, int parameterSize)
{
  if (parameterSize > 8) abort();
  GroveMP3V3_WriteCommand(0xa3, parameter, sizeof(parameter));

  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

void GroveMP3V3_DirIndexPlay(U8 *parameter, int parameterSize, int index)
{
  if (parameterSize != 5) abort();
  char param[parameterSize+2];
  for(int i=0; i<parameterSize; i++) param[i] = parameter[i];
  param[parameterSize+0] = index / 256;
  param[parameterSize+1] = index % 256;
  
  GroveMP3V3_WriteCommand(0xa4, param, sizeof(param));

  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

void GroveMP3V3_Pause()
{
  GroveMP3V3_WriteCommand(0xaa, NULL, 0);
  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

void GroveMP3V3_Stop()
{
  GroveMP3V3_WriteCommand(0xab, NULL, 0);
  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

void GroveMP3V3_FastForward()
{
  GroveMP3V3_WriteCommand(0xac, NULL, 0);
  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

void GroveMP3V3_Rewind()
{
  GroveMP3V3_WriteCommand(0xad, NULL, 0);
  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

void GroveMP3V3_VolumeControl(int volume)
{
  if (volume < 0) volume = 0;
  if (31 < volume) volume = 31;

  U8 parameter[] = { (U8)volume };
  GroveMP3V3_WriteCommand(0xae, parameter, sizeof(parameter));

  U8 operationCode;
  if (!GroveMP3V3_ReadReturn(&operationCode, NULL, 0)) abort();
  if (operationCode != 0x00) abort();
}

int GroveMP3V3_QueryCurrentVolume()
{
  GroveMP3V3_WriteCommand(0xc1, NULL, 0);

  U8 operationCode;
  U8 currentVolume;
  if (!GroveMP3V3_ReadReturn(&operationCode, &currentVolume, 1)) abort();
  if (operationCode != 0xc1) abort();

  return currentVolume;
}

int GroveMP3V3_QueryCurrentOperationState()
{
  GroveMP3V3_WriteCommand(0xc2, NULL, 0);

  U8 operationCode;
  U8 currentOperationState;
  if (!GroveMP3V3_ReadReturn(&operationCode, &currentOperationState, 1)) abort();
  if (operationCode != 0xc2) abort();

  return currentOperationState;
}

int GroveMP3V3_QueryTotalMusicFiles()
{
  GroveMP3V3_WriteCommand(0xc5, NULL, 0);

  U8 operationCode;
  U8 currentFiles[2];
  int count;
  if (!GroveMP3V3_ReadReturn(&operationCode, currentFiles, 2)) abort();
  if (operationCode != 0xc5) abort();

  count = currentFiles[0] * 256;//インデックスの上位
  count += currentFiles[1];//インデックスの下位

  return count;
}

void GroveMP3V3_WriteCommand(U8 commandCode, const U8 *parameter, int parameterSize)
{
  U8 length = 1 + 1 + parameterSize + 1;// Lengthを計算
  U8 sum = 0;// チェックサムの初期化
  // 以下チェックサムの計算
  sum += length;
  sum += commandCode;
  for (int i = 0; i < parameterSize; i++) sum += parameter[i];

  //シリアルポートへの書き込み
  SSerial.write(0x7e);// 開始コード
  SSerial.write(length);// Length
  SSerial.write(commandCode);// コマンド
  SSerial.write(parameter, parameterSize);// コマンド引数
  SSerial.write(sum);// チェックサム
  SSerial.write(0xef);// 終了コード
}

bool GroveMP3V3_ReadReturn(U8 *operationCode, U8 *returnValue, U8 returnValueSize)
{
  while (SSerial.available() < 1);// 受信待ち
  *operationCode = SSerial.read();// コマンドデータの受信

  for (int i = 0; i < returnValueSize; i++)
  {
    while (SSerial.available() < 1);// 受信待ち
    returnValue[i] = SSerial.read();// return値の受信
  }

  return true;
}
