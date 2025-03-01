void Scan_test()
{
  int i=0, j=0;
  while(1)
  {
    for(i = 0; i < 3; i++)
    {
      for(j = 0; j < 3; j++)
      {
        Serial.println(3*i+j+1);
        //移到坐标上方
        ikine(board_arm[i][j][0], board_arm[i][j][1], 30);
        Arm_move(1000);
        //下降，准备取子/落子
        ikine(board_arm[i][j][0], board_arm[i][j][1], 0);
        Arm_move(500);
        //取子/落子（暂不确定真空泵如何控制，先用延时代替）
        delay(500);
        //取子/落子完成，升起
        ikine(board_arm[i][j][0], board_arm[i][j][1], 30);
        Arm_move(500);
        //返回初始位置，动作完成
        ikine(0, 50, 0);
        Arm_move(1000);
        delay(1000);
      }
    }

    // for(int i=0; i<5; i++)
    // {
    //   ikine(black[i][0], black[i][1], 20);
    //   Arm_move(1000);
    //   //下降，准备取子/落子
    //   ikine(black[i][0], black[i][1], -15);
    //   Arm_move(500);
    //   //取子/落子（暂不确定真空泵如何控制，先用延时代替）
    //   delay(500);
    //   //取子/落子完成，升起
    //   ikine(black[i][0], black[i][1], 20);
    //   Arm_move(500);
    //   //返回初始位置，动作完成
    //   ikine(0, 50, 0);
    //   Arm_move(1000);
    //   delay(1000);
    // }

    // for(int i=0; i<5; i++)
    // {
    //   ikine(white[i][0], white[i][1], 20);
    //   Arm_move(1000);
    //   //下降，准备取子/落子
    //   ikine(white[i][0], white[i][1], -15);
    //   Arm_move(500);
    //   //取子/落子（暂不确定真空泵如何控制，先用延时代替）
    //   delay(500);
    //   //取子/落子完成，升起
    //   ikine(white[i][0], white[i][1], 20);
    //   Arm_move(500);
    //   //返回初始位置，动作完成
    //   ikine(0, 50, 0);
    //   Arm_move(1000);
    //   delay(1000);
    // }

  }
}

void function1()
{
  board_locate();
  while(1)
  {
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      return;
    }
    chess_select();
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      return;
    }
    while(strcmp(inputString1, "q1_end") != 0)
    {
      string1Complete = false;
      read_uart1();
      if(strcmp(inputString1, "exit") == 0)
      {
        string1Complete = false;
        return;
      }
    }
    Arm_Control(chess_x, chess_y, 0, 0);  //取子
    Arm_Control(board_arm[1][1][0], board_arm[1][1][1], 1, 0);  //落子
    //返回初始位置，动作完成
    ikine(0, 50, 0);
    Arm_move(1000);
    // inputString2 = "";
    string1Complete = false;
    digitalWrite(LED_pin, LOW);
    digitalWrite(Beep_pin, LOW);
    delay(1000);
    digitalWrite(LED_pin, HIGH);
    digitalWrite(Beep_pin, HIGH);
  }
}

void function2_3()
{
  board_locate();
  while(1)
  {
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      return;
    }
    chess_select();
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      return;
    }
    board_select();
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      return;
    }
    while(strcmp(inputString1, "q2_end") != 0 && strcmp(inputString1, "q3_end") != 0)
    {
      Serial.println(inputString1);
      string1Complete = false;
      read_uart1();
      if(strcmp(inputString1, "exit") == 0)
      {
        string1Complete = false;
        return;
      }
      Serial.println("error");
    }
    Serial.println("check");
    Arm_Control(chess_x, chess_y, 0, 0);  //取子
    Arm_Control(mark_x, mark_y, 1, 0);  //落子
    //返回初始位置，动作完成
    ikine(0, 50, 0);
    Arm_move(1000);
    string1Complete = false;
    Serial.println("complete");
    digitalWrite(LED_pin, LOW);
    digitalWrite(Beep_pin, LOW);
    delay(1000);
    digitalWrite(LED_pin, HIGH);
    digitalWrite(Beep_pin, HIGH);
  }
}

//color 0为黑，1为白
void start_action(bool color, int num, int blc)
{
  if(!color)
    Arm_Control(black[num][0], black[num][1], 0, 0);  //取黑子
  else
    Arm_Control(white[num][0], white[num][1], 0, 0);  //取白子

  Arm_Control(board_arm[blc / 3][blc % 3][0], board_arm[blc / 3][blc % 3][1], 1, 0);  //落子
  //返回初始位置，动作完成
  ikine(0, 50, 0);
  Arm_move(1000);

  Serial1.print("sys0=1\xff\xff\xff");

  //蜂鸣器响，灯亮，1.5s
  digitalWrite(LED_pin, LOW);
  digitalWrite(Beep_pin, LOW);
  delay(1000);
  digitalWrite(LED_pin, HIGH);
  digitalWrite(Beep_pin, HIGH);
}

void correct_action()
{
  Arm_Control(board_arm[mark_y][mark_x][0], board_arm[mark_y][mark_x][1], 0, 1);  //取“新”棋子
  Arm_Control(board_arm[lost_y][lost_x][0], board_arm[lost_y][lost_x][1], 1, 1);  //放回原位

  //返回初始位置，动作完成
  ikine(0, 50, 0);
  Arm_move(1000);

  Serial1.print("sys0=1\xff\xff\xff");

  //蜂鸣器响，灯亮，1.5s
  digitalWrite(LED_pin, LOW);
  digitalWrite(Beep_pin, LOW);
  delay(1000);
  digitalWrite(LED_pin, HIGH);
  digitalWrite(Beep_pin, HIGH);
}

int winner_check()
{
  for(int i=0; i<3; i++)
  {
    if(status[i][0] == status[i][1] && status[i][1] == status[i][2] && status[i][0] != 0)
      return status[i][0];
    if(status[0][i] == status[1][i] && status[1][i] == status[2][i] && status[0][i] != 0)
      return status[0][i];
  }
  if(status[0][0] == status[1][1] && status[1][1] == status[2][2] && status[1][1] != 0)
    return status[1][1];
  if(status[0][2] == status[1][1] && status[1][1] == status[2][0] && status[1][1] != 0)
    return status[1][1];
  return 0;
}

void ai_round(int color, int num)
{
  Serial.println("check");
  int max_x = 0, max_y = 0;
  int value[3][3];
  for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
    {
      // if(color == 1)
      //   value[i][j] = -10000;
      // else
      //   value[i][j] = 10000;
      value[i][j] = -10000;
    }

  for(int i=0; i<3; i++)
    for(int j=0; j<3; j++)
    {
      if(status[i][j] == 0)
      {
        status[i][j] = color;
        // if(color == 1)  //己方执黑棋
        //   value[i][j] = MiniMax(color, 0, false);
        // else  //己方执白棋
        //   value[i][j] = MiniMax(color, 0, true);
        value[i][j] = MiniMax(color, 0, false);
        status[i][j] = 0;
        // if((color == 1 && value[max_y][max_x] < value[i][j]) || (color == 2 && value[max_y][max_x] > value[i][j]))  //寻找最优点
        if(value[max_y][max_x] < value[i][j])
          max_x = j, max_y = i;
      }
    }
  Serial.println(3*max_y + max_x);
  Serial.println(value[max_y][max_x]);
  start_action(color-1, num, 3*max_y + max_x);
  status[max_y][max_x] = color;
}

int MiniMax(int color, int depth, bool isMax)
{
  int score, full = 1;

  //己方连成一线
  if(color == winner_check())
    return 100 - depth;
  //对方连成一线
  else if(winner_check() != 0)
    return -100 + depth;
  //检测棋盘状况
  else
  {
    for(int i=0; i<3; i++)
    {
      for(int j=0; j<3; j++)
        if(status[i][j] == 0)
        {
          full = 0;
          break;
        }
      if(full == 0) break;
    }
  }
  //棋盘已满，平局
  if(full == 1)
    return 0;
  //到达搜索最大深度，判断棋盘局势
  if(depth == 4)
  {
    int mine=0, opponent=0, empty=0, scores=0;
    //横排
    for(int i=0; i<3; i++)
    { 
      mine=0, opponent=0, empty=0;
      for(int j=0; j<3; j++)
      {
        if(status[i][j] == color) mine++;
        else if(status[i][j] == 0) empty++;
        else opponent++;
      }
      //空行数，双连子
      if(mine == 1 && empty == 2) scores++;
      if(mine == 2 && empty == 1) scores += 10;
      if(opponent == 1 && empty == 2) scores--;
      if(opponent == 2 && empty == 1) scores -= 10;
    }

    //竖列
    for(int i=0; i<3; i++)
    { 
      mine=0, opponent=0, empty=0;
      for(int j=0; j<3; j++)
      {
        if(status[j][i] == color) mine++;
        else if(status[j][i] == 0) empty++;
        else opponent++;
      }
      //空行数，双连子
      if(mine == 1 && empty == 2) scores++;
      if(mine == 2 && empty == 1) scores += 10;
      if(opponent == 1 && empty == 2) scores--;
      if(opponent == 2 && empty == 1) scores -= 10;
    }

    //斜线
    for(int i=0; i<3; i++)
    { 
      mine=0, opponent=0, empty=0;
      if(status[i][i] == color) mine++;
      else if(status[i][i] == 0) empty++;
      else opponent++;
      //空行数，双连子
      if(mine == 1 && empty == 2) scores++;
      if(mine == 2 && empty == 1) scores += 10;
      if(opponent == 1 && empty == 2) scores--;
      if(opponent == 2 && empty == 1) scores -= 10;
    }
    for(int i=0; i<3; i++)
    { 
      mine=0, opponent=0, empty=0;
      if(status[i][2-i] == color) mine++;
      else if(status[i][2-i] == 0) empty++;
      else opponent++;
      //空行数，双连子
      if(mine == 1 && empty == 2) scores++;
      if(mine == 2 && empty == 1) scores += 10;
      if(opponent == 1 && empty == 2) scores--;
      if(opponent == 2 && empty == 1) scores -= 10;
    }
    return scores;
  }
  //未到结束判断条件，继续分析
  if(isMax)
  {
    int best_score = -10000;
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        if(status[i][j] == 0)
        {
          // status[i][j] = 1;
          status[i][j] = color;
          score = MiniMax(color, depth + 1, false);
          status[i][j] = 0;
          best_score = max(score, best_score);
        }
    return best_score;
  }

  else
  {
    int best_score = 10000;
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        if(status[i][j] == 0)
        {
          // status[i][j] = 2;
          status[i][j] = ((color == 1)? 2: 1);
          score = MiniMax(color, depth + 1, true);
          status[i][j] = 0;
          best_score = min(score, best_score);
        }
    return best_score;
  }
}

void function4()
{
  board_locate();
  read_uart1();
  if(strcmp(inputString1, "exit") == 0)
  {
    string1Complete = false;
    return;
  }
  board_select();
  read_uart1();
  if(strcmp(inputString1, "exit") == 0)
  {
    string1Complete = false;
    return;
  }
  while(strcmp(inputString1, "q4_end") != 0)
  {
    string1Complete = false;
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      return;
    }
  }
  string1Complete = false;
  Serial1.print("sys0=0\xff\xff\xff");
  start_action(0, 0, block);
  status_last[block / 3][block % 3] = 1;
  for(int i=1; i<5; i++)
  {
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      for(int j=0; i<3; i++)
        for(int k=0; k<3; k++)
          status[j][k] = status_last[j][k] = 0;
      return;
    }
    while(strcmp(inputString1, "you4") != 0)
    {
      string1Complete = false;
      Serial.println(inputString1);
      read_uart1();
      if(strcmp(inputString1, "exit") == 0)
      {
        string1Complete = false;
        for(int j=0; i<3; i++)
          for(int k=0; k<3; k++)
            status[j][k] = status_last[j][k] = 0;
        return;
      }
    }
    string1Complete = false;
    Serial1.print("sys0=0\xff\xff\xff");
    digitalWrite(Chess_pin, HIGH);
    delay(50);
    digitalWrite(Chess_pin, LOW);
    read_uart4();
    string2Complete = false;
    get_chess_location();

    if(flag == 3)  //对手动棋盘上的棋子了
    {
      Serial.println("change");
      correct_action();
      status[mark_y][mark_x] = status_last[mark_y][mark_x];
      status[lost_y][lost_x] = status_last[lost_y][lost_x];
      flag = 0;
      i--;
      continue;
    }

    flag = winner_check();
    if(flag == 2)  //白棋胜，对手赢
    {
      Serial1.print("sys1=1\xff\xff\xff");
      break;
    }
    else if(flag == 1)   //黑棋胜，我方赢
    {
      Serial1.print("sys1=3\xff\xff\xff");
      break;
    }

    ai_round(1, i);

    //蜂鸣器/亮灯提示


    flag = winner_check();
    if(flag == 2)  //白棋胜，对手赢
    {
      Serial1.print("sys1=1\xff\xff\xff");
      break;
    }
    else if(flag == 1)   //黑棋胜，我方赢
    {
      Serial1.print("sys1=3\xff\xff\xff");
      break;
    }

    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        status_last[i][j] = status[i][j];
  }

  if(flag == 0)  //平局
    Serial1.print("sys1=2\xff\xff\xff");

  for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        status_last[i][j] = status[i][j] = 0;
}

void function5()
{
  board_locate();
  for(int i=0; i<5; i++)
  {
    read_uart1();
    if(strcmp(inputString1, "exit") == 0)
    {
      string1Complete = false;
      for(int j=0; i<3; i++)
        for(int k=0; k<3; k++)
          status[j][k] = status_last[j][k] = 0;
      return;
    }
    while(strcmp(inputString1, "you5") != 0)
    {
      string1Complete = false;
      read_uart1();
      if(strcmp(inputString1, "exit") == 0)
      {
        string1Complete = false;
        for(int j=0; i<3; i++)
          for(int k=0; k<3; k++)
            status[j][k] = status_last[j][k] = 0;
        return;
      }
    }
    Serial1.print("sys0=0\xff\xff\xff");
    string1Complete = false;
    digitalWrite(Chess_pin, HIGH);
    delay(50);
    digitalWrite(Chess_pin, LOW);
    read_uart4();
    string2Complete = false;
    get_chess_location();

    if(flag == 3)  //对手动棋盘上的棋子了
    {
      correct_action();
      status[mark_y][mark_x] = status_last[mark_y][mark_x];
      status[lost_y][lost_x] = status_last[lost_y][lost_x];
      flag = 0;
      i--;
      continue;
    }

    flag = winner_check();
    if(flag == 1)  //黑棋胜，对手赢
    {
      Serial1.print("sys1=1\xff\xff\xff");
      break;
    }
    else if(flag == 2)   //白棋胜，我方赢
    {
      Serial1.print("sys1=3\xff\xff\xff");
      break;
    }
    else if(flag == 0 && i == 4)  //平局
      break;
      
    ai_round(2, i);

    //蜂鸣器/亮灯提示


    flag = winner_check();
    if(flag == 1)  //黑棋胜，对手赢
    {
      Serial1.print("sys1=1\xff\xff\xff");
      break;
    }
    else if(flag == 2)   //白棋胜，我方赢
    {
      Serial1.print("sys1=3\xff\xff\xff");
      break;
    }
    
    for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        status_last[i][j] = status[i][j];
  }

  if(flag == 0)  //平局
    Serial1.print("sys1=2\xff\xff\xff");

  for(int i=0; i<3; i++)
      for(int j=0; j<3; j++)
        status_last[i][j] = status[i][j] = 0;
}
