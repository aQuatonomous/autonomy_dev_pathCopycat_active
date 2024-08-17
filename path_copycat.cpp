/* * * * * * * * * * * * * * * * * * * * * * * * *
 *   Path Copycat (path_copycat.cpp)
 *   Sabrina Button (sabrina.button@queensu.ca)
 *   aQuatonomous 2024-2025
 * * * * * * * * * * * * * * * * * * * * * * * * */

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

/*
   This node reads from the joypad. If the 'start' button is pressed, the node will begin
   saving movement commands to a file. When the 'start' button is pushed again, it will stop recording.
   If the 'X' button is pressed, it will write the commands to the joypad topic, mimicking the
   previous movement path.
*/

class PathCopycat : public rclcpp::Node
{

public:
    PathCopycat()
        : Node("path_copycat"), count_(0)
    {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "joypad", 10, std::bind(&PathCopycat::recieve_controls, this, _1));

        publisher_ = this->create_publisher<std_msgs::msg::String>("joypad", 10);

        // Attempt to transmit the path every 50ms
        timer_ = this->create_wall_timer(
            50ms, std::bind(&PathCopycat::transmit_path, this));

        float base_rec_timestamp = std::chrono::system_clock::now();
        float base_trans_timestamp = std::chrono::system_clock::now();
        int trans_index = 0;
        bool recording = false;
        bool transmitting = false;
    }

private:
    void recieve_controls(const std_msgs::msg::String::SharedPtr msg) const
    {
        // TODO(sbutton): Change this to use the joypad data structure
        if (msg->data == "start" && !recording)
        {
            base_rec_timestamp = std::chrono::system_clock::now();
            recording = !recording;
            RCLCPP_INFO(this->get_logger(), "Toggled recording to " + std::to_string(recording));
        }
        else if (msg->data == "X" && !transmitting)
        {
            base_trans_timestamp = std::chrono::system_clock::now();
            transmitting = !transmitting;
            RCLCPP_INFO(this->get_logger(), "Toggled transmission to " + std::to_string(transmitting);
        }

        if (recording)
        {
            // Zero the timestamp according to the base
            msg->data.timestamp = std::chrono::system_clock::now() - base_rec_timestamp;

            // Write the message to the file
            with open("saved_path.txt", "w") as file : file.write(msg->data);
        }
    }

    /*
        Transmit the path to the joypad topic using the saved path timestamps.
    */
    void transmit_path()
    {
        if (transmitting)
        {
            // Read the file
            with open("saved_path.txt", "r") as file : path = file.read();

            // Get the time since the last transmission
            float delta = std::chrono::system_clock::now() - base_trans_timestamp;

            // If delta is greater than the timestamp of the next message, send the message
            while (delta > path[trans_index].timestamp)
            {
                publisher_->publish(path[trans_index]);
                trans_index++;
            }
        }
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalPublisher>());
    rclcpp::shutdown();
    return 0;
}