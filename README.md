# Timelapse Rail Controller 
---

Stepper pins: 8, 9, 10, 11  
  
Bump Switch pin: A0  
  
Camera Trigger pin: 3  
  
Formula for calculating total delay time:  
(stabilisation) + (step delay * 8) + (shoot delay)  
    (1000)      + (step_delay * 8) +     (200)  
  
# Command Inputs:

| Argument | Function |
|:-:|:-:|
| < | Step Backward |
| > | Step Forward |
| a | Move to Start (requries bump stop) |
| p | Toggle Pause |
| r | Toggle Rotation |
| s | Start/Stop (stop resets frame and step counts) (requires bump stop) |
| m<step size> | Sets movement (step) size to the number given |
| d<delay time> | Sets delay between moves/shots |
| r<(1 or 0)>   | Force rotation direction. 1 = clockwise |
