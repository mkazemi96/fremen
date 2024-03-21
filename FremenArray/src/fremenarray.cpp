#include <memory>
#include <chrono>
#include <iostream>
#include "rclcpp/rclcpp.hpp"
#include "fremenarray/FremenArrayAction.hpp"
#include "actionlib/server/simple_action_server.h"
#include "CFrelement.h"

using namespace std::chrono_literals;
using FremenArrayAction = fremenarray::action::FremenArray;
using ActionServer = actionlib::SimpleActionServer<FremenArrayAction>;

class FremenArrayNode : public rclcpp::Node
{
public:
    FremenArrayNode()
        : Node("fremenarray"), server_(nullptr), frelementArray_(nullptr), values_(nullptr), numStates_(0)
    {
        server_ = std::make_shared<ActionServer>(this, "fremenarray", 
                std::bind(&FremenArrayNode::actionServerCallback, this, std::placeholders::_1));
        server_->start();
    }

private:
    void actionServerCallback(const std::shared_ptr<const FremenArrayAction::Goal> goal)
    {
        RCLCPP_INFO(get_logger(), "Received action request: %s", goal->operation.c_str());

        // Handle action request here...
    }

    std::shared_ptr<ActionServer> server_;
    CFrelement *frelementArray_;
    float *values_;
    uint32_t times_[1];
    int numStates_;
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<FremenArrayNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
