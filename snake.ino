#include <SPI.h>
#include <U8g2lib.h>
#include <math.h>
#include <string> 
#include <MPU9250.h>
using namespace std;
struct Point{ int x; int y;};
#define SPI_CLK 14
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI oled(U8G2_R2, 10, 15, 16);
MPU9250 imu;

//function prototypes
bool collission(int x1, int y1, int x2, int y2, int size1, int size2);

class Food{
  public:
    Point location;
    int size;
      
    Food(){
      location = {random(10, 120), random(10, 50)};
      size = 4;
    }
    
    void reset(){
      location = {random(10, 120), random(10, 50)};
    }
    
    void draw(){
      oled.drawCircle((int)location.x + size/2, (int)location.y+size/2, size/2, U8G2_DRAW_ALL);
    }

};

class Snake{
  public: 
    
    String name;   
    Point body[100];
    int direction;
    int length;
    int speedX;
    int speedY;
    int speed;
    int size;
    int type;
    int score;

    Snake(int t, String n){
      type = t;
      name = n;
      direction = random(0,4); // up:0, left:1, down:2, right:3
      speed = 3;
      size = 3;
      score = 0;
      length = 3;

      //random initial position
      body[0] = {random(10, 120), random(10, 50)};

      //populate body
      for(int i=1; i<length; i++){
        body[i] = {body[i-1].x - size, body[i-1].y};    
        Serial.println(body[i].x);
      }    
      
    }

  void draw(Food *food){

    //move body
    for(int i=length-1; i>=1; i--){
      body[i].x = body[i-1].x;
      body[i].y = body[i-1].y;

      if(type == 1) oled.drawFrame(body[i].x, body[i].y, size+1, size+1);
      else oled.drawFrame(body[i].x, body[i].y, size, size);
    }

    //move head according to direction
    if(direction == 0){
      speedY = - speed;     
      speedX = 0; 
    }
    else if(direction == 1){
      speedX = - speed;  
      speedY = 0;   
    }
    else if(direction == 2){
      speedY = speed;     
      speedX = 0;  
    }
    else if(direction == 3){
      speedX = speed;
      speedY = 0;         
    }
    body[0].x += speedX;
    body[0].y += speedY;        
    if(type == 1) oled.drawFrame(body[0].x, body[0].y, size+1, size+1);   
    else oled.drawFrame(body[0].x, body[0].y, size, size);

    //check boundary collission
    if(body[0].x + size > 128) body[0].x = 0; //right      
    else if(body[0].x < 0)body[0].x = 128; //left    
    else if(body[0].y < 0) body[0].y = 64; //up  
    else if(body[0].y+size > 64) body[0].y = 0;  //down
    
    //check food collission
    if(collission(food->location.x, food->location.y, body[0].x, body[0].y, size, size+1)){
      score++;
      body[length] = {130, 130};
      length++;
      food->reset();
    }
    
  }

  void changeDir(int d){
    //snake can't bend in opposite direction
    bool allow = true;
    if(direction == 0 && d == 2) allow = false;
    else if(direction == 1 && d == 3) allow = false;
    else if(direction == 2 && d == 0) allow = false;
    else if(direction == 3 && d == 1) allow = false;
    if(allow) direction = d;      
  }
  
};

class Game{
  public:    
    Snake* snakes[10]; // first one is you
    int numSnakes;    
    Food *food;

    elapsedMillis gameTimer;
    elapsedMillis dirTimer;
    elapsedMillis inputTimer;

    
    Game(){
      //initialize game components
      snakes[0] = new Snake(1, "Me");
      numSnakes = 3;
      for(int i=1; i<numSnakes; i++) snakes[i] = new Snake(0, "other"+String(i));
      food = new Food();
    }

    void draw(){      
      //snake and food
      for(int i=0; i<numSnakes; i++) snakes[i]->draw(food);  
      food->draw();  

      //score
      oled.setFont(u8g2_font_u8glib_4_tr);
      int offsetX = 1;
      int offsetY = 5;
      int gapY = 10;
      for(int i=0; i<numSnakes; i++){ 
        oled.setCursor(offsetX, offsetY + i*gapY);
        oled.print(snakes[i]->name + ": " + String(snakes[i]->score));        
      }
      
    } 

    bool collission(int x1, int y1, int x2, int y2, int size1, int size2) {
	  int w1 = size1;
	  int h1 = size1;
	  int w2 = size2;
	  int h2 = size2;
	  return ((abs(x1 - x2) * 2 < (w1 + w2)) && (abs(y1 - y2) * 2 < (h1 + h2)));
	}

	int getAccDir(){
	    int dir = 0;
	    if(abs(imu.ay) > abs(imu.ax)){
	    if(imu.ay < 0){
	      return 1;
	    }
	    else if(imu.ay > 0){
	      return 3;
	    }  
	  }
	  else{
	    if(imu.ax < 0){
	      return 0;
	    }
	    else if(imu.ax > 0){
	     return 2; 
	    }
	  }
	}  
};

Game *game;
void setup() {
  SPI.setSCK(SPI_CLK);   // move the SPI SCK pin from default of 13
  oled.begin();  // initialize the OLED
  Serial.begin(115200); 
  randomSeed(analogRead(0)); 
  game = new Game();
  
  //calibrate IMU
  byte c = imu.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);  
  imu.initMPU9250();
  imu.MPU9250SelfTest(imu.selfTest);
  imu.calibrateMPU9250(imu.gyroBias, imu.accelBias);
  imu.initAK8963(imu.factoryMagCalibration);
  imu.getAres(); 
}

void loop() { 
  
  if(game->gameTimer > 80){  
    oled.clearBuffer();//clear      
    game->draw();
    oled.sendBuffer();        
    game->gameTimer = 0;
  }

  if(game->dirTimer > 1000){
    for(int i=1; i<game->numSnakes; i++) game->snakes[i]->direction = random(0, 4);               
    Serial.println(game->dirTimer);
    game->dirTimer = 0;    
  }

  if(game->inputTimer > 100){
    //snake control
    imu.readAccelData(imu.accelCount); 
    imu.ax = (float)imu.accelCount[0]*imu.aRes;
    imu.ay = (float)imu.accelCount[1]*imu.aRes;
    imu.az = (float)imu.accelCount[2]*imu.aRes; 
    game->snakes[0]->changeDir(game->getAccDir());  
    game->inputTimer = 0;
  }
}


