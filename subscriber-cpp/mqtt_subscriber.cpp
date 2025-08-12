#include <mosquitto.h>
#include <mysql/mysql.h>    
#include <cmath>
#include <cstring>
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>            
#include <atomic>
#include <csignal>
#include <thread>
#include <cstdlib>          

// Deadband / tolerances
static constexpr double TEMP_DELTA = 0.1;   
static constexpr double HUM_DELTA  = 0.5;   
static constexpr double PRES_DELTA = 0.15;  
static constexpr int HEARTBEAT_SECONDS = 600; // 10 minutes

// MQTT / DB configuration
static const char* BROKER_HOST = "localhost";
static const int   BROKER_PORT = 1883;
static const char* TOPIC       = "weather/gruppe10";
static const char* CLIENT_ID   = "subscriber_gruppe10_cpp";

static const char* DB_HOST = "localhost";
static const unsigned int DB_PORT = 3306;
static const char* DB_USER = "vejruser";
static const char* DB_PASS = "StærkKode123!";
static const char* DB_NAME = "vejrstationsdb";

// in-memory cache to avoid spamming DB on tiny changes
struct LastSaved {
    double temp = NAN;
    double hum  = NAN;
    double pres = NAN;
    std::chrono::system_clock::time_point ts{};
    bool has_value = false;
};
static LastSaved g_last;
static std::atomic_bool g_running{true};

//  Helpers

// Return true if |now - oldv| >= delta, or if we never had a prior value.
bool changed_enough(double now, double oldv, double delta) {
    if (std::isnan(oldv)) return true;
    return std::fabs(now - oldv) >= delta;
}

// Format local time like "YYYY-MM-DD HH:MM:SS"
std::string now_local_timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm lt{};
#ifdef _WIN32
    localtime_s(&lt, &t);
#else
    localtime_r(&t, &lt);
#endif
    char buf[20];
    std::strftime(buf, sizeof(buf), "%F %T", &lt);
    return std::string(buf);
}


// OPen a new DB connection
MYSQL* db_connect() {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) return nullptr;
    // Some systems don’t expose MYSQL_OPT_SSL_MODE/SSL_MODE_DISABLED. Guard it.
#if defined(MYSQL_OPT_SSL_MODE) && defined(SSL_MODE_DISABLED)
    mysql_options(conn, MYSQL_OPT_SSL_MODE, (const void*)SSL_MODE_DISABLED);
#endif
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, nullptr, 0)) {
        std::cerr << "[DB] connect error: " << mysql_error(conn) << "\n";
        if (conn) mysql_close(conn);
        return nullptr;
    }
    return conn;
}

// On startup, read the last row once so deadband works after restarts.
void seed_last_saved_from_db() {
    MYSQL* conn = db_connect();
    if (!conn) {
        std::cerr << "[DB] Could not seed last_saved (connect failed)\n";
        return;
    }
    const char* q = "SELECT ts, temp, hum, pres FROM readings ORDER BY id DESC LIMIT 1";
    if (mysql_query(conn, q) != 0) {
        std::cerr << "[DB] seed query error: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "[DB] seed store_result error: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row && row[1] && row[2] && row[3]) {
        g_last.temp = std::stod(row[1]);
        g_last.hum  = std::stod(row[2]);
        g_last.pres = std::stod(row[3]);
        g_last.ts   = std::chrono::system_clock::now();
        g_last.has_value = true;
        std::cout << "[DB] Seeded last_saved: temp=" << g_last.temp
                  << " hum=" << g_last.hum << " pres=" << g_last.pres << "\n";
    } else {
        std::cout << "[DB] No previous rows; first insert will proceed.\n";
    }
    mysql_free_result(res);
    mysql_close(conn);
}

// Insert one row into the DB using a formatted SQL string.
bool insert_reading(const std::string& ts, double temp, double hum, double pres) {
    MYSQL* conn = db_connect();
    if (!conn) return false;
    char sql[512];
    std::snprintf(sql, sizeof(sql),
                  "INSERT INTO readings (ts,temp,hum,pres) VALUES ('%s', %.3f, %.3f, %.3f)",
                  ts.c_str(), temp, hum, pres);
    if (mysql_query(conn, sql) != 0) {
        std::cerr << "[DB] insert error: " << mysql_error(conn) << "\n";
        mysql_close(conn);
        return false;
    }
    mysql_close(conn);
    return true;
}

//  numeric JSON extractor: looks for "key": <number>
bool json_get_number(const std::string& payload, const char* key, double& out) {
    std::string kk = std::string("\"") + key + "\"";
    auto kpos = payload.find(kk);
    if (kpos == std::string::npos) return false;
    auto colon = payload.find(':', kpos);
    if (colon == std::string::npos) return false;
    auto end = payload.find_first_of(",}", colon+1);
    if (end == std::string::npos) end = payload.size();
    try {
        out = std::stod(payload.substr(colon+1, end-(colon+1)));
        return true;
    } catch (...) { return false; }
}

// MQTT callbacks

// Called once the TCP+MQTT connection succeeds; subscribe to the topic here.
void on_connect(struct mosquitto* m, void*, int rc) {
    if (rc == 0) {
        std::cout << "[MQTT] Connected\n";
        mosquitto_subscribe(m, nullptr, TOPIC, 0);
    } else {
        std::cerr << "[MQTT] Connect failed rc=" << rc << "\n";
    }
}


// Fired on every incoming message. We parse JSON, decide whether to insert, and write to DB if needed.
void on_message(struct mosquitto*, void*, const struct mosquitto_message* msg) {
    if (!msg || !msg->payload) return;
    std::string payload((char*)msg->payload, msg->payloadlen);

    double temp=0, hum=0, pres=0;
    if (!json_get_number(payload, "temp", temp) ||
        !json_get_number(payload, "hum",  hum)  ||
        !json_get_number(payload, "pres", pres)) {
        std::cerr << "[ERR] JSON parse failed, payload: " << payload << "\n";
        return;
    }

    // build local timestamp string for DB
    auto now = std::chrono::system_clock::now();
    std::string ts = now_local_timestamp();
    std::cout << "[MQTT] " << temp << "°C, " << hum << "%, " << pres << "hPa @ " << ts << "\n";

    // Decide if we should insert:
    //  first reading after start
    //  or any field changed by >= its delta
    //  or heartbeat interval elapsed since we last saved
    bool should_insert = false;
    if (!g_last.has_value) {
        should_insert = true; // First reading after boot
    } else {
        bool c_temp = changed_enough(temp, g_last.temp, TEMP_DELTA);
        bool c_hum  = changed_enough(hum,  g_last.hum,  HUM_DELTA);
        bool c_pres = changed_enough(pres, g_last.pres, PRES_DELTA);
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - g_last.ts).count();
        bool too_old = age >= HEARTBEAT_SECONDS; // Heartbeat insert
        should_insert = c_temp || c_hum || c_pres || too_old;
    }

    if (!should_insert) {
        std::cout << "[DB] Skipped (deadband + heartbeat not due)\n";
        return;
    }

    // Write, then update our in-memory last saved state.
    if (insert_reading(ts, temp, hum, pres)) {
        g_last.temp = temp; g_last.hum = hum; g_last.pres = pres;
        g_last.ts = now; g_last.has_value = true;
        std::cout << "[DB] Inserted\n";
    }
}

// shows disconnect reasons in logs
void on_disconnect(struct mosquitto*, void*, int rc) {
    std::cerr << "[MQTT] Disconnected rc=" << rc << "\n";
}

// Handle Ctrl+C / stop signals so we can cleanly exit the loop.
void handle_sig(int) { g_running = false; }


int main() {
    // Force Europe/Copenhagen timezone inside the process.
    setenv("TZ", "Europe/Copenhagen", 1);
    tzset();

    // Allow graceful shutdown updates logs, closes MQTT cleanly.
    std::signal(SIGINT, handle_sig);
    std::signal(SIGTERM, handle_sig);

    // Read last DB row so deadband/heartbeat behave right after restarts.
    seed_last_saved_from_db();

    // Init MQTT library and create a client with a fixed client-id.
    mosquitto_lib_init();
    mosquitto* m = mosquitto_new(CLIENT_ID, true, nullptr);
    if (!m) {
        std::cerr << "[MQTT] mosquitto_new failed\n";
        return 1;
    }

    // Register callbacks so libmosquitto can call back into our code.
    mosquitto_connect_callback_set(m, on_connect);
    mosquitto_message_callback_set(m, on_message);
    mosquitto_disconnect_callback_set(m, on_disconnect);

    // Open the TCP+MQTT connection. If this fails, bail out.
    if (mosquitto_connect(m, BROKER_HOST, BROKER_PORT, /*keepalive*/60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "[MQTT] connect() failed\n";
        mosquitto_destroy(m);
        mosquitto_lib_cleanup();
        return 1;
    }

    // Background loop
    if (mosquitto_loop_start(m) != MOSQ_ERR_SUCCESS) {
        std::cerr << "[MQTT] loop_start failed\n";
        mosquitto_destroy(m);
        mosquitto_lib_cleanup();
        return 1;
    }

    // Sit quietly and process messages until we’re told to exit.
    while (g_running) std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Clean shutdown: disconnect, stop loop, free resources.
    mosquitto_disconnect(m);
    mosquitto_loop_stop(m, true);
    mosquitto_destroy(m);
    mosquitto_lib_cleanup();
    return 0;
}

