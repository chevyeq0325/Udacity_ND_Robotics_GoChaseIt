#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the service
    if (!client.call(srv))
        ROS_ERROR("Failed to call service DriveToTarget");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    int white_pixel = 255;
   
    // Judge ball position by the number of white pixel
    int iLeftNum = 0;
    int iMidNum = 0;
    int iRightNum = 0;

    // Count pixel num
    unsigned int h, w, pxsize;
    pxsize = 3;//byte rgb

    for (h = 0; h < img.height; h++)
    {
         for (w = 0; w < img.width; w++)
         {
              if ((img.data[img.step*h+w*pxsize]==white_pixel)
                  &&(img.data[img.step*h+w*pxsize+1]==white_pixel)
                  &&(img.data[img.step*h+w*pxsize+2]==white_pixel))
              {
		   if (w < (int)(img.width / 3)) iLeftNum++;
                   else if (w < (int)(img.width * 2 / 3)) iMidNum++;
                   else iRightNum++;
              }
         }
    }

    //Debug
    ROS_INFO("White Pixel Num L:%d, M:%d, R:%d", iLeftNum,iMidNum,iRightNum);
   
    // Publish direction depend on white pixel num
    if (iLeftNum == 0 && iMidNum == 0 && iRightNum == 0)
    {
        //Stop
        ROS_INFO("NO TARGET STOP");
        drive_robot(0, 0);
    }
    else if (iLeftNum >= iMidNum && iLeftNum >= iRightNum)
    {
        // Drive to left
        ROS_INFO("DRIVE TO LEFT");
        drive_robot(0, 0.5);
    } else if (iLeftNum <= iMidNum && iMidNum >= iRightNum)
    {
        // Drive to forward
        ROS_INFO("DRIVE TO FORWARD");
        drive_robot(0.5, 0);
    } else if (iLeftNum <= iRightNum && iMidNum <= iRightNum)
    {
        // Drive to foward
        ROS_INFO("DRIVE TO RIGHT");
        drive_robot(0, -0.5);
    } else 
    {
        // Stop
        ROS_INFO("ERROR STOP");
        drive_robot(0, 0);
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}

