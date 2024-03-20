# **Blackjack Machine**
---  

<p align="center">
<img src="https://raw.githubusercontent.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/main/info/images/full_machine.png" alt="Blackjack Machine" width="500"/>
</p>

## ที่มาและความสำคัญ  

การเล่นเกม Blackjack นั้นจำเป็นที่จะต้องมีดีลเลอร์(Dealer) ที่เป็นมนุษย์คอยทำการแจกไพ่และทำการคืนเงินที่ผู้เล่นได้ทำการลงพนันไป ทางกลุ่มผู้จัดจึงมีความสนใจในการทำขั้นตอนการเล่นเกม Blackjack โดยที่ไม่จำเป็นต้องมีดีลเลอร์ที่เป็นมนุษย์คอยดำเนินเกมและทำการคืนเงิน โดยให้เกม Blackjack นั้นสามารถเล่นได้โดยมีแค่ผู้เล่นฺ

## รายละเอียดโครงการ

Blackjack Machine เป็นเครื่องเล่นเกม Blackjack ประกอบไปด้วย ดีลเลอร์(Dealer) ที่ทำหน้าที่ดำเนินเกม แจกไพ่ และประกาศผลผู้ชนะไปยังผู้เล่น(Player) ทั้งสองคน โดยผู้เล่นแต่ละคนสามารถหยอดเหรียญเพื่อทำการพนันเงิน หากผู้เล่นชนะดีลเลอร์จะได้รับรางวัลเป็น 2 เท่าจากเงินที่ลงไป เมื่อผู้เล่นต้องการจะเลิกเล่น สามารถกดปุ่มเพื่อถอนเงินที่เหลือออกจากเครื่องได้


## คุณสมบัติ
* หยอดเหรียญเพื่อแลกเป็นเครดิต(Credit) ในการลงพนัน
* จอ OLED แสดงสถานะเกมจากฝั่งดีลเลอร์
* จอ OLED แสดงสถานะและการควบคุมจากฝั่งผู้เล่น
* ผู้เล่นสามารถเลือกจำนวนเครดิตที่จะใช้พนันในแต่ละรอบ
* ผู้เล่นสามารถเลือกฮิต(Hit, ขอจั่วไพ่เพิ่ม) หรือ แสตนด์(Stand, ไม่จั่วไพ่เพิ่มแล้ว)
* การประกาศผลแพ้ชนะ และคำนวณเครดิตรางวัลของผู้เล่น
* แลกเครดิตที่ผู้เล่นมีคืนออกมาเป็นเหรียญ
* จอ OLED แสดงสถานะของเครื่องรับและจ่ายเหรียญ
* ปุ่มฉุกเฉินสำหรับบังคับจ่ายเหรียญคืน

## แนวคิดและหลักการ
Blackjack Machine ประกอบไปด้วยเครื่องที่เป็นดีลเลอร์ 1 เครื่อง ผู้เล่น 2 เครื่อง และเครื่องรับคืนเหรียญสำหรับแต่ละผู้เล่นอีก 2 เครื่อง โดยแต่ละเครื่องจะใช้ NodeMCU ESP32 เป็นหน่วยประมวลผล และใช้โปรโตคอล ESP-NOW ในการติดต่อสื่อการกันระหว่างเครื่องแบบไร้สาย(Wireless) 
<p align="center">
<img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/connection_diagram.jpeg?raw=true" width="500"/>
</p>  

ดีลเลอร์จะเป็นผู้ที่ดำเนินเกม ติดตามสถานะเกม และคอยจัดการให้ผู้เล่นทั้งสองอยู่ใน State ที่ถูกต้องและพร้อมเพรียงกันทั้งสองเครื่อง โดยดีลเลอร์จะมี State ของเกมอยู่ 4 แบบ คือ  
1. Waiting State ช่วงก่อนเริ่มเกม  
2. Betting State ช่วงตัดสินใจลงเครดิตพนันสำหรับผู้เล่น  
3. Playing State ช่วงตัดสินใจเลือกตัวเลือกการเล่นสำหรับผู้เล่น  
4. Result State ช่วงการตัดสินผลแพ้ชนะของผู้เล่น  

และในการติดต่อสื่อสารกันระหว่างเครื่อง จะใช้สัญญาณทั้งหมด 9 แบบ คือ  
1. Reset Signal สัญญาณที่ดีลเลอร์บอกให้ผู้เล่นเริ่มต้นใหม่  
2. State Change Signal สัญญาณที่ดีลเลอร์บอกให้ผู้เล่นเปลี่ยนสถานะ  
3. Start Signal สันญาณที่ผู้เล่นบอกดีลเลอร์ว่าพร้อมแล้ว  
4. Betting Signal สัญญาณที่ผู้เล่นบอกดีลเลอร์ว่าตัดสินใจลงเครดิตพนันแล้ว  
5. Hit Signal สัญญาณที่ผู้เล่นบอกดีลเลอร์ว่าขอจั่วไพ่เพิ่ม  
6. Stand Signal สัญญาณที่ผู้เล่นบอกดีลเลอร์ว่าไม่ขอจั่วไพ่เพิ่มแล้ว  
7. Result Signal สัญญาณที่ดีลเลอร์ประกาศผลตัดสินแพ้ชนะ  
8. Insert Signal สัญญาณที่เครื่องรับเหรียญบอกผู้เล่นว่ามีการหยอดเหรียญ  
9. Withdraw Signal สัญญาณที่ผู้เล่นบอกเครื่องรับเหรียญให้คืนเหรียญคืนตามจำนวนเครดิตที่มี  

ดีลเลอร์มีขั้นตอนการตัดสินใจตาม Flow Diaagram ด้านล่าง  

<p align="center">
<img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/flow_diagram.jpeg?raw=true" width="500"/>
</p>  

### **1. บอร์ดดีลเลอร์**

#### **Source code:** [Dealer](https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/tree/ab85e1bcdacbea44986f29a08d0389e6d3af254e/dealer)  

#### **Library**
  - esp\_now\.h
  - WiFi.h
  - SPI.h
  - Wire.h
  - Adafruit\_GFX.h
  - Adafruit\_SSD1306.h
  - card\_icon.h
  - card\_printer.h

#### **อุปกรณ์ที่ใช้** 
  - NodeMCU ESP32 \[จำนวน 1 ตัว]
  - OLED Screen SSD1306 128x32 \[จำนวน 1 ตัว]
  - Breadboard \[จำนวน 1 ตัว]  

#### **แผนผังวงจรบอร์ดดีลเลอร์**  
<p align="center">
  <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/dealer_board.png?raw=true" width="500"/><br/><br/></p>  

#### **การทำงานของบอร์ดดีลเลอร์**
  1. เมื่อเริ่มระบบ จะทำการลงทะเบียนบอร์ดผู้เล่นผ่านโปรโตคอล ESP-NOW เพื่อเริ่มการส่งข้อมูลระหว่างดีลเลอร์และผู้เล่น
  2. เริ่มต้นโดยทำการส่ง Reset Signal ไปยังบอร์ดผู้เล่นทั้งสอง เพื่อให้ทั้งดีลเลอร์และผู้เล่นอยู่ใน Waiting State และรอ Start Signal จากผู้เล่นทั้งสอง  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/dealer_1.png?raw=true" width="400"/><br/><br/>

  3. เมื่อผู้เล่นทั้งสองส่ง Start Signal มาแล้ว ดีลเลอร์จะขยับเข้าสู่ Betting State แล้วทำการส่งสัญญาณไปยังผู้เล่นให้เปลี่ยนเป็น Betting State แล้วดีลเลอร์จะรอ Betting Signal จากผู้เล่นทั้งสอง  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/dealer_2.png?raw=true" width="400"/><br/><br/>

  4. เมื่อผู้เล่นทั้งสองส่ง Betting Signal มาแล้ว ดีลเลอร์จะขยับเข้าสู่ Playing State โดยใน state นี้จะเริ่มโดยการที่ดีลเลอรสุ่มไพ่ของตัวเอง 1 ใบแล้วแสดงบนจอ OLED และทำการสุ่มไพ่ 2 ใบให้ผู้เล่นแต่ละคน และทำการรอ Hit/Stand Signal จากผู้เล่นทั้งสองคน  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/dealer_3.png?raw=true" width="400"/><br/><br/>

  5. ใน Playing State นี้ ดีลเลอร์จะได้รับ Signal 2 ประเภทจากผู้เล่น คือ  
    **1.Hit Signal** เป็นสัญญาณว่าผู้เล่นต้องการจั่วไพ่เพิ่ม ดีลเลอร์จะทำการสุ่มไพ่เพื่อส่งให้กับผู้เล่น  
    **2.Stand Signal** เป็นสัญญาณว่าผู้เล่นไม่ต้องการจั่วไพ่เพิ่มแล้ว เมื่อดีลเลอร์ได้รับ Stand Signal ครบจากผู้เล่นทั้งสอง ก็จะขยับเข้าสู่ Result State
  6. ใน Result State ดีลเลอร์จะจั่วไพ่จนกว่าจะมีค่ารวมอย่างน้อย 17 จากนั้นจะทำการคำนวณคะแนนของผู้เล่นทั้งสอง แล้วส่ง Result Signal ไปยังผู้เล่นเพื่อประกาศผลแพ้ชนะ หลังจากนั้น 5 วินาที ดีลเลอร์จะขยับไปยัง Waiting State ทำการส่ง Reset Signal เพื่อให้ผู้เล่นทั้งสองเปลี่ยนเป็น Waiting State เพื่อรอเริ่มเกมรอบถัดไป  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/dealer_4.png?raw=true" width="400"/><br/><br/>  

--- 
### **2. บอร์ดผู้เล่น**
#### **Source code:** [Player](https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/tree/ab85e1bcdacbea44986f29a08d0389e6d3af254e/player)  

#### **Library**
  - esp\_now\.h
  - WiFi.h
  - SPI.h
  - Wire.h
  - Adafruit\_GFX.h
  - Adafruit\_SSD1306.h
  - ezButton.h
  - ESP32RotaryEncoder.h
  - card\_icon.h
  - card\_printer.h  

#### **อุปกรณ์ที่ใช้**
  - NodeMCU ESP32 \[จำนวน 1 ตัว]
  - OLED Screen SSD1306 128x32 \[จำนวน 1 ตัว]
  - Rotary Encoder \[จำนวน 1ตัว]
  - Push Button \[จำนวน 2 ตัว]
  - Breadboard \[จำนวน 1 ตัว]  

#### **แผนผังวงจรบอร์ดผู้เล่น**  
  <p align="center">
  <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/player_board.png?raw=true" width="500"/><br/><br/></p>  

#### **การทำงานของบอร์ดผู้เล่น**  
  1. เมื่อเริ่มระบบจะทำการลงทะเบียนบอร์ดดีลเลอร์และบอร์ดรับคืนเหรียญ ผ่านโปรโตคอล ESP-NOW เพื่อเริ่มการส่งข้อมูลระหว่างดีลเลอร์ บอร์ดรับคืนเหรียญ และผู้เล่น
  2. บอร์ดผู้เล่นจะคอยรับสัญญาณจากบอร์ดดีลเลอร์ และทำการเปลี่ยน State ให้ถูกต้องตามสัญญาณที่ได้มา
  3. ใน Waiting State ผู้เล่นจะสามารถกดได้ 2 ปุ่ม คือ  
    **1. ปุ่มบน** เป็นปุ่มที่จะส่ง Withdraw Signal ไปยังบอร์ดรับคืนเหรียญ เพื่อทำการแลกเครดิตทั้งหมดที่ผู้เล่นมีออกมาเป็นเหรียญ  
    **2. ปุ่มล่าง** เป็นปุ่มที่จะส่ง Start Signal ไปยังบอร์ดดีลเลอร์    
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/player_1.png?raw=true" width="400"/><br/><br/>

  4. ใน Betting State ผู้เล่นจะสามารถกดได้ 1 ปุ่ม และใช้ตัวหมุนได้ โดย  
    **1. ตัวหมุน** ผู้เล่นจะใช้ตัวหมุนในการปรับจำนวนเครดิตที่ต้องการใช้ลงพนัน โดยจะแสดงผลบนหน้าจอ OLED  
    **2. ปุ่มล่าง** เมื่อผู้เล่นเลือกจำนวนเครดิตที่จะใช้ลงพนันแล้ว ให้กดปุ่มนี้เพื่อส่ง Betting Signal ไปยังบอร์ดดีลเลอร์  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/player_2.png?raw=true" width="400"/><br/><br/>

  5. ใน Playing State จะแสดงไพ่ที่ตัวเองมีบนจอ OLED เพื่อให้ผู้เล่นตัดสินใจระหว่าง  
    **1.ปุ่มบน** หากผู้เล่นพอใจกับไพ่ที่มีแล้ว ให้กดปุ่มนี้เพื่อทำการส่ง Stand Signal ไปยังบอร์ดดีลเลอร์  
    **2.ปุ่มล่าง** หากผู้เล่นยังไม่พอใจในไพ่ที่มี ให้กดปุ่มนี้เพื่อทำการส่ง Hit Signal ไปยังบอร์ดดีลเลอร์เพื่อขอจั่วไพ่  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/player_3.png?raw=true" width="400"/><br/><br/>

  6. เมื่อบอร์ดผู้เล่นได้รับ Result Signal จากบอร์ดดีลเลอร์ จะทำการประกาศผล โดยเป็นไปได้ 3 รูปแบบคือ  
    **1.ชนะ** หากผู้เล่นชนะดีลเลอร์ ผู้เล่นจะได้รับเครดิตคืนเป็นจำนวนสองเท่าของที่ลงพนันไว้  
    **2.เสมอ** หากผู้เล่นเสมอกับดีลเลอร์ ผู้เล่นจะได้รับเครดิตคืนเป็นจำนวนเท่ากับเครดิตที่ลงพนันไว้  
    **3.แพ้** หากผู้เล่นแพ้ดีลเลอร์ ผู้เล่นจะไม่ได้รับเครดิตที่ลงพนันไว้คืนเลย
---

### **3. บอร์ดรับและคืนเหรียญ**
#### **Source code:** [Creditor](https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/tree/ab85e1bcdacbea44986f29a08d0389e6d3af254e/creditor)
#### **Library**
  - esp\_now\.h
  - WiFi.h
  - SPI.h
  - Wire.h
  - Adafruit\_GFX.h
  - Adafruit\_SSD1306.h
  - ezButton.h
  - ESP32Servo.h
#### **อุปกรณ์ที่ใช้**
  - NodeMCU ESP32 \[จำนวน 1 ตัว]
  - OLED Screen SSD1306 128x32 \[จำนวน 1 ตัว]
  - SG90 Micro Servo \[จำนวน 1ตัว]
  - Push Button   \[จำนวน 1 ตัว]
  - Coin Acceptor \[จำนวน 1 ตัว]
  - 12V AC to DC Adaptor \[จำนวน 1 ตัว]
  - Resistor 10k \[จำนวน 1 ตัว]
  - Breadboard \[จำนวน 1 ตัว]
#### **แผนผังวงจรบอร์ดรับและคืนเหรียญ**  
  <p align="center">
  <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/creditor_board.png?raw=true" width="500"/><br/><br/></p>


#### **การทำงานของบอร์ดรับและคืนเหรียญ**
  1. เมื่อเริ่มระบบจะทำการลงทะเบียนบอร์ดผู้เล่นผ่านโปรโตคอล ESP-NOW เพื่อเริ่มการส่งข้อมูลระหว่างบอร์ดรับและคืนเหรียญกับบอร์ดผู้เล่น
  2. บอร์ดรับคืนเหรียญจะคอยรับสัญญาณจากเครื่องรับเหรียญ(Coin Acceptor) เมื่อผู้เล่นหยอดเหรียญเข้ามา เครื่องรับเหรียญจะส่งสัญญาณไปให้บอร์ดรับคืนเหรียญ และบอร์ดจะทำการเพิ่มจำนวนเหรียญที่มีในเครื่องและเพิ่มเครดิต ให้กับผู้เล่นคนนั้นให้ถูกต้องตามสัญญาณที่ได้มาและแสดงผลบนจอ OLED โดยผู้เล่นสามารถหยอดเหรียญเพื่อเพิ่มเครดิตได้ทั้งในระหว่าง Waiting State และ Betting State

      <p float="left">
      <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/creditor_1.png?raw=true" width="400"/>
      <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/creditor_2.png?raw=true" width="400"/>
      </p>
      <br/>

  3. การถอนเหรียญออกจากเครื่องนั้นมี 2 กรณี คือ  
    **1. กรณีที่ผู้เล่นอยากจะแลกเครดิตออกมาเป็นเหรียญ** เราจะรับ  Withdraw Signal จากบอร์ดของผู้เล่น บอร์ดรับคืนเหรียญก็จะทำการสั่งให้เซอร์โว(Servo) ดันเหรียญตามจำนวน เครดิตหารด้วย 100 (เครดิต / 100) ครั้ง เหรียญที่ถูกดันออกมาจะตกที่บริเวณช่องใต้เครื่องหยอดเหรียญ  
    **2. กรณีที่อยากจะนำเหรียญออกจากเครื่อง** จะมีปุ่มที่เมื่อกดแล้วจะทำการดันเหรียญออกไปเรื่อยๆจนกว่าจะปล่อยการกดออกจากปุ่ม โดยเป็นปุ่ม Emergency ที่ไม่ได้มีไว้ให้ผู้เล่นกดได้ตามใจชอบ แต่เป็นกรณีที่จะต้องเอาเหรียญออกจากเครื่อง  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/creditor_3.png?raw=true" width="400"/><br/><br/>

  4. โดยจอ OLED ที่เชื่อมต่อกับบอร์ดนี้นั้นจะแสดงเหรียญที่มีอยู่ในเครื่องรับเหรียญและระบุว่าเป็นของผู้เล่นคนไหน  
    <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/creditor_4.png?raw=true" width="400"/><br/><br/>

  5. ขณะคืนเหรียญ จอ OLED ก็จะแสดงจำนวนเหรียญที่จะต้องคืน ว่าเหลืออีกกี่เหรียญถึงจะคืนเสร็จสิ้น  
     <img src="https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/main/info/images/creditor_5.png?raw=true" width="400"/><br/><br/>
---

### **4. โมดูลแสดงกราฟฟิกไพ่บนจอ OLED ** 
เป็น Library ที่เขียนขึ้นเองเพื่อใช้สำหรับแสดงผลกราฟฟิกบนจอ OLED โดยจะนำไปใช้กับบอร์ดผู้เล่นและบอร์ดดีลเลอร์ ประกอบไปด้วย 2 โมดูล  
- [**card\_icon.h**](https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/ab85e1bcdacbea44986f29a08d0389e6d3af254e/dealer/card_icon.h) ไฟล์บิทแมพ(Bitmap) ของหน้าไพ่ทั้ง 4 แบบ ประกอบด้วย โพดำ โพแดง ข้าวหลามตัด  และดอกจิก แบ่งเป็นขนาดเล็ก 16x16 pixel และขนาดใหญ่ 23x24 pixel  
- [**card\_printer.h**](https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn/blob/ab85e1bcdacbea44986f29a08d0389e6d3af254e/dealer/card_printer.h) ฟังก์ชั่นสำหรับวาดภาพไพ่ ตามเลขไพ่และหน้าไพ่ที่กำหนด และสามารถกำหนดหมายเลขตำแหน่งของไพ่ที่จะแสดงบนหน้าจอ OLED ได้ตั้งแต่หมายเลขตำแหน่ง 1 ถึง 5  
---

## อุปกรณ์ (รวม)
- NodeMCU ESP32 \[จำนวน 5 ตัว]
- OLED Screen SSD1306 128x32 \[จำนวน 5 ตัว]
- Breadboard \[จำนวน 5 ตัว]
- Rotary Encoder \[จำนวน 2 ตัว]
- Push Button \[จำนวน 6 ตัว]
- SG90 Micro Servo \[จำนวน 2 ตัว]
- Coin Acceptor \[จำนวน 2 ตัว]
- 12V AC to DC Adaptor \[จำนวน 1 ตัว]
- Resistor 10k \[จำนวน 2 ตัว]
- Power Bank \[จำนวน 3 ตัว]

**Github** :[ bid-laew–ruay-suay-laew-kunn](https://github.com/Jeammm/bid-leaw-ruay-suay-leaw-kunn)  
**Youtube** :[ บิดแล้วรวย... ซวยแล้วคุณ](https://www.youtube.com/watch?v=K43_0yFJ1xQ)