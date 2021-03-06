// ------------------------------------------------------------------------------------------------
#include "Base.hpp"

// ------------------------------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <cstdarg>

// ------------------------------------------------------------------------------------------------
#include <queue>
#include <mutex>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <utility>

// ------------------------------------------------------------------------------------------------
#ifndef _WIN32
    #include <unistd.h>
    #include <signal.h>
#endif // _WIN32

// ------------------------------------------------------------------------------------------------
#include <httplib.h>

// ------------------------------------------------------------------------------------------------
#include <vcmp.h>
#include <SimpleIni.h>

/* ------------------------------------------------------------------------------------------------
 * SOFTWARE INFORMATION
*/
#define SMOD_NAME "Master-server Announce Plugin"
#define SMOD_AUTHOR "Sandu Liviu Catalin (S.L.C)"
#define SMOD_COPYRIGHT "Copyright (C) 2016 Sandu Liviu Catalin"
#define SMOD_HOST_NAME "SModAnnounceHost"
#define SMOD_VERSION 001
#define SMOD_VERSION_STR "0.0.1"
#define SMOD_VERSION_MAJOR 0
#define SMOD_VERSION_MINOR 0
#define SMOD_VERSION_PATCH 1

/* ------------------------------------------------------------------------------------------------
 * General options
*/

// ------------------------------------------------------------------------------------------------
namespace SMod {

// ------------------------------------------------------------------------------------------------
typedef ::std::string String;

// ------------------------------------------------------------------------------------------------
PluginFuncs*        _Func = nullptr;
PluginCallbacks*    _Clbk = nullptr;
PluginInfo*         _Info = nullptr;

// ------------------------------------------------------------------------------------------------
static ServerSettings       g_Settings;
static unsigned int         g_ServerVersion;
static unsigned int         g_UpdateInterval;

/* ------------------------------------------------------------------------------------------------
 * Output a message only if the _DEBUG was defined.
*/
void OutputDebug(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted user message to the console.
*/
void OutputMessage(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted error message to the console.
*/
void OutputError(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted verbose user message to the console.
*/
void VerboseMessage(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted verbose error message to the console.
*/
void VerboseError(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted user message to the console in a thread safe manner.
*/
void MtOutputMessage(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted error message to the console in a thread safe manner.
*/
void MtOutputError(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted verbose user message to the console in a thread safe manner.
*/
void MtVerboseMessage(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Output a formatted verbose error message to the console in a thread safe manner.
*/
void MtVerboseError(CCStr msg, ...);

/* ------------------------------------------------------------------------------------------------
 * Simple parser that extracts URI information from a string.
*/
struct URI
{
    /* ---------------------------------------------------------------------------------------------
     * Base constructor.
    */
    URI(CCStr address)
        : mHost()
        , mPort()
        , mPath()
        , mFull()
        , mAddr()
    {
        // Is there even an address to parse?
        if (!address || strlen(address) <= 0)
        {
            return;
        }
        // Skip the protocol if it was specified
        if (strncmp(address, "http://", 7) == 0)
        {
            address += 7; // Skip the protocol
        }
        else if (strncmp(address, "https://", 8) == 0)
        {
            address += 8; // Skip the protocol
        }
        // Find where the port starts
        CCStr port = strchr(address, ':');
        // Find where the path starts
        CCStr path = strchr(port ? port : address, '/');
        // Did we find the port separator?
        if (port)
        {
            // Copy everything until the port separator
            mHost.assign(address, port - address);
        }
        // Did we find the path separator?
        else if(path)
        {
            // Copy everything until the path separator
            mHost.assign(address, path - address);
        }
        // The entire address is the host
        else
        {
            // Copy the entire address
            mHost.assign(address);
        }
        // Should we skip the port separator?
        if (port)
        {
            ++port;
        }
        // Did we find both a port and path?
        if (port && path)
        {
            // Copy everything between the port and path separators
            mPort.assign(port, path - port);
        }
        else if (port)
        {
            // Copy everything after the port separator
            mPort.assign(port);
        }
        // Assign the default port
        else
        {
            mPort.assign("80");
        }
        // Copy the path if necessary
        if (path)
        {
            mPath.assign(path);
        }
        // Assign a default path just in case
        else
        {
            mPath.assign("/");
        }
        // Generate the full URI
        mFull.assign("http://");
        mFull.append(mHost);
        mFull += ':';
        mFull.append(mPort);
        mFull.append(mPath);
        mAddr.append(mHost);
        // Generate the connect address
        mAddr += ':';
        mAddr.append(mPort);
    }

    /* ---------------------------------------------------------------------------------------------
     * Copy constructor.
    */
    URI(const URI & o)
        : mHost(o.mHost)
        , mPort(o.mPort)
        , mPath(o.mPath)
        , mFull(o.mFull)
        , mAddr(o.mAddr)
    {
        /* ... */
    }

    /* ---------------------------------------------------------------------------------------------
     * Move constructor.
    */
    URI(URI && o)
        : mHost(std::forward< std::string >(o.mHost))
        , mPort(std::forward< std::string >(o.mPort))
        , mPath(std::forward< std::string >(o.mPath))
        , mFull(std::forward< std::string >(o.mFull))
        , mAddr(std::forward< std::string >(o.mAddr))
    {
        /* ... */
    }

    /* ---------------------------------------------------------------------------------------------
     * Copy assignment operator.
    */
    URI & operator = (const URI & o)
    {
        if (this != &o)
        {
            mHost = o.mHost;
            mPort = o.mPort;
            mPath = o.mPath;
            mFull = o.mFull;
            mAddr = o.mAddr;
        }
        return *this;
    }

    /* ---------------------------------------------------------------------------------------------
     * Move assignment operator.
    */
    URI & operator = (URI && o)
    {
        if (this != &o)
        {
            mHost = std::forward< std::string >(o.mHost);
            mPort = std::forward< std::string >(o.mPort);
            mPath = std::forward< std::string >(o.mPath);
            mFull = std::forward< std::string >(o.mFull);
            mAddr = std::forward< std::string >(o.mAddr);
        }
        return *this;
    }

    /* ---------------------------------------------------------------------------------------------
     * Retrieve the protocol as a c string.
    */
    CCStr Protocol() const
    {
        return "http://";
    }

    /* ---------------------------------------------------------------------------------------------
     * Retrieve the host address as a c string.
    */
    CCStr Host() const
    {
        return mHost.c_str();
    }

    /* ---------------------------------------------------------------------------------------------
     * Retrieve the port number as a c string.
    */
    CCStr Port() const
    {
        return mPort.c_str();
    }

    /* ---------------------------------------------------------------------------------------------
     * Retrieve the request path as a c string.
    */
    CCStr Path() const
    {
        return mPath.c_str();
    }

    /* ---------------------------------------------------------------------------------------------
     * Retrieve the full address as a c string.
    */
    CCStr Full() const
    {
        return mFull.c_str();
    }

    /* ---------------------------------------------------------------------------------------------
     * Retrieve the connect address as a c string.
    */
    CCStr Addr() const
    {
        return mAddr.c_str();
    }

    // ---------------------------------------------------------------------------------------------
    String          mHost; // The host name.
    String          mPort; // The port number.
    String          mPath; // The request path.
    String          mFull; // The full address.
    String          mAddr; // The address used to connect to.
};

/* ------------------------------------------------------------------------------------------------
 * Manages a connection to a master-server.
*/
struct Server
{
    using ClientPtr = std::unique_ptr< httplib::Client >;
    /* ---------------------------------------------------------------------------------------------
     * Base constructor.
    */
    Server(URI && addr)
        : m_Client(new httplib::Client(addr.mHost, std::stoi(addr.mPort)))
        , m_Fails(0), m_Valid(m_Client->is_valid())
        , m_Addr(std::forward< URI >(addr)), m_Headers{{"User-Agent", "VCMP/0.4"}}, m_Params()
    {
        if (!m_Valid)
        {
            VerboseError("Master-server '%s' was marked as invalid",
                            m_Addr.Full());
        }
        else
        {
            m_Client->set_follow_location(true);
            m_Client->set_compress(false);
        }
    }

    /* ---------------------------------------------------------------------------------------------
     * Copy constructor. (disabled)
    */
    Server(const Server &) = delete;

    /* ---------------------------------------------------------------------------------------------
     * Move constructor.
    */
    Server(Server && o)
        : m_Client(std::forward< ClientPtr >(o.m_Client))
        , m_Fails(o.m_Fails), m_Valid(o.m_Valid)
        , m_Addr(std::forward< URI >(o.m_Addr))
        , m_Headers(std::forward< httplib::Headers >(o.m_Headers))
        , m_Params(std::forward< httplib::Params >(o.m_Params))
    {
    }

    /* ---------------------------------------------------------------------------------------------
     * Destructor.
    */
    ~Server()
    {

    }

    /* ---------------------------------------------------------------------------------------------
     * Copy assignment operator. (disabled)
    */
    Server & operator = (const Server &) = delete;

    /* ---------------------------------------------------------------------------------------------
     * Move assignment operator.
    */
    Server & operator = (Server && o)
    {
        if (this != &o)
        {
            m_Client = std::forward< ClientPtr >(o.m_Client);
            m_Fails = o.m_Fails;
            m_Valid = o.m_Valid;
            m_Addr = std::forward< URI >(o.m_Addr);
            m_Headers = std::forward< httplib::Headers >(o.m_Headers);
            m_Params = std::forward< httplib::Params >(o.m_Params);
        }
        return *this;
    }

    /* ---------------------------------------------------------------------------------------------
     * Implicit conversion to boolean.
    */
    operator bool () const
    {
        return m_Valid;
    }

    /* ---------------------------------------------------------------------------------------------
     * Retrieve the associated master-server address.
    */
    const URI & GetURI() const
    {
        return m_Addr;
    }

    /* ---------------------------------------------------------------------------------------------
     * Increase the failure count and see whether updates should stop being sent on this server.
    */
    void Failed()
    {
        // After one thousand failed attempts, there's no point in insisting
        if (++m_Fails >= 1000)
        {
            MtVerboseError("Master-server '%s' was marked as invalid after %u failures",
                            m_Addr.Full(), m_Fails);
            // Block further updates
            m_Valid = false;
        }
    }

    /* ---------------------------------------------------------------------------------------------
     * Make the server valid again and continue to send updates.
    */
    void MakeValid()
    {
        // Reset the counter
        m_Fails = 0;
        // Allow further updates
        m_Valid = true;
    }

    /* ---------------------------------------------------------------------------------------------
     * Create the server version header.
    */
    void ConfigureServer()
    {
        m_Headers.emplace("VCMP-Version", std::to_string(g_ServerVersion));
        m_Params.emplace("port", std::to_string(g_Settings.port));
    }

    /* ---------------------------------------------------------------------------------------------
     * Send the payload to the associated server to keep the server alive in the master-list.
    */
    void Update()
    {
        // This master-list working?
        if (!m_Valid || !m_Client)
        {
            MtVerboseMessage("Skipping invalid master-list: `%s`", m_Addr.Full());
            return; // No point int trying to announce to thi server anymore
        } else MtVerboseMessage("Announcing on master-list: `%s`", m_Addr.Full());
        auto res = m_Client->Post(m_Addr.Path(), m_Headers, m_Params);
        MtVerboseMessage("Master-list (%s) responded with code: %d", m_Addr.Full(), res->status);
        // Identify response code
        switch (res->status)
        {
            case 400:
            {
                MtVerboseError("Master-server '%s' denied request due to malformed data", m_Addr.Full());
                // This operation failed
                Failed();
            } break;
            case 403:
            {
                MtVerboseError("Master-server '%s' denied request, server version may not have been accepted", m_Addr.Full());
                // This operation failed
                Failed();
            } break;
            case 405:
            {
                MtVerboseError("Master-server '%s' denied request, GET is not supported", m_Addr.Full());
                // This operation failed
                Failed();
            } break;
            case 408:
            {
                MtVerboseError("Master-server '%s' timed out while trying to reach your server; are your ports forwarded?", m_Addr.Full());
                // This operation failed
                Failed();
            } break;
            case 500:
            {
                MtVerboseError("Master-server '%s' had an unexpected error while processing your request", m_Addr.Full());
                // This operation failed
                Failed();
            } break;
            case 200:
            {
                MtVerboseMessage("Successfully announced on master-server '%s'", m_Addr.Full());
                // This operation succeeded. Carry on with the rest
                MakeValid();
            } break;
            default: /* Unknown response */ break;
        }
    }

private:

    // ---------------------------------------------------------------------------------------------
    ClientPtr           m_Client; // The associated server connection.
    unsigned            m_Fails; // How many attempts to create a connection failed.
    bool                m_Valid; // Whether we should completely ignore this master-server.
    URI                 m_Addr; // The master-server address information.
    httplib::Headers    m_Headers; // Request headers.
    httplib::Params     m_Params; // Request parameters.
};

// ------------------------------------------------------------------------------------------------
typedef std::vector< Server >                       Servers;
typedef std::queue< std::pair< String, bool > >     Messages;

// ------------------------------------------------------------------------------------------------
static bool                 g_Verbose = false; // Enable or disable verbose messages
static bool                 g_Announce = false; // Allow the announce loop to continue
static std::thread          g_Thread; // Announce thread
static std::mutex           g_Mutex; // Global mutex

// ------------------------------------------------------------------------------------------------
static Servers              g_Servers; // List of servers to be updated
static Messages             g_Messages; // Messages queued from the announce thread

/* ------------------------------------------------------------------------------------------------
 * The main thread responsible for updating the specified master-servers.
*/
void AnnounceThread(Servers && servers)
{
    MtVerboseMessage("Announce thread started.");
    // Enter the announcement loop
    while (g_Announce)
    {
        // Grab the current time point
        std::chrono::time_point< std::chrono::steady_clock > next = std::chrono::steady_clock::now();
        // Set the time-point for next update
        next += std::chrono::seconds(g_UpdateInterval);
        // Tell the master-list we're alive
        for (auto & server : servers)
        {
            server.Update();
        }
        // Grab the current time point
        std::chrono::time_point<std::chrono::steady_clock> curr;
        // Sleep until the next appointed update time point
        do {
            // Sleep for 1/4'th of a second
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            // Check elapsed time
            curr = std::chrono::steady_clock::now();
        } while (g_Announce && (next > curr));
    }
}

/* ------------------------------------------------------------------------------------------------
 * Flush queued messages to the console output.
*/
void FlushMessages()
{
    // Acquire a global lock
    std::lock_guard< std::mutex > lock(g_Mutex);
    // Output any queued messages
    while (!g_Messages.empty())
    {
        // Identify the message type and send it
        if (g_Messages.front().second)
        {
            OutputMessage("%s", g_Messages.front().first.c_str());
        }
        else
        {
            OutputError("%s", g_Messages.front().first.c_str());
        }
        // Pop the message from the queue
        g_Messages.pop();
    }
}

/* ------------------------------------------------------------------------------------------------
 * The server was initialized and this plug-in must initialize as well.
*/
static uint8_t OnServerInitialise(void)
{
    // Obtain the server settings. We only need the port number
    _Func->GetServerSettings(&g_Settings);
    // Obtain the server version. This doesn't change much
    g_ServerVersion = _Func->GetServerVersion();
    // Attempt to generate the update payload
    for (auto & server : g_Servers)
    {
        // Attempt to generate the payload
        server.ConfigureServer();
    }
    // See if any servers are left
    if (g_Servers.empty())
    {
        // We don't want to receive events anymore
        _Clbk->OnServerInitialise       = nullptr;
        _Clbk->OnServerShutdown         = nullptr;
        _Clbk->OnServerFrame            = nullptr;
    }
    else
    {
        // Enable the announce thread to run if there are servers
        g_Announce = true;
        // Create the announce thread
        g_Thread = std::thread(AnnounceThread, std::move(g_Servers));
        // Notify that the plug-in was successfully initialized
        VerboseMessage("Announce plug-in was successfully initialized");
    }
    // Allow the server to continue
    return 1;
}

/* ------------------------------------------------------------------------------------------------
 * The server was shutdown and this plug-in must terminate as well.
*/
static void OnServerShutdown(void)
{
    // The server may still send callbacks
    _Clbk->OnServerInitialise       = nullptr;
    _Clbk->OnServerShutdown         = nullptr;
    _Clbk->OnServerFrame            = nullptr;
    // Tell the announce thread to stop
    g_Announce = false;
    // Wait for the announce thread to finish
    if (g_Thread.joinable())
    {
        g_Thread.join();
    }
    // Flush any remaining messages
    FlushMessages();
}

static void OnServerFrame(float /*delta*/)
{
    // Flush any queued messages
    FlushMessages();
}

// ------------------------------------------------------------------------------------------------
void OutputMessageImpl(CCStr msg, va_list args)
{
#if defined(WIN32) || defined(_WIN32)
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csb_before;
    GetConsoleScreenBufferInfo( hstdout, &csb_before);
    SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN);
    printf("[ANNOUNCE] ");

    SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
    vprintf(msg, args);
    puts("");

    SetConsoleTextAttribute(hstdout, csb_before.wAttributes);
#else
    printf("%c[0;32m[ANNOUNCE]%c[0;37m", 27, 27);
    vprintf(msg, args);
    puts("");
#endif
}

// ------------------------------------------------------------------------------------------------
void OutputErrorImpl(CCStr msg, va_list args)
{
#if defined(WIN32) || defined(_WIN32)
    HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO csb_before;
    GetConsoleScreenBufferInfo( hstdout, &csb_before);
    SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("[ANNOUNCE] ");

    SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
    vprintf(msg, args);
    puts("");

    SetConsoleTextAttribute(hstdout, csb_before.wAttributes);
#else
    printf("%c[0;32m[ANNOUNCE]%c[0;37m", 27, 27);
    vprintf(msg, args);
    puts("");
#endif
}

// ------------------------------------------------------------------------------------------------
void OutputDebug(CCStr msg, ...)
{
#ifdef _DEBUG
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Call the output function
    OutputMessageImpl(msg, args);
    // Finalize the arguments list
    va_end(args);
#else
    SMOD_UNUSED_VAR(msg);
#endif
}

// ------------------------------------------------------------------------------------------------
void OutputMessage(CCStr msg, ...)
{
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Call the output function
    OutputMessageImpl(msg, args);
    // Finalize the arguments list
    va_end(args);
}

// ------------------------------------------------------------------------------------------------
void OutputError(CCStr msg, ...)
{
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Call the output function
    OutputErrorImpl(msg, args);
    // Finalize the arguments list
    va_end(args);
}

// ------------------------------------------------------------------------------------------------
void VerboseMessage(CCStr msg, ...)
{
    // Are verbose messages allowed?
    if (!g_Verbose)
    {
        return;
    }
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Call the output function
    OutputMessageImpl(msg, args);
    // Finalize the arguments list
    va_end(args);
}

// ------------------------------------------------------------------------------------------------
void VerboseError(CCStr msg, ...)
{
    // Are verbose messages allowed?
    if (!g_Verbose)
    {
        return;
    }
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Call the output function
    OutputErrorImpl(msg, args);
    // Finalize the arguments list
    va_end(args);
}

// ------------------------------------------------------------------------------------------------
static void QueueMtMsg(bool type, CCStr msg, va_list args)
{
    // Create a copy of the specified arguments list
    va_list args_cpy;
    va_copy(args_cpy, args);
    // Allocate a moderately large string
    String buffer(128, '\0');
    // Attempt to run the specified format
    Int32 size = vsnprintf(&buffer[0], buffer.size(), msg, args);
    // See if a larger buffer is necessary
    if (size > 0 && static_cast< unsigned >(size) > buffer.size())
    {
        // Resize the string buffer to the required size
        buffer.resize(static_cast< unsigned >(size+1), '\0');
        // Attempt to run the specified format
        size = vsnprintf(&buffer[0], buffer.size(), msg, args_cpy);
    }
    // Finalize the arguments list copy
    va_end(args_cpy);
    // See if the format failed
    if (size < 0)
    {
        return;
    }
    // Remove unwanted characters
    buffer.resize(size+1);
    // Acquire a global lock
    std::lock_guard< std::mutex > lock(g_Mutex);
    // Emplace the message into the message queue
    g_Messages.emplace(std::move(buffer), type);
}

// ------------------------------------------------------------------------------------------------
void MtOutputMessage(CCStr msg, ...)
{
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Attempt to generate and queue the message
    QueueMtMsg(true, msg, args);
    // Finalize the arguments list
    va_end(args);
}

// ------------------------------------------------------------------------------------------------
void MtOutputError(CCStr msg, ...)
{
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Attempt to generate and queue the message
    QueueMtMsg(false, msg, args);
    // Finalize the arguments list
    va_end(args);
}

// ------------------------------------------------------------------------------------------------
void MtVerboseMessage(CCStr msg, ...)
{
    // Are verbose messages allowed?
    if (!g_Verbose)
    {
        return;
    }
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Attempt to generate and queue the message
    QueueMtMsg(true, msg, args);
    // Finalize the arguments list
    va_end(args);
}

// ------------------------------------------------------------------------------------------------
void MtVerboseError(CCStr msg, ...)
{
    // Are verbose messages allowed?
    if (!g_Verbose)
    {
        return;
    }
    // Initialize the arguments list
    va_list args;
    va_start(args, msg);
    // Attempt to generate and queue the message
    QueueMtMsg(false, msg, args);
    // Finalize the arguments list
    va_end(args);
}

} // Namespace:: SMod

// ------------------------------------------------------------------------------------------------
#ifdef SMOD_OS_WINDOWS
BOOL WINAPI ConsoleHandler(DWORD signal) {

    if (signal == CTRL_C_EVENT) SMod::g_Announce = false;
    return FALSE;
}
#else
void ConsoleHandler(int s){
    SMod::g_Announce = false;
}
#endif // SMOD_OS_WINDOWS

// ------------------------------------------------------------------------------------------------
SMOD_API_EXPORT unsigned int VcmpPluginInit(PluginFuncs* functions, PluginCallbacks* callbacks, PluginInfo* info)
{
    using namespace SMod;
    // Output plug-in header
    puts("");
    OutputMessage("------------------------------------------------------------------");
    OutputMessage("Plug-in: %s", SMOD_NAME);
    OutputMessage("Author: %s", SMOD_AUTHOR);
    OutputMessage("Legal: %s", SMOD_COPYRIGHT);
    OutputMessage("------------------------------------------------------------------");
    puts("");
    // Store server proxies
    _Func = functions;
    _Clbk = callbacks;
    _Info = info;
    // Assign plug-in version
    _Info->pluginVersion = SMOD_VERSION;
    _Info->apiMajorVersion = PLUGIN_API_MAJOR;
    _Info->apiMinorVersion = PLUGIN_API_MINOR;
    // Assign the plug-in name
    snprintf(_Info->name, sizeof(_Info->name), "%s", SMOD_HOST_NAME);
    // Create the configuration loader
    CSimpleIniA conf(false, true, true);
    // Attempt to load the configurations from disk
    const SI_Error ini_ret = conf.LoadFile("announce.ini");
    // See if the configurations could be loaded
    if (ini_ret < 0)
    {
        switch (ini_ret)
        {
            case SI_FAIL:   OutputError("Failed to load the configuration file. Probably invalid");
            // fall through
            case SI_NOMEM:  OutputError("Run out of memory while loading the configuration file");
            // fall through
            case SI_FILE:   OutputError("Failed to load the configuration file: announce.ini");
            // fall through
            default:        OutputError("Failed to load the configuration file for some unforeseen reason");
        }
        // Plug-in failed to load configurations
        return SMOD_FAILURE;
    }
    // See if the plug-in should output verbose information
    g_Verbose = conf.GetBoolValue("Options", "Verbose", false);
    // Configure update interval
    {
        long value = conf.GetLongValue("Options", "UpdateInterval", 60);
        // Should there be a limit here, higher than 1 second? (we dumb or evil enough to abuse it?)
        g_UpdateInterval = value <= 0 ? 1 : static_cast< unsigned int >(value);
    }
    // Attempt to retrieve the list of specified master-servers
    CSimpleIniA::TNamesDepend servers;
    conf.GetAllValues("Servers", "Address", servers);
    // See if any server address was specified
    if (servers.size() <= 0)
    {
        VerboseError("No master-servers specified. No reason to load the plug-in.");
        // No point in loading the plug-in
        return SMOD_FAILURE;
    }
    // Sort the list in it's original order
    servers.sort(CSimpleIniA::Entry::LoadOrder());
    // Process each specified server addresses
    for (const auto & elem : servers)
    {
        // See if there's even something that could resemble a server address
        if (!elem.pItem)
        {
            continue;
        }
        // Attempt to extract URI information from the currently processed address
        URI addr(elem.pItem);
        // See if a valid host could be extracted
        if (addr.mHost.empty())
        {
            VerboseError("Master-server '%s' is an ill formed address", elem.pItem);
        }
        // Construct a server instance using this address
        else
        {
            // Show which master-server is added to the list
            VerboseMessage("Master-server '%s' added to the announce list", addr.Full());
            // Create the server instance
            g_Servers.emplace_back(std::move(addr));
        }
    }
    // See if any server was valid
    if (g_Servers.size() <= 0)
    {
        VerboseError("No master-servers specified. No reason to load the plug-in.");
        // No point in loading the plug-in
        return SMOD_FAILURE;
    }
    // Bind to the server callbacks
    _Clbk->OnServerInitialise       = OnServerInitialise;
    _Clbk->OnServerShutdown         = OnServerShutdown;
    _Clbk->OnServerFrame            = OnServerFrame;
#ifdef SMOD_OS_WINDOWS
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        VerboseError("Could not set control handler.");
    }
#else
    struct sigaction sig_int_hnd;
    sig_int_hnd.sa_handler = ConsoleHandler;
    sigemptyset(&sig_int_hnd.sa_mask);
    sig_int_hnd.sa_flags = 0;
    sigaction(SIGINT, &sig_int_hnd, NULL);
#endif // SMOD_OS_WINDOWS
    // Notify that the plug-in was successfully loaded
    VerboseMessage("Successfully loaded %s", SMOD_NAME);
    // Done!
    return SMOD_SUCCESS;
}
