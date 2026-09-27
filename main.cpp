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

        if (token.empty() ||
            token.front() == '.' || token.front() == ':' ||
            token.back() == '.' || token.back() == ':') //reject incomplete candidate tokens
        {
            continue; //reject this entire candidate
        }

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
                break; //stop checking this candidate
            }

            if (part.length() > 1 && part[0] == '0') //reject leading zeros such as 01 or 001
            {
                valid = false; //mark the candidate invalid
                break; //stop checking this candidate
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
    string input; //store the full line entered by the user
    unsigned long address; //store the extracted 32-bit ipv4 value
    int port; //store the extracted port number

    while (true) //keep accepting input until the user enters end
    {
        cout << "Enter a string (or 'END' to quit): "; //display the input prompt
        getline(cin, input); //read the entire line including spaces

        if (input == "END") //check whether the user wants to quit
        {
            cout << "Program terminated." << endl; //display termination message
            break; //exit the loop
        }

        if (extractIPv4(input, address, port)) //try to extract a valid ipv4 address
        {
            unsigned long first = (address >> 24) & 255; //extract the first octet
            unsigned long second = (address >> 16) & 255; //extract the second octet
            unsigned long third = (address >> 8) & 255; //extract the third octet
            unsigned long fourth = address & 255; //extract the fourth octet

            cout << "Extracted IPv4 address: "; //begin the success message

            cout << first << "." //print the first octet
                 << second << "." //print the second octet
                 << third << "." //print the third octet
                 << fourth; //print the fourth octet

            cout << " (decimal value: " << address << ", port: "; //print the decimal value

            if (port == -1) //check whether a port was included
            {
                cout << "none"; //display none when there was no port
            }
            else
            {
                cout << port; //display the extracted port
            }

            cout << ")" << endl; //finish the success message
        }
        else
        {
            cout << "Invalid input: no valid IPv4 address found" << endl; //display failure message
        }
    }

    return 0; //end the program
}