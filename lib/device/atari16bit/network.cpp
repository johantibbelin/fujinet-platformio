#ifdef BUILD_ACSI

/**
 * N: Firmware
 */

#include "network.h"

#include <cstring>
#include <algorithm>

#include "../../include/debug.h"

#include "utils.h"

#include "status_error_codes.h"
#include "TCP.h"
#include "UDP.h"
#include "Test.h"
#include "Telnet.h"
#include "TNFS.h"
#include "FTP.h"
#include "HTTP.h"
#include "SSH.h"
#include "SMB.h"

//using namespace std;

/**
 * Constructor
 */
ACSINetwork::ACSINetwork()
{
    receiveBuffer = new string();
    transmitBuffer = new string();
    specialBuffer = new string();

    receiveBuffer->clear();
    transmitBuffer->clear();
    specialBuffer->clear();
}

/**
 * Destructor
 */
ACSINetwork::~ACSINetwork()
{
    receiveBuffer->clear();
    transmitBuffer->clear();
    specialBuffer->clear();

    if (receiveBuffer != nullptr)
        delete receiveBuffer;
    if (transmitBuffer != nullptr)
        delete transmitBuffer;
    if (specialBuffer != nullptr)
        delete specialBuffer;
}

/** ACSI COMMANDS ***************************************************************/

/**
 * ACSI Open command
 * Called in response to 'O' command. Instantiate a protocol, pass URL to it, call its open
 * method. Also set up RX interrupt.
 */
void ACSINetwork::open()
{
    // Debug_printf("ACSINetwork::open()\n");

    // string d;

    // ACSI_send_ack();

    // memset(response,0,sizeof(response));
    // ACSI_recv_buffer(response, 256);

    // Debug_printf("ACSINetwork::open url %s\n", response);
    
    // ACSI_send_ack();

    // channelMode = PROTOCOL;

    // open_aux1 = cmdFrame.aux1;
    // open_aux2 = cmdFrame.aux2;

    // // Shut down protocol if we are sending another open before we close.
    // if (protocol != nullptr)
    // {
    //     protocol->close();
    //     delete protocol;
    //     protocol = nullptr;
    // }

    // // Reset status buffer
    // network_status.reset();

    // Debug_printf("open()\n");

    // // Parse and instantiate protocol
    // d=string((char *)response,256);
    // parse_and_instantiate_protocol(d);

    // if (protocol == nullptr)
    // {
    //     return;
    // }

    // // Attempt protocol open
    // if (protocol->open(urlParser, &cmdFrame) == true)
    // {
    //     network_status.error = protocol->error;
    //     Debug_printf("Protocol unable to make connection. Error: %d\n", protocol->error);
    //     delete protocol;
    //     protocol = nullptr;
    //     return;
    // }

    // json.setProtocol(protocol);

    // ACSI_send_complete();
}

/**
 * ACSI Close command
 * Tear down everything set up by open(), as well as RX interrupt.
 */
void ACSINetwork::close()
{
    // Debug_printf("ACSINetwork::close()\n");

    // ACSI_send_ack();

    // network_status.reset();

    // // If no protocol enabled, we just signal complete, and return.
    // if (protocol == nullptr)
    // {
    //     return;
    // }

    // // Ask the protocol to close
    // protocol->close();

    // // Delete the protocol object
    // delete protocol;
    // protocol = nullptr;

    // ACSI_send_complete();
}

/**
 * @brief Perform read of the current JSON channel
 * @param num_bytes Number of bytes to read
 */
bool ACSINetwork::read_channel_json(unsigned short num_bytes)
{
    if (num_bytes > json_bytes_remaining)
        json_bytes_remaining=0;
    else
        json_bytes_remaining-=num_bytes;

    return false;
}

/**
 * Perform the channel read based on the channelMode
 * @param num_bytes - number of bytes to read from channel.
 * @return TRUE on error, FALSE on success. Passed directly to bus_to_computer().
 */
bool ACSINetwork::read_channel(unsigned short num_bytes)
{
    bool _err = false;

    switch (channelMode)
    {
    case PROTOCOL:
        _err = protocol->read(num_bytes);
        break;
    case JSON:
        err = read_channel_json(num_bytes);
        break;
    }
    return _err;
}

/**
 * ACSI Write command
 * Write # of bytes specified by aux1/aux2 from tx_buffer out to ACSI. If protocol is unable to return requested
 * number of bytes, return ERROR.
 */
void ACSINetwork::write()
{
    // Debug_printf("ACSINetwork::write()\n");

    // memset(response, 0, sizeof(response));
    // ACSI_send_ack();

    // uint16_t num_bytes = (cmdFrame.aux2 << 8) + cmdFrame.aux1;

    // ACSI_recv_buffer(response, num_bytes);
    // ACSI_send_ack();

    // *transmitBuffer += string((char *)response, num_bytes);
    // err = write_channel(num_bytes);

    // ACSI_send_complete();
}

void ACSINetwork::read()
{
    // Debug_printf("ACSINetwork::read()\n");

    // uint16_t num_bytes = (cmdFrame.aux2 << 8) + cmdFrame.aux1;
    // Debug_printf("ACSINetwork::read %u bytes\n", num_bytes);


    // ACSI_send_ack();

    // // Check for rx buffer. If NULL, then tell caller we could not allocate buffers.
    // if (receiveBuffer == nullptr)
    // {
    //     network_status.error = NETWORK_ERROR_COULD_NOT_ALLOCATE_BUFFERS;
    //     ACSI_send_error();
    //     return;
    // }

    // // If protocol isn't connected, then return not connected.
    // if (protocol == nullptr)
    // {
    //     network_status.error = NETWORK_ERROR_COULD_NOT_ALLOCATE_BUFFERS;
    //     ACSI_send_error();
    //     return;
    // }

    // // Do the channel read
    // err = read_channel(num_bytes);

    // ACSI_send_buffer((uint8_t *)receiveBuffer->data(), num_bytes);
    // ACSI_flush();
    // receiveBuffer->erase(0, num_bytes);

    // Debug_printf("ACSINetwork::read sent %u bytes\n", num_bytes);

    // ACSI_send_complete();
}

/**
 * Perform the correct write based on value of channelMode
 * @param num_bytes Number of bytes to write.
 * @return TRUE on error, FALSE on success. Used to emit ACSI_error or ACSI_complete().
 */
bool ACSINetwork::write_channel(unsigned short num_bytes)
{
    bool err = false;

    switch (channelMode)
    {
    case PROTOCOL:
        err = protocol->write(num_bytes);
        break;
    case JSON:
        Debug_printf("JSON Not Handled.\n");
        err = true;
        break;
    }
    return err;
}

bool ACSINetwork::status_channel_json(NetworkStatus *ns)
{
    ns->connected = json_bytes_remaining > 0;
    ns->error = json_bytes_remaining > 0 ? 1 : 136;
    ns->rxBytesWaiting = json_bytes_remaining;
    return false; // for now
}

/**
 * ACSI Status Command. First try to populate NetworkStatus object from protocol. If protocol not instantiated,
 * or Protocol does not want to fill status buffer (e.g. due to unknown aux1/aux2 values), then try to deal
 * with them locally. Then serialize resulting NetworkStatus object to ACSI.
 */
void ACSINetwork::status()
{
    // Debug_printf("ACSINetwork::status()\n");

    // NetworkStatus s;
    
    // ACSI_send_ack();

    // switch (channelMode)
    // {
    // case PROTOCOL:
    //     err = protocol->status(&s);
    //     break;
    // case JSON:
    //     err = status_channel_json(&s);
    //     break;
    // }

    // uint16_t bytes_waiting = (s.rxBytesWaiting > RC2014_TX_BUFFER_SIZE) ?
    //         RC2014_TX_BUFFER_SIZE : s.rxBytesWaiting;

    // response[0] = bytes_waiting & 0xFF;
    // response[1] = bytes_waiting >> 8;
    // response[2] = s.connected;
    // response[3] = s.error;
    // response_len = 4;
    // //receiveMode = STATUS;

    // ACSI_send_buffer(response, response_len);
    // ACSI_flush();
    
    // ACSI_send_complete();

}

/**
 * JSON functionality
 */
void ACSINetwork::ACSI_parse_json()
{
    // Debug_printf("ACSINetwork::parse_json()\n");

    // ACSI_send_ack();

    // json.parse();

    // ACSI_send_complete();
}

void ACSINetwork::ACSI_set_json_query()
{
    // uint8_t in[256];
    // const char *inp = NULL;
    // uint8_t *tmp;
    // Debug_printf("ACSINetwork::set_json_query()\n");

    // memset(in, 0, sizeof(in));

    // ACSI_send_ack();

    // ACSI_recv_buffer(in, sizeof(in));
    // ACSI_send_ack();

    // // strip away line endings from input spec.
    // for (int i = 0; i < 256; i++)
    // {
    //     if (in[i] == 0x0A || in[i] == 0x0D || in[i] == 0x9b)
    //         in[i] = 0x00;
    // }

    // inp = strrchr((const char *)in, ':');
    // Debug_printf("#1 %s\n",inp);
    // inp++;
    // json.setReadQuery(string(inp),cmdFrame.aux2);
    // json_bytes_remaining = json.readValueLen();
    // tmp = (uint8_t *)malloc(json.readValueLen());
    // json.readValue(tmp,json_bytes_remaining);
    // *receiveBuffer += string((const char *)tmp,json_bytes_remaining);
    // free(tmp);

    // Debug_printf("Query set to %s\n",inp);
    // ACSI_send_complete();
}


/**
 * @brief set channel mode
 */
void ACSINetwork::ACSI_set_channel_mode()
{
    // ACSI_send_ack();

    // switch (cmdFrame.aux2)
    // {
    // case 0:
    //     channelMode = PROTOCOL;
    //     ACSI_send_complete();
    //     break;
    // case 1:
    //     channelMode = JSON;
    //     ACSI_send_complete();
    //     break;
    // default:
    //     ACSI_send_error();
    // }
}


/**
 * Get Prefix
 */
void ACSINetwork::get_prefix()
{
    // ACSI_send_ack();

    // Debug_printf("ACSINetwork::ACSI_getprefix(%s)\n", prefix.c_str());
    // memcpy(response, prefix.data(), prefix.size());
    // response_len = prefix.size();

    // ACSI_send_complete();
}

/**
 * Set Prefix
 */
void ACSINetwork::set_prefix(unsigned short s)
{
    // uint8_t prefixSpec[256];
    // string prefixSpec_str;

    // memset(prefixSpec, 0, sizeof(prefixSpec));

    // ACSI_recv_buffer(prefixSpec, s);
    // ACSI_send_ack();

    // prefixSpec_str = string((const char *)prefixSpec);
    // prefixSpec_str = prefixSpec_str.substr(prefixSpec_str.find_first_of(":") + 1);
    // Debug_printf("ACSINetwork::ACSI_set_prefix(%s)\n", prefixSpec_str.c_str());

    // if (prefixSpec_str == "..") // Devance path N:..
    // {
    //     vector<int> pathLocations;
    //     for (int i = 0; i < prefix.size(); i++)
    //     {
    //         if (prefix[i] == '/')
    //         {
    //             pathLocations.push_back(i);
    //         }
    //     }

    //     if (prefix[prefix.size() - 1] == '/')
    //     {
    //         // Get rid of last path segment.
    //         pathLocations.pop_back();
    //     }

    //     // truncate to that location.
    //     prefix = prefix.substr(0, pathLocations.back() + 1);
    // }
    // else if (prefixSpec_str[0] == '/') // N:/DIR
    // {
    //     prefix = prefixSpec_str;
    // }
    // else if (prefixSpec_str.empty())
    // {
    //     prefix.clear();
    // }
    // else if (prefixSpec_str.find_first_of(":") != string::npos)
    // {
    //     prefix = prefixSpec_str;
    // }
    // else // append to path.
    // {
    //     prefix += prefixSpec_str;
    // }

    // Debug_printf("Prefix now: %s\n", prefix.c_str());
    // ACSI_send_complete();
}

/**
 * Set login
 */
void ACSINetwork::set_login()
{
    // uint8_t loginspec[USERNAME_BUFFER_SIZE];
    // uint16_t num_bytes = cmdFrame.aux1;

    // Debug_printf("ACSINetwork::set_login()\n");

    // ACSI_send_ack();
    // memset(loginspec, 0, sizeof(loginspec));

    // if (num_bytes > 0) {
    //     auto rlen = ACSI_recv_buffer(loginspec, num_bytes);
    //     Debug_printf("ACSINetwork::set_login read %d (of %d)\n", rlen, num_bytes);
    //     ACSI_send_ack();

    //     login = string((char *)loginspec, num_bytes);
    //     Debug_printf("ACSINetwork::set_login %s\n", login.c_str());
    // } else {
    //     login = "";
    //     Debug_printf("ACSINetwork::set_login emptied\n");
    // }
    // ACSI_send_complete();
}

/**
 * Set password
 */
void ACSINetwork::set_password()
{
    // uint8_t passwordspec[PASSWORD_BUFFER_SIZE];
    // uint16_t num_bytes = cmdFrame.aux1;

    // Debug_printf("ACSINetwork::set_password()\n");
    // ACSI_send_ack();
    // memset(passwordspec, 0, sizeof(passwordspec));

    // if (num_bytes > 0) {
    //     ACSI_recv_buffer(passwordspec, num_bytes);
    //     ACSI_send_ack();

    //     password = string((char *)passwordspec, num_bytes);
    //     Debug_printf("ACSINetwork::set_password %s\n", password.c_str());
    // } else {
    //     password = "";
    //     Debug_printf("ACSINetwork::set_login emptied\n");
    // }
    // ACSI_send_complete();
}


/**
 * Process incoming ACSI command
 * @param comanddata incoming 4 bytes containing command and aux bytes
 * @param checksum 8 bit checksum
 */
void ACSINetwork::process(uint32_t commanddata, uint8_t checksum)
{
    cmdFrame.commanddata = commanddata;
    cmdFrame.checksum = checksum;

    switch (cmdFrame.comnd)
    {
    case 'O':
        open();
        break;
    case 'C':
        close();
        break;
    case 'R':
        read();
        break;
    case 'W':
        write();
        break;
    case 'P':
        if (channelMode == JSON)
            ACSI_parse_json();
        break;
    case 'Q':
        if (channelMode == JSON)
            ACSI_set_json_query();
        break;
    case 'S':
        status();
        break;
    case 0xFC:
        ACSI_set_channel_mode();
        break;
    case 0xFD:
        set_login();
        break;
    case 0xFE:
        set_password();
        break;
    default:
        Debug_printf("ACSI network: unimplemented command: 0x%02x", cmdFrame.comnd);

    }
}

/** PRIVATE METHODS ************************************************************/

/**
 * Instantiate protocol object
 * @return bool TRUE if protocol successfully called open(), FALSE if protocol could not open
 */
bool ACSINetwork::instantiate_protocol()
{
    if (urlParser == nullptr)
    {
        Debug_printf("ACSINetwork::open_protocol() - urlParser is NULL. Aborting.\n");
        return false; // error.
    }

    // Convert to uppercase
    std::transform(urlParser->scheme.begin(), urlParser->scheme.end(), urlParser->scheme.begin(), ::toupper);

    if (urlParser->scheme == "TCP")
    {
        protocol = new NetworkProtocolTCP(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "UDP")
    {
        protocol = new NetworkProtocolUDP(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "TEST")
    {
        protocol = new NetworkProtocolTest(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "TELNET")
    {
        protocol = new NetworkProtocolTELNET(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "TNFS")
    {
        protocol = new NetworkProtocolTNFS(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "FTP")
    {
        protocol = new NetworkProtocolFTP(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "HTTP" || urlParser->scheme == "HTTPS")
    {
        protocol = new NetworkProtocolHTTP(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "SSH")
    {
        protocol = new NetworkProtocolSSH(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else if (urlParser->scheme == "SMB")
    {
        protocol = new NetworkProtocolSMB(receiveBuffer, transmitBuffer, specialBuffer);
    }
    else
    {
        Debug_printf("Invalid protocol: %s\n", urlParser->scheme.c_str());
        return false; // invalid protocol.
    }

    if (protocol == nullptr)
    {
        Debug_printf("ACSINetwork::open_protocol() - Could not open protocol.\n");
        return false;
    }

    if (!login.empty())
    {
        protocol->login = &login;
        protocol->password = &password;
    }

    Debug_printf("ACSINetwork::open_protocol() - Protocol %s opened.\n", urlParser->scheme.c_str());
    return true;
}

void ACSINetwork::parse_and_instantiate_protocol(string d)
{
    deviceSpec = d;

    // Invalid URL returns error 165 in status.
    if (parseURL() == false)
    {
        Debug_printf("Invalid devicespec: %s\n", deviceSpec.c_str());
        statusByte.byte = 0x00;
        statusByte.bits.client_error = true;
        err = NETWORK_ERROR_INVALID_DEVICESPEC;
        return;
    }

    Debug_printf("Parse and instantiate protocol: %s\n", deviceSpec.c_str());

    // Instantiate protocol object.
    if (instantiate_protocol() == false)
    {
        Debug_printf("Could not open protocol.\n");
        statusByte.byte = 0x00;
        statusByte.bits.client_error = true;
        err = NETWORK_ERROR_GENERAL;
        return;
    }
}

/**
 * Is this a valid URL? (Used to generate ERROR 165)
 */
bool ACSINetwork::isValidURL(EdUrlParser *url)
{
    if (url->scheme == "")
        return false;
    else if ((url->path == "") && (url->port == ""))
        return false;
    else
        return true;
}

/**
 * Preprocess deviceSpec given aux1 open mode. This is used to work around various assumptions that different
 * disk utility packages do when opening a device, such as adding wildcards for directory opens.
 *
 * The resulting URL is then sent into EdURLParser to get our URLParser object which is used in the rest
 * of ACSINetwork.
 *
 * This function is a mess, because it has to be, maybe we can factor it out, later. -Thom
 */
bool ACSINetwork::parseURL()
{
    string url;
    string unit = deviceSpec.substr(0, deviceSpec.find_first_of(":") + 1);

    if (urlParser != nullptr)
        delete urlParser;

    // Prepend prefix, if set.
    if (prefix.length() > 0)
        deviceSpec = unit + prefix + deviceSpec.substr(deviceSpec.find(":") + 1);
    else
        deviceSpec = unit + deviceSpec.substr(string(deviceSpec).find(":") + 1);

    Debug_printf("ACSINetwork::parseURL(%s)\n", deviceSpec.c_str());

    // Strip non-ascii characters.
    util_strip_nonascii(deviceSpec);

    // Process comma from devicespec (DOS 2 COPY command)
    // processCommaFromDevicespec();

    if (cmdFrame.aux1 != 6) // Anything but a directory read...
    {
        std::replace(deviceSpec.begin(), deviceSpec.end(), '*', '\0'); // FIXME: Come back here and deal with WC's
    }

    // // Some FMSes add a dot at the end, remove it.
    // if (deviceSpec.substr(deviceSpec.length() - 1) == ".")
    //     deviceSpec.erase(deviceSpec.length() - 1, string::npos);

    // Remove any spurious spaces
    deviceSpec = util_remove_spaces(deviceSpec);

    // chop off front of device name for URL, and parse it.
    url = deviceSpec.substr(deviceSpec.find(":") + 1);
    urlParser = EdUrlParser::parseUrl(url);

    Debug_printf("ACSINetwork::parseURL transformed to (%s, %s)\n", deviceSpec.c_str(), url.c_str());

    return isValidURL(urlParser);
}

void ACSINetwork::set_translation()
{
    // trans_aux2 = cmdFrame.aux2;
    // ACSI_complete();
}

void ACSINetwork::set_timer_rate()
{
    // timerRate = (cmdFrame.aux2 * 256) + cmdFrame.aux1;

    // // Stop extant timer
    // timer_stop();

    // // Restart timer if we're running a protocol.
    // if (protocol != nullptr)
    //     timer_start();

    // ACSI_complete();
}


#endif /* NEW_TARGET */
