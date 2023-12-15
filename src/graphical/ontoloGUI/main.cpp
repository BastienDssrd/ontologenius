#include "include/ontologenius/graphical/ontoloGUI/DarkStyle.h"
#include "include/ontologenius/graphical/ontoloGUI/ontologui.h"

#include <QApplication>

#include <csignal>
#include <thread>

#include "ontologenius/compat/ros.h"

#include <ament_index_cpp/get_package_share_directory.hpp>

void spinThread(bool* run)
{
    ontologenius::compat::ros::Node::get().spin();
}

#include <chrono>

using namespace std::chrono_literals;

int main(int argc, char *argv[])
{
    ontologenius::compat::ros::Node::init(argc, argv, "ontoloGUI");

    QApplication a(argc, argv);

    a.setStyle(new DarkStyle);

    std::string path = ament_index_cpp::get_package_share_directory("ontologenius");
    path = path + "/docs/images/ontologenius.ico";

    QIcon icon(QString::fromStdString(path));
    a.setWindowIcon(icon);

    ontoloGUI w;
    w.show();

    bool run = true;

    w.init();
    w.wait();

    w.start();
    w.loadReasoners();

    std::thread spin_thread(spinThread, &run);

    signal(SIGINT, SIG_DFL);
    auto a_exec = a.exec();

    ontologenius::compat::ros::Node::shutdown();

    run = false;
    spin_thread.join();

    return a_exec;
}
