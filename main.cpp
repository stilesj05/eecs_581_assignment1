#include <iostream>
#include <string>
#include <vector>
#include <cctype>

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
        if (!isdigit(static_cast<unsigned char>(str[i])) && //check if the character is not a digit
            str[i] != '.' && //check if the character is not a period
            str[i] != ':') //check if the character is not a colon
        {
            i++; //skip garbage characters
            continue; //keep searching
        }

        size_t start = i; //remember where the candidate token begins

        while (i < str.length() && //stay inside the string
               (isdigit(static_cast<unsigned char>(str[i])) || //allow digits
                str[i] == '.' || //allow periods
                str[i] == ':')) //allow colons
        {
            i++; //move through the entire candidate token
        }

        string token = str.substr(start, i - start); //extract the entire candidate token

        if (token.empty() || //reject an empty token
            token.front() == '.' || token.front() == ':' || //reject a token starting with a separator
            token.back() == '.' || token.back() == ':') //reject a token ending with a separator
        {
            continue; //reject this entire candidate
        }

        string addressPart = token; //assume the entire token is the address
        string portPart; //store the optional port
        int port = -1; //use -1 when no port is present

        size_t colonPos = token.find(':'); //look for a colon

        if (colonPos != string::npos) //check if a port separator exists
        {
            if (token.find(':', colonPos + 1) != string::npos) //check for a second colon
            {
                continue; //reject tokens containing more than one colon
            }

            addressPart = token.substr(0, colonPos); //get the address before the colon
            portPart = token.substr(colonPos + 1); //get the port after the colon

            if (portPart.empty()) //make sure something follows the colon
            {
                continue; //reject an empty port
            }

            unsigned long portValue = 0; //store the numeric port value
            bool validPort = true; //assume the port is valid

            for (char c : portPart) //process every character in the port
            {
                if (!isdigit(static_cast<unsigned char>(c))) //make sure the port contains only digits
                {
                    validPort = false; //mark the port invalid
                    break; //stop checking the port
                }

                portValue = portValue * 10 + (c - '0'); //manually convert and accumulate the digit

                if (portValue > 65535) //make sure the port is within range
                {
                    validPort = false; //mark the port invalid
                    break; //stop checking the port
                }
            }

            if (!validPort) //check whether the port failed validation
            {
                continue; //reject the entire candidate
            }

            port = static_cast<int>(portValue); //store the valid port
        }

        vector<int> octets; //store the four ipv4 octets
        size_t position = 0; //current position in the address portion
        bool valid = true; //assume the address is valid

        while (position < addressPart.length()) //process each octet
        {
            size_t dotPos = addressPart.find('.', position); //find the next period

            string part; //store one octet as text

            if (dotPos == string::npos) //check if this is the final octet
            {
                part = addressPart.substr(position); //get everything remaining
                position = addressPart.length(); //finish processing the address
            }
            else
            {
                part = addressPart.substr(position, dotPos - position); //get the current octet
                position = dotPos + 1; //move past the period
            }

            if (part.empty()) //check for an empty octet
            {
                valid = false; //mark the address invalid
                break; //stop checking this candidate
            }

            if (part.length() > 1 && part[0] == '0') //reject leading zeros such as 01 or 001
            {
                valid = false; //mark the address invalid
                break; //stop checking this candidate
            }

            unsigned long value = 0; //store the numeric value of the octet

            for (char c : part) //process every character in the octet
            {
                if (!isdigit(static_cast<unsigned char>(c))) //make sure the octet contains only digits
                {
                    valid = false; //mark the address invalid
                    break; //stop processing this octet
                }

                value = value * 10 + (c - '0'); //manually convert and accumulate the digit

                if (value > 255) //make sure the octet is between 0 and 255
                {
                    valid = false; //mark the address invalid
                    break; //stop processing this octet
                }
            }

            if (!valid) //check whether this octet failed validation
            {
                break; //stop checking this candidate
            }

            octets.push_back(static_cast<int>(value)); //store the valid octet
        }

        if (!valid || octets.size() != 4) //make sure exactly four valid octets were found
        {
            continue; //reject this candidate
        }

        outAddress = //combine the four octets into one 32-bit value
            (static_cast<unsigned long>(octets[0]) << 24) | //place the first octet in bits 24-31
            (static_cast<unsigned long>(octets[1]) << 16) | //place the second octet in bits 16-23
            (static_cast<unsigned long>(octets[2]) << 8) | //place the third octet in bits 8-15
            static_cast<unsigned long>(octets[3]); //place the fourth octet in bits 0-7

        outPort = port; //store the port or -1 if no port was present

        return true; //a valid ipv4 address was found
    }

    return false; //no valid ipv4 address was found
}

int main()
{
    string input; //store the full line entered by the user
    unsigned long address; //store the extracted 32-bit ipv4 value
    int port; //store the extracted port number

    while (true) //continue until the user enters end
    {
        cout << "Enter a string (or 'END' to quit): "; //display the prompt
        getline(cin, input); //read the entire input line

        if (input == "END") //check whether the user wants to quit
        {
            cout << "Program terminated." << endl; //display the termination message
            break; //exit the input loop
        }

        if (extractIPv4(input, address, port)) //attempt to extract a valid ipv4 address
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

            cout << " (decimal value: " << address << ", port: "; //print the decimal address value

            if (port == -1) //check whether a port was included
            {
                cout << "none"; //display none when no port was present
            }
            else
            {
                cout << port; //display the port number
            }

            cout << ")" << endl; //finish the success message
        }
        else
        {
            cout << "Invalid input: no valid IPv4 address found" << endl; //display the failure message
        }
    }

    return 0; //end the program
}