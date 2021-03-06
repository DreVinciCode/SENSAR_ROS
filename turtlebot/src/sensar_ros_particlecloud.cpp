/*****************************
sensar_ros_particlecloud.cpp
SENSAR
Andre Cleaver
Tufts University
*****************************/

#include "geometry_msgs/PoseArray.h"
#include "geometry_msgs/PoseStamped.h"
#include "ros/ros.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "tf2_ros/transform_listener.h"
#include <iostream>
#include <sstream>
#include <string>

const std::string TOPIC_IN  = "/particlecloud";
const std::string TOPIC_OUT = "/SENSAR/particlecloud";
const std::string FRAME_IN  = "map";
const std::string FRAME_OUT = "base_link";

using namespace std;

const int FREQUENCY = 30;

geometry_msgs::TransformStamped transformToBase_link;
geometry_msgs::PoseArray latestMsg;
geometry_msgs::PoseArray previousMsg;
ros::Publisher particleCloudPub;

geometry_msgs::PoseArray poseTransformation(geometry_msgs::PoseArray input)
{
    geometry_msgs::PoseArray transformed;
	int arraySize = input.poses.size(); 

    std::vector<geometry_msgs::Pose> poses; 
    
    transformed.header = input.header;
    transformed.header.frame_id = FRAME_OUT;
    
    
	for (int i = 0; i < arraySize; i++) 
	{
		geometry_msgs::PoseStamped stampedPose;
		stampedPose.header = input.header;
	    stampedPose.pose = input.poses[i];
    	tf2::doTransform(stampedPose, stampedPose, transformToBase_link);
		poses.push_back(stampedPose.pose);	
	}
	
	transformed.poses = poses;
	
    return transformed;
}

void publishLatest()
{
    particleCloudPub.publish(poseTransformation(latestMsg));
}

void cloudCallback(const geometry_msgs::PoseArray::ConstPtr& inMsg)
{
    latestMsg = *inMsg;
    publishLatest();
}

int main (int argc, char **argv)
{
    ros::init(argc, argv, "sensar_ros_particlecloud");
    ros::NodeHandle n;

    particleCloudPub = n.advertise<geometry_msgs::PoseArray>(TOPIC_OUT,5);
    ros::Subscriber globalSub  = n.subscribe(TOPIC_IN, 5, cloudCallback);

    tf2_ros::Buffer tBuffer;
    tf2_ros::TransformListener tf2_listener (tBuffer);

    ros::Rate rate(FREQUENCY);

    while(ros::ok())
    {
		try
        {
            transformToBase_link = tBuffer.lookupTransform(FRAME_OUT, FRAME_IN, ros::Time(0), ros::Duration(1.0));
        }
        catch(tf2::TransformException e)
        {
            ROS_INFO("%s \n", e.what());
        }
        
        ros::spinOnce();    
        publishLatest();
        rate.sleep();
    }

    return 0;
}
