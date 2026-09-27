#include <iostream>
#include <string>
#include <vector>

using namespace std;

// Returns true if a valid address was found, false otherwise.
// On success: outAddress holds the 32-bit value,
// and outPort holds the port number, or -1 if no port was present.
// On failure: outAddress is set to 0 and outPort is set to -1.
bool extractIPv4(const string& str, unsigned long& outAddress, int& outPort)
{
    outAddress = 0; //set default address for failure
    outPort = -1; //set default port for failure

    size_t i = 0; //current position in the input string

    while (i < str.length()) //search through the entire string
    {
        //skip anything that cannot be part of an IPv4 token
        if (!isdigit(static_cast<unsigned char>(str[i])) &&
            str[i] != '.' &&
            str[i] != ':')
        {
            i++; //move to the next character
            continue; //keep searching
        }

        size_t start = i; //remember where the candidate token begins

        //collect the entire run of digits, periods, and colons
        while (i < str.length() &&
               (isdigit(static_cast<unsigned char>(str[i])) ||
                str[i] == '.' ||
                str[i] == ':'))
        {
            i++; //move through the candidate token
        }

        string token = str.substr(start, i - start); //extract the whole candidate

        string addressPart = token; //assume the entire token is the address
        string portPart; //store the optional port
        int port = -1; //-1 means that no port was provided

        size_t colonPos = token.find(':'); //look for a port separator

        if (colonPos != string::npos) //a colon was found
        {
            //more than one colon makes the entire candidate invalid
            if (token.find(':', colonPos + 1) != string::npos)
            {
                continue; //reject this token
            }

            addressPart = token.substr(0, colonPos); //get the IPv4 portion
            portPart = token.substr(colonPos + 1); //get the port portion

            if (portPart.empty()) //a colon must be followed by a port
            {
                continue; //reject this token
            }

            for (char c : portPart) //check every character in the port
            {
                if (!isdigit(static_cast<unsigned char>(c))) //ports must contain only digits
                {
                    portPart.clear(); //mark the port as invalid
                    break;
                }
            }

            if (portPart.empty()) //the port contained invalid data
            {
                continue; //reject this token
            }

            try
            {
                unsigned long portValue = stoul(portPart); //convert the port to a number

                if (portValue > 65535) //valid TCP/UDP ports are 0 through 65535
                {
                    continue; //reject an out-of-range port
                }

                port = static_cast<int>(portValue); //store the valid port
            }
            catch (...)
            {
                continue; //reject a port that cannot be converted
            }
        }

        vector<int> octets; //store the four IPv4 octets
        size_t position = 0; //current position in the address portion
        bool valid = true; //assume the candidate is valid

        while (position < addressPart.length()) //parse each octet
        {
            size_t dotPos = addressPart.find('.', position); //find the next period

            string part; //store one octet as text

            if (dotPos == string::npos) //this is the final octet
            {
                part = addressPart.substr(position); //get everything remaining
                position = addressPart.length(); //finish parsing
            }
            else
            {
                part = addressPart.substr(position, dotPos - position); //get text before the period
                position = dotPos + 1; //move past the period
            }

            if (part.empty()) //empty octets are invalid
            {
                valid = false; //mark the candidate invalid
                break;
            }

            for (char c : part) //check that the octet contains only digits
            {
                if (!isdigit(static_cast<unsigned char>(c)))
                {
                    valid = false; //mark the candidate invalid
                    break;
                }
            }

            if (!valid) //stop if the octet was invalid
            {
                break;
            }

            try
            {
                unsigned long value = stoul(part); //convert the octet to a number

                if (value > 255) //IPv4 octets must be between 0 and 255
                {
                    valid = false; //mark the candidate invalid
                    break;
                }

                octets.push_back(static_cast<int>(value)); //save the valid octet
            }
            catch (...)
            {
                valid = false; //conversion failed
                break;
            }
        }

        if (!valid || octets.size() != 4) //IPv4 addresses must have exactly four valid octets
        {
            continue; //reject this candidate
        }

        //combine the four octets into one 32-bit value
        outAddress =
            (static_cast<unsigned long>(octets[0]) << 24) |
            (static_cast<unsigned long>(octets[1]) << 16) |
            (static_cast<unsigned long>(octets[2]) << 8) |
            static_cast<unsigned long>(octets[3]);

        outPort = port; //store the port or -1 if there was no port

        return true; //a valid IPv4 address was found
    }

    return false; //no valid IPv4 address was found
}

int main()
{
    string input; //store the user's input
    unsigned long address; //store the 32-bit IPv4 value
    int port; //store the optional port

    getline(cin, input); //read the entire line

    if (extractIPv4(input, address, port)) //try to find a valid IPv4 address
    {
        cout << "Address: " << address << endl; //display the 32-bit value
        cout << "Port: " << port << endl; //display the port
    }
    else
    {
        cout << "No valid IPv4 address found." << endl; //report failure
    }

    return 0; //end the program
}