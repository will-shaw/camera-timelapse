/* Timelapse Rail Controller 
 *
 * Stepper pins: 8, 9, 10, 11
 *
 * Bump Switch pin: A0
 *
 * Camera Trigger pin: 3
 *
 * Formula for calculating total delay time:
 * (stabilisation) + (step delay * 8) + (shoot delay)
 *     (1000)      + (step_delay * 8) +     (200)
 *
 * Command Inputs:
 * 
 *    <             | Step Backward
 *    >             | Step Forward
 *    a             | Move to Start (requires bump_stop)
 *    p             | Sets paused/unpaused
 *    r             | Toggle rotation direction
 *    s             | Start/Stop (stop resets frame and step counts) (requires bump stop)
 *    m<step size>  | Sets movement (step) size to the number given
 *    d<delay time> | Sets delay between moves/shots
 *    r<(1 or 0)>   | Sets rotation direction specifically. 1 = clockwise
 *
 */

#define trigger 3
#define bump_stop A0
#define stabilisation 1000

#define motor_pin_1 8
#define motor_pin_2 9
#define motor_pin_3 10
#define motor_pin_4 11

int step_delay = 2;
int step_move = 5;
int step_count = 0;
int frame = 0;

boolean isPaused = true;
boolean isClockwise = false;
boolean isTiming = false;
boolean isReset = false;
boolean isStart = false;

unsigned long delay_time = 1000;
unsigned long start;
unsigned long timeout = millis();

String in = "";

void setup() {
    pinMode(motor_pin_1, OUTPUT);
    pinMode(motor_pin_2, OUTPUT);
    pinMode(motor_pin_3, OUTPUT);
    pinMode(motor_pin_4, OUTPUT);
    digitalWrite(motor_pin_1,LOW);
    digitalWrite(motor_pin_2,LOW);
    digitalWrite(motor_pin_3,LOW);
    digitalWrite(motor_pin_4,LOW);
    pinMode(trigger, OUTPUT);
    digitalWrite(trigger, LOW);
    Serial.begin(9600);
    printMove();
    printDelay();
}

void loop() {
    getCommand();
    if (!isPaused) {
        step(step_move);
        wait(stabilisation);
        shoot();
        wait(delay_time);
    } else {
        wait(1000);
    }
    if (millis() >= timeout + 1000) {
        Serial.println('w');
        timeout = millis();
    }
}

void getCommand() {
    while (Serial.available() > 0) {
        in += (char) Serial.read();
        delay(2);
    }
    
    if (in.length() > 1) {
        if (in.startsWith("r")) {
            isClockwise = (boolean) in.substring(1).toInt();
            printRotation();
            step_move = 0 - step_move;
            printMove();
        
        } else if (in.startsWith("m")) {
            step_move = isClockwise ? (0 - (in.substring(1).toInt())) : (in.substring(1).toInt());
            printMove();
        
        } else if (in.startsWith("d")) {
            delay_time = (in.substring(1).toInt());
            printDelay();
        }
        
    } else if (in.equals(">")) {
        step(isClockwise ? step_move : 0 - step_move);
        
    } else if (in.equals("<")) {
        step(isClockwise ? 0 - step_move : step_move);
        
    } else if (in.equals("p")) {
        isPaused = !isPaused;
        printPause();
        
    } else if (in.equals("r")) {
        isClockwise = !isClockwise;
        printRotation();
        step_move = 0 - step_move;
        printMove();
        
    } else if (in.equals("s")) {
        frame = 0;
        step_count = 0;
        Serial.println(!isPaused ? "Frames Reset" : "Active");
        Serial.println(isClockwise ? "A-C move to start" : "C move to start");
        moveToStart();
        //isPaused = !isPaused;
        
    } else if (in.equals("a")) {
        moveToStart();

    } else if (in.equals("i")) {
        printMove();
        printDelay();
        printPause();
        printFrame();
        printStepCount();
        printRotation();
        
    } else if (!in.equals("")){
        Serial.println("Unknown Command");
        
    }
    in = "";
}

void step(int m) {
    m = abs(m);
    Serial.println('j');
    while ((isStart || analogRead(bump_stop) <= 100) && m > 0 && !isStopByte()) {
        isClockwise ? forward() : reverse();
        m--;
        step_count++;
        if (step_count > 10) {
          isStart = false;
        }
    }
    if (analogRead(bump_stop) > 100) {
      Serial.println("Stopped");
      isReset = true;
      moveToCenter();
    }
}

void shoot() {
    digitalWrite(trigger, HIGH);
    delay(200);
    digitalWrite(trigger, LOW);
    frame++;
    printFrame();
}

boolean isStopByte() {
    if (Serial.available() > 0) {
        if ((char) Serial.peek() == 's' || (char) Serial.peek() == 'p') {
            Serial.println((char)Serial.peek());
            return true; 
        }
    }
    return false;
}

void moveToStart() {
    Serial.println('o');
    while (analogRead(bump_stop) <= 100 && !isStopByte()) {
        isClockwise ? reverse() : forward();
    }
    Serial.println('w');
    isStart = true;
    step_count = 0;
}

void moveToCenter() {
    Serial.println('c');
    int steps = (step_count / 2), count = 0;
    while (isReset && !isStopByte() && count < steps) {
        isClockwise ? reverse() : forward();
        count++;
    }
    isReset = false;
    isPaused = true;
    printPause();
}

void wait(unsigned long t) {
    unsigned long ms = millis();
    while (!isPaused && ms + t > millis()) {
        if (Serial.available()) {
            getCommand();
        }
    }
}

void forward() {
    digitalWrite(motor_pin_1, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_4, LOW);
    delay(step_delay);
    digitalWrite(motor_pin_2, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_1, LOW);
    delay(step_delay);
    digitalWrite(motor_pin_3, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_2, LOW);
    delay(step_delay);
    digitalWrite(motor_pin_4, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_3, LOW);
    delay(step_delay);
}  

void reverse() {
    digitalWrite(motor_pin_4, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_1, LOW);
    delay(step_delay);
    digitalWrite(motor_pin_3, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_4, LOW);
    delay(step_delay);
    digitalWrite(motor_pin_2, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_3, LOW);
    delay(step_delay);
    digitalWrite(motor_pin_1, HIGH);
    delay(step_delay);
    digitalWrite(motor_pin_2, LOW);
    delay(step_delay);
}

void timer() {
    if (!isTiming) {
        start = micros();
    } else {
        Serial.println("RT: " + (String)(micros() - start));
    }
    isTiming = !isTiming;
}

void printMove() {
    Serial.println("m" + (String) step_move);
}

void printDelay() {
    Serial.println("d" + (String) delay_time);
}

void printPause() {
    Serial.println(isPaused ? "p" : "j");
}

void printRotation() {
    Serial.println(isClockwise ? "c" : "ac");
}

void printFrame() {
    Serial.println("f" + (String) frame);
}

void printStepCount() {
    Serial.println("s:" + (String) step_count);
}