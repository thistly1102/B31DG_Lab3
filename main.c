#include <arduino.h>

//Define Input and Output Pins on ESP32C
#define T1_OUTPUT 18
#define T2_INPUT 0
#define T3_INPUT 1
#define T4_INPUT 2
#define T4_LED 19
#define T6_BUTTON 3
#define T7_LED 7

//Variables for Tasks 6 and 7
unsigned int buttonToggle = 0;
unsigned int lastButtonRead = 0;
unsigned int LEDSet = 0;

//Reading Values for Task 4
double reading1 = 0;
double reading2 = 0;
double reading3 = 0;
double reading4 = 0;
double read_average = 0;
unsigned int T4_counter = 1;

//Run Counter for each task for Monitoring of any violations
int T1Runs, T2Runs, T3Runs, T4Runs, T5Runs, T6Runs, T7Runs;
int T1Violations, T2Violations, T3Violations, T4Violations, T5Violations, T6Violations, T7Violations;

// Declare a mutex Semaphore Handle which we will use to manage the operation of the tasks.
// It will be used to ensure only one Task is accessing the resources at any time.
SemaphoreHandle_t Semaphore;

// Declare the event queue handle used in tasks 6 and 7
QueueHandle_t queueT7;

// Define the Global Structure for Tasks 2 and 3 Values
typedef struct 
{
    int T2Frequency;
    int T3Frequency;
} 
PeriodGlobalStructure;

// Define a global variable of type PeriodGlobalStructure
PeriodGlobalStructure globalStructure;

// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600);  // Starts the serial communication and sets baud rate to 9600

  //Define Pins for each task
  pinMode(T1_OUTPUT, OUTPUT);
  pinMode(T2_INPUT, INPUT);
  pinMode(T3_INPUT, INPUT);
  pinMode(T4_INPUT, INPUT);
  pinMode(T4_LED, OUTPUT);
  pinMode(T6_BUTTON, INPUT);
  pinMode(T7_LED, OUTPUT);

  if (Semaphore == NULL)
  {
    Semaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
  }
  if (Semaphore != NULL)
  {
    xSemaphoreGive(Semaphore);  //Release Semaphore for other tasks to take
  }
  // Now set up tasks to run independently.
  xTaskCreate(task1, "SignalOut", 800, NULL, 1, NULL);
  xTaskCreate(task2, "T2FreqRead", 800, NULL, 2, NULL);
  xTaskCreate(task3, "T3FreqRead", 800, NULL, 3, NULL);
  xTaskCreate(task4, "T4AnalogueRead", 800, NULL, 4, NULL);
  xTaskCreate(task5, "PeriodOutput", 800, NULL, 5, NULL);
  xTaskCreate(task6, "ButtonMonitor", 800, NULL, 6, NULL);
  xTaskCreate(task7, "LEDControl", 800, NULL, 7, NULL);
  //Additional Task for displaying violations
  xTaskCreate(Monitor, "Monitor", 1700, NULL, 8, NULL);


  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

///// TASK 1 /////
void task1(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(4); // Task period (in milliseconds)
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake(Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      //Increment task 1 run counter
      T1Runs = T1Runs + 1;

      ////////////////// Calculate total number of violations //////////////////
      int currentFrame, lastFrame;
      if(currentFrame == NULL){
        currentFrame = LastWakeTime; //initialize first frame at the time the task was first run
      }                                      
      currentFrame = currentFrame + Period; //increment current frame by exactly 4ms

      // Check if the task to test has run every 4ms
      if (LastWakeTime > currentFrame || LastWakeTime < lastFrame) {
        T1Violations = T1Violations + 1;     // Task has not run within 4ms frame
      } 

      lastFrame = currentFrame; //Save current frame as the last frame for the next run

      /////Run Task 1/////

      digitalWrite(T1_OUTPUT, HIGH);  // turn the LED on (HIGH is the voltage level)
      delayMicroseconds(200);         //Delay 200us
      digitalWrite(T1_OUTPUT, LOW);   // turn the LED off by making the voltage LOW
      delayMicroseconds(50);          //Delay 50us
      digitalWrite(T1_OUTPUT, HIGH);  // turn the LED on (HIGH is the voltage level)
      delayMicroseconds(30);          //Delay 30us
      digitalWrite(T1_OUTPUT, LOW);   // turn the LED off (LOW is the voltage level)

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //Obtain Minimum Remaining Stack Size within Task
      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask 1 Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}

///// TASK 2 /////
void task2(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(20); // Task period (in milliseconds)
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      //Increment task 2 run counter
      T2Runs = T2Runs + 1;

      ////////////////// Calculate total number of violations //////////////////
      int currentFrame, lastFrame;
      if(currentFrame == NULL){
        currentFrame = LastWakeTime; //initialize first frame at the time the task was first run
      }                                      
      currentFrame = currentFrame + Period; //increment current frame by exactly 20ms

      // Check if the task to test has run every 20ms
      if (LastWakeTime > currentFrame || LastWakeTime < lastFrame) {
        T2Violations = T2Violations + 1;     // Task has not run within 20ms frame
      } 

      lastFrame = currentFrame; //Save current frame as the last frame for the next run

      /////Run Task 2/////
      int Freq_Input_High = pulseIn(T2_INPUT, HIGH, 3000);  //us
      //Calculate Frequency
      globalStructure.T2Frequency = 1000000 / (2 * Freq_Input_High);  //Hz

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask 2 Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}

///// TASK 3 /////
void task3(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(8); // Task period (in milliseconds)
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      //Increment task 3 run counter
      T3Runs = T3Runs + 1;

      ////////////////// Calculate total number of violations //////////////////
      int currentFrame, lastFrame;
      if(currentFrame == NULL){
        currentFrame = LastWakeTime; //initialize first frame at the time the task was first run
      }                                      
      currentFrame = currentFrame + Period; //increment current frame by exactly 8ms

      // Check if the task to test has run every 8ms
      if (LastWakeTime > currentFrame || LastWakeTime < lastFrame) {
        T3Violations = T3Violations + 1;     // Task has not run within 8ms frame
      } 

      lastFrame = currentFrame; //Save current frame as the last frame for the next run

      /////Run Task 3/////

      int Freq_Input_High = pulseIn(T3_INPUT, HIGH, 3000);  //us
      //Calculate Frequency
      globalStructure.T3Frequency = 1000000 / (2 * Freq_Input_High);  //Hz

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //Obtain Minimum Remaining Stack Size within Task
      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask 3 Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}

///// TASK 4 /////
void task4(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(20); // Task period (in milliseconds)
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      //Increment task 4 run counter
      T4Runs = T4Runs + 1;

      ////////////////// Calculate total number of violations //////////////////
      int currentFrame, lastFrame;
      if(currentFrame == NULL){
        currentFrame = LastWakeTime; //initialize first frame at the time the task was first run
      }                                      
      currentFrame = currentFrame + Period; //increment current frame by exactly 20ms

      // Check if the task to test has run every 20ms
      if (LastWakeTime > currentFrame || LastWakeTime < lastFrame) {
        T4Violations = T4Violations + 1;     // Task has not run within 20ms frame
      } 

      lastFrame = currentFrame; //Save current frame as the last frame for the next run

      /////Run Task 4/////
      //Change between four different Readings every task operation
      if (T4_counter == 5)
      {
        T4_counter = 1;
        reading1 = analogRead(T4_INPUT);    
      }
      else if (T4_counter == 2)
      {
        reading2 = analogRead(T4_INPUT);
      }
      else if (T4_counter == 3)
      {
        reading3 = analogRead(T4_INPUT);
      }
      else if (T4_counter == 4)
      {
        reading4 = analogRead(T4_INPUT);
      }
    
      read_average = (reading1 + reading2 + reading3 + reading4) / 4;  //Calculate average

      //If average reading is greater than half of maximum value (4095/2=2047)
      if (read_average > 2047) {
        digitalWrite(T4_LED, HIGH);  //Error LED HIGH
      } else {
        digitalWrite(T4_LED, LOW);  //Error LED LOW
      }
      T4_counter = T4_counter + 1;

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //Obtain Minimum Remaining Stack Size within Task
      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask 4 Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}

///// TASK 5 /////
void task5(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(100); // Task period (in milliseconds) 
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      //Increment task 5 run counter
      T5Runs = T5Runs + 1;

      ////////////////// Calculate total number of violations //////////////////
      int currentFrame, lastFrame;
      if(currentFrame == NULL){
        currentFrame = LastWakeTime; //initialize first frame at the time the task was first run
      }                                      
      currentFrame = currentFrame + Period; //increment current frame by exactly 100ms

      // Check if the task to test has run every 100ms
      if (LastWakeTime > currentFrame || LastWakeTime < lastFrame) {
        T5Violations = T5Violations + 1;     // Task has not run within 100ms frame
      } 

      lastFrame = currentFrame; //Save current frame as the last frame for the next run

      /////Run Task 5/////

      if (globalStructure.T2Frequency < 333)
      {
        globalStructure.T2Frequency = 333;
      }
        if (globalStructure.T3Frequency < 500)
      {
        globalStructure.T3Frequency = 500;
      }
      int T2_Freq = map(globalStructure.T2Frequency, 333, 1000, 0, 99); //Scaling to between 0 - 99 for frequencies between 333-1000Hz
      int T3_Freq = map(globalStructure.T3Frequency, 500, 1000, 0, 99); //Scaling to between 0 - 99 for frequencies between 500-1000Hz

      T2_Freq = constrain(T2_Freq, 0, 99);  //Constrain to ensure values are between 0-99
      T3_Freq = constrain(T3_Freq, 0, 99);  //Constrain to ensure values are between 0-99
      // print T2 and T3 Frequencies
      Serial.print("\n");
      Serial.print(T2_Freq);
      Serial.print(",");
      Serial.print(T3_Freq);

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //Obtain Minimum Remaining Stack Size within Task
      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask 5 Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}

///// TASK 6 /////
void task6(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(100); // Task period (in milliseconds)
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      //Increment task 6 run counter
      T6Runs = T6Runs + 1;

      ////////////////// Calculate total number of violations //////////////////
      int currentFrame, lastFrame;
      if(currentFrame == NULL){
        currentFrame = LastWakeTime; //initialize first frame at the time the task was first run
      }                                      
      currentFrame = currentFrame + Period; //increment current frame by exactly 100ms

      // Check if the task to test has run every 100ms
      if (LastWakeTime > currentFrame || LastWakeTime < lastFrame) {
        T6Violations = T6Violations + 1;     // Task has not run within 100ms frame
      } 

      lastFrame = currentFrame; //Save current frame as the last frame for the next run

      /////Run Task 6/////

      queueT7 = xQueueCreate(10, sizeof(int)); //Create Event Queue for LED
      buttonToggle = digitalRead(T6_BUTTON);  //Get Button State
      //Ensure multiple toggles are not sent on same button push
      if (lastButtonRead - buttonToggle != 0 && buttonToggle == 1)  //If lastButtonRead and buttonToggle are both HIGH then does not run
      {
        xQueueSend(queueT7, &buttonToggle, 0);  //Send event to Queue  
        lastButtonRead = 1; //Set HIGH for next task run
      }
      else    //When lastButtonRead and buttonToggle are both HIGH or buttonToggle is LOW
      {
        if(buttonToggle == 1) //While Button is held down, lastButtonRead does not change
        { lastButtonRead = 1; }
        else
        { lastButtonRead = 0; }
        
        buttonToggle = 0;
        xQueueSend(queueT7, &buttonToggle, 0);  //Send event to Queue
      }

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //Obtain Minimum Remaining Stack Size within Task
      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask 6 Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}

///// TASK 7 /////
void task7(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(100); // Task period (in milliseconds)
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      //Increment task 7 run counter
      T7Runs = T7Runs + 1;

      ////////////////// Calculate total number of violations //////////////////
      int currentFrame, lastFrame;
      if(currentFrame == NULL){
        currentFrame = LastWakeTime; //initialize first frame at the time the task was first run
      }                                      
      currentFrame = currentFrame + Period; //increment current frame by exactly 100ms

      // Check if the task to test has run every 100ms
      if (LastWakeTime > currentFrame || LastWakeTime < lastFrame) {
        T7Violations = T7Violations + 1;     // Task has not run within 100ms frame
      } 

      lastFrame = currentFrame; //Save current frame as the last frame for the next run

      /////Run Task 7/////

      unsigned int LEDToggle;
      xQueueReceive(queueT7, &LEDToggle, 1);  // Wait for an event to be received from the queue
      if (LEDToggle == 1) //If the Button is Toggled, Toggle LED
      {
        if (LEDSet == 0)  //If LED is OFF
        {
          digitalWrite(T7_LED, HIGH); //Turn LED ON
          LEDSet = 1;
        }
        else if (LEDSet == 1) //If LED is ON
        {
          digitalWrite(T7_LED, LOW);  //Turn LED OFF
          LEDSet = 0;          
        }     
      }
      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //Obtain Minimum Remaining Stack Size within Task
      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask 7 Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}

///// Monitor Task - Runs every 10s/////
void Monitor(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Period = pdMS_TO_TICKS(10000); // Task period (in milliseconds)
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {

      //Serial Print the performance summary of Successful Runs against the Expected Number of Runs
      Serial.printf("\nPERFORMANCE SUMMARY:");    
      Serial.print("\nTask 1 Violations: "); Serial.print(T1Violations); Serial.print("/"); Serial.print(T1Runs);
      Serial.print("\nTask 2 Violations: "); Serial.print(T2Violations); Serial.print("/"); Serial.print(T2Runs);
      Serial.print("\nTask 3 Violations: "); Serial.print(T3Violations); Serial.print("/"); Serial.print(T3Runs);
      Serial.print("\nTask 4 Violations: "); Serial.print(T4Violations); Serial.print("/"); Serial.print(T4Runs);
      Serial.print("\nTask 5 Violations: "); Serial.print(T5Violations); Serial.print("/"); Serial.print(T5Runs);
      Serial.print("\nTask 6 Violations: "); Serial.print(T6Violations); Serial.print("/"); Serial.print(T6Runs);
      Serial.print("\nTask 7 Violations: "); Serial.print(T7Violations); Serial.print("/"); Serial.print(T7Runs);
      //Reset Runs and Violations to 0 for next 10s monitor
      T1Runs = 0; T1Violations = 0;
      T2Runs = 0; T2Violations = 0;
      T3Runs = 0; T3Violations = 0;
      T4Runs = 0; T4Violations = 0;
      T5Runs = 0; T5Violations = 0;
      T6Runs = 0; T6Violations = 0;
      T7Runs = 0; T7Violations = 0;

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.

      //Obtain Minimum Remaining Stack Size within Task
      //uint32_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      //Serial.print("\nTask Monitor Stack High Water Mark: "); Serial.print(uxHighWaterMark);
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Period );
  }
}
