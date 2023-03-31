#include <arduino.h>

//Define Input and Output Pins on ESP32C
#define T1_OUTPUT 18
#define T2_INPUT 0
#define T3_INPUT 1
#define T4_INPUT 2
#define T4_LED 19
#define T6_BUTTON 3
#define T7_LED 7

unsigned int T2_Freq = 0;            //Frequency Reading for Task 2
unsigned int T3_Freq = 0;            //Frequency Reading for Task 3
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

//Run and Violation Counter for each task
int T1Runs, T2Runs, T3Runs, T4Runs, T5Runs, T6Runs, T7Runs;
int T1Violations, T2Violations, T3Violations, T4Violations, T5Violations, T6Violations, T7Violations;

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only one Task is accessing this resource at any time.
SemaphoreHandle_t Semaphore;

// Declare the event queue handle used in tasks 6 and 7
QueueHandle_t queueT7;

// Define the Global Structure for Tasks 2 and 3 Values
typedef struct {
    int T2_Struct;
    int T3_Struct;
} 
frequencyGlobalStructure;

// Define a global variable of type frequencyGlobalStructure
frequencyGlobalStructure freqStruct;

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
    xSemaphoreGive(Semaphore);
  }
  // Now set up tasks to run independently.
  xTaskCreate(task1, "SignalOut", 3000, NULL, 1, NULL);
  xTaskCreate(task2, "T2FreqRead", 3000, NULL, 2, NULL);
  xTaskCreate(task3, "T3FreqRead", 3000, NULL, 3, NULL);
  xTaskCreate(task4, "T4AnalogueRead", 3000, NULL, 4, NULL);
  xTaskCreate(task5, "FrequencyOutput", 3000, NULL, 5, NULL);
  xTaskCreate(task6, "ButtonMonitor", 3000, NULL, 6, NULL);
  xTaskCreate(task7, "LEDControl", 3000, NULL, 7, NULL);


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
  const TickType_t Frequency = pdMS_TO_TICKS(4); // Task period (in milliseconds)
  
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 250 ticks to see if it becomes free.
    if (xSemaphoreTake(Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      digitalWrite(T1_OUTPUT, HIGH);   // turn the LED on (HIGH is the voltage level)
      delayMicroseconds(200);      //Delay 200us
      digitalWrite(T1_OUTPUT, LOW);    // turn the LED off by making the voltage LOW
      delayMicroseconds(50);      //Delay 50us
      digitalWrite(T1_OUTPUT, HIGH);   // turn the LED on (HIGH is the voltage level)
      delayMicroseconds(30);      //Delay 30us
      digitalWrite(T1_OUTPUT, LOW);   // turn the LED off (LOW is the voltage level)

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Frequency );
  }
}

///// TASK 2 /////
void task2(void *pvParameters)
{
  
  TickType_t LastWakeTime;
  const TickType_t Frequency = pdMS_TO_TICKS(20); // Task period (in milliseconds)
  
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 10 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      int Freq_Input_High = pulseIn(T2_INPUT, HIGH, 3000);  //us
      freqStruct.T2_Struct = 1000000 / (2 * Freq_Input_High);  //Hz

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Frequency );
  }
}

///// TASK 3 /////
void task3(void *pvParameters)
{
  
  TickType_t LastWakeTime;
  const TickType_t Frequency = pdMS_TO_TICKS(8); // Task period (in milliseconds)
  
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 10 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      int Freq_Input_High = pulseIn(T3_INPUT, HIGH, 3000);  //us
      freqStruct.T3_Struct = 1000000 / (2 * Freq_Input_High);  //Hz

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Frequency );
  }
}

///// TASK 4 /////
void task4(void *pvParameters)
{

  TickType_t LastWakeTime;
  const TickType_t Frequency = pdMS_TO_TICKS(20); // Task period (in milliseconds)
  
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 10 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
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
        digitalWrite(T4_LED, HIGH);  //Error LED High
      } else {
        digitalWrite(T4_LED, LOW);  //Error LED Low
      }
      T4_counter = T4_counter + 1;

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Frequency );
  }
}

///// TASK 5 /////
void task5(void *pvParameters)
{
  TickType_t LastWakeTime;
  const TickType_t Frequency = pdMS_TO_TICKS(100); // Task period (in milliseconds)
  
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 10 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      if (freqStruct.T2_Struct < 333)
      {
        freqStruct.T2_Struct = 333;
      }
        if (freqStruct.T3_Struct < 500)
      {
        freqStruct.T3_Struct = 500;
      }
      T2_Freq = map(freqStruct.T2_Struct, 333, 1000, 0, 99); //Scaling to between 0 - 99 for frequencies between 333-1000Hz
      T3_Freq = map(freqStruct.T3_Struct, 500, 1000, 0, 99); //Scaling to between 0 - 99 for frequencies between 500-1000Hz

      T2_Freq = constrain(T2_Freq, 0, 99);  //Constrain to ensure values are between 0-99
      T3_Freq = constrain(T3_Freq, 0, 99);  //Constrain to ensure values are between 0-99
      // print T2 and T3 Frequencies
      Serial.print("\n");
      Serial.print(T2_Freq);
      Serial.print(",");
      Serial.print(T3_Freq);

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.
    }
    vTaskDelayUntil( &LastWakeTime, Frequency );
  }
}

///// TASK 6 /////
void task6(void *pvParameters)
{
  
  TickType_t LastWakeTime;
  const TickType_t Frequency = pdMS_TO_TICKS(100); // Task period (in milliseconds)
  
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 10 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      queueT7 = xQueueCreate(10, sizeof(int)); //Create Event Queue for LED
      buttonToggle = digitalRead(T6_BUTTON) == HIGH;  //Get Button State
      //Ensure multiple toggles are not sent on same button push
      if (lastButtonRead == 1 && buttonToggle == 1)  //If lastButtonRead and buttonToggle are both HIGH then does not run
      {
        buttonToggle = 0;
        xQueueSend(queueT7, &buttonToggle, 0);  //Send event to Queue
      }
      else if (lastButtonRead == 0 && buttonToggle == 1)
      {
        xQueueSend(queueT7, &buttonToggle, 0);  //Send event to Queue  
        lastButtonRead = 1;
      }
      else
      {
        buttonToggle = 0;
        xQueueSend(queueT7, &buttonToggle, 0);  //Send event to Queue        
      }

      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Frequency );
  }
}

///// TASK 7 /////
void task7(void *pvParameters)
{
  
  TickType_t LastWakeTime;
  const TickType_t Frequency = pdMS_TO_TICKS(100); // Task period (in milliseconds)
  
  // Initialize LastWakeTime to the current tick count
  LastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //See if we can obtain the semaphore. If the semaphore is not available wait 10 ticks to see if it becomes free.
    if (xSemaphoreTake( Semaphore, ( TickType_t ) 250 ) == pdTRUE)
    {
      unsigned int LEDToggle;
      xQueueReceive(queueT7, &LEDToggle, 1);  // Wait for an event to be received from the queue
      if (LEDToggle == 1)
      {
        if (LEDSet == 0)
        {
          digitalWrite(T7_LED, HIGH);
          LEDSet = 1;
        }
        else if (LEDSet == 1)
        {
          digitalWrite(T7_LED, LOW);
          LEDSet = 0;          
        }     
      }
      xSemaphoreGive( Semaphore ); // Now give the Serial Port for others.
    }
    // Wait until the end of the task period
    vTaskDelayUntil( &LastWakeTime, Frequency );
  }
}
