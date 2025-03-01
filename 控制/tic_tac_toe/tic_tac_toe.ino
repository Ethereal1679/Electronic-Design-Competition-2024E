#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#define LED_pin A0
#define Beep_pin A1
#define pick_pin A8
#define drop_pin A9

Servo pump;
Servo valve;

//摄像头到机械臂坐标系变换
#define offset_x -10  //-8  以7、8号格上方交点为基准
#define offset_y 138  //130
#define mm_per_pixel 30 / 32.0

#define Board_pin A10
#define Chess_pin A11

//机械臂长度
#define L1 155
#define L2 165
#define end 30

#define PI 3.141592653589793238462

SoftwareSerial Serial4(10, 11);

float servo_angle[3];
char inputString1[50] = "", inputString2[50] = "";          //保存串口接收的字符串
bool string1Complete = false, string2Complete = false;      //串口接收完成标志位
int cnt1, cnt2;
int mode = 0, block = 0;
int board[3][3][2];
float board_arm[3][3][2];
int edge[8];

float white[5][2] = {-75, 82, -82, 111, -87, 140, -90, 173, -97, 213};
float black[5][2] = {90, 85, 93, 116, 97, 147, 102, 175, 110, 215};

int status[3][3];  //当前棋盘状态 0未下，1黑棋，2白棋 
int status_last[3][3];  //上一回合棋盘状态

int flag = 0, chess_x, chess_y;
int mark_x = 0, mark_y = 0, lost_x = 0, lost_y = 0;

void mode_select()
{
  Serial.println(inputString1);
  if(strcmp(inputString1, "q1") == 0)
    function1();
  else if(strcmp(inputString1, "q2") == 0 || strcmp(inputString1, "q3") == 0)
    function2_3();
  else if(strcmp(inputString1, "q4") == 0)
    function4();
  else if(strcmp(inputString1, "q5") == 0)
    function5();
  
}

void chess_select()
{
  Serial.println(inputString1);
  string1Complete = false;
  if(strcmp(inputString1, "b1") == 0)
    chess_x = black[0][0], chess_y = black[0][1];
  else if(strcmp(inputString1, "w1") == 0)
    chess_x = white[0][0], chess_y = white[0][1];
  else if(strcmp(inputString1, "b2") == 0)
    chess_x = black[1][0], chess_y = black[1][1];
  else if(strcmp(inputString1, "w2") == 0)
    chess_x = white[1][0], chess_y = white[1][1];
  else if(strcmp(inputString1, "b3") == 0)
    chess_x = black[2][0], chess_y = black[2][1];
  else if(strcmp(inputString1, "w3") == 0)
    chess_x = white[2][0], chess_y = white[2][1];
  else if(strcmp(inputString1, "b4") == 0)
    chess_x = black[3][0], chess_y = black[3][1];
  else if(strcmp(inputString1, "w4") == 0)
    chess_x = white[3][0], chess_y = white[3][1];
  else if(strcmp(inputString1, "b5") == 0)
    chess_x = black[4][0], chess_y = black[4][1];
  else if(strcmp(inputString1, "w5") == 0)
    chess_x = white[4][0], chess_y = white[4][1];

  Serial.println("check");
}
void board_select()
{
  Serial.println(inputString1);
  string1Complete = false;
  if(strcmp(inputString1, "pos_1") == 0)
    mark_x = board_arm[0][0][0], mark_y = board_arm[0][0][1], block = 0;
  else if(strcmp(inputString1, "pos_2") == 0)
    mark_x = board_arm[0][1][0], mark_y = board_arm[0][1][1], block = 1;
  else if(strcmp(inputString1, "pos_3") == 0)
    mark_x = board_arm[0][2][0], mark_y = board_arm[0][2][1], block = 2;
  else if(strcmp(inputString1, "pos_4") == 0)
    mark_x = board_arm[1][0][0], mark_y = board_arm[1][0][1], block = 3;
  else if(strcmp(inputString1, "pos_5") == 0)
    mark_x = board_arm[1][1][0], mark_y = board_arm[1][1][1], block = 4;
  else if(strcmp(inputString1, "pos_6") == 0)
    mark_x = board_arm[1][2][0], mark_y = board_arm[1][2][1], block = 5;
  else if(strcmp(inputString1, "pos_7") == 0)
    mark_x = board_arm[2][0][0], mark_y = board_arm[2][0][1], block = 6;
  else if(strcmp(inputString1, "pos_8") == 0)
    mark_x = board_arm[2][1][0], mark_y = board_arm[2][1][1], block = 7;
  else if(strcmp(inputString1, "pos_9") == 0)
    mark_x = board_arm[2][2][0], mark_y = board_arm[2][2][1], block = 8;
  Serial.println("check");
}

/*OpenMV视角
   9  8  7
   6  5  4
   3  2  1
*/
void board_locate()
{
  // char* location;
  digitalWrite(Board_pin, HIGH);
  delay(50);
  digitalWrite(Board_pin, LOW);
  read_uart4();
  // location = strtok(inputString2, ",");
  // edge[0] = atoi(location);
  for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
    {
      board[i][j][0] = (int)*((unsigned char *)(inputString2 + 12*(2-i) + 4*(2-j))) + ((int)*((unsigned char *)(inputString2 + 12*(2-i) + 4*(2-j) + 1)) << 8);
      board[i][j][1] = (int)*((unsigned char *)(inputString2 + 12*(2-i) + 4*(2-j) + 2)) + ((int)*((unsigned char *)(inputString2 + 12*(2-i) + 4*(2-j) + 3)) << 8);
    }

  if((float)(board[0][0][1] - board[0][1][1]) / (board[0][0][0] - board[0][1][0]) <= -1)
  {
    int b_temp[3][3][2];
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        b_temp[i][j][0] = board[i][j][0], b_temp[i][j][1] = board[i][j][1];
    board[0][0][0] = b_temp[0][1][0], board[0][0][1] = b_temp[0][1][1];
    board[0][1][0] = b_temp[0][2][0], board[0][1][1] = b_temp[0][2][1];
    board[0][2][0] = b_temp[1][2][0], board[0][2][1] = b_temp[1][2][1];
    board[1][0][0] = b_temp[0][0][0], board[1][0][1] = b_temp[0][0][1];
    board[1][2][0] = b_temp[2][2][0], board[1][2][1] = b_temp[2][2][1];
    board[2][0][0] = b_temp[1][0][0], board[2][0][1] = b_temp[1][0][1];
    board[2][1][0] = b_temp[2][0][0], board[2][1][1] = b_temp[2][0][1];
    board[2][2][0] = b_temp[2][1][0], board[2][2][1] = b_temp[2][1][1];
  }
  if((float)(board[0][2][1] - board[0][1][1]) / (board[0][2][0] - board[0][1][0]) >= 1)
  {
    int b_temp[3][3][2];
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        b_temp[i][j][0] = board[i][j][0], b_temp[i][j][1] = board[i][j][1];
    board[0][0][0] = b_temp[1][0][0], board[0][0][1] = b_temp[1][0][1];
    board[0][1][0] = b_temp[0][0][0], board[0][1][1] = b_temp[0][0][1];
    board[0][2][0] = b_temp[0][1][0], board[0][2][1] = b_temp[0][1][1];
    board[1][0][0] = b_temp[2][0][0], board[1][0][1] = b_temp[2][0][1];
    board[1][2][0] = b_temp[0][2][0], board[1][2][1] = b_temp[0][2][1];
    board[2][0][0] = b_temp[2][1][0], board[2][0][1] = b_temp[2][1][1];
    board[2][1][0] = b_temp[2][2][0], board[2][1][1] = b_temp[2][2][1];
    board[2][2][0] = b_temp[1][2][0], board[2][2][1] = b_temp[1][2][1];
  }
  //OpenMV坐标转换为机械臂坐标，当前高度下30mm ≈ 36像素  320*240
  //假设机械臂与摄像头中心x值相同，y值差170mm
  for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
    {
      board_arm[i][j][0] = (160 - board[i][j][0]) * mm_per_pixel + offset_x;
      board_arm[i][j][1] = (board[i][j][1] - 120) * mm_per_pixel + offset_y;
    }
  
  // inputString2 = "";
  string2Complete = false;
  Serial.println("check");

}

void get_chess_location()
{
  int chess_x, chess_y, i=0;
  for(i=0; i<9; i++)
  {
    status[i/3][i%3] = (int)*((unsigned char *)(inputString2 + (8-i)));
    Serial.println(status[i/3][i%3]);
  }
  
  for(int j=0; j<3; j++)
    for(int k=0; k<3; k++)
    {
      if(status[j][k] != status_last[j][k] && status_last[j][k] == 0)  //该棋盘格被对手落子
        mark_x = k, mark_y = j;
      else if(status[j][k] != status_last[j][k] && status[j][k] == 0)  //该棋盘格的棋子不见了
        flag = 3, lost_x = k, lost_y = j;
    }
  
}
//====================================================
//机械臂运动学解算
//====================================================
void ikine(float x, float y, float z)
{
  servo_angle[0] = angle_2_servo_actual((-atan2(x, y) * 180 / PI * 0.85 + 90));  //底座
  servo_angle[1] = acos((pow(L1, 2) + pow(sqrt(pow(x, 2) + pow(y, 2)) - end, 2) + pow(z, 2) - pow(L2, 2)) / (2 * L1 * sqrt(pow(sqrt(pow(x, 2) + pow(y, 2)) - end, 2) + pow(z, 2))));  //大臂
  if(z < 0) servo_angle[1] -= acos((sqrt(pow(x, 2) + pow(y, 2)) - end) / sqrt(pow(sqrt(pow(x, 2) + pow(y, 2)) - end, 2) + pow(z, 2)));
  else servo_angle[1] += acos((sqrt(pow(x, 2) + pow(y, 2)) - end) / sqrt(pow(sqrt(pow(x, 2) + pow(y, 2)) - end, 2) + pow(z, 2)));
  servo_angle[2] = acos((pow(sqrt(pow(x, 2) + pow(y, 2)) - end, 2) + pow(z, 2) - pow(L1, 2) - pow(L2, 2)) / (2.0 * L1 * L2)) - servo_angle[1];//小臂
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.println(z);
  // Serial.print(",");


  servo_angle[1] = angle_2_servo_actual(servo_angle[1] * 180 / PI);
  servo_angle[2] = angle_2_servo_actual(90 - servo_angle[2] * 180 / PI);

  // Serial.print(servo_angle[0]);
  // Serial.print(",");
  // Serial.print(servo_angle[1]);
  // Serial.print(",");
  // Serial.println(servo_angle[2]);
}

//====================================================
//机械臂总控制函数
//====================================================
void Arm_Control(float x, float y, int state, int fix)
{
  if(state == 0)  //取子
  {
    //移动到目标上方
    if(fix == 1)  //取棋盘上被移动的棋子
      ikine(x, y, 60);
    else  //取普通棋子
      ikine(x, y, 10);
    Arm_move(1000);
    delay(300);
    //下降，准备取子
    if(fix == 1)  //取棋盘上被移动的棋子
      ikine(x, y, 0);
    else  //取普通棋子
      ikine(x, y, -8);
    Arm_move(500);
    delay(200);
    //吸取
    pump.write(180);
    delay(500);
    pump.write(0);
    //上升，取子完成 
    ikine(x, y, 60);
    Arm_move(500);
    delay(200);

    //返回初始位置，动作完成
    ikine(0, 50, 0);
    Arm_move(1000);
  }
  else  //落子
  {
    //移动到目标上方
    ikine(x, y, 60);
    Arm_move(1000);
    delay(300);
    //下降，准备落子
    ikine(x, y, 17);
    Arm_move(500);
    delay(200);
    //释放
    valve.write(180);
    delay(500);
    valve.write(0);
    //上升，落子完成 
    ikine(x, y, 60);
    Arm_move(500);
    delay(200);
  }
}

//====================================================
//机械臂移动函数
//====================================================
void Arm_move(int move_time)
{
  Servo_move(7, servo_angle[0], 4 * move_time / 5);
  delay(20);
  Servo_move(23, servo_angle[1], 4 * move_time / 5);
  delay(20);
  Servo_move(22, servo_angle[2], 4 * move_time / 5);
  delay(move_time);
}
//====================================================
//舵机旋转函数
//====================================================
void Servo_move(uint8_t id,uint16_t angle,uint16_t move_time){ //设定舵机移动角度
    char buff[100];
    sprintf(buff,"#%03dP%04dT%04d!",id,angle,move_time);
    Serial3.println(buff);
}

//=========================================================
//                  角度到实际舵机的值的转换函数
//========================================================
uint16_t angle_2_servo_actual(float angle){
  uint16_t result;
  if((angle * (1000/90) + 500) >= 2500) result = 2500;
  else if((angle * (1000/90) + 500) <= 500) result = 500;
  else result = (uint16_t) (angle * (1000/90) + 500);
  return result;
}

void read_uart1()
{
  while(1)
  {
    if(Serial1.available())
    {
      char inChar = (char)Serial1.read();
      if (inChar == 'a')        //结束符
      {
        inputString1[cnt1] = '\0';
        string1Complete = true;
        cnt1 = 0;
      }
      else
        inputString1[cnt1++] = inChar;
    }
    if(string1Complete)
      break;
  }
}

void read_uart4()
{
  while(1)
  {
    if(Serial4.available())
    {
      char inChar = (char)Serial4.read();
      if (inChar == 0x5B)        //结束符
      {
        inputString2[cnt2] = '\0';
        string2Complete = true;
        cnt2 = 0;
      }
      else
        inputString2[cnt2++] = inChar;
    }
    if(string2Complete)
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  //串口1接串口屏，串口2接openmv，串口3控制舵机
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial3.begin(115200);
  Serial4.begin(9600);
  pinMode(15, INPUT);
  pinMode(17, INPUT);
  pinMode(19, INPUT);
  pinMode(Board_pin, OUTPUT);
  pinMode(Chess_pin, OUTPUT);
  pinMode(LED_pin, OUTPUT);
  pinMode(Beep_pin, OUTPUT);
  digitalWrite(Board_pin, LOW);
  digitalWrite(Chess_pin, LOW);
  digitalWrite(LED_pin, HIGH);
  digitalWrite(Beep_pin, HIGH);
  delay(100);
  pump.attach(pick_pin);
  valve.attach(drop_pin);
  pump.write(0);
  valve.write(0);
  delay(3000);

  ikine(0, 80, 30);
  Arm_move(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  // if(string2Complete)
  read_uart1();
  if(string1Complete)
  {
    string1Complete = false;
    mode_select();
    // board_locate();
    // Scan_test();
  }

}

//串口1中断事件
// void serialEvent1() 
// {
//   while (Serial1.available()) 
//   {
//     char inChar = (char)Serial1.read();
//     Serial.print(inChar);
//     if (inChar == 'a')        //结束符
//     {
//       inputString1[cnt1] = '\0';
//       string1Complete = true;
//       cnt1 = 0;
//     }
//     else
//       inputString1[cnt1++] = inChar;
    
//   }  
// }
 
//串口0中断事件
// void serialEvent()
// {
//   while (Serial.available()) 
//   {
//     char inChar = (char)Serial.read();
//     // Serial.print(inChar);
//     if (inChar == 0x5B)        //结束符
//     {
//       inputString2[cnt2] = '\0';
//       string2Complete = true;
//       cnt2 = 0;
//     }
//     else
//       inputString2[cnt2++] = inChar;
    
//   }  
// }
