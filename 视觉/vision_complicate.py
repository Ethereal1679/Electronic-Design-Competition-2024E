import sensor, image, time, pyb, random, math
from pyb import UART, Pin, SPI
import json
import ustruct

# 初始化摄像头
sensor.reset()
sensor.set_pixformat(sensor.RGB565)  # 使用彩色图像
sensor.set_framesize(sensor.QVGA)    # 320x240
sensor.skip_frames(time = 2000)
fbuf = bytearray(320)#建立帧缓冲区，对于每个RGB565像素，帧缓冲区都需2字节
clock = time.clock()

cs  = Pin("P3", Pin.OUT_OD)
rst = Pin("P7", Pin.OUT_PP)
rs  = Pin("P8", Pin.OUT_PP)

# 定义横屏/竖屏
USE_HORIZONTAL  = True         # 定义横屏
IMAGE_INVER     = True         # 旋转180°

# TFT resolution 240*320
X_MIN_PIXEL = 0
Y_MIN_PIXEL = 0
if USE_HORIZONTAL:
    X_MAX_PIXEL = 320               # 定义屏幕宽度
    Y_MAX_PIXEL = 240               # 定义屏幕高度
else:
    X_MAX_PIXEL = 240               # 定义屏幕宽度
    Y_MAX_PIXEL = 320               # 定义屏幕高度

# 常用颜色表 						#可去除
RED     = 0XF800
GREEN   = 0X07E0
BLUE    = 0X001F
BLACK   = 0X0000
YELLOW  = 0XFFE0
WHITE   = 0XFFFF

CYAN    = 0X07FF
BRIGHT_RED = 0XF810
GRAY1   = 0X8410
GRAY2   = 0X4208

# OpenMV  SPI2 总线  8位数据模式
spi = SPI(2, SPI.MASTER, baudrate=int(10000000000/66*0.06), polarity=0, phase=0,bits=8)

# SPI 写命令
def write_command_byte(c):
    cs.low()
    rs.low()
    spi.send(c)
    cs.high()

# SPI 写数据
def write_data_byte(c):
    cs.low()
    rs.high()
    spi.send(c)
    cs.high()

def write_command(c, *data): #命令数据一起写，先写命令 第二个开始为数据位。如果只写一个，则代表不写数据只写命令。
    write_command_byte(c)
    if data:
        for d in data:
            if d > 255:
                write_data_byte(d >> 8)
                write_data_byte(d&0xFF)
            else:
                write_data_byte(d)

def write_image(img):
    cs.low()
    rs.high()
    #spi.send(img)
#修改代码的核心##########################################################################
    if(True): #全发
        for m in img: 					#把一帧图像的对象取出来，放到帧缓存区中
            fbuf=m
            for i in range(0,320):		#每行每行的发送
                spi.send(fbuf[i]>>8)	#先发第N行的第I个数据的高八位
                spi.send(fbuf[i]&0xFF)	#再发低八位
            #print(fbuf)
    else:#只发一行固定的数据
        for i in range(0,320):
            spi.send(fbuf[i])
            spi.send(fbuf[i+1]&0xFF)
##########################################################################################
    cs.high()

def SetXY(xpos, ypos):
    write_command(0x2A, xpos>>8, xpos&0XFF)
    write_command(0x2B, ypos>>8, ypos&0XFF)
    write_command(0x2C)

def SetRegion(xStar, yStar, xEnd, yEnd):
    write_command(0x2A, xStar>>8, xStar&0XFF, xEnd>>8, xEnd&0XFF)
    write_command(0x2B, yStar>>8, yStar&0XFF, yEnd>>8, yEnd&0XFF)
    write_command(0x2C)

# 在指定位置绘制一个点
def DrawPoint(x, y, Color):
    SetXY(x, y)
    write_data_byte(Color >> 8)
    write_data_byte(Color&0XFF)

def ReadPoint(x, y):
    data = 0
    SetXY(x, y)
    write_data_byte(data)
    return data

def Clear(Color):
    global X_MAX_PIXEL, Y_MAX_PIXEL
    SetRegion(0, 0, X_MAX_PIXEL-1 , Y_MAX_PIXEL-1 )
    #cs.low()
    #rs.high()
    for i in range(0, Y_MAX_PIXEL):
        for m in range(0, X_MAX_PIXEL):
            write_data_byte(Color >> 8)
            write_data_byte(Color&0xFF)

def LCDinit():
    rst.low()
    time.sleep_ms(100)
    rst.high()
    time.sleep_ms(100)

    write_command(0xCB, 0x39, 0x2c, 0x00, 0x34, 0x02)
    write_command(0xCF, 0x00, 0XAA, 0XE0)
    write_command(0xE8, 0x85, 0x11, 0x78)
    write_command(0xEA, 0x00, 0x00)
    write_command(0xED, 0x67, 0x03, 0X12, 0X81)
    write_command(0xF7, 0x20)
    write_command(0xC0, 0x21)       # Power control, VRH[5:0]
    write_command(0xC1, 0x11)       # Power control, SAP[2:0];BT[3:0]
    write_command(0xC5, 0x24, 0x3C) # VCM control, 对比度调节
    write_command(0xC7, 0xB7)       # VCM control2, --

    # Memory Data Access Control
    if USE_HORIZONTAL:      # //C8   //48 68竖屏//28 E8 横屏
        if IMAGE_INVER:
            write_command(0x36, 0xE8)   # 从右到左 e8/68
        else:
            write_command(0x36, 0x28)   # 从左到右 28
    else:
        if IMAGE_INVER:
            write_command(0x36, 0xC8)   # 从下到上刷
        else:
            write_command(0x36, 0x48)   # 从上到下刷

    global X_MAX_PIXEL, Y_MAX_PIXEL
    SetRegion(0, 0, X_MAX_PIXEL - 1, Y_MAX_PIXEL - 1)

    # Interface Pixel Format
    write_command(0x3A, 0x55)

    write_command(0xB1, 0x00, 0x18)
    write_command(0xB6, 0x08, 0x82, 0x27)   # Display Function Control
    write_command(0xF2, 0x00)               # 3Gamma Function Disable
    write_command(0x26, 0x01)               # Gamma curve selected
    write_command(0xB7, 0x06)

    # set gamma
    write_command(0xE0, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00)
    write_command(0XE1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F)

    write_command(0x11) # sleep_ms Exit
    time.sleep_ms(120)

    # Display On
    write_command(0x29)
    write_command(0x2C)
#    bl.high()   # 拉背光

def display(img):
    write_command(0x2C)
    write_image(img)



p0 = pyb.Pin("P9", pyb.Pin.IN, pyb.Pin.PULL_DOWN)
p1 = pyb.Pin("P6", pyb.Pin.IN, pyb.Pin.PULL_DOWN)

# 定义图像窗口的尺寸
frame_width = 320
frame_height = 240

uart = UART(3,9600)   #定义串口3变量
uart.init(9600, bits=8, parity=None, stop=1) # init with given parameters
time.sleep(3)

point_temp = [(0,0)] * 9
points = [(0,0)] * 9
chess = [0] * 9
count = 0

kernel_size = 1 # 3x3==1, 5x5==2, 7x7==3, etc.
kernel = [-2, -1,  0, \
          -1,  1,  1, \
           0,  1,  2]

board_threshold = (26, 61, 16, 57, 7, 38)
black_threshold = (3, 35, -15, 9, -3, 8)
white_threshold = (70, 92, -7, 26, -17, 17)


#坐标排序函数
def sort_points(points):
    # 按y坐标排序
    points = sorted(points, key=lambda p: p[1])
    # 将排序后的点分成三行，每行三个点
    rows = [points[i:i+3] for i in range(0, len(points), 3)]
    # 对每一行按x坐标排序
    sorted_points = [sorted(row, key=lambda p: p[0]) for row in rows]
    # 将排序后的点合并成一个列表
    sorted_points = [point for row in sorted_points for point in row]
    return sorted_points


#串口发送数据
def sending_center():
    global uart
    global points
    data = ustruct.pack("<hhhhhhhhhhhhhhhhhhb",#b一个字节，h是一个整型
                        points[0][0],#数据大小不能超过一个字节也就是255
                        points[0][1],
                        points[1][0],
                        points[1][1],
                        points[2][0],
                        points[2][1],
                        points[3][0],
                        points[3][1],
                        points[4][0],
                        points[4][1],
                        points[5][0],
                        points[5][1],
                        points[6][0],
                        points[6][1],
                        points[7][0],
                        points[7][1],
                        points[8][0],
                        points[8][1],
                        0x5B
                        )
    uart.write(data)


def sending_chess():
    global uart
    global chess
    data = ustruct.pack("<bbbbbbbbbb",
                        chess[0],
                        chess[1],
                        chess[2],
                        chess[3],
                        chess[4],
                        chess[5],
                        chess[6],
                        chess[7],
                        chess[8],
                        0x5B
                        )
    uart.write(data)
    time.sleep(3)



def get_chessboard():
    global point_temp
    global points
    while True:
        clock.tick()
        img = sensor.snapshot().lens_corr(1.7).histeq(adaptive=True, clip_limit=2)
        img.laplacian(1, sharpen=True)
    #    img.gaussian(1, unsharp=True)
        lcd.display(img)
        count = 0
        for blob in img.find_blobs([board_threshold], roi = (65, 65, 165, 165), pixels_threshold = 800, area_threshold=800):
            # These values depend on the blob not being circular - otherwise they will be shaky.
            if blob.elongation() > 0.5:
                img.draw_edges(blob.min_corners(), color=(255, 0, 0))
                img.draw_line(blob.major_axis_line(), color=(0, 255, 0))
                img.draw_line(blob.minor_axis_line(), color=(0, 0, 255))
            # These values are stable all the time.
            img.draw_rectangle(blob.rect())
            img.draw_cross(blob.cx(), blob.cy())
            # Note - the blob rotation is unique to 0-180 only.
            img.draw_keypoints(
                [(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=20
            )

            point_temp[count] = (blob.cx(), blob.cy())
            count = count + 1
            print(blob)
        print(clock.fps())

        if count == 9:
            points = sort_points(point_temp)
            break
        else:
            point_temp = [(0,0)] * 9
            continue

    print(points)
    display(img) # Take a picture and display the image.
    sending_center()


def get_chess():
    global points
    global chess
    count = 0
    while True:
        clock.tick()
        img = sensor.snapshot().lens_corr(1.7)
    #    img.gaussian(1, unsharp=True)
        lcd.display(img)
        for i in range(9):
            _blobs = img.find_blobs([black_threshold, white_threshold], roi = (points[i][0]-23, points[i][1]-23, 46, 46))
            if _blobs == [] and chess[i] == 0:
                chess[i] = 0

            for blob in _blobs:
                # 滤除棋盘边框
                if blob.w() > 40 or blob.h() > 40:
                    continue
                if blob.w() < 20 or blob.h() < 20:
                    continue

                # 验证色块是否为圆形
                aspect_ratio = blob.w() / blob.h()
                if aspect_ratio < 0.6 or aspect_ratio > 1.4:  # 假设棋子的宽高比接近1
                    continue
                if blob.code() == 1: # 黑色
                    cir = blob.enclosing_circle()
                    img.draw_circle(cir[0], cir[1], cir[2], color=(0, 255, 0))
                    chess[i] = 1

                    if blob.elongation() > 0.5:
                        img.draw_edges(blob.min_corners(), color=(255, 0, 0))
                        img.draw_line(blob.major_axis_line(), color=(0, 255, 0))
                        img.draw_line(blob.minor_axis_line(), color=(0, 0, 255))
                    # These values are stable all the time.
                    img.draw_rectangle(blob.rect())
                    img.draw_cross(blob.cx(), blob.cy())
                    # Note - the blob rotation is unique to 0-180 only.
                    img.draw_keypoints(
                        [(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=20
                    )

                elif blob.code() == 2: # 白色
                    cir = blob.enclosing_circle()
                    img.draw_circle(cir[0], cir[1], cir[2], color=(0, 0, 255))
                    chess[i] = 2

                    if blob.elongation() > 0.5:
                        img.draw_edges(blob.min_corners(), color=(255, 0, 0))
                        img.draw_line(blob.major_axis_line(), color=(0, 255, 0))
                        img.draw_line(blob.minor_axis_line(), color=(0, 0, 255))
                    # These values are stable all the time.
                    img.draw_rectangle(blob.rect())
                    img.draw_cross(blob.cx(), blob.cy())
                    # Note - the blob rotation is unique to 0-180 only.
                    img.draw_keypoints(
                        [(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=20
                    )
        count = count + 1
        if count == 20:
            print(chess)
            sending_chess()
            chess = [0] * 9
            count = 0
            break
#        print(chess)
#        chess = [0] * 9
    display(img) # Take a picture and display the image.
#    sending_center()


LCDinit() # Initialize the lcd screen.
Clear(BLUE)
display(sensor.snapshot().lens_corr(1.7))
while True:
    clock.tick()
    img=sensor.snapshot().lens_corr(1.7)  #Take a picture
    print(clock.fps())
    if p0.value() == 1:
        get_chessboard()

    if p1.value() == 1:
        get_chess()

#get_chessboard()
#get_chess()
