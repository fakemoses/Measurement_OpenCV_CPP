# Measurement_OpenCV_CPP
Measuring 4 corners object using OpenCV and C++

## Requirements

- OpenCV Version 4.0 and above
- Working camera

- Please change the source name to your own filename.

## Compiling the script

``` 
g++ -o  output main.cpp `pkg-config --cflags --libs opencv4`
```
Make sure the path for the opencv is valid. For more information, please refer to the official documentation below.

[Official documentation](https://docs.opencv.org/master/df/d65/tutorial_table_of_content_introduction.html)

## Notice

- Some bugs are not yet solved. Mathematical expression and lack of accuracy for the detection are yet to be solved.
