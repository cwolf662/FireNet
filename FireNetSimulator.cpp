#include <iostream>
#include <string>
#include <sstream>
#include <conio.h>
#include <random>
#include <mqtt/async_client.h>

const std::string DEFAULT_PORT = "1883";
const std::string DEFAULT_USERNAME = "firenet_user";
const std::string DEFAULT_TOPIC = "firenet";

std::string getInput(const std::string& prompt, const std::string& defaultValue = "")
{
    std::string input;

    std::cout << prompt;

    if (!defaultValue.empty())
        std::cout << " [" << defaultValue << "]";

    std::cout << ": ";
    std::getline(std::cin, input);

    return input.empty() ? defaultValue : input;
}

std::string getPassword()
{
    std::string password;
    char ch;

    std::cout << "Password: ";

    while ((ch = _getch()) != '\r')
    {
        if (ch == '\b')
        {
            if (!password.empty())
            {
                password.pop_back();
                std::cout << "\b \b";
            }
        }
        else if (ch != '\0' && ch != -32)
        {
            password += ch;
            std::cout << '*';
        }
    }

    std::cout << std::endl;
    return password;
}

std::string generateClientID()
{
    std::random_device rd;
    return "FireNetSimulator-" + std::to_string(rd());
}

int main()
{
    try
    {
        std::cout << "====================================\n";
        std::cout << "       FireNet MQTT Simulator\n";
        std::cout << "====================================\n\n";

        std::string host = getInput("Broker IP/hostname");
        if (host.empty())
        {
            std::cout << "A broker address is required.\n";
            return 1;
        }

        std::string port = getInput("Port", DEFAULT_PORT);
        std::string clientID = getInput("Client ID", generateClientID());
        std::string username = getInput("Username", DEFAULT_USERNAME);
        std::string password = getPassword();
        std::string topic = getInput("MQTT topic", DEFAULT_TOPIC);

        if (password.empty())
        {
            std::cout << "A password is required.\n";
            return 1;
        }

        std::string serverAddress = "tcp://" + host + ":" + port;

        mqtt::async_client client(serverAddress, clientID);

        mqtt::connect_options connOpts;
        connOpts.set_user_name(username);
        connOpts.set_password(password);
        connOpts.set_clean_session(true);

        std::cout << "\nConnecting to FireNet broker...\n";
        client.connect(connOpts)->wait();

        std::cout << "Connected successfully!\n";

        bool running = true;

        while (running)
        {
            std::cout << "\n====================================\n";
            std::cout << "       FireNet MQTT Simulator\n";
            std::cout << "====================================\n";
            std::cout << "1. Send FIRE ALERT\n";
            std::cout << "2. Send NORMAL STATUS\n";
            std::cout << "3. Disconnect and Exit\n";
            std::cout << "\nSelect an option: ";

            std::string choice;
            std::getline(std::cin, choice);

            std::string payload;

            if (choice == "1")
            {
                payload = R"({"alert":1,"message":"Fire detected!"})";
            }
            else if (choice == "2")
            {
                payload = R"({"alert":0,"message":"Temperature normal."})";
            }
            else if (choice == "3")
            {
                running = false;
                continue;
            }
            else
            {
                std::cout << "Invalid selection. Try again.\n";
                continue;
            }

            mqtt::message_ptr message =
                mqtt::make_message(topic, payload);

            message->set_qos(1);

            client.publish(message)->wait();

            std::cout << "\nMessage published successfully:\n";
            std::cout << payload << "\n";
        }

        std::cout << "\nDisconnecting from broker...\n";
        client.disconnect()->wait();

        std::cout << "Disconnected successfully.\n";
        std::cout << "FireNet Simulator closed.\n";
    }
    catch (const mqtt::exception& e)
    {
        std::cerr << "\nMQTT Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}