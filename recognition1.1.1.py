# Edge Impulse - OpenMV Image Classification Example
import sensor, image, time, os, tf, uos, gc

from machine import I2C  #VL53L1X所用
from vl53l1x import VL53L1X
from pyb import UART
import json   #字符格式

Car_threshold=(0, 68, 28, -71, 31, 127)#汽车特征值
Motorcycle_threshold=(58, 100, 33, 126, -128, 127)#摩托车特征值

sensor.reset()                         # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565)    # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)      # Set frame size to QVGA (320x240)
sensor.set_windowing((320, 240))       # Set 320x240 window.
sensor.skip_frames(time=2000)          # Let the camera adjust.

net = "trained.tflite"
labels = [line.rstrip('\n') for line in open("labels.txt")]

i2c = I2C(2)
distance = VL53L1X(i2c)
carlocate =1 #0左 1中 2右
motorcyclelocate=1 #0左 1中 2右

a=0 #用来输出物体类别 0没物体 1街道 2摩托i车 3 汽车
dis=0#测距的
###########################神经网络##########################################
net = None
labels = None

try:    #用来加载神经网络文件的
    # load the model, alloc the model file on the heap if we have at least 64K free after loading
    net = tf.load("trained.tflite", load_to_fb=uos.stat('trained.tflite')[6] > (gc.mem_free() - (64*1024)))
except Exception as e:  #如果没有发现神经网络文件，就会显示无法显示
    print(e)
    raise Exception('Failed to load "trained.tflite", did you copy the .tflite and labels.txt file onto the mass-storage device? (' + str(e) + ')')

try:        #用来加载训练后的文本文件
    labels = [line.rstrip('\n') for line in open("labels.txt")]
except Exception as e:     #同理
    raise Exception('Failed to load "labels.txt", did you copy the .tflite and labels.txt file onto the mass-storage device? (' + str(e) + ')')


uart = UART(1, 115200)    #串口的波特率
clock = time.clock()     #打开时钟
while(True):
    clock.tick()
    data=[]   #存放发送的数据

    img = sensor.snapshot()     #获得图像


#############################神经网络部分##########################################################
    for obj in net.classify(img, min_scale=1.0, scale_mul=0.8, x_overlap=0.5, y_overlap=0.5):

        predictions_list = list(zip(labels, obj.output()))

        for i in range(len(predictions_list)):
            print("%s = %f" % (predictions_list[i][0], predictions_list[i][1]))
            maxp=predictions_list[0][1]
            i=0
            j=0
            for i in range(3):
                 if predictions_list[i][1]>maxp:
                   j=i   #获得最大值的位置
                   maxp=predictions_list[i][1]   #获取最大值的相似度
            if maxp <0.5:
                a=0    #相似度过小，则会显示安全
            else:
             maxp=0
             if predictions_list[j][0] =='road':
                a=1
             elif predictions_list[j][0] =='motorcycle':
                a=2
             elif predictions_list[j][0] =='car':
                a=3



#################################图片获取部分###############################################################
        dis=distance.read()    #获得距离
        blobs1 = img.find_blobs([Car_threshold])   #寻找汽车或摩托车特征
        blobs2 = img.find_blobs([Motorcycle_threshold])
        print("distance:%d ,carlocate:%d ,motorcyclelocate:%d,kind:%d \r\n"%(dis,carlocate,motorcyclelocate,a))
        data.append((dis,carlocate,motorcyclelocate,a))    #将距离，汽车位置，摩托车位置和相似度打包
        data_out = json.dumps(set(data))      #以json格式转换
        uart.write(data_out +'\n')    #发送json数据格式
        time.sleep(50)
        print(data)
        #uart.write("distance:%d ,carlocate:%d ,motorcyclelocate:%d,kind:%d \r\n"%(dis,carlocate,motorcyclelocate,a))
        if len(blobs1) > 0:
           c = blobs1[0]   #将图片分成三块，每106一块，然后根据所在位置，完成判断和圈出物体
           if c[5]>0 and c[5]<106:   #左边
             #img.draw_rectangle(c[0:4], color = (255, 0, 0))
             img.draw_rectangle(c.x(),c.y(),80,60,color = (255, 0, 0))
             img.draw_cross(c[5], c[6],color = (255, 0, 0))
             print("car in left")
             carlocate =0 #0左 1中 2右
           if c[5]>106 and c[5]<213:  #中间
             #img.draw_rectangle(c[0:4], color = (255, 0, 0))
             img.draw_rectangle(c.x(),c.y(),80,60,color = (255, 0, 0))
             img.draw_cross(c[5], c[6],color = (255, 0, 0))
             print("car behind")
             carlocate =1 #0左 1中 2右
           if c[5]>216 and c[5]<320:  #右边
             #img.draw_rectangle(c[0:4], color = (255, 0, 0))
             img.draw_rectangle(c.x(),c.y(),80,60,color = (255, 0, 0))
             img.draw_cross(c[5], c[6],color = (255, 0, 0))
             print("car in right")
             carlocate =2 #0左 1中 2右

        if len(blobs2) > 0:
           b = blobs2[0]
           if b[5]>0 and b[5]<106:
             #img.draw_rectangle(b[0:4], color = (0, 255, 0))
             img.draw_rectangle(b.x(),b.y(),70,50,color = (0, 255, 0))
             img.draw_cross(b[5], b[6],color = (0, 255, 0))
             print("Motorcycle in left")
             motorcyclelocate=0 #0左 1中 2右
           if b[5]>106 and b[5]<213:
             #img.draw_rectangle(b[0:4], color = (0, 255, 0))
             img.draw_rectangle(b.x(),b.y(),70,50,color = (0, 255, 0))
             img.draw_cross(b[5], b[6],color = (0, 255, 0))
             print("Motorcycle in behind")
             motorcyclelocate=1 #0左 1中 2右
           if b[5]>216 and b[5]<320:
             #img.draw_rectangle(b[0:4], color = (0, 255, 0))
             img.draw_rectangle(b.x(),b.y(),70,50,color = (0, 255, 0))
             img.draw_cross(b[5], b[6],color = (0, 255, 0))
             print("Motorcycle in right")
             motorcyclelocate=2 #0左 1中 2右

        #print(clock.fps(), "fps")
